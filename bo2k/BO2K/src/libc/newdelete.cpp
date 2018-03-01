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

// Modified New/Delete Operators

#include<windows.h>

void * __cdecl operator new( unsigned int cb )
{
	// No fail new!
	void *pMem;
	do {
		pMem=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,cb);
		if(pMem==NULL) Sleep(2000);
	} while(pMem==NULL);

	return pMem;
}

void __cdecl operator delete( void * p )
{
	HeapFree(GetProcessHeap(),0,p);
}
