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

#include <windows.h>
#include <shlwapi.h>

int GetComCtlVersion(LPDWORD pdwMajor, LPDWORD pdwMinor)
{
	HINSTANCE hComCtl;
	DLLGETVERSIONPROC pDllGetVersion;
	DLLVERSIONINFO dvi;      
	
	// Check inputs

	if( IsBadWritePtr(pdwMajor, sizeof(DWORD)) || 
        IsBadWritePtr(pdwMinor, sizeof(DWORD)))   
		return -1;
	
	// Load the DLL
	
	hComCtl = LoadLibrary("comctl32.dll");
	if(!hComCtl) return -1;

		
	// You must get this function explicitly because earlier versions of the DLL 
	// don't implement this function. That makes the lack of implementation of the 
	// function a version marker in itself. 

	pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hComCtl, TEXT("DllGetVersion"));
	if(!pDllGetVersion) {
		FreeLibrary(hComCtl);
		*pdwMajor = 4;
		*pdwMinor = 0;
		return 0;
	}
	
	ZeroMemory(&dvi, sizeof(dvi));
	dvi.cbSize = sizeof(dvi);   
	if(!SUCCEEDED((*pDllGetVersion)(&dvi)))  {
		FreeLibrary(hComCtl);
		return -1;
	}
	
	*pdwMajor = dvi.dwMajorVersion;
	*pdwMinor = dvi.dwMinorVersion;
	
	FreeLibrary(hComCtl);
	return 0;   
}
