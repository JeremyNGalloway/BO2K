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

#ifndef __INC_CMD_HICLIENT_H
#define __INC_CMD_HICLIENT_H

#include<windows.h>

#pragma pack(push,1)

#pragma warning(disable:4200)

typedef struct {
	BYTE bAction;
	BYTE bDevice;
	union {
		struct {
			WORD wPosX;
			WORD wPosY;
		} mouse;
		struct {
			BYTE bVirtKey;
			BYTE bScanCode;
			DWORD dwKeyFlags;
		} keybd;
		struct {
			DWORD dwDataLen;
		} message;
	};
} HIJACK_HEADER;

#define HA_MOVE         1
#define HA_LBUTTONDOWN  2
#define HA_LBUTTONUP    3
#define HA_MBUTTONDOWN  4
#define HA_MBUTTONUP    5
#define HA_RBUTTONDOWN  6
#define HA_RBUTTONUP    7
#define HA_LBUTTONDBL   8
#define HA_MBUTTONDBL   9
#define HA_RBUTTONDBL   10
#define HA_KEYDOWN      11
#define HA_KEYUP        12
#define HA_OWNDEVICE    13
#define HA_FREEDEVICE   14
#define HA_MESSAGE      15
#define HA_SUCCESS      16
#define HA_FAILURE      17

#define HBF_LBUTTON 1
#define HBF_MBUTTON 2
#define HBF_RBUTTON 4

#define HD_KEYBD      0
#define HD_MOUSE      1

#pragma pack(pop)

int CreateHijackClient(HWND hParent);

#endif