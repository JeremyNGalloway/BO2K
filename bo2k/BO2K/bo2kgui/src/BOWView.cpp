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

// BOWView.cpp : implementation of the CBOWView class
//

#include "stdafx.h"
#include "bo2kgui.h"

#include "BOWDoc.h"
#include "BOWview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBOWView

IMPLEMENT_DYNCREATE(CBOWView, CView)

BEGIN_MESSAGE_MAP(CBOWView, CView)
	//{{AFX_MSG_MAP(CBOWView)
	ON_WM_ERASEBKGND()
	ON_WM_RBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBOWView construction/destruction

CBOWView::CBOWView()
{
	m_dwSolidColor=AfxGetApp()->GetProfileInt("Background","Solid Color",RGB(40,40,40));
	m_bImage=AfxGetApp()->GetProfileInt("Background","Bkgnd Image",0);
	m_strImgFile=AfxGetApp()->GetProfileString("Background","Image File",NULL);
	m_bTile=AfxGetApp()->GetProfileInt("Background","Tile",0);

	m_pImgDC=NULL;
}

CBOWView::~CBOWView()
{
	AfxGetApp()->WriteProfileInt("Background","Solid Color",m_dwSolidColor);
	AfxGetApp()->WriteProfileInt("Background","Bkgnd Image",m_bImage);
	AfxGetApp()->WriteProfileString("Background","Image File",m_strImgFile);
	AfxGetApp()->WriteProfileInt("Background","Tile",m_bTile);
	
	if(m_pImgDC!=NULL)
		delete m_pImgDC;
}

BOOL CBOWView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CBOWView drawing

void CBOWView::OnDraw(CDC* pDC)
{
	CBOWDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CBOWView diagnostics

#ifdef _DEBUG
void CBOWView::AssertValid() const
{
	CView::AssertValid();
}

void CBOWView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CBOWDoc* CBOWView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CBOWDoc)));
	return (CBOWDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBOWView message handlers

void CBOWView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
	if(m_bImage) {
		HBITMAP hbm=(HBITMAP)LoadImage(NULL,m_strImgFile,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
		if(hbm==NULL) {
			m_bImage=FALSE;	
		} else {
			cbmBitmap.m_hObject=hbm;
		}
	}
}	



BOOL CBOWView::OnEraseBkgnd(CDC* pDC) 
{
	RECT r;
	GetClientRect(&r);
	int x,y;

	if(!(m_bTile && m_bImage)) {
		pDC->FillSolidRect(&r,m_dwSolidColor);
	}
	if(m_bImage) {
		if(m_pImgDC==NULL) {
			m_pImgDC=new CDC();
			m_pImgDC->CreateCompatibleDC(pDC);
			m_pImgDC->SelectObject(cbmBitmap);
		}
		BITMAP bmp;
		cbmBitmap.GetBitmap(&bmp);
		if(m_bTile) {
			for(y=0;y<(r.bottom-r.top);y+=bmp.bmHeight) {
				for(x=0;x<(r.right-r.left);x+=bmp.bmWidth) {
					pDC->BitBlt(x,y,bmp.bmWidth,bmp.bmHeight,m_pImgDC,0,0,SRCCOPY);
				}
			}
		} else {
			pDC->BitBlt(
				((r.right-r.left)/2)-(bmp.bmWidth/2),
				((r.bottom-r.top)/2)-(bmp.bmHeight/2),
				bmp.bmWidth, bmp.bmHeight, m_pImgDC, 0, 0, SRCCOPY);
		}
	}
		
	return TRUE;

}


void CBOWView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	HMENU hmenu=LoadMenu(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_BKGNDPOPUP));
	HMENU hsubmenu=GetSubMenu(hmenu,0);
	
	CheckMenuItem(hsubmenu,IDM_BITMAP,m_bImage?MF_CHECKED:MF_UNCHECKED);
	CheckMenuItem(hsubmenu,IDM_TILE,m_bTile?MF_CHECKED:MF_UNCHECKED);

	ClientToScreen(&point);
	int cmd=TrackPopupMenu(hsubmenu,TPM_RETURNCMD|TPM_RIGHTBUTTON,point.x,point.y,0,GetSafeHwnd(),NULL);
	switch(cmd) {
	case IDM_TILE:
		m_bTile=!m_bTile;
		break;
	case IDM_SOLIDCOLOR:
		{
			CColorDialog ccd;
			if(ccd.DoModal()==IDOK) {
				m_dwSolidColor=ccd.GetColor();
			}
		}
		break;
	case IDM_BITMAP:
		{
			m_bImage=!m_bImage;
			if(m_bImage) {
				CFileDialog cfd(TRUE,".bmp",m_strImgFile,OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR|OFN_HIDEREADONLY,"Bitmap Files (*.bmp)|*.bmp||");
				if(cfd.DoModal()==IDOK) {
					m_strImgFile=cfd.GetPathName();
				} else {
					m_bImage=FALSE;
				}
			}
		}
	}

	DestroyMenu(hmenu);

	// Update bitmap/image
	if(m_bImage) {
		HBITMAP hbm=(HBITMAP)LoadImage(NULL,m_strImgFile,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
		if(hbm==NULL) {
			m_bImage=FALSE;	
		} else {
			cbmBitmap.DeleteObject();
			cbmBitmap.m_hObject=hbm;
			delete m_pImgDC;
			m_pImgDC=NULL;
		}
	}	

	Invalidate();
}
