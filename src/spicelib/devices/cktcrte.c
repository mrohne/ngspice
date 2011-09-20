/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

/* CKTcrtElement(ckt,type,inModPtr,inInstPtr,name,subname)
 *
 * Create a device of the specified type, with the given name, using
 * the specified model in the named circuit.  */

#include <config.h>
#include <ngspice/devdefs.h>
#include <ngspice/sperror.h>

#include "dev.h"
#include <ngspice/memory.h>

int
CKTcrtElt(CKTcircuit *ckt, GENmodel *modPtr, GENinstance **inInstPtr, IFuid name)
{
  GENinstance *instPtr = NULL;             /* instPtr points to the data struct for per-instance data */

    SPICEdev **DEVices;
    int error;
    int type;

    DEVices = devices();

    if(!modPtr)
	return E_NOMOD;

    type = modPtr->GENmodType;

    error = CKTfndDev(ckt, &type, &instPtr, name, modPtr, NULL);

    if (error == OK) { 
        if (inInstPtr)
	    *inInstPtr = instPtr;
        return E_EXISTS;
    } else if (error != E_NODEV)
	return error;

#ifdef TRACE
    /*------  SDB debug statement  -------*/
    printf("In CKTcrtElt, about to tmalloc new model, type = %d. . . \n", type);
#endif

    instPtr = (GENinstance *) tmalloc((size_t) *DEVices[type]->DEVinstSize);
    if (instPtr == NULL)
	return E_NOMEM;

    /* PN: adding instance number for statistical purpose */
    ckt->CKTstat->STATdevNum[type].instNum ++;
    ckt->CKTstat->STATtotalDev ++;

#if 0
    printf("device: %s number %d\n",
           DEVices[type]->DEVpublic.name, ckt->CKTstat->STATdevNum[type].instNum);
#endif

    instPtr->GENname = name;

    instPtr->GENmodPtr = modPtr; 

    instPtr->GENnextInstance = modPtr->GENinstances; 

    modPtr->GENinstances = instPtr;

    if(inInstPtr != NULL)
	*inInstPtr = instPtr;

    return OK;
}
