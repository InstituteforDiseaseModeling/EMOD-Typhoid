/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <set>
#include "BoostLibWrapper.h"
#include "NodeHIV.h"
#include "IndividualSTI.h"
#include "IIndividualHumanHIV.h"

namespace Kernel
{
    class IHIVInfection;
    class IHIVSusceptibility;
    class IndividualHumanHIV : public IndividualHumanSTI, public IIndividualHumanHIV
    {
    public:
        friend class SimulationHIV;
        DECLARE_QUERY_INTERFACE()
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING();

        virtual ~IndividualHumanHIV(void);
        static   IndividualHumanHIV *CreateHuman( INodeContext *context, 
                                                  suids::suid _suid, 
                                                  float monte_carlo_weight = 1.0f, 
                                                  float initial_age = 0.0f, 
                                                  int gender = int(Gender::MALE), 
                                                  float initial_poverty = 0.5f );
        virtual void InitializeHuman() override;
        virtual bool Configure( const Configuration* config ) override;
        virtual void Update( float currenttime, float dt ) override;

        // Infections and Susceptibility
        virtual void CreateSusceptibility( float imm_mod=1.0f, float risk_mod=1.0f ) override;

        virtual bool HasHIV() const override;

        virtual IInfectionHIV* GetHIVInfection() const override;
        virtual ISusceptibilityHIV* GetHIVSusceptibility() const override;
        virtual IHIVInterventionsContainer* GetHIVInterventionsContainer() const override;
        virtual bool UpdatePregnancy(float dt=1) override;
        virtual ProbabilityNumber getProbMaternalTransmission() const override;

        // healthcare interactions
        virtual std::string toString() const override;

    protected:
        IndividualHumanHIV( suids::suid id = suids::nil_suid(), 
                            float monte_carlo_weight = 1.0f, 
                            float initial_age = 0.0f, 
                            int gender = 0, 
                            float initial_poverty = 0.5f );
        
        virtual Infection* createInfection(suids::suid _suid) override;
        virtual void setupInterventionsContainer() override;
        ISusceptibilityHIV * hiv_susceptibility;

        // from HIVPerson (HIV branch), kto: clean up these comments later
        // individual characteristics, node-agnostic for now.
        //
        // medical chart variables
        bool has_active_TB;

        // variables for reporting
        unsigned int pos_num_partners_while_CD4500plus;
        unsigned int neg_num_partners_while_CD4500plus;

    private:

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, IndividualHumanHIV& human, const unsigned int  file_version );
#endif
    };
}
