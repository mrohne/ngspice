/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Jeffrey M. Hsu
**********/

/*
    $Header: /cvsroot/ngspice/ngspice/ng-spice-rework/src/include/ngspice/ftedbgra.h,v 1.1 2011/08/20 17:43:07 rlar Exp $

    External definitions for the graph database module.
*/

extern GRAPH *currentgraph;

extern GRAPH *NewGraph(void);

extern GRAPH *FindGraph(int id);

extern GRAPH *CopyGraph(GRAPH *graph);

extern void RemoveWindow(GRAPH*);
