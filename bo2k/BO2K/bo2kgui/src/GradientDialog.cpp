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

// GradientDialog.cpp : implementation file
//

#include "stdafx.h"
#include "bo2kgui.h"
#include "GradientDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GradientDialog dialog


CGradientDialog::CGradientDialog()
{
	CommonConstruct();
}


CGradientDialog::CGradientDialog(UINT uResource, CWnd* pParent /*=NULL*/)
	: CDialog(uResource, pParent)
{
	CommonConstruct();
}


CGradientDialog::CGradientDialog(LPCTSTR pszResource, CWnd* pParent /*=NULL*/)
	: CDialog(pszResource, pParent)
{
	CommonConstruct();
}

CGradientDialog::~CGradientDialog()
{
	if(m_pGradBits) {
		free(m_pGradBits);
		m_pGradBits=NULL;
	}
}

void CGradientDialog::CommonConstruct()
{
	VERIFY(m_HollowBrush.CreateStockObject(HOLLOW_BRUSH));

	m_bGradient=FALSE;
	m_pGradBits=NULL;

	//{{AFX_DATA_INIT(CGradientDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CGradientDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGradientDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}



HBRUSH CGradientDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	switch(nCtlColor) {
	case CTLCOLOR_STATIC:
		// The Slider Control has CTLCOLOR_STATIC, but doesn't let
		// the background shine through,
		TCHAR lpszClassName[255];
		GetClassName(pWnd->m_hWnd, lpszClassName, 255);
		if(lstrcmp(lpszClassName, TRACKBAR_CLASS) == 0)
			return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
		
	case CTLCOLOR_BTN:
		// let static controls shine through
		pDC->SetBkMode(TRANSPARENT);
		return HBRUSH(m_HollowBrush);
		
	default:
		break;
	}
	
	// if we reach this line, we haven't set a brush so far
	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}


BEGIN_MESSAGE_MAP(CGradientDialog, CDialog)
	//{{AFX_MSG_MAP(CGradientDialog)
	ON_WM_CTLCOLOR()
 	ON_WM_ERASEBKGND()
	ON_WM_QUERYNEWPALETTE()
    ON_WM_PALETTECHANGED()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGradientDialog message handlers

void CGradientDialog::SetGradient()
{
	m_bGradient=FALSE;
}

void CGradientDialog::SetGradient(COLORREF c1, COLORREF c2, int dir)
{
	m_bGradient=TRUE;
	m_Color1=c1;
	m_Color2=c2;
	m_nNumColors=236;
	m_nDir=dir;
	CreateGradPalette();
	CreateGradientBitmap();
}


BOOL CGradientDialog::OnEraseBkgnd(CDC* pDC) 
{
	if(m_bGradient) {
		CRect rc;
		int w,h;
		GetClientRect(rc);
		w=rc.Width();
		h=rc.Height();
		if(w==0 || h==0) {
			return CDialog::OnEraseBkgnd(pDC);		
		}

		if(m_pGradBits==NULL) return FALSE;

		StretchDIBits(pDC->GetSafeHdc(),0,0,w,h,0,0,320,240,m_pGradBits,&m_biGradBitInfo,DIB_RGB_COLORS,SRCCOPY);

	} else {
		return CDialog::OnEraseBkgnd(pDC);
	}

	return TRUE;
}

//BEGIN_MESSAGE_MAP(CGradpalWnd, CFrameWnd)
    //ON_WM_PAINT()
//END_MESSAGE_MAP()

BOOL CGradientDialog::OnQueryNewPalette()
{
    CClientDC dc(this);
    
    CPalette *pPalOld = dc.SelectPalette(&m_Pal, FALSE);
    
    BOOL bRet = dc.RealizePalette();
    
    dc.SelectPalette(pPalOld, FALSE);
    
    if (bRet)
        // some colors changed
        this->Invalidate();
    
    return bRet;
}

void CGradientDialog::OnPaletteChanged(CWnd *pFocusWnd)
{
    if (pFocusWnd != this)
        this->OnQueryNewPalette();
}

BOOL CGradientDialog::CreateGradPalette()
{
    if (m_Pal.GetSafeHandle() != NULL) {
		m_Pal.DeleteObject();
	}
    
    BOOL bSucc = FALSE;
        
    LPLOGPALETTE lpPal = (LPLOGPALETTE)new BYTE[sizeof(LOGPALETTE) +
                                                sizeof(PALETTEENTRY) *
                                                m_nNumColors];
    
    if (lpPal != NULL)
    {
		int nIndex;
        lpPal->palVersion = 0x300;
        lpPal->palNumEntries = m_nNumColors;
        
        PALETTEENTRY *ppe = lpPal->palPalEntry;
        int r1=GetRValue(m_Color1);
		int r2=GetRValue(m_Color2);
		int g1=GetGValue(m_Color1);
		int g2=GetGValue(m_Color2);
		int b1=GetBValue(m_Color1);
		int b2=GetBValue(m_Color2);

        for (nIndex = 0; nIndex < m_nNumColors; nIndex++)
        {
            ppe->peRed = (BYTE) r1 + MulDiv((r2-r1),nIndex,m_nNumColors-1);
            ppe->peGreen = (BYTE) g1 + MulDiv((g2-g1),nIndex,m_nNumColors-1);
            ppe->peBlue = (BYTE) b1 + MulDiv((b2-b1),nIndex,m_nNumColors-1);
            ppe->peFlags = (BYTE)0;

			m_PalVal[nIndex]=(ppe->peRed << 16) | (ppe->peGreen << 8) | (ppe->peBlue);
			
            ppe++;
        }
        
        bSucc = m_Pal.CreatePalette(lpPal);
			
        delete [](PBYTE)lpPal;
    }
    
    return bSucc;
}

