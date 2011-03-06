/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
Author:	1991 David A. Gates, U. C. Berkeley CAD Group
Modified: 2001 Paolo Nenzi
**********/

#include "ngspice.h"
#include "numcards.h"
#include "numgen.h"
#include "numenum.h"
#include "methdefs.h"
#include "sperror.h"
#include "devdefs.h"
#include "suffix.h"


extern int METHnewCard(void**,void*);
extern int METHparam(int,IFvalue*,void*);


IFparm METHpTable[] = {
  IP("devtol",	METH_DABSTOL,	IF_REAL,    "Absolute tolerance on device equations"),
  IP("dabstol",	METH_DABSTOL,	IF_REAL,    "Absolute tolerance on device equations"),
  IP("dreltol",	METH_DRELTOL,	IF_REAL,    "Relative tolerance on device equations"),
  IP("onecarrier",METH_ONEC,	IF_FLAG,    "Solve for majority carriers only"),
  IP("ac.analysis",METH_ACANAL,	IF_STRING,  "AC solution technique"),
  IP("frequency",METH_OMEGA,	IF_REAL,    "AC default frequency"),
  IP("nomobderiv",METH_NOMOBDERIV,IF_FLAG,  "Ignore mobility derivatives"),
  IP("itlim",	METH_ITLIM,	IF_INTEGER, "Iteration limit"),
  IP("voltpred",METH_VOLTPRED,	IF_FLAG,    "Perform DC voltage prediction")
};

IFcardInfo METHinfo = {
  "method",
  "Specify parameters and types of simulation methods",
  NUMELEMS(METHpTable),
  METHpTable,

  METHnewCard,
  METHparam,
  NULL
};

int
METHnewCard(void **inCard, void *inModel)
{
    METHcard *tmpCard, *newCard;
    GENnumModel *model = (GENnumModel *)inModel;

    tmpCard = model->GENmethods;
    if (!tmpCard) { /* First in list */
        newCard = NEW( METHcard );
        if (!newCard) {
            *inCard = (void *)NULL;
            return(E_NOMEM);
        }
        newCard->METHnextCard = (METHcard *)NULL;
        *inCard = (void *)newCard;
        model->GENmethods = newCard;
    } else { /* Only one card of this type allowed */
	*inCard = (void *)tmpCard;
    }
    return(OK);
}

int
METHparam(int param, IFvalue *value, void *inCard)
{
    METHcard *card = (METHcard *)inCard;

    switch (param) {
	case METH_DABSTOL:
	    card->METHdabstol = value->rValue;
	    card->METHdabstolGiven = TRUE;
	    break;
	case METH_DRELTOL:
	    card->METHdreltol = value->rValue;
	    card->METHdreltolGiven = TRUE;
	    break;
	case METH_OMEGA:
	    card->METHomega = 2.0 * M_PI * value->rValue;
	    card->METHomegaGiven = TRUE;
	    break;
	case METH_ONEC:
	    card->METHoneCarrier = value->iValue;
	    card->METHoneCarrierGiven = TRUE;
	    break;
	case METH_NOMOBDERIV:
	    card->METHmobDeriv = !(value->iValue);
	    card->METHmobDerivGiven = TRUE;
	    break;
	case METH_ACANAL:
	    if ( cinprefix( value->sValue, "direct", 1 ) ) {
		card->METHacAnalysisMethod = DIRECT;
	        card->METHacAnalysisMethodGiven = TRUE;
	    } else if ( cinprefix( value->sValue, "sor", 1 ) ) {
		card->METHacAnalysisMethod = SOR;
	        card->METHacAnalysisMethodGiven = TRUE;
	    }
	    break;
	case METH_ITLIM:
	    card->METHitLim = value->iValue;
	    card->METHitLimGiven = TRUE;
	    break;
	case METH_VOLTPRED:
	    card->METHvoltPred = value->iValue;
	    card->METHvoltPredGiven = TRUE;
	    break;
	default:
	    return(E_BADPARM);
	    break;
    }
    return(OK);
}
