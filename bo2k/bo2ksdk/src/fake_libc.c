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

// Fake LIBC Functions

#include<windows.h>

void * __cdecl malloc(size_t size)
{
	// No fail malloc!
	void *pMem;
	do {
		pMem=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,size);
		if(pMem==NULL) Sleep(2000);
	} while(pMem==NULL);

	return pMem;
}

void __cdecl free(void *ptr)
{
	HeapFree(GetProcessHeap(),0,ptr);
}

void * __cdecl memcpy(void *dst, const void *src, size_t count)
{
	void * ret = dst;
	
	while (count--) {
		*(char *)dst = *(char *)src;
		dst = (char *)dst + 1;
		src = (char *)src + 1;
	}
	
	return(ret);
}

void * __cdecl memmove(void * dst, const void * src, size_t count)
{
	void * ret = dst;
	
	if (dst <= src || (char *)dst >= ((char *)src + count)) {
		while (count--) {
			*(char *)dst = *(char *)src;
			dst = (char *)dst + 1;
			src = (char *)src + 1;
		}
	}
	else {
		dst = (char *)dst + count - 1;
		src = (char *)src + count - 1;
		
		while (count--) {
			*(char *)dst = *(char *)src;
			dst = (char *)dst - 1;
			src = (char *)src - 1;
		}
	}
	
	return(ret);
}


void * __cdecl memset(void *dst, int val, size_t count)
{
	void * ret = dst;
	while(count--) {
		*(char *)dst = (char)val;
		dst = (char *)dst +1;
	}

	return(ret);
}


int __cdecl memcmp(const void * buf1, const void * buf2, size_t count)
{
	if(!count) return(0);
	
	while (--count && *(char *)buf1 == *(char *)buf2) {
		buf1 = (char *)buf1 + 1;
		buf2 = (char *)buf2 + 1;
	}
	
	return( *((unsigned char *)buf1) - *((unsigned char *)buf2) );
}


int __cdecl _isspace(x)
{
	return (x==' ');
}
int __cdecl _isdigit(x)
{
	return ((x>='0') && (x<='9'));
}


int __cdecl atoi(const char *nptr)
{
	return (int)atol(nptr);
}

long __cdecl atol(const char *nptr)
{
	int c;              /* current char */
	long total;         /* current total */
	int sign;           /* if '-', then negative, otherwise positive */
	
	/* skip whitespace */
	while ( _isspace((int)(unsigned char)*nptr) )
		++nptr;
	
	c = (int)(unsigned char)*nptr++;
	sign = c;           /* save sign indication */
	if (c == '-' || c == '+')
		c = (int)(unsigned char)*nptr++;    /* skip sign */
	
	total = 0;
	
	while (_isdigit(c)) {
		total = 10 * total + (c - '0');     /* accumulate digit */
		c = (int)(unsigned char)*nptr++;    /* get next char */
	}
	
	if (sign == '-')
		return -total;
	else
		return total;   /* return result, negated if necessary */
}

char * __cdecl strrchr(const char * str, int ch)
{
	char *start = (char *)str;
	
	while (*str++);
	while (--str!=start && *str!=(char)ch);
	if(*str==(char)ch) return((char *)str);
	
	return(NULL);
}

int __cdecl strncmp(const char *first, const char *last, size_t count)
{
	if (!count) return(0);
	
	while (--count && *first && *first == *last) {
		first++;
		last++;
	}
	
	return(*(unsigned char *)first - *(unsigned char *)last);
}

int __cdecl strnicmp( const char * first, const char * last, size_t count)
{
	int f,l;

	if(count) {
		do {
			if(((f=(unsigned char)(*(first++))) >= 'A') && (f <= 'Z') )
				f -= 'A' - 'a';
			
			if (((l=(unsigned char)(*(last++))) >= 'A') && (l <= 'Z') )
				l -= 'A' - 'a';			
		} while (--count && f && (f == l));
	
		return( f - l );
	}
	
	return(0);
}
