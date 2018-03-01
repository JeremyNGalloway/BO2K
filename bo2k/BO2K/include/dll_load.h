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

#ifndef __INC_DLL_LOAD_H
#define __INC_DLL_LOAD_H

#include<windows.h>

#define NTSIGNATURE(ptr) ((LPVOID)((BYTE *)(ptr) + ((PIMAGE_DOS_HEADER)(ptr))->e_lfanew))
#define SIZE_OF_NT_SIGNATURE (sizeof(DWORD))
#define PEFHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE))
#define OPTHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE+sizeof(IMAGE_FILE_HEADER)))
#define SECHDROFFSET(ptr) ((LPVOID)((BYTE *)(ptr)+((PIMAGE_DOS_HEADER)(ptr))->e_lfanew+SIZE_OF_NT_SIGNATURE+sizeof(IMAGE_FILE_HEADER)+sizeof(IMAGE_OPTIONAL_HEADER)))
#define RVATOVA(base,offset) ((LPVOID)((DWORD)(base)+(DWORD)(offset)))
#define VATORVA(base,offset) ((LPVOID)((DWORD)(offset)-(DWORD)(base)))

#define SIZE_OF_PARAMETER_BLOCK 4096
#define IMAGE_PARAMETER_MAGIC 0xCDC31337
#define MAX_DLL_PROCESSES 256
#define DLL_ATTACH 0
#define DLL_DETACH 1

// NEW FLAGS
#define REBIND_IMAGE_IMPORTS 0x00000100
#define RWX_PERMISSIONS 0x00000200
#define FORCE_LOAD_NEW_IMAGE 0x00000400

// Internal functions
BOOL MapDLLFromImage(void *pDLLFileImage, void *pMemoryImage);
BOOL PrepareDLLImage(void *pMemoryImage, DWORD dwImageSize, BOOL bResolve, BOOL bRebind);

// Primary functions
void InitializeDLLLoad(void);
void KillDLLLoad(void);
HMODULE GetDLLHandle(char *svName);
DWORD GetDLLFileName(HMODULE hModule, LPTSTR lpFileName, DWORD nSize);
FARPROC GetDLLProcAddress(HMODULE hModule, LPCSTR lpProcName);
FARPROC SetDLLProcAddress(HMODULE hModule, LPCSTR lpProcName, FARPROC fpAddr);
BOOL ResetDLLProcAddress(HMODULE hModule, LPCSTR lpProcName);
HMODULE LoadDLLFromImage(void *pDLLFileImage, char *svMappingName, DWORD dwFlags);
HMODULE LoadDLLEx(LPCTSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);
HMODULE LoadDLL(LPCTSTR lpLibFileName);
BOOL FreeDLL(HMODULE hLibModule);




#endif