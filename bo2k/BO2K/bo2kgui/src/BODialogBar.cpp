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

// BODialogBar.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "BODialogBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBODialogBar

CBODialogBar::CBODialogBar()
{
	m_cxOffset			= 5;
	m_cyOffset			= 2;
	
}

CBODialogBar::~CBODialogBar()
{
}

int CBODialogBar::SetDialog(CWnd *pWnd)
{
	SetChild(pWnd);
	return 0;
}

CWnd *CBODialogBar::GetDialog(void)
{
	return m_pChildWnd;
}

void CBODialogBar::Hide(void)
{
	GetDockingFrame()->ShowControlBar(this,FALSE,FALSE);
}

void CBODialogBar::Show(void)
{
	GetDockingFrame()->ShowControlBar(this,TRUE,FALSE);
}


BEGIN_MESSAGE_MAP(CBODialogBar, CCJControlBar)
	//{{AFX_MSG_MAP(CBODialogBar)
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBODialogBar message handlers

void CBODialogBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos) 
{
	CControlBar::OnWindowPosChanged(lpwndpos);
	
	if (m_bButtons)
	{
		if (IsFloating())
		{
			m_btnClose.ShowWindow(SW_HIDE);
			m_btnMinim.ShowWindow(SW_HIDE);
		}
		
		else
		{
			m_btnClose.ShowWindow(SW_SHOW);
			m_btnMinim.ShowWindow(SW_SHOW);

			CRect rcClose(GetButtonRect());
			CRect rcMinim(GetButtonRect());

			if (IsHorzDocked()) {
				rcMinim.OffsetRect(0,14);
			}

			else {
				rcClose.OffsetRect(14,0);
			}

			m_btnClose.MoveWindow(rcClose);
			m_btnMinim.MoveWindow(rcMinim);

            Invalidate();     // Added
		}
	}

	if (m_pChildWnd->GetSafeHwnd()) {
		CRect rc;
		GetChildRect(rc);
	//	rc.DeflateRect(1,1);
		m_pChildWnd->MoveWindow(rc);
	}	
}

void CBODialogBar::GetChildRect(CRect &rect)
{
	GetClientRect(&rect);
	
	if (!IsFloating())
	{
		if (IsVertDocked()) {
			rect.left	+= m_cxOffset;
			rect.right	-= m_cxOffset;
			rect.top	+= (GetMinExt()-5);
			rect.bottom	-= (m_iTrackBorderSize+3);
		}

		else if(IsHorzDocked()) {
			rect.left	+= (GetMinExt()-5);
			rect.right	-= (m_iTrackBorderSize+3);
			rect.top	+= m_cyOffset;
			rect.bottom -= m_cyOffset;
		}

		if( rect.left > rect.right || rect.top > rect.bottom )
			rect = CRect(0,0,0,0);
	}
}

void CBODialogBar::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	
	CCJControlBar::OnClose();
}
