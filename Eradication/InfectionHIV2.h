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

#include "InfectionSTI.h"
#include "InfectionHIV.h"
#include "SusceptibilityHIV.h"

namespace Kernel
{
    class InfectionHIV2 : public InfectionSTI, public IInfectionHIV, protected InfectionHIVConfig
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
    public:
        virtual ~InfectionHIV2(void);
        static Infection *CreateInfection(IIndividualHumanContext *context, suids::suid _suid);

        virtual void SetParameters(StrainIdentity* infstrain=NULL, int incubation_period_override = -1);
        virtual void Update(float dt, Susceptibility* immunity = NULL);

        virtual float GetInfectiousness() const;
        
        virtual float GetWHOStage() const;
        virtual NaturalNumber GetViralLoad() const;
        virtual float GetPrognosis() const;
        virtual const HIVInfectionStage::Enum& GetStage() const;
        virtual void SetupSuppressedDiseaseTimers();
        virtual void ApplySuppressionDropout();
        virtual void ApplySuppressionFailure();

    protected:
        InfectionHIV2();
        InfectionHIV2(IIndividualHumanContext *context);

        virtual void  Initialize(suids::suid _suid);
        HIVInfectionStage::Enum m_infection_stage;
        float _viralLoadTimer;
        float _hivStageTimer;

        IIndividualHumanHIV * hiv_parent;
        bool viral_suppression_active;

    private:

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        // Serialization
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, InfectionHIV& inf, const unsigned int file_version );
#endif
    };
}
