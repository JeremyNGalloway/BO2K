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

#ifndef __INC_COMMNET_H
#define __INC_COMMNET_H

#include<windows.h>
#include<auth.h>

struct bo_command_header {
	int cmdlen;
	char flags;
	int command;
	int comid;
	int nArg1;
	int nArg2Len;
	int nArg3Len;
};

#define CMDFLAG_COMMAND 1
#define CMDFLAG_REPLY   2
	
void BreakDownCommand(BYTE *pInBuffer, int *cmdlen, int *command, int *comid, int *nArg1, char **svArg2, char **svArg3);
BYTE *BuildCommandRequest(int command, int comid, int nArg1, char *svArg2, char *svArg3, int *pnReqLen);
BYTE *BuildCommandReply(int comid, int nReplyCode, char *svReply, int *pnReqLen);
void FreeCommandMemory(BYTE *pCmd);
#ifdef __BO2KSERVER__
int IssueAuthCommandRequest(CAuthSocket *cas_from, int command, int comid, int nArg1, char *svArg2, char *svArg3);
int IssueAuthCommandReply(CAuthSocket *cas_from, int comid, int nReplyCode, char *svReply);
#endif

#endif