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

#ifndef __INC_CMD_CLIENT_H
#define __INC_CMD_CLIENT_H

#include<windows.h>

#pragma pack(push,1)

#pragma warning(disable:4200)

typedef struct {
	WORD wPosX;
	WORD wPosY;
	WORD wSizeX;
	WORD wSizeY;
	WORD wCurPosX;
	WORD wCurPosY;
	WORD wFlags;
	DWORD dwSize;
} VIDSTREAM_HEADER;

#define VHF_FULLFRAME 1
#define VHF_FRAMEDIFF 2

#pragma pack(pop)

int CreateVidStreamClient(HWND hParent);

#endif