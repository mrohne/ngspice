
/*
 * Dummy interface stuff, for nutmeg. This is the easiest way of
 * making sure that nutmeg doesn't try to load spice in also.
 */

#include "ngspice.h"
#include "ifsim.h"
#include "sperror.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "fteinp.h"
#include "nutmegif.h"


struct variable * nutif_getparam(CKTcircuit *ckt, char **name, char *param, int ind, int do_model)
{
    NG_IGNORE(do_model);
    NG_IGNORE(ind);
    NG_IGNORE(param);
    NG_IGNORE(name);
    NG_IGNORE(ckt);

    return ((struct variable *) NULL);
}


