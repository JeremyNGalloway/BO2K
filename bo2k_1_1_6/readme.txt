BO2K Core Package and Plugins
Version v1.1.6


Table of Contents:

1 - Introduction
2 - Licencing and Legal Information
3 - Installation and Usage
3.1 - Installation
3.2 - Help and Support
3.3 - Third-Party Plugins
3.4 - Development Plugins (unstable)
4 - Version History
5 - Credits, Contacts and Acknowledgments
6 - Conclusion and Notes

1 -- Introduction

BO2K is a remote administration tool for Windows systems. It comes with a client
and a server. The server is lightweight and inobtrusive. A dynamic plugin
architechture allows for easy system extension.

This release consists of the binaries for the core applications and plugins.
Included files are:

bo2k.exe - Server program    ver 1.1.5
bo2kgui.exe - Client program ver 1.3.1
bo2kcfg.exe - Server configuration Utility  ver 1.2.0.5

auth_null.dll - Null authentication module
enc_null.dll - Null encryption module
io_tcp.dll - TCP IO module
io_udp.dll - UDP IO module

srv_interface.dll	ver 1.3
srv_control.dll		ver 1.2
srv_regfile.dll		ver 1.3
srv_system.dll		ver 1.0
srv_inetcmd.dll		ver 1.0
srv_legacy.dll		ver 1.0
srv_reverser.dll	ver 1.4
srv_rootkit.dll		ver 1.1

cli_botool		ver 1.5.0.7

misc_livekeylog.dll	ver 1.0
misc_bochat.dll		ver 1.0
misc_bopeep.dll		ver 1.4


