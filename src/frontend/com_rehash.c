/*************
* com_rehash.c
* $Id: com_rehash.c,v 1.7 2011/08/20 17:27:10 rlar Exp $
************/

#include <config.h>
#include <ngspice/ngspice.h>

#include <ngspice/wordlist.h>
#include "com_rehash.h"
#include <ngspice/cpextern.h>
#include "control.h"
#include "parser/unixcom.h"

void
com_rehash(wordlist *wl)
{
    char *s;

    NG_IGNORE(wl);

    if (!cp_dounixcom) {
        fprintf(cp_err, "Error: unixcom not set.\n");
        return;
    }
    s = getenv("PATH");
    if (s)
        cp_rehash(s, TRUE);
    else
        fprintf(cp_err, "Error: no PATH in environment.\n");
    return;
}
