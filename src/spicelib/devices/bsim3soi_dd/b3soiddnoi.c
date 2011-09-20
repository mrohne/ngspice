/**********
Copyright 1999 Regents of the University of California.  All rights reserved.
Author: Weidong Liu and Pin Su         Feb 1999
Author: 1998 Samuel Fung, Dennis Sinitsky and Stephen Tang
File: b3soiddnoi.c          98/5/01
Modofied by Paolo Nenzi 2002
**********/

/*
 * Revision 2.1  99/9/27 Pin Su 
 * BSIMDD2.1 release
 */

#include <ngspice/ngspice.h>
#include "b3soidddef.h"
#include <ngspice/cktdefs.h>
#include <ngspice/iferrmsg.h>
#include <ngspice/noisedef.h>
#include <ngspice/suffix.h>
#include <ngspice/const.h>  /* jwan */

/*
 * B3SOIDDnoise (mode, operation, firstModel, ckt, data, OnDens)
 *    This routine names and evaluates all of the noise sources
 *    associated with MOSFET's.  It starts with the model *firstModel and
 *    traverses all of its insts.  It then proceeds to any other models
 *    on the linked list.  The total output noise density generated by
 *    all of the MOSFET's is summed with the variable "OnDens".
 */

/*
 Channel thermal and flicker noises are calculated based on the value
 of model->B3SOIDDnoiMod.
 If model->B3SOIDDnoiMod = 1,
    Channel thermal noise = SPICE2 model
    Flicker noise         = SPICE2 model
 If model->B3SOIDDnoiMod = 2,
    Channel thermal noise = B3SOIDD model
    Flicker noise         = B3SOIDD model
 If model->B3SOIDDnoiMod = 3,
    Channel thermal noise = SPICE2 model
    Flicker noise         = B3SOIDD model
 If model->B3SOIDDnoiMod = 4,
    Channel thermal noise = B3SOIDD model
    Flicker noise         = SPICE2 model
 */


static double
B3SOIDDStrongInversionNoiseEval(double vgs, double vds, B3SOIDDmodel *model, 
                                B3SOIDDinstance *here, double freq, 
				double temp)
{
struct b3soiddSizeDependParam *pParam;
double cd, esat, DelClm, EffFreq, N0, Nl, Vgst;
double T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, Ssi;

    pParam = here->pParam;
    cd = fabs(here->B3SOIDDcd) * here->B3SOIDDm;
    if (vds > here->B3SOIDDvdsat)
    {   esat = 2.0 * pParam->B3SOIDDvsattemp / here->B3SOIDDueff;
	T0 = ((((vds - here->B3SOIDDvdsat) / pParam->B3SOIDDlitl) + model->B3SOIDDem)
	   / esat);
        DelClm = pParam->B3SOIDDlitl * log (MAX(T0, N_MINLOG));
    }
    else 
        DelClm = 0.0;
    EffFreq = pow(freq, model->B3SOIDDef);
    T1 = CHARGE * CHARGE * 8.62e-5 * cd * temp * here->B3SOIDDueff;
    T2 = 1.0e8 * EffFreq * model->B3SOIDDcox
       * pParam->B3SOIDDleff * pParam->B3SOIDDleff;
    Vgst = vgs - here->B3SOIDDvon;
    N0 = model->B3SOIDDcox * Vgst / CHARGE;
    if (N0 < 0.0)
	N0 = 0.0;
    Nl = model->B3SOIDDcox * (Vgst - MIN(vds, here->B3SOIDDvdsat)) / CHARGE;
    if (Nl < 0.0)
	Nl = 0.0;

    T3 = model->B3SOIDDoxideTrapDensityA
       * log(MAX(((N0 + 2.0e14) / (Nl + 2.0e14)), N_MINLOG));
    T4 = model->B3SOIDDoxideTrapDensityB * (N0 - Nl);
    T5 = model->B3SOIDDoxideTrapDensityC * 0.5 * (N0 * N0 - Nl * Nl);

    T6 = 8.62e-5 * temp * cd * cd;
    T7 = 1.0e8 * EffFreq * pParam->B3SOIDDleff
       * pParam->B3SOIDDleff * pParam->B3SOIDDweff * here->B3SOIDDm;
    T8 = model->B3SOIDDoxideTrapDensityA + model->B3SOIDDoxideTrapDensityB * Nl
       + model->B3SOIDDoxideTrapDensityC * Nl * Nl;
    T9 = (Nl + 2.0e14) * (Nl + 2.0e14);

    Ssi = T1 / T2 * (T3 + T4 + T5) + T6 / T7 * DelClm * T8 / T9;

    return Ssi;
}

