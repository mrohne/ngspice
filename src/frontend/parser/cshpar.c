/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
$Id: cshpar.c,v 1.4 2011/08/20 17:27:11 rlar Exp $
**********/

/*
 * The main entry point for cshpar.
 */


#include <ngspice/ngspice.h>
#include <ngspice/cpdefs.h>
#include <signal.h>
#include "cshpar.h"

#ifdef HAVE_SGTTY_H
#include <sgtty.h>
#else
#ifdef HAVE_TERMIO_H
#include <termio.h>
#else
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#endif
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_PWD_H
#include <pwd.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif


/* Things go as follows:
 * (1) Read the line and do some initial quoting (by setting the 8th bit),
 *  and command ignoring. Also deal with command completion.
 * (2) Do history substitutions. (!, ^)
 * (3) Do alias substitution.
 * 
 * In front.c these things get done:
 * (4) Do variable substitution. ($varname)
 * (5) Do backquote substitution. (``)
 * (6) Do globbing. (*, ?, [], {}, ~)
 * (7) Do io redirection.
 */

/* static functions */
static void pwlist(wordlist *wlist, char *name);


wordlist *
cp_parse(char *string)
{
    wordlist *wlist;

    wlist = cp_lexer(string);

    if (!string)
        cp_event++;

    if (!wlist || !wlist->wl_word)
        return (wlist);

    pwlist(wlist, "Initial parse");

    wlist = cp_histsubst(wlist);
    if (!wlist || !wlist->wl_word)
        return (wlist);
    pwlist(wlist, "After history substitution");
    if (cp_didhsubst) {
        wl_print(wlist, stdout);
        putc('\n', stdout);
    }

    /* Add the word list to the history. */
    /* MW. If string==NULL we do not have to do this, and then play
     * with cp_lastone is not needed, but watch out cp_doalias */
    if ((*wlist->wl_word) && !(string))
        cp_addhistent(cp_event - 1, wlist);

    wlist = cp_doalias(wlist);
    pwlist(wlist, "After alias substitution");
    pwlist(wlist, "Returning ");
    return (wlist);
}

static void
pwlist(wordlist *wlist, char *name)
{
    wordlist *wl;

    if (!cp_debug)
        return;
    fprintf(cp_err, "%s : [ ", name);
    for (wl = wlist; wl; wl = wl->wl_next)
        fprintf(cp_err, "%s ", wl->wl_word);
    fprintf(cp_err, "]\n");
    return;
}

