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

#include<windows.h>

char *BreakString(char *svLines, char *svTok)
{
	char *p,*s;
	char *tok;
	p=svLines;
	if(p==NULL) return NULL;

	while(*p!='\0') {
		for(s=p,tok=svTok;(*s)&&(*tok);s++,tok++) {
			if((*s)!=(*tok)) break;
		}
		if((*tok)=='\0') {
			*p='\0';
			return p+lstrlen(svTok);
		}
		p++;
	}
	return NULL;
}

// What escapestring returns must be FREEd.
char *EscapeString(char *svStr)
{
	// Determine final size
	int nEscapes=0,i,count;
	count=lstrlen(svStr);
	for(i=0;i<count;i++) {
		if(svStr[i]=='\a') nEscapes++;
		else if(svStr[i]=='\b') nEscapes++;
		else if(svStr[i]=='\f') nEscapes++;
		else if(svStr[i]=='\n') nEscapes++;
		else if(svStr[i]=='\r') nEscapes++;
		else if(svStr[i]=='\t') nEscapes++;
		else if(svStr[i]=='\v') nEscapes++;
		else if(svStr[i]=='\\') nEscapes++;
		else if(svStr[i]<' ' || svStr[i]>'~') nEscapes+=3;
	}
	
	// Allocate output buffer
	char *svOutBuf=(char*)malloc(lstrlen(svStr)+nEscapes+1);

	// Escape things
	int j=0;
	for(i=0;i<count;i++) {
		char c;
		c=svStr[i];
		if(c>=' ' && c<='~') { svOutBuf[j]=c; j++; }
		else if(c=='\a') { svOutBuf[j]='\\'; svOutBuf[j+1]='a'; j+=2; }
		else if(c=='\b') { svOutBuf[j]='\\'; svOutBuf[j+1]='b'; j+=2; }
		else if(c=='\f') { svOutBuf[j]='\\'; svOutBuf[j+1]='f'; j+=2; }
		else if(c=='\n') { svOutBuf[j]='\\'; svOutBuf[j+1]='n'; j+=2; }
		else if(c=='\r') { svOutBuf[j]='\\'; svOutBuf[j+1]='r'; j+=2; }
		else if(c=='\t') { svOutBuf[j]='\\'; svOutBuf[j+1]='t'; j+=2; }
		else if(c=='\v') { svOutBuf[j]='\\'; svOutBuf[j+1]='v'; j+=2; }
		else if(c=='\\') { svOutBuf[j]='\\'; svOutBuf[j+1]='\\'; j+=2; }
		else {
			wsprintf(svOutBuf+j,"\\x%1.1X%1.1X",(c>>4),c&15);
			j+=4;
		}
	}
	svOutBuf[j]='\0';

	return svOutBuf;
}


char *UnescapeString(char *svStr)
{
	int len=lstrlen(svStr);
    char *svTemp;

	// Count number of '%' characters
	int nCount;
	nCount=0;
	svTemp=svStr;
	while(*svTemp) {
		if(*svTemp=='%') nCount++;
		svTemp++;
	}

	// Allocate memory
	svTemp=(char *)malloc(lstrlen(svStr)+1+nCount);
	if(svTemp==NULL) return NULL;
	memset(svTemp,0,lstrlen(svStr)+1+nCount);
	
	// Convert string to preserve '%' chars
	char *svCvtTo, *svCvtFrom;
	svCvtFrom=svStr;
	svCvtTo=svTemp;
	while(*svCvtFrom) {
		if(*svCvtFrom=='%') {
			*(svCvtTo++)='%';
		}
		*(svCvtTo++)=*(svCvtFrom++);
	}

	// Convert escape chars (funky kludge, eh?)
	wsprintf(svStr,svTemp);
	free(svTemp);
	
	return svStr;
}