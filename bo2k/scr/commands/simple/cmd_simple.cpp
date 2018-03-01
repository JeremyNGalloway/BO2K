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
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<commandloop.h>
#include<bocomreg.h>
#include<cmd\cmd_simple.h>
#include<dll_load.h>
#include<winnt.h>
#include<main.h>


int CmdProc_Ping(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char svLine[512];
	wsprintf(svLine,"Ping reply from %.469s.\n",svArg2);
	IssueAuthCommandReply(cas_from, comid, 0, svLine);
	return 0;
}

int CmdProc_Query(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	int i;
	char svLine[512];
		
	// Version
	PIMAGE_OPTIONAL_HEADER poh;
    poh=(PIMAGE_OPTIONAL_HEADER) OPTHDROFFSET(g_module);
	wsprintf(svLine,"--> Version: Back Orifice 2000 (BO2K) v%1.1u.%1.1u\n\n",poh->MajorImageVersion,poh->MinorImageVersion);
	IssueAuthCommandReply(cas_from, comid, 1, svLine);

	// Extension Commands
	IssueAuthCommandReply(cas_from, comid, 1, "--> Extension Commands:\n");

	for(i=0;i<MAX_BO_COMMANDS;i++) {
		if(command_handler_table[i]!=NULL) {
			if(command_description_table[i].bNativeComm==FALSE) {
				char *svA1, *svA2, *svA3;
				svA1=command_description_table[i].svArgDesc1;
				if(svA1==NULL) svA1="";
				svA2=command_description_table[i].svArgDesc2;
				if(svA2==NULL) svA2="";
				svA3=command_description_table[i].svArgDesc3;
				if(svA3==NULL) svA3="";
				
				wsprintf(svLine,"(%d) %.64s\\%.64s|%.64s|%.64s|%.64s\n", i,
					command_description_table[i].svFolderName,
					command_description_table[i].svCommName,
					svA1,
					svA2,
					svA3);
	
				IssueAuthCommandReply(cas_from, comid, 1,svLine);
			}
		}
	}
	IssueAuthCommandReply(cas_from, comid, 1, "--> End Extension Commands\n\n");

	// IO Handlers
	IssueAuthCommandReply(cas_from, comid, 1, "--> IO Handlers:\n");
	for(i=0;i<MAX_IO_HANDLERS;i++) {
		char *svQuery;
		if((svQuery=g_pIOHandler->Query(i))!=NULL) {
			wsprintf(svLine,"(%d) %.100s\n",i,svQuery);
			IssueAuthCommandReply(cas_from, comid, 1, svLine);
		}
	}
	IssueAuthCommandReply(cas_from, comid, 1, "--> End IO Handlers:\n\n");

	// Encryption Modules
	IssueAuthCommandReply(cas_from, comid, 1, "--> Encryption Handlers:\n");
	for(i=0;i<MAX_IO_HANDLERS;i++) {
		char *svQuery;
		if((svQuery=g_pEncryptionHandler->Query(i))!=NULL) {
			wsprintf(svLine,"(%d) %.100s\n",i,svQuery);
			IssueAuthCommandReply(cas_from, comid, 1, svLine);
		}
	}
	IssueAuthCommandReply(cas_from, comid, 1, "--> End Encryption Handlers\n\n");
	
	// Auth Handlers
	IssueAuthCommandReply(cas_from, comid, 1, "--> Auth Handlers:\n");
	for(i=0;i<MAX_AUTH_HANDLERS;i++) {
		char *svQuery;
		if((svQuery=g_pAuthHandler->Query(i))!=NULL) {
			wsprintf(svLine,"(%d) %.100s\n",i,svQuery);
			IssueAuthCommandReply(cas_from, comid, 1, svLine);
		}
	}
	IssueAuthCommandReply(cas_from, comid, 1, "--> End Auth Handlers:\n\n");

	// End of query
	IssueAuthCommandReply(cas_from, comid, 0, "");
	
	return 0;
}