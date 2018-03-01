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
#include<cmd\cmd_compress.h>
#include<lzhcompress.h>

int CmdProc_FreezeFile(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	CLzhCompress *clc;

	clc=new CLzhCompress();

	IssueAuthCommandReply(cas_from,comid,1,"Freeze started... please wait.\n");
	if(clc->lzh_freeze_file(svArg2,svArg3)==-1) {
		IssueAuthCommandReply(cas_from,comid,0,"Error opening file.\n");
		delete clc;
		return 1;
	}

	IssueAuthCommandReply(cas_from,comid,0,"File frozen.\n");
	delete clc;
	return 0;
}

int CmdProc_MeltFile(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	CLzhCompress *clc;

	clc=new CLzhCompress();

	IssueAuthCommandReply(cas_from,comid,1,"Melt started... please wait.\n");
	if(clc->lzh_melt_file(svArg2,svArg3)==-1) {
		IssueAuthCommandReply(cas_from,comid,0,"Error opening file.\n");
		delete clc;
		return 1;
	}

	IssueAuthCommandReply(cas_from,comid,0,"File melted.\n");
	delete clc;
		
	return 0;
}
