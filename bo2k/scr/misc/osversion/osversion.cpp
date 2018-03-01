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
#include<bo_debug.h>
#include<osversion.h>

BOOL g_bIsWinNT;

// Determine Operating System Version
void GetOSVersion(void)
{
	OSVERSIONINFO osvi;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	
	if(GetVersionEx(&osvi)==FALSE) {
		DebugMessageBox( HWND_DESKTOP, "Unable to get version info", "GetOSVersion()", MB_OK );
	}

	if(osvi.dwPlatformId==VER_PLATFORM_WIN32s) {
		DebugMessageBox( HWND_DESKTOP, "This application does not run under WIN32s!", "Error", MB_OK );
	}
	
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) 
		g_bIsWinNT = 1;
	else 
		g_bIsWinNT = 0;
}
