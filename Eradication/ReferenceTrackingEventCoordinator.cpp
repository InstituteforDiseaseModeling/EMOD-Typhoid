/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "ReferenceTrackingEventCoordinator.h"
#include "SimulationConfig.h"

static const char * _module = "ReferenceTrackingEventCoordinator";

namespace Kernel
{
    IMPLEMENT_FACTORY_REGISTERED(ReferenceTrackingEventCoordinator)
    IMPL_QUERY_INTERFACE2(ReferenceTrackingEventCoordinator, IEventCoordinator, IConfigurable)

    ReferenceTrackingEventCoordinator::ReferenceTrackingEventCoordinator()
    : target_coverage( 0.0 ) // no great reason for this value
    {
    }

    QuickBuilder
    ReferenceTrackingEventCoordinator::GetSchema()
    {
        return QuickBuilder( jsonSchemaBase );
    }

    bool
    ReferenceTrackingEventCoordinator::Configure(
        const Configuration * inputJson
    )
    {
        float update_period = DAYSPERYEAR;
        initConfigComplexType("Time_Value_Map", &year2ValueMap, "Map of times (years) to coverages." );
        initConfigTypeMap("Update_Period", &update_period, "Gap between distribution updates.", 1.0, 10*DAYSPERYEAR, DAYSPERYEAR );
        initConfigTypeMap("End_Year", &end_year, "Final date at which this set of targeted coverages should be applied (expiration)", 0.0, 2200.0, 0.0 ); // min, max, default years

        auto ret = StandardInterventionDistributionEventCoordinator::Configure( inputJson );
        num_repetitions = -1; // unlimited
        if( JsonConfigurable::_dryrun == false )
        {
            float dt = GET_CONFIGURABLE(SimulationConfig)->Sim_Tstep;
            tsteps_between_reps = update_period/dt; // this won't be precise, depending on math.
        }
        return ret;
    }

    // Obviously don't need this if it's not doing anything useful.
    void ReferenceTrackingEventCoordinator::Update( float dt )
    {
        // Check if it's time for another distribution
        if( parent->GetSimulationTime().Year() >= end_year )
        {
            LOG_INFO_F( "ReferenceTrackingEventCoordinator expired.\n" );
            distribution_complete = true;
        }
        return StandardInterventionDistributionEventCoordinator::Update( dt );
    }

    // The purpose of this function is to calculate the existing coverage of the intervention in question
    // and then to set the target coverage based on the error between measured and configured (for current time).
    void ReferenceTrackingEventCoordinator::preDistribute()
    {
        // Two variables that will be used by lambda function that's called for each individual;
        // these vars accumulate values across the population. 
        NonNegativeFloat totalWithIntervention = 0.0f;
        NonNegativeFloat totalQualifyingPop = 0.0f;

        // This is the function that will be called for each individual in this node (event_context)
        INodeEventContext::individual_visit_function_t fn = 
            [ this, &totalWithIntervention, &totalQualifyingPop ](IIndividualHumanEventContext *ihec)
        {
            if( qualifiesDemographically( ihec ) )
            {
                auto mcw = ihec->GetMonteCarloWeight();
                totalQualifyingPop += mcw;
                auto better_ptr = ihec->GetInterventionsContext();
                // Check whether this individual has a non-zero quantity of this intervention, based on C++ mangled typename.
                totalWithIntervention += ( better_ptr->GetInterventionsByType( typeid( *_di ).name() ).size() > 0 ? mcw : 0 );
            }
        };

        // foreach node...
        for (auto event_context : cached_nodes)
        {
            event_context->VisitIndividuals( fn ); // does not return value, updates total existing coverage by capture
        }
        release_assert( totalQualifyingPop > 0 );
        Fraction currentCoverageForIntervention = totalWithIntervention/totalQualifyingPop;
        NonNegativeFloat totalWithoutIntervention = totalQualifyingPop - totalWithIntervention;
        float default_value = 0.0f;
        float year = parent->GetSimulationTime().Year();
        target_coverage  = year2ValueMap.getValueLinearInterpolation(year, default_value);

        float totalToIntervene = ( target_coverage * totalQualifyingPop ) - totalWithIntervention;
        NO_LESS_THAN( totalToIntervene, 0 );

        if( totalWithoutIntervention > 0 )
        {
            demographic_coverage = totalToIntervene / totalWithoutIntervention;
        }
        else
        {
            demographic_coverage = 0.0f;
        }
        LOG_INFO_F( "Setting demographic_coverage to %f based on target_coverage = %f, currentCoverageForIntervention = %f, total without intervention  = %f, total with intervention = %f.\n",
                    demographic_coverage,
                    (float) target_coverage,
                    (float) currentCoverageForIntervention,
                    (float) totalWithoutIntervention,
                    (float) totalWithIntervention
                );
    }

#if USE_JSON_SERIALIZATION
    // IJsonSerializable Interfaces
    void ReferenceTrackingEventCoordinator::JSerialize( IJsonObjectAdapter* root, JSerializer* helper ) const
    {
        root->BeginObject();
        root->EndObject();
    }

    void ReferenceTrackingEventCoordinator::JDeserialize( IJsonObjectAdapter* root, JSerializer* helper )
    {
    }
#endif


#if USE_BOOST_SERIALIZATION
// TODO: Consolidate with serialization code in header.
#include <boost/serialization/export.hpp>
    template<class Archive>
    void serialize(Archive &ar, ReferenceTrackingEventCoordinator &ec, const unsigned int v)
    {
    }
#endif
}

