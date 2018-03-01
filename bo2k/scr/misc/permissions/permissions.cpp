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
#include<functions.h>

void IssueSecurityDescriptor(CAuthSocket *cas_from, int comid, PSECURITY_DESCRIPTOR psd)
{
	char svBuffer[1024];

	SID *pOwner, *pGroup;
	PACL pAcl;
	char svOwnerName[200];
	char svOwnerDomain[200];
	char svGroupName[200];
	char svGroupDomain[200];
	SID_NAME_USE snuse;
	BOOL bDacl,bDef;
	DWORD dwNameLen, dwDomainLen;
	
	pGetSecurityDescriptorOwner(psd,(LPVOID *) &pOwner,&bDef);
	pGetSecurityDescriptorGroup(psd,(LPVOID *) &pGroup,&bDef);
	
	if(pOwner!=NULL) {
		dwNameLen=256; dwDomainLen=256;
		pLookupAccountSid(NULL,pOwner,svOwnerName,&dwNameLen,svOwnerDomain,&dwDomainLen,&snuse);
	} else {
		lstrcpy(svOwnerDomain,"NONE");
		lstrcpy(svOwnerName,"NONE");
	}
	if(pGroup!=NULL) {
		dwNameLen=256; dwDomainLen=256;
		pLookupAccountSid(NULL,pGroup,svGroupName,&dwNameLen,svGroupDomain,&dwDomainLen,&snuse);
	} else {
		lstrcpy(svGroupDomain,"NONE");
		lstrcpy(svGroupName,"NONE");
	}

	wsprintf(svBuffer,"   Owner: %s\\%s  Group: %s\\%s\n",svOwnerDomain,svOwnerName,svGroupDomain,svGroupName);
	IssueAuthCommandReply(cas_from,comid,1,svBuffer);
	
	pGetSecurityDescriptorDacl(psd,&bDacl,&pAcl,&bDef);
	if(bDacl) {
		int j;
		for(j=0;j<pAcl->AceCount;j++) {
			void *pAce;
			char svName[200];
			char svDomain[200];
			SID *pSid;
			ACCESS_MASK mask;
			
			if(pGetAce(pAcl, j, &pAce)) {
				char *svAcc;
				switch(((ACE_HEADER *)pAce)->AceType) {
				case ACCESS_ALLOWED_ACE_TYPE:
					svAcc="ACCEPT";
					pSid=(SID *)&(((ACCESS_ALLOWED_ACE *)pAce)->SidStart);
					mask=((ACCESS_ALLOWED_ACE *)pAce)->Mask;
					break;
				case ACCESS_DENIED_ACE_TYPE:
					svAcc="DENIED";
					pSid=(SID *)&(((ACCESS_DENIED_ACE *)pAce)->SidStart);
					mask=((ACCESS_DENIED_ACE *)pAce)->Mask;
					break;
				default:
					svAcc="UNKNOWN:";
					break;
				}
				
				dwNameLen=256; dwDomainLen=256;
				pLookupAccountSid(NULL,pSid,svName,&dwNameLen,svDomain,&dwDomainLen,&snuse);
				wsprintf(svBuffer,"   %s: %.200s\\%.200s (%s%s%s%s%s%s%s%s%s:0x%4.4X)\n",
					svAcc,
					svDomain,
					svName,
					(mask&(1<<16))?"D ":"",
					(mask&(1<<17))?"RC ":"",
					(mask&(1<<18))?"WD ":"",
					(mask&(1<<19))?"WO ":"",
					(mask&(1<<20))?"S ":"",
					(mask&(1<<28))?"GA ":"",
					(mask&(1<<29))?"GX ":"",
					(mask&(1<<30))?"GW ":"",
					(mask&(1<<31))?"GR ":"",
					(WORD)mask);

				IssueAuthCommandReply(cas_from,comid,1,svBuffer);
			}
		}
	}			
}
