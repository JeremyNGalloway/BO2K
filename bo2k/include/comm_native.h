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

#ifndef __INC_COMM_NATIVE_H
#define __INC_COMM_NATIVE_H

// Simple commands
#define		BO_COMMANDS_SIMPLE		0

#define		BO_PING					(BO_COMMANDS_SIMPLE+0)
#define		BO_QUERY				(BO_COMMANDS_SIMPLE+1)

// System Commands
#define		BO_COMMANDS_SYSTEM		10

#define		BO_SYSREBOOT			(BO_COMMANDS_SYSTEM+0)
#define		BO_SYSLOCKUP			(BO_COMMANDS_SYSTEM+1)
#define		BO_SYSLISTPASSWORDS		(BO_COMMANDS_SYSTEM+2)
#define		BO_SYSINFO				(BO_COMMANDS_SYSTEM+3)
#define		BO_SYSVIEWCONSOLE		(BO_COMMANDS_SYSTEM+4)

// Input
#define		BO_COMMANDS_INPUT		20

#define		BO_SYSLOGKEYS			(BO_COMMANDS_INPUT+0)
#define		BO_SYSENDKEYLOG			(BO_COMMANDS_INPUT+1)
#define		BO_SYSLOGVIEW			(BO_COMMANDS_INPUT+2)
#define		BO_SYSLOGDELETE 		(BO_COMMANDS_INPUT+3)
	
// GUI Commands
#define		BO_COMMANDS_GUI			30
#define		BO_SYSMESSAGEBOX		(BO_COMMANDS_GUI+0)
		
// TCP/IP
#define		BO_COMMANDS_TCPIP		40

#define		BO_REDIRADD				(BO_COMMANDS_TCPIP+0)
#define		BO_APPADD				(BO_COMMANDS_TCPIP+1)
#define		BO_HTTPENABLE			(BO_COMMANDS_TCPIP+2)
#define		BO_TCPFILERECEIVE		(BO_COMMANDS_TCPIP+3)
#define		BO_PORTLIST				(BO_COMMANDS_TCPIP+4)
#define		BO_PORTDEL				(BO_COMMANDS_TCPIP+5)
#define		BO_TCPFILESEND			(BO_COMMANDS_TCPIP+6)
	
// M$ Networking Commands
#define		BO_COMMANDS_MSNET		50

#define		BO_NETEXPORTADD			(BO_COMMANDS_MSNET+0)
#define		BO_NETEXPORTDELETE		(BO_COMMANDS_MSNET+1)
#define		BO_NETEXPORTLIST		(BO_COMMANDS_MSNET+2)
#define		BO_NETVIEW				(BO_COMMANDS_MSNET+3)
#define		BO_NETUSE				(BO_COMMANDS_MSNET+4)
#define		BO_NETDELETE			(BO_COMMANDS_MSNET+5)
#define		BO_NETCONNECTIONS		(BO_COMMANDS_MSNET+6)
		
// Process Handling
#define		BO_COMMANDS_PROCESS		60

#define		BO_PROCESSLIST			(BO_COMMANDS_PROCESS+0)
#define		BO_PROCESSKILL			(BO_COMMANDS_PROCESS+1)
#define		BO_PROCESSSPAWN			(BO_COMMANDS_PROCESS+2)

// Registry Management
#define		BO_COMMANDS_REGISTRY	70

#define		BO_REGISTRYCREATEKEY	(BO_COMMANDS_REGISTRY+0)
#define		BO_REGISTRYSETVALUE		(BO_COMMANDS_REGISTRY+1)
#define		BO_REGISTRYGETVALUE		(BO_COMMANDS_REGISTRY+2)
#define		BO_REGISTRYDELETEKEY	(BO_COMMANDS_REGISTRY+3)
#define		BO_REGISTRYDELETEVALUE	(BO_COMMANDS_REGISTRY+4)
#define		BO_REGISTRYRENAMEKEY	(BO_COMMANDS_REGISTRY+5)
#define		BO_REGISTRYRENAMEVALUE	(BO_COMMANDS_REGISTRY+6)
#define		BO_REGISTRYENUMKEYS		(BO_COMMANDS_REGISTRY+7)
#define		BO_REGISTRYENUMVALS		(BO_COMMANDS_REGISTRY+8)

