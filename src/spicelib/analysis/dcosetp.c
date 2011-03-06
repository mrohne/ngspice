/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "ngspice.h"
#include "ifsim.h"
#include "iferrmsg.h"
#include "cktdefs.h"
#include "opdefs.h"


#include "analysis.h"

/* ARGSUSED */
int 
DCOsetParm(CKTcircuit *ckt, JOB *anal, int which, IFvalue *value)
{
    NG_IGNORE(ckt);
    NG_IGNORE(anal);
    NG_IGNORE(value);

    switch(which) {

    default:
	break;
    }
    return(E_BADPARM);
}


SPICEanalysis DCOinfo  = {
    { 
        "OP",
        "D.C. Operating point analysis",

        0,
        NULL,
    },
    sizeof(OP),
    NODOMAIN,
    1,
    DCOsetParm,
    DCOaskQuest,
    NULL,
    DCop
};
