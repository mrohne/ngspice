/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#ifndef ngspice_GENDEFS_H
#define ngspice_GENDEFS_H

#include "ngspice/typedefs.h"
#include "ngspice/ifsim.h"



        /* definitions used to describe generic devices */

/* information used to describe a single instance */

struct GENinstance {
    GENmodel *GENmodPtr;    /* backpointer to model */
    GENinstance *GENnextInstance;   /* pointer to next instance of
                                     * current model*/
    IFuid GENname;  /* pointer to character string naming this instance */
    int GENstate;   /* state index number */
    int GENnode[7]; /* node numbers to which this instance is connected to */
                    /* carefull, thats overlayed into the actual device structs */
};


/* per model data */

struct GENmodel {       /* model structure for a resistor */
    int GENmodType;             /* type index of this device type */
    GENmodel *GENnextModel;     /* pointer to next possible model in
                                 * linked list */
    GENinstance *GENinstances;  /* pointer to list of instances that have this
                                 * model */
    IFuid GENmodName;           /* pointer to character string naming this model */
};

#endif
