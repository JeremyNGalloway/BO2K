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

// bo2kcfg.h : main header file for the BO2KCFG application
//

#if !defined(AFX_BO2KCFG_H__BA3A1E06_E042_11D2_ADC4_0000F8084273__INCLUDED_)
#define AFX_BO2KCFG_H__BA3A1E06_E042_11D2_ADC4_0000F8084273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CBo2kcfgApp:
// See bo2kcfg.cpp for the implementation of this class
//

class CBo2kcfgApp : public CWinApp
{
public:
	CBo2kcfgApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBo2kcfgApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	void SetRegKey(char *svKey);

// Implementation

	//{{AFX_MSG(CBo2kcfgApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BO2KCFG_H__BA3A1E06_E042_11D2_ADC4_0000F8084273__INCLUDED_)
