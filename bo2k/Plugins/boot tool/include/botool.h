/*  BOTOOL - Back Orifice 2000 File System and Registry Browser/Editor
    Copyright (C) 1999, L0pht Heavy Industries, Inc.

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

#ifndef __INC_BOTOOL_H
#define __INC_BOTOOL_H

#include<windows.h>
#include<plugins.h>
#include<bocomreg.h>
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>

// ---------------- Global Linkage Variables -----------------
extern CEncryptionHandler				*g_pEncHandler;
extern CIOHandler						*g_pIOHandler;
extern CAuthHandler						*g_pAuthHandler;
extern TYPEOF_RegisterCommand			*RegisterCommand;
extern TYPEOF_UnregisterCommand			*UnregisterCommand;
extern TYPEOF_RegisterClientMenu		*RegisterClientMenu;
extern TYPEOF_UnregisterClientMenu		*UnregisterClientMenu;
extern TYPEOF_IssueAuthCommandRequest	*IssueAuthCommandRequest;
extern TYPEOF_IssueAuthCommandReply		*IssueAuthCommandReply;
extern TYPEOF_ConnectAuthSocket			*ConnectAuthSocket;
extern TYPEOF_ListenAuthSocket			*ListenAuthSocket;
extern TYPEOF_InteractiveConnect		*InteractiveConnect;
extern TYPEOF_InteractiveListen 		*InteractiveListen;

// ------------- Function Implementations ------------------
#define MAX_THREADS 64

extern HINSTANCE g_hInstance;
extern char g_szToolOptions[];
extern BOOL g_bActive;
extern long g_nNumThreads;

#endif
