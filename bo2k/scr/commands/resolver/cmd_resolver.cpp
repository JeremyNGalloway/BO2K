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
#include<cmd\cmd_resolver.h>
#include<strhandle.h>

int IssueHostent(CAuthSocket *cas_from, int comid, struct hostent *he)
{
	char svBuffer[1024];

	// Print FQDN
	wsprintf(svBuffer,"Fully Qualified Domain Name: %.512s\n",he->h_name);
	IssueAuthCommandReply(cas_from,comid,1,svBuffer);
	
	// Print Addresses
	int i;
	IssueAuthCommandReply(cas_from,comid,1,"IP Addresses:\n");
	for(i=0;i<(he->h_length/4);i++) {
		wsprintf(svBuffer,"  %u.%u.%u.%u\n",
			*(BYTE *)(he->h_addr_list[i]),
			*((BYTE *)(he->h_addr_list[i])+1),
			*((BYTE *)(he->h_addr_list[i])+2),
			*((BYTE *)(he->h_addr_list[i])+3));
		IssueAuthCommandReply(cas_from,comid,1,svBuffer);
	}


	if(he->h_aliases!=NULL) {
		char **psvName;
		psvName=he->h_aliases;
		IssueAuthCommandReply(cas_from,comid,1,"Alternate Names:\n");
		while(*psvName!=NULL) {
			wsprintf(svBuffer,"  %.512s\n",*psvName);
			IssueAuthCommandReply(cas_from,comid,1,svBuffer);
			psvName++;
		}
	}

	return 0;
}


int CmdProc_ResolveHost(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	struct hostent *he;
	he=gethostbyname(svArg2);
	if(he==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Unable to resolve hostname.\n");
		return 1;
	}

	IssueHostent(cas_from,comid,he);

	IssueAuthCommandReply(cas_from,comid,0,"End of hostname record.\n");	
	
	return 0;
}

int CmdProc_ResolveAddr(CAuthSocket *cas_from, int comid, DWORD nArg1, char *svArg2, char *svArg3)
{
	char addr[4];
	char *svAddr,*svNext;
	int i;
	
	memset(addr,0,4);
	svAddr=svArg2;
	for(i=0;i<4;i++) {
		if(svAddr==NULL) break;
		svNext=BreakString(svAddr,".");
		addr[i]=atoi(svAddr);
		svAddr=svNext;
	}
	
	struct hostent *he;
	he=gethostbyaddr(addr,4,AF_INET);
	if(he==NULL) {
		IssueAuthCommandReply(cas_from,comid,0,"Unable to resolve host address.\n");
		return 1;
	}

	IssueHostent(cas_from,comid,he);

	IssueAuthCommandReply(cas_from,comid,0,"End of hostname record.\n");	
	
	return 0;
}
