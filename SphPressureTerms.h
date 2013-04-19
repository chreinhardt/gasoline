/* 
   Macros for function: 
       void SphPressureTermsSym(PARTICLE *p,int nSmooth,NN *nnList,SMF *smf)
   in smoothfcn.c

   This code is used 3 times.  Constrained to use Macros (not functions) because 
   Macro definitions change between uses as follows:
   1) with p and q active
     hash define PACTIVE(xxx) xxx
     hash define QACTIVE(xxx) xxx
   2) with just p active
     hash define PACTIVE(xxx) xxx
     hash define QACTIVE(xxx) 
   3) with just q active 
     hash define PACTIVE(xxx) 
     hash define QACTIVE(xxx) xxx

   All Macros and Variables not defined here are defined in smoothfcn.c
 */

#ifdef DRHODT
#define DRHODTACTIVE(xxx) xxx
#ifdef RTFORCE
#define RHO_DIVV(a,b) (b)
#else
#define RHO_DIVV(a,b) (a)
#endif
#else
#define DRHODTACTIVE(xxx) 
#endif

#ifdef DIFFUSION 
#ifdef MASSDIFF
#define MASSDIFFFAC(pother_) ((pother_)->fMass)
#define DIFFUSIONBase() double diffbase = 4*smf->dMetalDiffusionCoeff*(p->diff+q->diff) \
     /((p->fDensity+q->fDensity)*(p->fMass+q->fMass)); 
#define DIFFUSIONMass() \
    { double diff = diffbase*(p->fMass - q->fMass); \
      PACTIVE( p->fMassDot += diff*p->fMass*rq ); \
      QACTIVE( q->fMassDot -= diff*q->fMass*rp ); \
    }
#define DIFFUSIONVelocity() \
    { double diff0 = diffbase*(p->v[0] - q->v[0]); \
      double diff1 = diffbase*(p->v[1] - q->v[1]); \
      double diff2 = diffbase*(p->v[2] - q->v[2]); \
      PACTIVE( ACCEL(p,0) += diff0*rq*MASSDIFFFAC(q) ); \
      QACTIVE( ACCEL(q,0) -= diff0*rp*MASSDIFFFAC(p) ); \
      PACTIVE( ACCEL(p,1) += diff1*rq*MASSDIFFFAC(q) ); \
      QACTIVE( ACCEL(q,1) -= diff1*rp*MASSDIFFFAC(p) ); \
      PACTIVE( ACCEL(p,2) += diff2*rq*MASSDIFFFAC(q) ); \
      QACTIVE( ACCEL(q,2) -= diff2*rp*MASSDIFFFAC(p) ); \
    }
#else
#define MASSDIFFFAC(pother_) 1
#define DIFFUSIONBase() double diffbase = 2*smf->dMetalDiffusionCoeff*(p->diff+q->diff) \
     /(p->fDensity+q->fDensity); 
#define DIFFUSIONMass()
#define DIFFUSIONVelocity()
#endif
#else
#define DIFFUSIONBase()
#define DIFFUSIONMass()
#define DIFFUSIONVelocity()
#endif


#ifdef DIFFUSION
#ifdef UNONCOOL
#define DIFFUSIONThermaluNoncool() \
        { double diffuNc = diffTh*(p->uNoncoolPred-q->uNoncoolPred); \
        PACTIVE( p->uNoncoolDotDiff += diffuNc*rq );        \
        QACTIVE( q->uNoncoolDotDiff -= diffuNc*rp );        \
        }
#else
#define DIFFUSIONThermaluNoncool()  
#endif
#ifdef DIFFUSIONPRICE
#define DIFFUSIONThermal() \
    { double irhobar = 2/(p->fDensity+q->fDensity);     \
     double vsig = sqrt(fabs(qPoverRho2*q->fDensity*q->fDensity - pPoverRho2*p->fDensity*p->fDensity)*irhobar); \
     double diffTh = smf->dThermalDiffusionCoeff*0.5*(ph+sqrt(0.25*BALL2(q)))*irhobar*vsig; \
     double diffu = diffTh*(p->uPred-q->uPred);             \
     PACTIVE( p->uDotDiff += diffu*rq );                     \
     QACTIVE( q->uDotDiff-= diffu*rp );                     \
     DIFFUSIONThermaluNoncool(); }
