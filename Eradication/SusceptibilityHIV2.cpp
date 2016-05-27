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

#include <stdafx.h>
#include "SusceptibilityHIV2.h"
#include "IIndividualHumanHIV.h"

static const char * _module = "SusceptibilityHIV2";

namespace Kernel
{
    //
    // Constructors and Initializers
    // 
    // Encapsulate the actual creation of the Suscept object
    Susceptibility *
    SusceptibilityHIV2::CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod)
    {
        auto *newsusceptibility = _new_ SusceptibilityHIV2(context);
        newsusceptibility->Initialize(age, immmod, riskmod);
        return newsusceptibility;
    }

    // ctor
    SusceptibilityHIV2::SusceptibilityHIV2()
    : SusceptibilitySTI()
    , hiv_parent( NULL )
    , viral_suppression_active( false )
    {
    }

    // ctor 2
    SusceptibilityHIV2::SusceptibilityHIV2(IIndividualHumanContext *context)
    : SusceptibilitySTI( context )
    , hiv_parent( NULL )
    , viral_suppression_active( false )
    {
    }

    // dtor
    SusceptibilityHIV2::~SusceptibilityHIV2(void)
    {
    }

    // This is where we do any initialization stuff. This is where we'd want to set up our CD4 compartment
    // transition timers.
    void
    SusceptibilityHIV2::Initialize(float _age, float _immmod, float _riskmod)
    {
        Susceptibility::Initialize(_age, _immmod, _riskmod);
        if( s_OK != parent->QueryInterface(GET_IID(IIndividualHumanHIV), (void**)&hiv_parent) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent", "IIndividualHumanHIV", "IndividualHuman" );
        }
        IIndividualHumanHIV * hiv_parent = nullptr;
        release_assert( parent );
        if (parent->QueryInterface(GET_IID(IIndividualHumanHIV), (void**)&hiv_parent) != s_OK)
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent", "IIndividualHumanHIV", "IIndividualHumanContext" );
        }
        CD4count = CD4_HIGH;
        viral_suppression_active = false;
    }

    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityHIV2)
        HANDLE_INTERFACE(ISusceptibilityHIV)
    END_QUERY_INTERFACE_BODY(SusceptibilityHIV2)

    //
    // Getters
    // 

    // A very simple getter you'd expect to find in Susceptibility. Class is free to set and manage CD4
    // count per its model. Changes to CD4 should be done in Update.
    float
    SusceptibilityHIV2::GetCD4count()
    const
    {
        switch( CD4count )
        {
            case CD4_HIGH:
                return 500;

            case CD4_MEDIUM:
                return 300;

            case CD4_LOW:
                return 100;

            default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "<CD4count>" );
        }
    }

    // A very simple timer getter. Note that everyone has a susceptibility object, infected or not, so
    // given that there is an implict assumption (is that redundant?) in the function name, it's an
    // open question about what it should return if called on an uninfected individual.
    float
    SusceptibilityHIV2::GetTimeSinceHIV()
    const
    {
        if (!hiv_parent->HasHIV())
        {
            LOG_WARN_F("GetTimeSinceHIV() was called on uninfected individual %d\n", parent->GetSuid().data);
            return -1;
        }

        return time_since_HIV;
    }

    // This function is a little bit specific to our primary intra-host model because prognosis is calculated
    // first. May be no-op in alternate model.
    const ProbabilityNumber  
    SusceptibilityHIV2::GetPrognosisCompletedFraction()
    const
    {
        return 0;
    }

    //
    // Setters
    // 

    // Everything has an Update. This ages our Suscept object (which is basically a cache of individual age),
    // calls into the base class, and increments our time_since_HIV timer at a minimum. For the alternate
    // model it should also figure out if we've moved into next CD4 compartment (which could be death).
    void SusceptibilityHIV2::Update(float dt)
    {
        age += dt; // tracks age for immune purposes
        SusceptibilitySTI::Update(dt);

        release_assert( hiv_parent );
        if (hiv_parent->HasHIV())
        {
            time_since_HIV += dt;
            auto draw = randgen->e();
            switch( CD4count )
            {
                case CD4_HIGH:
                    if( draw < 0.001 )
                    {
                        CD4count  = CD4_MEDIUM;
                    }
                break;

                case CD4_MEDIUM:
                    if( draw < 0.0001 )
                    {
                        CD4count  = CD4_LOW;
                    }
                break;

                case CD4_LOW:
                    if( draw < 0.01 )
                    {
                        // DIE!
                    }
                break;

                default:
                    throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "<CD4count>" );
                break; // symmetry :)
            }
        }

        // do random number draw and progress CD4 perhaps
        if( viral_suppression_active == false )
        {
            return;
        }

    }

    // This function is a little bit specific to our primary intra-host model because prognosis is calculated
    // first and those individuals with seeded infections have to have the CD4 initialized to a lower value.
    void
    SusceptibilityHIV2::FastForward( const IInfectionHIV * const, float dt )
    {
    }

    // This function exists for TB-HIV. We almost certainly won't use this alternate model for TB-HIV coinfection
    void
    SusceptibilityHIV2::Generate_forward_CD4()
    {
        throw IllegalOperationException( __FILE__, __LINE__, __FUNCTION__ );
    }

    // This function is where we get notified that the individual just started ART and start making CD4
    // adjustments appropriate to the model.
    void
    SusceptibilityHIV2::ApplyARTOnset()
    {
        viral_suppression_active = true;
    }

    // This function is the counter-point to ApplyARTOnset. This is where we'd resume CD4 decline for individuals
    // no longer on viral suppression medication (e.g., ART).
    void
    SusceptibilityHIV2::TerminateSuppression()
    {
        viral_suppression_active = false;
    }

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
    template<class Archive>
    void serialize(Archive & ar, SusceptibilityHIV2 &sus, const unsigned int  file_version )
    {
        // Serialize fields
//        ar & sus.m_is_immune_competent;

        // Serialize base class
        ar & boost::serialization::base_object<Kernel::SusceptibilitySTI>(sus);
    }

    INSTANTIATE_BOOST_SERIALIZATION_HACKS(SusceptibilityHIV2);
    template void serialize( boost::mpi::packed_iarchive&, SusceptibilityHIV2 &obj, unsigned int file_version );
    template void serialize( boost::mpi::packed_oarchive&, SusceptibilityHIV2 &obj, unsigned int file_version );
    INSTANTIATE_SERIALIZER(SusceptibilityHIV2, boost::archive::binary_iarchive);
    INSTANTIATE_SERIALIZER(SusceptibilityHIV2, boost::archive::binary_oarchive);
#endif
}

