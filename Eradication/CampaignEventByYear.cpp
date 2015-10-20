#include "CampaignEventByYear.h"
#include "SimulationHIV.h"

namespace Kernel
{
#ifndef DISABLE_HIV
    //
    // CampaignEventByYear class here.
    //
    IMPL_QUERY_INTERFACE1(CampaignEventByYear, IConfigurable)
    IMPLEMENT_FACTORY_REGISTERED(CampaignEventByYear)

    CampaignEventByYear::CampaignEventByYear()
    {
    }

    bool
    CampaignEventByYear::Configure(
        const Configuration * inputJson
    )
    {
        float start_year;
        initConfigTypeMap( "Start_Year", &start_year, Start_Year_DESC_TEXT, 0 );
        initConfigComplexType( "Nodeset_Config", &nodeset_config, Nodeset_Config_DESC_TEXT );
        initConfigComplexType( "Event_Coordinator_Config", &event_coordinator_config, Event_Coordinator_Config_DESC_TEXT );

        // Bypasss CampaignEvent base class so that we don't break without Start_Day!
        bool ret = JsonConfigurable::Configure( inputJson );
        // Throw in some error handling here. Base_Year may not be present. 
        /*if( base_year == 0 )
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Base_Year", "0", "Start_Year", std::to_string( start_year ).c_str() );
        }*/
        start_day = (start_year - SimulationHIV::base_year) * DAYSPERYEAR;
        return ret;
    }

    QuickBuilder CampaignEventByYear::GetSchema()
    {
        return QuickBuilder( jsonSchemaBase );
    }

    CampaignEventByYear::~CampaignEventByYear()
    {
    }
#endif
}
