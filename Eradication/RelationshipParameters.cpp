/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "RelationshipParameters.h"

static const char * _module = "RelationshipParameters";

namespace Kernel {

#define Coital_Act_Rate_DESC_TEXT "Rate for coital acts"
#define Duration_Weibull_Heterogeneity_DESCT_TEXT "Inverse of Weibull shape (1/kappa) parameter of relationship duration (in years)."
#define Duration_Weibull_Scale_DESC_TEXT "Weibull scale parameter of relationship duration (in years)."
#define Condom_Usage_DESC_TEXT "Min=Left-asymptote of the sigmoidally-varying condom usage probability;\
 Max=Right-asymptote of the sigmoidally-varying condom usage probability;\
 Mid=Year in which the sigmoidally-varying condom usage probability reaches an inflection point half way between the early and late values;\
 Rate=Governs the speed of transition between early and late values of the condom usage sigmoid for relationship. The slope at the mid-year is proportional to the rate."
#define Migration_Actions_DESC_TEXT "TBD"
#define Migration_Actions_Distribution_DESC_TEXT "TBD"

    RelationshipParameters::RelationshipParameters( RelationshipType::Enum type )
        : JsonConfigurable()
        , m_Type( type )
        , m_CoitalActRate(0.0)
        , m_DurationWeibullHeterogeneity(0.0)
        , m_DurationWeibullScale(0.0)
        , m_CondomUsage()
        , m_MigrationActions()
        , m_MigrationActionsCDF()
    {
        initConfigTypeMap( "Coital_Act_Rate",                &m_CoitalActRate,                Coital_Act_Rate_DESC_TEXT, FLT_EPSILON,    20.0f,  0.33f );
        initConfigTypeMap( "Duration_Weibull_Heterogeneity", &m_DurationWeibullHeterogeneity, Duration_Weibull_Heterogeneity_DESCT_TEXT,  0,    100.0f, 1 );
        initConfigTypeMap( "Duration_Weibull_Scale",         &m_DurationWeibullScale,         Duration_Weibull_Scale_DESC_TEXT,           0,   FLT_MAX, 1 );
        initConfigTypeMap( "Condom_Usage_Probability",       &m_CondomUsage,                  Condom_Usage_DESC_TEXT );
    }

    RelationshipParameters::~RelationshipParameters()
    {
    }

    bool RelationshipParameters::Configure( const Configuration* config )
    {
        std::vector<std::string> ma_strings;
        std::vector<float> ma_dist;
        initConfigTypeMap( "Migration_Actions",              &ma_strings, Migration_Actions_DESC_TEXT );
        initConfigTypeMap( "Migration_Actions_Distribution", &ma_dist,    Migration_Actions_Distribution_DESC_TEXT, 0, 1 );

        bool ret = JsonConfigurable::Configure( config );
        if( ret )
        {
            if( ma_strings.size() != ma_dist.size() )
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Migration_Actions.size()", ma_strings.size(), 
                                                                                          "Migration_Actions_Distribution.size()", ma_dist.size() );
            }
            else if( ma_strings.size() == 0 )
            {
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "There must be at least one Migration Action defined." );
            }

            for( auto ma_str : ma_strings )
            {
                m_MigrationActions.push_back( (RelationshipMigrationAction::Enum)RelationshipMigrationAction::pairs::lookup_value( ma_str.c_str() ) );
            }

            float sum = 0.0;
            for( auto dist : ma_dist )
            {
                sum += dist ;
                m_MigrationActionsCDF.push_back( sum );
            }
            if( (sum < 0.9999) || (1.0001 < sum) )
            {
                std::ostringstream msg;
                msg << "The Migration_Actions_Distribution values do not add up to 1.0.";
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }
            // ensure that the last value in the CDF is exactly 1
            m_MigrationActionsCDF[ m_MigrationActionsCDF.size()-1 ] = 1.0 ;
        }
        return ret;
    }
}