// Multimedia Controls 
#define		BO_COMMANDS_MM			80

#define		BO_MMCAPFRAME			(BO_COMMANDS_MM+0)
#define		BO_MMCAPAVI				(BO_COMMANDS_MM+1)
#define		BO_MMPLAYSOUND			(BO_COMMANDS_MM+2)
#define		BO_MMLOOPSOUND			(BO_COMMANDS_MM+3)
#define		BO_MMSTOPSOUND			(BO_COMMANDS_MM+4)
#define		BO_MMLISTCAPS			(BO_COMMANDS_MM+5)
#define		BO_MMCAPSCREEN			(BO_COMMANDS_MM+6)
		
// File and Directory Commands
#define		BO_COMMANDS_FILE		90

#define		BO_DIRECTORYLIST		(BO_COMMANDS_FILE+0)
#define		BO_FILEFIND				(BO_COMMANDS_FILE+1)
#define		BO_FILEDELETE			(BO_COMMANDS_FILE+2)
#define		BO_FILEVIEW				(BO_COMMANDS_FILE+3)
#define		BO_FILERENAME			(BO_COMMANDS_FILE+4)
#define		BO_FILECOPY				(BO_COMMANDS_FILE+5)
#define		BO_DIRECTORYMAKE		(BO_COMMANDS_FILE+6)
#define		BO_DIRECTORYDELETE		(BO_COMMANDS_FILE+7)
#define		BO_SETFILEATTR			(BO_COMMANDS_FILE+8)
#define		BO_RECEIVEFILE			(BO_COMMANDS_FILE+9)
#define		BO_SENDFILE				(BO_COMMANDS_FILE+10)
#define		BO_EMITFILE				(BO_COMMANDS_FILE+11)
#define		BO_LISTTRANSFERS		(BO_COMMANDS_FILE+12)
#define		BO_CANCELTRANSFER		(BO_COMMANDS_FILE+13)
	
// File Compression
#define		BO_COMMANDS_COMPRESS	110

#define		BO_FILEFREEZE			(BO_COMMANDS_COMPRESS+0)
#define		BO_FILEMELT				(BO_COMMANDS_COMPRESS+1)
	
// Resolver
#define		BO_COMMANDS_DNS			120

#define		BO_RESOLVEHOST			(BO_COMMANDS_DNS+0)
#define		BO_RESOLVEADDR			(BO_COMMANDS_DNS+1)
		
// Server Control
#define		BO_COMMANDS_SERVERCTRL	130

#define		BO_SHUTDOWNSERVER		(BO_COMMANDS_SERVERCTRL+0)
#define		BO_RESTARTSERVER		(BO_COMMANDS_SERVERCTRL+1)
#define		BO_LOADPLUGINDLL		(BO_COMMANDS_SERVERCTRL+2)
#define		BO_DEBUGPLUGINDLL		(BO_COMMANDS_SERVERCTRL+3)
#define		BO_LISTPLUGINDLLS		(BO_COMMANDS_SERVERCTRL+4)
#define		BO_REMOVEPLUGINDLL		(BO_COMMANDS_SERVERCTRL+5)
#define		BO_STARTCOMMANDSOCKET	(BO_COMMANDS_SERVERCTRL+6)
#define		BO_LISTCOMMANDSOCKETS	(BO_COMMANDS_SERVERCTRL+7)
#define		BO_STOPCOMMANDSOCKET	(BO_COMMANDS_SERVERCTRL+8)


// Legacy Buttplug interface
#define		BO_COMMANDS_BUTTPLUGS	140

#define		BO_PLUGINEXECUTE		(BO_COMMANDS_BUTTPLUGS+0)
#define		BO_PLUGINLIST			(BO_COMMANDS_BUTTPLUGS+1)
#define		BO_PLUGINKILL			(BO_COMMANDS_BUTTPLUGS+2)


int RegisterNativeCommands(void);

#endif