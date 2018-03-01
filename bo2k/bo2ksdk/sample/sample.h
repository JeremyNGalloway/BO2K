/*  Back Orifice 2000 - Remote Administration Suite
    Copyright (C) 1999, Cult Of The Dead Cow

    This file is free software, and not subject to GNU Public License
	restrictions; you can redistribute it and/or modify it in any way 
	you see fit. This file is suitable for inclusion in a derivative
	work, regardless of license on the work or availability of source code
	to the work. If you redistribute this file, you must leave this
	header intact.
    
	This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

	The author of this program may be contacted at dildog@l0pht.com. */

#ifndef __INC_SAMPLE_H
#define __INC_SAMPLE_H

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
extern char g_szSampleOptions[];
extern BOOL g_bActive;
extern long g_nNumThreads;
extern int g_nSampleCommand;


// command definitions

#endif
