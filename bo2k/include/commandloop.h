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

#ifndef __INC_COMMANDLOOP_H
#define __INC_COMMANDLOOP_H

#include<auth.h>
#include<iohandler.h>
#include<encryption.h>

#define MAX_COMMAND_SOCKETS 64
#define MAX_COMMAND_CONNECTIONS 256

extern CIOHandler *g_pIOHandler;
extern CEncryptionHandler *g_pEncryptionHandler;
extern CAuthHandler *g_pAuthHandler;

extern CAuthSocket *g_pCommSock[MAX_COMMAND_SOCKETS];
extern CAuthSocket *g_pConnSock[MAX_COMMAND_CONNECTIONS];
extern int g_nCommCount, g_nConnCount;
extern BOOL g_bBO2KFinished;
extern char g_szStartupOptions[];

void CommandHandlerLoop(void);

#endif