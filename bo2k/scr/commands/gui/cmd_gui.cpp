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
#include<iohandler.h>
#include<cmd\cmd_gui.h>


static DWORD WINAPI MBThread(LPVOID parm)
{
	char *svText = (char *) parm;
	char *svMsg,*svTitle;

	svMsg=svText;
	svTitle=svText+lstrlen(svMsg)+1;

	MessageBox(GetDesktopWindow(),svMsg,svTitle,MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL);

	free(parm);

	return 0;
}



int CmdProc_SysMessageBox(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char *ptr;
	int msglen;
	DWORD tid;

	msglen=lstrlen(svArg3)+lstrlen(svArg2)+2;
	ptr=(char *) malloc(msglen);
	if(ptr==NULL) {
		IssueAuthCommandReply(cas_from, comid, 0, "Error allocating memory.\n");
		return -1;
	}

	lstrcpy(ptr,svArg3);
	lstrcpy(ptr+lstrlen(svArg3)+1,svArg2);
	
	CreateThread(NULL,0,MBThread,ptr,0,&tid);

	IssueAuthCommandReply(cas_from, comid, 0, "Dialog box displayed.\n");
	return 0;
}