int
B3SOIDDnoise (int mode, int operation, GENmodel *inModel, CKTcircuit *ckt, 
              Ndata *data, double *OnDens)
{
B3SOIDDmodel *model = (B3SOIDDmodel *)inModel;
B3SOIDDinstance *here;
struct b3soiddSizeDependParam *pParam;
char name[N_MXVLNTH];
double tempOnoise;
double tempInoise;
double noizDens[B3SOIDDNSRCS];
double lnNdens[B3SOIDDNSRCS];

double vgs, vds, Slimit;
double T1, T10, T11;
double Ssi, Swi;

int i;

    /* define the names of the noise sources */
    static char *B3SOIDDnNames[B3SOIDDNSRCS] =
    {   /* Note that we have to keep the order */
	".rd",              /* noise due to rd */
			    /* consistent with the index definitions */
	".rs",              /* noise due to rs */
			    /* in B3SOIDDdefs.h */
	".id",              /* noise due to id */
	".1overf",          /* flicker (1/f) noise */
        ".fb", 		    /* noise due to floating body */
	""                  /* total transistor noise */
    };

    for (; model != NULL; model = model->B3SOIDDnextModel)
    {    for (here = model->B3SOIDDinstances; here != NULL;
	      here = here->B3SOIDDnextInstance)
	 {    
	 
	      if (here->B3SOIDDowner != ARCHme)
	              continue;

	      pParam = here->pParam;
	      switch (operation)
	      {  case N_OPEN:
		     /* see if we have to to produce a summary report */
		     /* if so, name all the noise generators */

		      if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0)
		      {   switch (mode)
			  {  case N_DENS:
			          for (i = 0; i < B3SOIDDNSRCS; i++)
				  {    (void) sprintf(name, "onoise.%s%s",
					              here->B3SOIDDname,
						      B3SOIDDnNames[i]);
                                       data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
                                       if (!data->namelist)
					   return(E_NOMEM);
		                       SPfrontEnd->IFnewUid (ckt,
			                  &(data->namelist[data->numPlots++]),
			                  NULL, name, UID_OTHER,
					  NULL);
				       /* we've added one more plot */
			          }
			          break;
		             case INT_NOIZ:
			          for (i = 0; i < B3SOIDDNSRCS; i++)
				  {    (void) sprintf(name, "onoise_total.%s%s",
						      here->B3SOIDDname,
						      B3SOIDDnNames[i]);
                                       data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
                                       if (!data->namelist)
					   return(E_NOMEM);
		                       SPfrontEnd->IFnewUid (ckt,
			                  &(data->namelist[data->numPlots++]),
			                  NULL, name, UID_OTHER,
					  NULL);
				       /* we've added one more plot */

			               (void) sprintf(name, "inoise_total.%s%s",
						      here->B3SOIDDname,
						      B3SOIDDnNames[i]);
                                       data->namelist = TREALLOC(IFuid, data->namelist, data->numPlots + 1);
                                       if (!data->namelist)
					   return(E_NOMEM);
		                       SPfrontEnd->IFnewUid (ckt,
			                  &(data->namelist[data->numPlots++]),
			                  NULL, name, UID_OTHER,
					  NULL);
				       /* we've added one more plot */
			          }
			          break;
		          }
		      }
		      break;
	         case N_CALC:
		      switch (mode)
		      {  case N_DENS:
		              NevalSrc(&noizDens[B3SOIDDRDNOIZ],
				       &lnNdens[B3SOIDDRDNOIZ], ckt, THERMNOISE,
				       here->B3SOIDDdNodePrime, here->B3SOIDDdNode,
				       here->B3SOIDDdrainConductance * here->B3SOIDDm);

		              NevalSrc(&noizDens[B3SOIDDRSNOIZ],
				       &lnNdens[B3SOIDDRSNOIZ], ckt, THERMNOISE,
				       here->B3SOIDDsNodePrime, here->B3SOIDDsNode,
				       here->B3SOIDDsourceConductance * here->B3SOIDDm);

                              switch( model->B3SOIDDnoiMod )
			      {  case 1:
			         case 3:
			              NevalSrc(&noizDens[B3SOIDDIDNOIZ],
				               &lnNdens[B3SOIDDIDNOIZ], ckt, 
					       THERMNOISE, here->B3SOIDDdNodePrime,
				               here->B3SOIDDsNodePrime,
                                               (2.0 / 3.0 * fabs(here->B3SOIDDm * (here->B3SOIDDgm
				               + here->B3SOIDDgds
					       + here->B3SOIDDgmbs))));
				      break;
			         case 2:
			         case 4:
		                      NevalSrc(&noizDens[B3SOIDDIDNOIZ],
				               &lnNdens[B3SOIDDIDNOIZ], ckt,
					       THERMNOISE, here->B3SOIDDdNodePrime,
                                               here->B3SOIDDsNodePrime,
					       (here->B3SOIDDueff
					       * fabs((here->B3SOIDDqinv * here->B3SOIDDm)
					       / (pParam->B3SOIDDleff
					       * pParam->B3SOIDDleff))));
				      break;
			      }
		              NevalSrc(&noizDens[B3SOIDDFLNOIZ], NULL,
				       ckt, N_GAIN, here->B3SOIDDdNodePrime,
				       here->B3SOIDDsNodePrime, (double) 0.0);

                              switch( model->B3SOIDDnoiMod )
			      {  case 1:
			         case 4:
			              noizDens[B3SOIDDFLNOIZ] *= model->B3SOIDDkf
					    * exp(model->B3SOIDDaf
					    * log(MAX(fabs(here->B3SOIDDcd * here->B3SOIDDm),
					    N_MINLOG)))
					    / (pow(data->freq, model->B3SOIDDef)
					    * pParam->B3SOIDDleff
				            * pParam->B3SOIDDleff
					    * model->B3SOIDDcox);
				      break;
			         case 2:
			         case 3:
			              vgs = *(ckt->CKTstates[0] + here->B3SOIDDvgs);
		                      vds = *(ckt->CKTstates[0] + here->B3SOIDDvds);
			              if (vds < 0.0)
			              {   vds = -vds;
				          vgs = vgs + vds;
			              }
                                      if (vgs >= here->B3SOIDDvon + 0.1)
			              {   Ssi = B3SOIDDStrongInversionNoiseEval(vgs,
					      vds, model, here, data->freq,
					      ckt->CKTtemp);
                                          noizDens[B3SOIDDFLNOIZ] *= Ssi;
			              }
                                      else 
			              {   pParam = here->pParam;
				          T10 = model->B3SOIDDoxideTrapDensityA
					      * 8.62e-5 * ckt->CKTtemp;
		                          T11 = pParam->B3SOIDDweff * here->B3SOIDDm
					      * pParam->B3SOIDDleff
				              * pow(data->freq, model->B3SOIDDef)
				              * 4.0e36;
		                          Swi = T10 / T11 * here->B3SOIDDcd * here->B3SOIDDm
				              * here->B3SOIDDcd * here->B3SOIDDm;
                                          Slimit = B3SOIDDStrongInversionNoiseEval(
				               here->B3SOIDDvon + 0.1, vds, model,
					       here, data->freq, ckt->CKTtemp);
				          T1 = Swi + Slimit;
				          if (T1 > 0.0)
                                              noizDens[B3SOIDDFLNOIZ] *= (Slimit
								    * Swi) / T1; 
				          else
                                              noizDens[B3SOIDDFLNOIZ] *= 0.0;
			              }
				      break;
			      }

		              lnNdens[B3SOIDDFLNOIZ] =
				     log(MAX(noizDens[B3SOIDDFLNOIZ], N_MINLOG));

			      /* Low frequency excess noise due to FBE */
		              NevalSrc(&noizDens[B3SOIDDFBNOIZ], &lnNdens[B3SOIDDFBNOIZ],
				          ckt, SHOTNOISE, here->B3SOIDDsNodePrime,
				          here->B3SOIDDbNode, 
                                          2.0 * model->B3SOIDDnoif * here->B3SOIDDibs *
					  here->B3SOIDDm);

		              noizDens[B3SOIDDTOTNOIZ] = noizDens[B3SOIDDRDNOIZ]
						     + noizDens[B3SOIDDRSNOIZ]
						     + noizDens[B3SOIDDIDNOIZ]
						     + noizDens[B3SOIDDFLNOIZ]
						     + noizDens[B3SOIDDFBNOIZ];
		              lnNdens[B3SOIDDTOTNOIZ] = 
				     log(MAX(noizDens[B3SOIDDTOTNOIZ], N_MINLOG));

		              *OnDens += noizDens[B3SOIDDTOTNOIZ];

		              if (data->delFreq == 0.0)
			      {   /* if we haven't done any previous 
				     integration, we need to initialize our
				     "history" variables.
				    */

			          for (i = 0; i < B3SOIDDNSRCS; i++)
				  {    here->B3SOIDDnVar[LNLSTDENS][i] =
					     lnNdens[i];
			          }

			          /* clear out our integration variables
				     if it's the first pass
				   */
			          if (data->freq ==
				      ((NOISEAN*) ckt->CKTcurJob)->NstartFreq)
				  {   for (i = 0; i < B3SOIDDNSRCS; i++)
				      {    here->B3SOIDDnVar[OUTNOIZ][i] = 0.0;
				           here->B3SOIDDnVar[INNOIZ][i] = 0.0;
			              }
			          }
		              }
			      else
			      {   /* data->delFreq != 0.0,
				     we have to integrate.
				   */
			          for (i = 0; i < B3SOIDDNSRCS; i++)
				  {    if (i != B3SOIDDTOTNOIZ)
				       {   tempOnoise = Nintegrate(noizDens[i],
						lnNdens[i],
				                here->B3SOIDDnVar[LNLSTDENS][i],
						data);
				           tempInoise = Nintegrate(noizDens[i]
						* data->GainSqInv, lnNdens[i]
						+ data->lnGainInv,
				                here->B3SOIDDnVar[LNLSTDENS][i]
						+ data->lnGainInv, data);
				           here->B3SOIDDnVar[LNLSTDENS][i] =
						lnNdens[i];
				           data->outNoiz += tempOnoise;
				           data->inNoise += tempInoise;
				           if (((NOISEAN*)
					       ckt->CKTcurJob)->NStpsSm != 0)
					   {   here->B3SOIDDnVar[OUTNOIZ][i]
						     += tempOnoise;
				               here->B3SOIDDnVar[OUTNOIZ][B3SOIDDTOTNOIZ]
						     += tempOnoise;
				               here->B3SOIDDnVar[INNOIZ][i]
						     += tempInoise;
				               here->B3SOIDDnVar[INNOIZ][B3SOIDDTOTNOIZ]
						     += tempInoise;
                                           }
			               }
			          }
		              }
		              if (data->prtSummary)
			      {   for (i = 0; i < B3SOIDDNSRCS; i++)
				  {    /* print a summary report */
			               data->outpVector[data->outNumber++]
					     = noizDens[i];
			          }
		              }
		              break;
		         case INT_NOIZ:
			      /* already calculated, just output */
		              if (((NOISEAN*)ckt->CKTcurJob)->NStpsSm != 0)
			      {   for (i = 0; i < B3SOIDDNSRCS; i++)
				  {    data->outpVector[data->outNumber++]
					     = here->B3SOIDDnVar[OUTNOIZ][i];
			               data->outpVector[data->outNumber++]
					     = here->B3SOIDDnVar[INNOIZ][i];
			          }
		              }
		              break;
		      }
		      break;
	         case N_CLOSE:
		      /* do nothing, the main calling routine will close */
		      return (OK);
		      break;   /* the plots */
	      }       /* switch (operation) */
	 }    /* for here */
    }    /* for model */

    return(OK);
}