#else
#ifndef NODIFFUSIONTHERMAL
/* Default -- thermal diffusion */
#define DIFFUSIONThermal() \
    { double diffTh = 2*smf->dThermalDiffusionCoeff*(p->diff+q->diff)/(p->fDensity+q->fDensity); \
      double diffu = diffTh*(p->uPred-q->uPred);                              \
      PACTIVE( p->uDotDiff += diffu*rq*MASSDIFFFAC(q) );                \
      QACTIVE( q->uDotDiff -= diffu*rp*MASSDIFFFAC(p) );                \
      DIFFUSIONThermaluNoncool(); }
#else
#define DIFFUSIONThermal()
#endif
#endif

#define DIFFUSIONMetals() \
    { double diff = diffbase*(p->fMetals - q->fMetals); \
      PACTIVE( p->fMetalsDot += diff*rq*MASSDIFFFAC(q) ); \
      QACTIVE( q->fMetalsDot -= diff*rp*MASSDIFFFAC(p) ); }
#ifdef STARFORM
#define DIFFUSIONMetalsOxygen() \
    { double diff = diffbase*(p->fMFracOxygen - q->fMFracOxygen); \
      PACTIVE( p->fMFracOxygenDot += diff*rq*MASSDIFFFAC(q) ); \
      QACTIVE( q->fMFracOxygenDot -= diff*rp*MASSDIFFFAC(p) ); }
#define DIFFUSIONMetalsIron() \
    { double diff = diffbase*(p->fMFracIron - q->fMFracIron); \
      PACTIVE( p->fMFracIronDot += diff*rq*MASSDIFFFAC(q) ); \
      QACTIVE( q->fMFracIronDot -= diff*rp*MASSDIFFFAC(p) ); }
#else 
#define DIFFUSIONMetalsOxygen() 
#define DIFFUSIONMetalsIron() 
#endif /* STARFORM */
#else /* No diffusion */
#define DIFFUSIONThermal()
#define DIFFUSIONMetals() 
#define DIFFUSIONMetalsOxygen() 
#define DIFFUSIONMetalsIron() 
#endif

#ifdef VARALPHA
#define ALPHA (smf->alpha*0.5*(p->alpha+q->alpha))
#define BETA  (smf->beta*0.5*(p->alpha+q->alpha))
#else
#define ALPHA (smf->alpha)
#define BETA  (smf->beta)
#endif
#define SETDTNEW_PQ(dt_)  { if (dt_ < p->dtNew) p->dtNew=dt_; \
                            if (dt_ < q->dtNew) q->dtNew=dt_; \
                            if (4*q->dt < p->dtNew) p->dtNew = 4*q->dt; \
                            if (4*p->dt < q->dtNew) q->dtNew = 4*p->dt; }
          
#ifdef VSIGVISC
#define ARTIFICIALVISCOSITY(visc_,dt_) { absmu = -dvdotdr*smf->a            \
            /sqrt(nnList[i].fDist2); /* mu multiply by a to be consistent with physical c */ \
        if (absmu>p->mumax) p->mumax=absmu; /* mu terms for gas time step */ \
		if (absmu>q->mumax) q->mumax=absmu; \
		visc_ = (ALPHA*(pc + q->c) + BETA*1.5*absmu); \
		dt_ = smf->dtFacCourant*ph/visc_;     \
		visc_ = SWITCHCOMBINE(p,q)*visc_ \
		    *absmu/(pDensity + q->fDensity); }
