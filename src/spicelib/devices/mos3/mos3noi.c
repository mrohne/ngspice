/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1987 Gary W. Ng
Modified: 2000 AlansFixes
**********/

#include "ngspice/ngspice.h"
#include "mos3defs.h"
#include "ngspice/cktdefs.h"
#include "ngspice/iferrmsg.h"
#include "ngspice/noisedef.h"
#include "ngspice/suffix.h"

/*
 * MOS3noise (mode, operation, firstModel, ckt, data, OnDens)
 *    This routine names and evaluates all of the noise sources
 *    associated with MOSFET's.  It starts with the model *firstModel and
 *    traverses all of its insts.  It then proceeds to any other models
 *    on the linked list.  The total output noise density generated by
 *    all of the MOSFET's is summed with the variable "OnDens".
 */


int
MOS3noise (int mode, int operation, GENmodel *genmodel, CKTcircuit *ckt, 
           Ndata *data, double *OnDens)
{
    NOISEAN *job = (NOISEAN *) ckt->CKTcurJob;

    MOS3model *firstModel = (MOS3model *) genmodel;
    MOS3model *model;
    MOS3instance *inst;
    char name[N_MXVLNTH];
    double tempOnoise;
    double tempInoise;
    double noizDens[MOS3NSRCS];
    double lnNdens[MOS3NSRCS];
    int i;

    /* define the names of the noise sources */

    static char *MOS3nNames[MOS3NSRCS] = {       /* Note that we have to keep the order */
	"_rd",              /* noise due to rd */        /* consistent with thestrchr definitions */
	"_rs",              /* noise due to rs */        /* in MOS3defs.h */
	"_id",              /* noise due to id */
	"_1overf",          /* flicker (1/f) noise */
	""                  /* total transistor noise */
    };

    for (model=firstModel; model != NULL; model=model->MOS3nextModel) {
	for (inst=model->MOS3instances; inst != NULL; inst=inst->MOS3nextInstance) {
	    if (inst->MOS3owner != ARCHme) continue;
        
	    switch (operation) {

	    case N_OPEN:

		/* see if we have to to produce a summary report */
		/* if so, name all the noise generators */

		if (job->NStpsSm != 0) {
		    switch (mode) {

		    case N_DENS:
			for (i=0; i < MOS3NSRCS; i++) {
			    (void)sprintf(name,"onoise_%s%s",inst->MOS3name,MOS3nNames[i]);


data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
if (!data->namelist) return(E_NOMEM);
		SPfrontEnd->IFnewUid (ckt,
			&(data->namelist[data->numPlots++]),
			NULL, name, UID_OTHER, NULL);
				/* we've added one more plot */


			}
			break;

		    case INT_NOIZ:
			for (i=0; i < MOS3NSRCS; i++) {
			    (void)sprintf(name,"onoise_total_%s%s",inst->MOS3name,MOS3nNames[i]);


data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
if (!data->namelist) return(E_NOMEM);
		SPfrontEnd->IFnewUid (ckt,
			&(data->namelist[data->numPlots++]),
			NULL, name, UID_OTHER, NULL);
				/* we've added one more plot */


			    (void)sprintf(name,"inoise_total_%s%s",inst->MOS3name,MOS3nNames[i]);


data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
if (!data->namelist) return(E_NOMEM);
		SPfrontEnd->IFnewUid (ckt,
			&(data->namelist[data->numPlots++]),
			NULL, name, UID_OTHER, NULL);
				/* we've added one more plot */



			}
			break;
		    }
		}
		break;

	    case N_CALC:
		switch (mode) {

		case N_DENS:
		    NevalSrc(&noizDens[MOS3RDNOIZ],&lnNdens[MOS3RDNOIZ],
				 ckt,THERMNOISE,inst->MOS3dNodePrime,inst->MOS3dNode,
				 inst->MOS3drainConductance);

		    NevalSrc(&noizDens[MOS3RSNOIZ],&lnNdens[MOS3RSNOIZ],
				 ckt,THERMNOISE,inst->MOS3sNodePrime,inst->MOS3sNode,
				 inst->MOS3sourceConductance);

		    NevalSrc(&noizDens[MOS3IDNOIZ],&lnNdens[MOS3IDNOIZ],
				 ckt,THERMNOISE,inst->MOS3dNodePrime,inst->MOS3sNodePrime,
                                 (2.0/3.0 * fabs(inst->MOS3gm)));

		    NevalSrc(&noizDens[MOS3FLNOIZ], NULL, ckt,
				 N_GAIN,inst->MOS3dNodePrime, inst->MOS3sNodePrime,
				 (double)0.0);
		    noizDens[MOS3FLNOIZ] *= model->MOS3fNcoef * 
				 exp(model->MOS3fNexp *
				 log(MAX(fabs(inst->MOS3cd),N_MINLOG))) /
				 (data->freq *
				 (inst->MOS3w - 2*model->MOS3widthNarrow) *
				 (inst->MOS3l - 2*model->MOS3latDiff) *
				 model->MOS3oxideCapFactor * model->MOS3oxideCapFactor);
		    lnNdens[MOS3FLNOIZ] = 
				 log(MAX(noizDens[MOS3FLNOIZ],N_MINLOG));

		    noizDens[MOS3TOTNOIZ] = noizDens[MOS3RDNOIZ] +
						     noizDens[MOS3RSNOIZ] +
						     noizDens[MOS3IDNOIZ] +
						     noizDens[MOS3FLNOIZ];
		    lnNdens[MOS3TOTNOIZ] = 
				 log(MAX(noizDens[MOS3TOTNOIZ], N_MINLOG));

		    *OnDens += noizDens[MOS3TOTNOIZ];

		    if (data->delFreq == 0.0) { 

			/* if we haven't done any previous integration, we need to */
			/* initialize our "history" variables                      */

			for (i=0; i < MOS3NSRCS; i++) {
			    inst->MOS3nVar[LNLSTDENS][i] = lnNdens[i];
			}

			/* clear out our integration variables if it's the first pass */

			if (data->freq == job->NstartFreq) {
			    for (i=0; i < MOS3NSRCS; i++) {
				inst->MOS3nVar[OUTNOIZ][i] = 0.0;
				inst->MOS3nVar[INNOIZ][i] = 0.0;
			    }
			}
		    } else {   /* data->delFreq != 0.0 (we have to integrate) */
			for (i=0; i < MOS3NSRCS; i++) {
			    if (i != MOS3TOTNOIZ) {
				tempOnoise = Nintegrate(noizDens[i], lnNdens[i],
				      inst->MOS3nVar[LNLSTDENS][i], data);
				tempInoise = Nintegrate(noizDens[i] * data->GainSqInv ,
				      lnNdens[i] + data->lnGainInv,
				      inst->MOS3nVar[LNLSTDENS][i] + data->lnGainInv,
				      data);
				inst->MOS3nVar[LNLSTDENS][i] = lnNdens[i];
				data->outNoiz += tempOnoise;
				data->inNoise += tempInoise;
				if (job->NStpsSm != 0) {
				    inst->MOS3nVar[OUTNOIZ][i] += tempOnoise;
				    inst->MOS3nVar[OUTNOIZ][MOS3TOTNOIZ] += tempOnoise;
				    inst->MOS3nVar[INNOIZ][i] += tempInoise;
				    inst->MOS3nVar[INNOIZ][MOS3TOTNOIZ] += tempInoise;
                                }
			    }
			}
		    }
		    if (data->prtSummary) {
			for (i=0; i < MOS3NSRCS; i++) {     /* print a summary report */
			    data->outpVector[data->outNumber++] = noizDens[i];
			}
		    }
		    break;

		case INT_NOIZ:        /* already calculated, just output */
		    if (job->NStpsSm != 0) {
			for (i=0; i < MOS3NSRCS; i++) {
			    data->outpVector[data->outNumber++] = inst->MOS3nVar[OUTNOIZ][i];
			    data->outpVector[data->outNumber++] = inst->MOS3nVar[INNOIZ][i];
			}
		    }    /* if */
		    break;
		}    /* switch (mode) */
		break;

	    case N_CLOSE:
		return (OK);         /* do nothing, the main calling routine will close */
		break;               /* the plots */
	    }    /* switch (operation) */
	}    /* for inst */
    }    /* for model */

return(OK);
}
