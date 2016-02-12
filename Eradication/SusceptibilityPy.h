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

#include "Susceptibility.h"
#include "StrainIdentity.h"

#ifdef ENABLE_PYTHON

namespace Kernel
{
    class SusceptibilityPyConfig: public JsonConfigurable
    {
        GET_SCHEMA_STATIC_WRAPPER(SusceptibilityPyConfig)

        friend class IndividualHumanPy;

    public:
        virtual bool Configure( const Configuration* config );
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
        static float x_population_immunity;
    };

    class ISusceptibilityPy : public ISupports
    {
    public:
        //virtual void SetNewInfectionByStrain(StrainIdentity* infection_strain) = 0;
        //immunity
   };

    class ISusceptibilityPyReportable : public ISupports
    {
        public:
        //virtual void GetSheddingTiter(float sheddingTiters[])               const = 0; 
    };

    class SusceptibilityPy :
        public Susceptibility,
        public SusceptibilityPyConfig,
        public ISusceptibilityPy,
        public ISusceptibilityPyReportable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()

    protected:
    public:
        static SusceptibilityPy *CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod);
        void Initialize(float _age, float _immmod, float _riskmod);
        SusceptibilityPy(IIndividualHumanContext *context);
	SusceptibilityPy() {}
        virtual ~SusceptibilityPy(void);

        virtual void Update(float dt = 0.0);

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class boost::serialization::access;
	/*
        template<class Archive>
        friend void serialize(Archive & ar, SusceptibilityPy&, const unsigned int  file_version );
	*/
#endif
    };
}
#endif
