/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "ngspice.h"
#include "tskdefs.h"
#include "jobdefs.h"
#include "ifsim.h"
#include "iferrmsg.h"
#include "ckt.h"


/* ARGSUSED */
int
CKTdelTask(CKTcircuit *ckt, TSKtask *task)
{
    JOB *job;
    JOB *old=NULL;

    NG_IGNORE(ckt);

    for(job = task->jobs; job; job=job->JOBnextJob){
        if(old) FREE(old);
        old=job;
    }
    if(old)FREE(old);
    FREE(task);
    return(OK);
}
