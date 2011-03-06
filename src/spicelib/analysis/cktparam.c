/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

    /* CKTparam
     *  attach the given parameter to the specified device in the given circuit
     */

#include "ngspice.h"
#include "cktdefs.h"
#include "ifsim.h"
#include "devdefs.h"
#include "sperror.h"


/* ARGSUSED */
int
CKTparam(CKTcircuit *ckt, GENinstance *fast, int param, IFvalue *val, IFvalue *selector)
{
    int type;
    GENinstance *myfast = /*fixme*/ fast;

    NG_IGNORE(ckt);

    type = myfast->GENmodPtr->GENmodType;
    if(((*DEVices[type]).DEVparam)) {
        return(((*((*DEVices[type]).DEVparam)) (param,val,myfast,selector)));
    } else {
        return(E_BADPARM);
    }
}