#else
#define ARTIFICIALVISCOSITY(visc_,dt_) { double hav=0.5*(ph+sqrt(0.25*BALL2(q)));  /* h mean */ \
		absmu = -hav*dvdotdr*smf->a  \
		    /(nnList[i].fDist2+0.01*hav*hav); /* mu multiply by a to be consistent with physical c */ \
		if (absmu>p->mumax) p->mumax=absmu; /* mu terms for gas time step */ \
		if (absmu>q->mumax) q->mumax=absmu; \
		visc_ = (ALPHA*(pc + q->c) + BETA*2*absmu);	\
		dt_ = smf->dtFacCourant*hav/(0.625*(pc + q->c)+0.375*visc_); \
		visc_ = SWITCHCOMBINE(p,q)*visc_ \
		    *absmu/(pDensity + q->fDensity); }
#endif

            /* Force Calculation between particles p and q */
	    DRHODTACTIVE( PACTIVE( p->fDivv_PdV -= rq/p->fDivv_Corrector/RHO_DIVV(pDensity,q->fDensity)*dvdotdr; )); 
	    DRHODTACTIVE( QACTIVE( q->fDivv_PdV -= rp/p->fDivv_Corrector/RHO_DIVV(q->fDensity,pDensity)*dvdotdr; )); 
	    DRHODTACTIVE( PACTIVE( p->fDivv_PdVcorr -= rq/RHO_DIVV(pDensity,q->fDensity)*dvdotdr; ));
        DRHODTACTIVE( QACTIVE( q->fDivv_PdVcorr -= rp/RHO_DIVV(q->fDensity,pDensity)*dvdotdr; ));
        PACTIVE( p->uDotPdV += rq*PRES_PDV(pPoverRho2,qPoverRho2)*dvdotdr; );
        QACTIVE( q->uDotPdV += rp*PRES_PDV(qPoverRho2,pPoverRho2)*dvdotdr; );
//if (p->iOrder == 865177) printf("sphp %d: %g %g %g %g\n",p->iOrder,p->u,pPoverRho2,dvdotdr,rq*PRES_PDV(pPoverRho2,qPoverRho2)*dvdotdr );
//if (q->iOrder == 865177) printf("sphq %q: %g %g %g %g\n",q->iOrder,q->u,qPoverRho2,dvdotdr,rp*PRES_PDV(qPoverRho2,pPoverRho2)*dvdotdr );
        PACTIVE( Accp = (PRES_ACC(pPoverRho2f,qPoverRho2f)); );
        QACTIVE( Accq = (PRES_ACC(qPoverRho2f,pPoverRho2f)); );
	    if (dvdotdr>=0.0) {
            dt = smf->dtFacCourant*ph/(2*(pc > q->c ? pc : q->c));	
            }
	    else {  
            ARTIFICIALVISCOSITY(visc,dt); /* Calculate Artificial viscosity terms and associated dt */		
            PACTIVE( p->uDotAV += rq*(0.5*visc)*dvdotdr; );
            QACTIVE( q->uDotAV += rp*(0.5*visc)*dvdotdr; );
            PACTIVE( Accp += visc; );
            QACTIVE( Accq += visc; );
		}
        SETDTNEW_PQ(dt);
	    PACTIVE( Accp *= rq*aFac; );/* aFac - convert to comoving acceleration */
	    QACTIVE( Accq *= rp*aFac; );
	    PACTIVE( ACCEL(p,0) -= Accp * dx; );
	    PACTIVE( ACCEL(p,1) -= Accp * dy; );
	    PACTIVE( ACCEL(p,2) -= Accp * dz; );
	    QACTIVE( ACCEL(q,0) += Accq * dx; );
	    QACTIVE( ACCEL(q,1) += Accq * dy; );
	    QACTIVE( ACCEL(q,2) += Accq * dz; );

        DIFFUSIONBase();
        DIFFUSIONThermal();
        DIFFUSIONMetals();
        DIFFUSIONMetalsOxygen();
        DIFFUSIONMetalsIron();
        DIFFUSIONMass();
 	    DIFFUSIONVelocity(); 