2 -- Licencing and Legal Information

	BO2K v1.1.6
	Copyright (C) 2007 Bo2k Development Team (http://www.bo2k.com/)
	
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA


3 -- Installation and Usage

3.1 -- Installation

Just unzip the package on a directory of your choice. See the on-line documentation
(links below) for usage instructions.


3.2 -- Help and Support

Support for this package is provided on-line. Please check the following websites:

- BO2K Website:
http://www.bo2k.com/

- Project's Tracker:
https://sourceforge.net/tracker/?group_id=4487

- Forums:
http://bo2k.sf.net/forums/

- Documentation:
http://www.bo2k.com/documentation.html

- Mailing Lists:
https://sourceforge.net/tracker/?group_id=4487


3.3 -- Third-Party Plugins (stable)

Third-party plugins for BO2K 1.1.x are avaliable at:

http://www.bo2k.com/software/bo2k11.html
or
https://sourceforge.net/project/showfiles.php?group_id=4487&release_id=55979


3.4 -- Development Plugins (not necessarily stable)

Latest developer plugins for BO2K 1.1.x are avaliable at:

http://bo2k.sourceforge.net/Development/

These plugins are the latest available however they are in development and may be unstable.


4 -- Version History

---< 1.1.6 ( 25 Mar 07 ) >---

This version is mainly in support of the new bopeep plugin.

- New Client (ver 1.3.1)

a. Modifications to the plugin button on the Bo Server Connection Dialog
b. Client now checks for proper version of botool and bopeep
c. Modifications to accept reverse connections from bopeep.


- New cli_botool.dll  (ver 1.5.0.7)

a. Modified to accept proper command for connections through the plugin button and right click from server list
b. Now checks for proper version of client


- New misc_bopeep.dll  (ver 1.4)

a. Memory leak for client and server is fixed 
b. New control "ROAM" cuts screen link with remote mouse and allows you to scroll around screen using buttons which will appear around display screen when "ROAM" radio button is selected...updates every second
c. Full Screen button allows you to view the entire remote screen within the display frame...updates every 5 seconds.
d. New Control quality spinners allow you to control the display quality on the fly...range from 10 to 95.
e. Reverse connection now works for both viewing and hijack.
f. Now checks for proper version of client


--------Known issues--------
The reverse connect for botool can get out of sync sometimes...if it happens close down client and restart.
Insidious mode is flakey...don't use it
Remote to remote file transfer is broken for now......
Repeated attempts for reverse registry connect can cause client to not recognize the botool plugin
Hijack does not always register a mouse click.
Copy button on bopeep client doesn't work.


*************************************************************************************************
---< 1.1.5 ( 18 Jan 07 ) >---

The first thing that must be stated is that backward compatibility has been compromised to an extent. For the most part you can still communicate between older servers and the new client or older clients and the new server etc. but behavior will be erratic when attempting some commands. It is highly recommended that you replace all older servers and their plugins with all the latest from this package.

- New Server ver 1.1.5

a. Rewritten to accommodate rootkit for injected servers using "browser only" option


- New Client (ver 1.2.0.3)

a. Deletion of listener sockets fixed to prevent crash
b. Insert and Delete keys now work for listener sockets
c. Client "hang" on command query during some initial connects fixed
d. Reverse logic rewritten to allow for more than one server from same remote address
e. Client crash when reverse botool connect requested and botool plugin not loaded fixed


- New Server Configuration Utility (ver 1.2.0.5)

a. ENTER and ESC keys now trapped to prevent the tab page from blanking.


- New cli_botool.dll  (ver 1.5.0.6)

a. Disconnect menu item on botool registry client fixed
b. Fatal hang when attempting to view REG_MULTI_SZ values fixed
c. Crash protection when botool registry client can't read a value/key
d. Remote process spawn fixed so that the correct message shows if unsuccessful


- New srv_rootkit.dll (ver 1.1)

a. Rootkit now works correctly for all settings including the "browser only" injection option


- New srv_reverser.dll (ver 1.4)

a. Changes to code to improve reverse connections.
b. Changes to how communications sent to client for reverse botool connections.


--------Known issues--------
The reverse connect for botool can get out of sync sometimes...if it happens close down client and restart.
Insidious mode is flakey...don't use it
Remote to remote file transfer is broken for now......
Repeated attempts for reverse registry connect can cause client to not recognize the botool plugin

************************************************************************************

1.1.4 (15/12/06)

- New Server (ver 1.1.4)

a. Stealth now works on all windows systems including Windows XP SP 1 & 2 (injection)
b. Process hop now works on NT/XP systems allowing you to hop from one app to another turning bo2k effectively into the "Ghost in the machine"
c. New option "Listen Socket" allows you to turn on or off bo2k's listening socket for traditional connections...this is useful for when the reverse plugin is installed and you don't want bo2k detected as a listening server.
d. New option "Browser Only" allows you to keep bo2k sitting inert in the background until the default browser starts at which time bo2k will inject itself into the browser and the reverse plugin will attempt outbound connections...effective against firewalls.
e. Delete server changed for injected servers.


- New Client (ver 1.2)

a. Client redesigned to place listening sockets on a bottom panel and normal connections on the top panel (Thanks to James Keane for the design)
b. Code re-written to allow for unlimited number of listening sockets
c. The "Save workspace" option now allows for listening sockets to be saved in either active or inactive state.
d. Control buttons added for listening socket control, open/close, delete, edit.
e. Code rewritten to allow for easier reverse botool connection, instead of having to set up a new listen port for every botool connection, botool will now use the listen port used for server connection.
f. Splash screen and credits changed.


- New Server Configuration Utility (ver 1.2.0.3)

a. Complete re-design using a tabbed interface allowing for easier viewing of loaded plugins and variable settings.
b. New XP look
c. Allows for drag and drop for loading plugins.
d. New config dump option for easier debugging help.


- New srv_regfile.dll (ver 1.3)

a. Now allows for proper display of Large files and and allows for transfers of files larger than 100 Mb.
b. Allows for resumable uploads and downloads (Must be used with latest botool [ver 1.5.0.2])


- New srv_control.dll (ver 1.2)

a. New command Simple-> "Show Host" will show the Application that bo2k is currently injected into.
b. New option for "Load Plugin" allows server to load plugins that are in a "frozen" state from having used the bo2k "Freeze" command.


- New srv_interface.dll (ver 1.3)

a. Keylogging fixed
b. Plugin size reduced.


- New srv_reverser.dll (ver 1.3)

a. Now allows for resumable uploads and downloads.
b. New command "Browser Start" allows for automatic start and connection to the botool browser client
c. New command "Registry Start" allows for automatic start and connection to the botool registry client.


- New cli_botool.dll  (ver 1.5.0.3)

a. Upload and download now on own threads
b. Drag and drop for uploads reconnected
c. Large files now displayed properly
d. Upload and download of large files now supported
e. Freeze and Melt of local files now supported
f. Direct connect capablilites enabled
g. Reverse connection now automatic when server Reverse=> BoTool connect command issued

************************************************************************************

1.1.3 (23/11/04)

- New Server 

a. "delete original file" re-enabled
b. "eradicate server" now actually removes the server from the remote machine as well as all 	registry entries
c. Stealth is re-enabled as part of the server...save me the rhetoric...there are legitimate 		reasons for stealth in a server. It will effectively hide bo2k from any win9x process 	list but unfortunatly it can't hide in NT type systems.
d. Latest Plugins from the Novice page included

- New Client 
a. Allows option to get rid of disconnect request when closing dialog box
b. Allows "right click" on server list window for connecting plugins
c. Includes a new "plugins" button which allows for quick connects with all details filled in    	http://bo2k.sourceforge.net/novice/Client.htm
d. Latest Plugins from the Novice page included


************************************************************************************		

1.1.2 (01/10/2003)
	- Merged Novice's plugin changes to the main source tree. Increased stability and 
	compatibility with Windows 2000 and XP Operating Systems.


5 -- Credits, Contacts and Acknowledgments

Packager: Novice, novice222 at users dot sourceforge dot net
Contributing Developer: Novice, novice222 at users dot sourceforge dot net
Thanks to Pillowtop, soupnut, DiggerX and Fluffy for testing. 


6 -- Conclusion and Notes

The following websites may be of interest:

http://www.bo2k.com/software/bo2k11.html
http://bo2k.sf.net/Development
