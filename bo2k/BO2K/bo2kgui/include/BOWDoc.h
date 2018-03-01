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

// BOWDoc.h : interface of the CBOWDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BOWDOC_H__720D769F_A7E5_11D2_ADC4_0000F8084273__INCLUDED_)
#define AFX_BOWDOC_H__720D769F_A7E5_11D2_ADC4_0000F8084273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CBOServerDlg;
class CBODialogBar;

#pragma pack(push,1)
typedef struct __serverinfo {
	// Permanent state
	char svName[64];
	char svConnectionType[64];
	char svAddress[512];
	char svEncryption[64];
	char svAuthentication[64];
	
	// Temporary state
	BOOL bOpened;
	CBOServerDlg *pServerDlg;
	CBODialogBar *pServerDlgBDB;
	
	struct __serverinfo *next;
} SERVER_INFO;
#pragma pack(pop)

class CBOWDoc : public CDocument
{


protected: // create from serialization only
	CBOWDoc();
	DECLARE_DYNCREATE(CBOWDoc)

	
// Attributes
public:
	SERVER_INFO *m_pHeadSI;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBOWDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CBOWDoc();
	void *InsertServer(const char *svName,
	                   const char *svConnectionType,
					   const char *svAddress,
					   const char *svEncryption,
					   const char *svAuthentication);
	void EditServer(SERVER_INFO *si,
		            const char *svName,
	                const char *svConnectionType,
					const char *svAddress,
					const char *svEncryption,
				    const char *svAuthentication);

	void DeleteServer(void *psi);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CBOWDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BOWDOC_H__720D769F_A7E5_11D2_ADC4_0000F8084273__INCLUDED_)
