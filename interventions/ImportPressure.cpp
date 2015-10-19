/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ImportPressure.h"

#include "Exceptions.h"
#include "InterventionFactory.h"
#include "NodeEventContext.h"  // for ISporozoiteChallengeConsumer methods
#include "SimulationConfig.h"

static const char * _module = "ImportPressure";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(ImportPressure)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(INodeDistributableIntervention)
        HANDLE_ISUPPORTS_VIA(INodeDistributableIntervention)
    END_QUERY_INTERFACE_BODY(ImportPressure)

    IMPLEMENT_FACTORY_REGISTERED(ImportPressure)

    QuickBuilder ImportPressure::GetSchema()
    {
        return QuickBuilder( jsonSchemaBase );
    }

    ImportPressure::ImportPressure() 
        : duration_counter(0)
        , num_imports(0)
    {
        LOG_DEBUG_F( "ctor\n" );
        initSimTypes( 1, "GENERIC_SIM" );
    }

    ImportPressure::~ImportPressure() 
    {
        LOG_DEBUG_F( "dtor: expired = %d\n", expired );
    }
#define IP_Durations_DESC_TEXT "Durations to apply import pressure over."
#define IP_Daily_Import_Pressures_DESC_TEXT "daily importation pressures to apply"

    bool ImportPressure::Configure( const Configuration * inputJson )
    {
        LOG_DEBUG_F( "%s\n", __FUNCTION__ );
        // TODO: specification for rate, seasonality, and age-biting function
        initConfigTypeMap( "Durations",              &durations,              IP_Durations_DESC_TEXT,               0, INT_MAX, 1 );
        initConfigTypeMap( "Daily_Import_Pressures", &daily_import_pressures, IP_Daily_Import_Pressures_DESC_TEXT , 0, FLT_MAX, 0 );

        bool configured = Outbreak::Configure( inputJson );

        if(durations.size() != (daily_import_pressures.size()))
        {        
            throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                "ImportPressure requires Durations must be the same size as Daily_Import_Pressures" );        
        }

        ConstructDistributionCalendar();

        return configured;
    }

    bool ImportPressure::Distribute(INodeEventContext *context, IEventCoordinator2* pEC)
    {
        return BaseNodeIntervention::Distribute( context, pEC );
    }

    void ImportPressure::ConstructDistributionCalendar()  
    {
        LOG_DEBUG_F( "%s\n", __FUNCTION__ );
        while (!durations.empty())
        {
            NaturalNumber duration = durations.back();
            NonNegativeFloat pressure = daily_import_pressures.back();
            durations_and_pressures.push_back(std::make_pair(duration, pressure));
            durations.pop_back();
            daily_import_pressures.pop_back();
        }      
    }

    void ImportPressure::SetContextTo(INodeEventContext *context) 
    { 
        LOG_DEBUG_F( "%s\n", __FUNCTION__ );
        parent = context; 
    }

    // The durations are used in sequence as a series of relative deltas from the Start_Day
    // of the campaign event.
    // t(now).......+t0....+t1...............+t2.+t3....+t4
    void ImportPressure::Update( float dt )
    {
        // Throw away any entries with durations less than current value of duration_counter, and reset duration_counter
        while( durations_and_pressures.size() > 0 && 
               duration_counter >= durations_and_pressures.back().first )
        {
            LOG_DEBUG_F( "Discarding input entry with duration/pressure of %f/%f\n", (float) durations_and_pressures.back().first, (float) durations_and_pressures.back().second );
            durations_and_pressures.pop_back();
            duration_counter = 0;
        }

        NonNegativeFloat daily_import_pressure = 0.0f; // 0 -> FLT_MAX, mean 'target'
        if (duration_counter < durations_and_pressures.back().first)
        {
            duration_counter += dt;
            daily_import_pressure = durations_and_pressures.back().second;   
            LOG_DEBUG_F("Duration counter = %f, import_pressure = %0.2f\n", (float) duration_counter, (float) daily_import_pressure);
        }
 
        // Convert the Poisson rate into a number of events
        num_imports = randgen->Poisson(daily_import_pressure*dt);
        if (num_imports == 0)
        {
            LOG_DEBUG_F( "Poisson draw returned 0\n" );
            return;
        }
        else
        {       
            LOG_DEBUG_F("Duration counter = %f, import_cases = %d\n", (float) duration_counter, (int) num_imports);
            StrainIdentity* strain_identity = GetNewStrainIdentity(parent);

            IOutbreakConsumer *ioc;
            if (s_OK == parent->QueryInterface(GET_IID(IOutbreakConsumer), (void**)&ioc))
            {
                ioc->AddImportCases(strain_identity, import_age, num_imports);
            }
            delete strain_identity;
        }
    }
}

 
