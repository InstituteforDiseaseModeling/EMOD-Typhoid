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

#include "stdafx.h"
#include "HIVDrugTypeParameters.h"

#include "Exceptions.h"
#include <algorithm> // for transform
#include <cctype> // for tolower

static const char * _module = "HIVDTP";

Kernel::HIVDrugTypeParameters::tHIVDTPMap Kernel::HIVDrugTypeParameters::_hivdtMap;

namespace Kernel
{
    HIVDrugTypeParameters::HIVDrugTypeParameters( const std::string &drugName )
    { 
        LOG_DEBUG_F( "ctor: drugName = %s\n", drugName.c_str() );

        initConfigTypeMap( "Mutation", &mutation, HIV_DTP_Mutation_DESC_TEXT);
        initConfigTypeMap( "Mutation_Fitness_Cost", &mutation_fitness_cost, HIV_DTP_Mutation_Fitness_Cost_DESC_TEXT, 0.0f, 1.0f, 0.9f );
        initConfigTypeMap( "Mutation_Fold_Change_IC50", &mutation_change_IC50, HIV_DTP_Mutation_Fold_Change_IC50_DESC_TEXT, 1.0f, 100.0f, 1.0f );
        initConfigTypeMap( "Mutation_Increase_Slope", &mutation_increase_slope, HIV_DTP_Mutation_Increase_Slope_DESC_TEXT, -1.0f, 1.0f, 0.0f );

        initConfigTypeMap( "Mutant_Equilibrium_Frequency", &equilibrium_frequency, HIV_DTP_Mutant_Equilibrium_Frequency_DESC_TEXT, -1.0f, 1.0f, 0.0f );
        initConfigTypeMap( "Mutant_Reservoir_Exit", &equilibrium_frequency, HIV_DTP_Mutation_Fitness_Cost_DESC_TEXT, 0.0f, 1000.0f, 0.0f );
        
        initConfigTypeMap( "PKPD_Cmax", &pkpd_Cmax__uMol, HIV_DTP_PKPD_Cmax_DESC_TEXT, 0.0f, 1000.0f, 0.0f );
        initConfigTypeMap( "PKPD_Cmin", &pkpd_Cmin__uMol, HIV_DTP_PKPD_Cmin_DESC_TEXT, 0.0f, 1.0f, 0.0f );
        initConfigTypeMap( "PKPD_Halflife", &pkpd_halflife__hours, HIV_DTP_PKPD_Halflife_DESC_TEXT, 0.0f, 10000.0f, 24.0f );
        initConfigTypeMap( "Dose_Interval", &dose_interval__days, HIV_DTP_Dose_Interval_DESC_TEXT, 0.0f, 10.0f, 1.0f );
    }

    HIVDrugTypeParameters::~HIVDrugTypeParameters()
    {
        LOG_DEBUG( "dtor\n" );
    }

    HIVDrugTypeParameters* HIVDrugTypeParameters::CreateHIVDrugTypeParameters(
        const std::string &drugName
    )
    { 
        LOG_DEBUG( "Create\n" );

        // The HIVDTP construct exists to cache once-per-simulation values of one or more drugs.
        // Until there can be different parameters for the same drug in a single simulation,
        // at which point we would probably just make those configurable properties of the drug itself,
        // there is no need to keep leaking memory here.

        HIVDrugTypeParameters* params = NULL;
        tHIVDTPMap::const_iterator itMap = _hivdtMap.find(drugName);

        if ( itMap == _hivdtMap.end() )
        {
            params = _new_ HIVDrugTypeParameters( drugName );
            params->Initialize(drugName);
            if( !JsonConfigurable::_dryrun )
            {
                try
                {
                    Configuration* drug_config = Configuration::CopyFromElement( (*EnvPtr->Config)["HIV_Drug_Params"][drugName] );
                    params->Configure( drug_config );

                    // Check validity of dosing regimen
                    float sim_tstep = (*EnvPtr->Config)["Simulation_Timestep"].As<Number>();
                    float updates_per_tstep = (*EnvPtr->Config)["Infection_Updates_Per_Timestep"].As<Number>();
                    if ( params->dose_interval__days < sim_tstep/updates_per_tstep )
                    {
                        std::ostringstream oss;
                        oss << "time_between_doses (" << params->dose_interval__days << ") is less than dt (" << sim_tstep/updates_per_tstep << ")" << std::endl;
                        throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, oss.str().c_str() );
                    }
                }
                catch(json::Exception &e)
                {
                    // Exception getting parameter block for drug of type "drugName" from config.json
                    throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, e.what() ); 
                }
            }
            _hivdtMap[ drugName ] = params;
            return params;
        }
        else
        {
            return itMap->second;
        }
    }

    bool
    HIVDrugTypeParameters::Configure(
        const ::Configuration *config
    )
    {
        LOG_DEBUG( "Configure\n" );

        initConfig( "Drug_Class", hiv_drug_class, config,
                    MetadataDescriptor::Enum("Drug_Class", HIV_DTP_Drug_Class_DESC_TEXT, MDD_ENUM_ARGS(HIVDrugClass)) );

        if( hiv_drug_class == HIVDrugClass::NucleosideReverseTranscriptaseInhibitor)
        {
            initConfig( "NRTI_Nucleoside_Analog", nucleoside_analog, config, MetadataDescriptor::Enum("Nucleoside_Analog",
                HIV_DTP_Nucleoside_Analog_DESC_TEXT, MDD_ENUM_ARGS(ReverseTranscriptaseNucleosideAnalog)) );
        }

        return JsonConfigurable::Configure( config );
    }

    QueryResult
    HIVDrugTypeParameters::QueryInterface(
        iid_t iid, void **ppvObject
    )
    {
        throw NotYetImplementedException(  __FILE__, __LINE__, __FUNCTION__ );
    }

    void HIVDrugTypeParameters::Initialize(const std::string &drugName)
    {
        LOG_DEBUG_F( "HIVDrugTypeParameters::Initialize: drug name = %s\n", drugName.c_str() );

        int eDrug = HIVDrugClass::pairs::lookup_value(drugName.c_str());

        if ( eDrug < 0 )
        {
            LOG_WARN_F("Anti-HIV drug name in HIV_Drug_Params block (%s) is not one of the standard cases.\n", drugName.c_str());
        }

        _drugName = drugName;
    }

}

