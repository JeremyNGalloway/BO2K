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

#include <windows.h> 
#include <winperf.h> 
#include <perfdata.h> 
 
 
// FirstCounter() - Find the first counter in pObject. 
//                  Returns a pointer to the first counter.  If pObject is NULL 
//                  then NULL is returned. 

PPERF_COUNTER FirstCounter (PPERF_OBJECT pObject) 
{ 
    if (pObject) 
        return (PPERF_COUNTER)((PCHAR) pObject + pObject->HeaderLength); 
    else 
        return NULL; 
} 
 
 
 
 
// NextCounter() - Find the next counter of pCounter. 
//                 If pCounter is the last counter of an object type, bogus data 
//                 maybe returned.  The caller should do the checking. 
//                 Returns a pointer to a counter.  If pCounter is NULL then 
//                 NULL is returned. 

PPERF_COUNTER NextCounter (PPERF_COUNTER pCounter) 
{ 
    if (pCounter) 
        return (PPERF_COUNTER)((PCHAR) pCounter + pCounter->ByteLength); 
    else 
        return NULL; 
} 
 
 
 
 
// FindCounter() - Find a counter specified by TitleIndex. 
//                 Returns a pointer to the counter.  If counter is not found 
//                 then NULL is returned. 

PPERF_COUNTER FindCounter (PPERF_OBJECT pObject, DWORD TitleIndex) 
{ 
	PPERF_COUNTER pCounter; 
	DWORD         i = 0; 
	
    if (pCounter = FirstCounter (pObject)) 
        while (i < pObject->NumCounters) 
		{ 
            if (pCounter->CounterNameTitleIndex == TitleIndex) 
                return pCounter; 
			
            pCounter = NextCounter (pCounter); 
            i++; 
		} 
		
		return NULL; 
		
} 
 
 
 
 
//  CounterData() - Returns counter data for an object instance.  
//                  If pInst or pCount is NULL then NULL is returned.

PVOID CounterData (PPERF_INSTANCE pInst, PPERF_COUNTER pCount) 
{ 
	PPERF_COUNTER_BLOCK pCounterBlock; 
	
    if (pCount && pInst) 
	{ 
        pCounterBlock = (PPERF_COUNTER_BLOCK)((PCHAR)pInst + pInst->ByteLength); 
        return (PVOID)((PCHAR)pCounterBlock + pCount->CounterOffset); 
	} 
    else 
        return NULL; 
} 

 