void CGradientDialog::CreateGradientBitmap(void)
{
	int x,y,w,h;
	CRect rectangle;
	w=320;
	h=240;
	
	if(m_pGradBits!=NULL) {
		free(m_pGradBits);
		m_pGradBits=NULL;
	}
	
	m_pGradBits=(DWORD*) malloc(w*h*sizeof(DWORD));
	memset(&m_biGradBitInfo,0,sizeof(BITMAPINFO));
	m_biGradBitInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	m_biGradBitInfo.bmiHeader.biWidth=320;
	m_biGradBitInfo.bmiHeader.biHeight=240;
	m_biGradBitInfo.bmiHeader.biPlanes=1;
	m_biGradBitInfo.bmiHeader.biBitCount=32;
	m_biGradBitInfo.bmiHeader.biCompression=BI_RGB;
	
	if(m_nDir==0) {
		for(y=0;y<h;y++) {
			for(x=0;x<w;x++) {
				*(m_pGradBits+(y*w)+x)=m_PalVal[MulDiv(m_nNumColors,y,h)];
			}
		}
	}
	else if(m_nDir==1) {
		for(y=0;y<h;y++) {
			int l,r;
			l=MulDiv((m_nNumColors/2),y,h);
			r=l+(m_nNumColors/2)-1;
			for(x=0;x<w;x++) {
				*(m_pGradBits+(y*w)+x)=m_PalVal[l+MulDiv((r-l),x,w)];
			}
		}
	}
	else if(m_nDir==2) {
		for(x=0;x<w;x++) {
			for(y=0;y<h;y++) {
				*(m_pGradBits+(y*w)+x)=m_PalVal[MulDiv(m_nNumColors,x,w)];
			}
		}
	}
	else if(m_nDir==3) {
		for(y=0;y<h;y++) {
			int l,r;
			r=MulDiv((m_nNumColors/2),y,h);
			l=r+(m_nNumColors/2)-1;
			for(x=0;x<w;x++) {
				*(m_pGradBits+(y*w)+x)=m_PalVal[l+MulDiv((r-l),x,w)];
			}
		}
	}
}

void CGradientDialog::OnRButtonDown(UINT nFlags, CPoint point) 
{
	if(m_bGradient) {
		HMENU hmenu=LoadMenu(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_GRADIENTMENU));
		HMENU hsubmenu=GetSubMenu(hmenu,0);
		ClientToScreen(&point);
		int nCmd=TrackPopupMenu(hsubmenu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_NONOTIFY|TPM_RETURNCMD|TPM_RIGHTBUTTON,
			point.x,point.y,0,GetSafeHwnd(),NULL);
		
		COLORREF dwFace=GetSysColor(COLOR_3DFACE);
		COLORREF dwLight=GetSysColor(COLOR_3DHILIGHT);
		COLORREF dwShadow=GetSysColor(COLOR_3DSHADOW);
		COLORREF dwFadeFace=dwFace;
		COLORREF dwFadeLight=RGB(255-((255-GetRValue(dwLight))/2),
			255-((255-GetGValue(dwLight))/2),
			255-((255-GetBValue(dwLight))/2));
		COLORREF dwFadeShadow=RGB(GetRValue(dwShadow)/2,
			GetGValue(dwShadow)/2,
			GetBValue(dwShadow)/2);

		switch(nCmd) {
		case IDM_COLOR_0:
			SetGradient(dwFace,dwLight,1);
			break;
		case IDM_COLOR_1:
			SetGradient(dwShadow,dwFace,1);
			break;
		case IDM_COLOR_2:
			SetGradient(dwFadeFace,dwFadeLight,1);
			break;
		case IDM_COLOR_3:
			SetGradient(dwFadeShadow,dwFadeFace,1);
			break;
		case IDM_COLOR_RED:
			SetGradient(RGB(216,115,115),RGB(226,66,66),1);
			break;
		case IDM_COLOR_ORANGE:
			SetGradient(RGB(240,148,80),RGB(234,107,72),1);
			break;
		case IDM_COLOR_YELLOW:
			SetGradient(RGB(239,235,136),RGB(172,170,56),1);
			break;
		case IDM_COLOR_GREEN:
			SetGradient(RGB(131,213,81),RGB(127,172,126),1);
			break;
		case IDM_COLOR_BLUE:
			SetGradient(RGB(25,174,222),RGB(120,177,203),1);
			break;
		case IDM_COLOR_VIOLET:
			SetGradient(RGB(150,129,183),RGB(102,110,176),1);
			break;
		case IDM_COLOR_BROWN:
			SetGradient(RGB(203,185,156),RGB(158,139,117),1);
			break;
		}
		InvalidateRect(NULL,TRUE);
		UpdateWindow();
		DestroyMenu(hmenu);
	}
		
	CDialog::OnRButtonDown(nFlags, point);
}
