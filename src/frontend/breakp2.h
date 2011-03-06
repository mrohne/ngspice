/*************
 * Header file for breakp2.c
 * 1999 E. Rouat
 * $Id: breakp2.h,v 1.3 2010/10/16 15:58:14 rlar Exp $
 ************/

#ifndef BREAKP2_H_INCLUDED
#define BREAKP2_H_INCLUDED


void com_save(wordlist *wl);
void com_save2(wordlist *wl, char *name);
void settrace(wordlist *wl, int what, char *name);

extern struct dbcomm *dbs;
extern int debugnumber;

#endif
