/*****************************************************************************

Copyright (c) 2014 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#pragma once
#include "SusceptibilityHIV.h"

namespace Kernel
{
    class SusceptibilityHIV2 : public SusceptibilitySTI, virtual public ISusceptibilityHIV, public SusceptibilityHIVConfig
    {
    public:
        friend class IndividualHumanCoinfection;
        friend class IndividualHumanHIV;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
        //virtual bool Configure( const Configuration* config );

        virtual ~SusceptibilityHIV2(void);
        static Susceptibility *CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod);

        virtual void Update(float dt = 0.0);
        //virtual void UpdateInfectionCleared();

        // disease specific functions 
        virtual float GetCD4count() const;
        virtual float GetTimeSinceHIV() const;
        virtual void  Generate_forward_CD4();
        virtual void  FastForward( const IInfectionHIV * const, float dt );
        virtual void  ApplyARTOnset();
        virtual const ProbabilityNumber GetPrognosisCompletedFraction() const;
        virtual void  TerminateSuppression();

    protected:
        //disease specific params 
        SusceptibilityHIV2();
        SusceptibilityHIV2(IIndividualHumanContext *context);
        IIndividualHumanHIV * hiv_parent;

        void Initialize(float age, float immmod, float riskmod);

        // additional members of SusceptibilityHIV (params)
        float         time_since_HIV;
        typedef enum {
            CD4_LOW,
            CD4_MEDIUM,
            CD4_HIGH
        } tCd4Compartment;
        tCd4Compartment		  CD4count;
        bool viral_suppression_active;

    private:

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        // Serialization
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, SusceptibilityHIV2& sus, const unsigned int  file_version );
#endif
    };
}
