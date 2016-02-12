/*****************************************************************************

Copyright (c) 2015 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#pragma once
#include "Infection.h"

namespace Kernel
{
    class InfectionPyConfig : public JsonConfigurable
    {
        friend class IndividualPy;
        GET_SCHEMA_STATIC_WRAPPER(InfectionPyConfig)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        bool Configure( const Configuration* config );

    protected:
       
    };

    class IInfectionPy : public ISupports
    {
        public:
        virtual void Clear() = 0;
    };

    class InfectionPy
        : public Infection
        , public IInfectionPy 
        , protected InfectionPyConfig
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        static InfectionPy *CreateInfection(IIndividualHumanContext *context, suids::suid _suid);
        virtual ~InfectionPy(void);

        virtual void SetParameters(StrainIdentity* infstrain = NULL, int incubation_period_override = -1);
        virtual void InitInfectionImmunology(Susceptibility* _immunity);
        virtual void Update(float dt, Susceptibility* _immunity = NULL);
        void SetMCWeightOfHost(float ind_mc_weight);
        virtual void Clear();

        // InfectionPyReportable methods
    protected:
        InfectionPy(); 

        const SimulationConfig* params();

        InfectionPy(IIndividualHumanContext *context);
        void Initialize(suids::suid _suid);

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;

        template<class Archive>
        friend void serialize(Archive & ar, InfectionPy& inf, const unsigned int file_version );
#endif
    };
}

