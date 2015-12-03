/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "ReportTB.h"
#include "ReportTBHIV.h" // for base class
//#include "NodeTB.h"

#include "IndividualCoinfection.h"

static const char* _module = "ReportTBHIV";

namespace Kernel 
{

    static const char * _all_TB_HIV_label= "TB HIV+ Prevalence ";
    static const char * _latent_TB_HIV_prevalence_label = "Latent TB in HIV+ Prevalence";
    static const char * _HIV_prevalence_label = "HIV prevalence";
    static const char * _active_TB_HIV_prevalence_label = "Active TB in HIV+ Prevalence";

    static const char * _active_mean_CD4_TB_HIV_label = "Mean CD4 count coinfected";
    static const char * _active_TB_CD4_500_label = "Prevalence Active TB CD4 > 500";
    static const char * _active_TB_CD4_500_350_label = "Prevalence Active TB  350< CD4< 500";
    static const char * _active_TB_CD4_less_350_label = "Prevalence Acitve TB CD4 < 350";

    ReportTBHIV::ReportTBHIV()
        :ReportTB()
    {}

    void ReportTBHIV::BeginTimestep()
    {
        ReportTB::BeginTimestep();

        m_CD4count_TB_and_HIV_pos_persons = 0.0f;
        m_TB_active_500_350 = 0.0f;
        m_TB_active_above_500 = 0.0f;
        m_TB_active_below_350 = 0.0f;

        m_allTB_HIV_persons = 0.0f;
        m_active_TB_HIV_persons = 0.0f;
        m_latent_TB_HIV_persons = 0.0f;
        m_HIV_persons = 0.0f;
    }

    void ReportTBHIV::EndTimestep( float currentTime, float dt )
    {
        ReportTB::EndTimestep( currentTime, dt );

        m_allTB_HIV_persons = 0.0;
        m_active_TB_HIV_persons = 0.0;
        m_latent_TB_HIV_persons = 0.0;
        m_HIV_persons = 0.0f;
        m_CD4count_TB_and_HIV_pos_persons = 0;
        m_TB_active_500_350 = 0;
        m_TB_active_below_350 = 0;
        m_TB_active_above_500 = 0;
    }

    void ReportTBHIV::populateSummaryDataUnitsMap(std::map<std::string, std::string> &units_map)
    {
        ReportTB::populateSummaryDataUnitsMap(units_map);

        // Additional TB/HIV channels
        units_map[_all_TB_HIV_label]             = "Infected fraction";
        units_map[_active_TB_HIV_prevalence_label]       = "Infected fraction";
        units_map[ _latent_TB_HIV_prevalence_label]       = "Infected fraction";
        units_map[ _HIV_prevalence_label]       = "Infected fraction";
        units_map[_active_mean_CD4_TB_HIV_label]       = "CD4 count";
        units_map[_active_TB_CD4_500_label]          = "Fraction";
        units_map[_active_TB_CD4_500_350_label]    = "Fraction";
        units_map [_active_TB_CD4_less_350_label]  = "Fraction";
    }

    void
        ReportTBHIV::postProcessAccumulatedData()
    {
        // make sure to normalize Mean CD4 count BEFORE HIV Prevalence in TB positive
        normalizeChannel(_active_mean_CD4_TB_HIV_label, _all_TB_HIV_label);
        // Normalize TB-specific summary data channels, note do this before "Infected" and Active/Latent prevalence is normalized in ReportTB

        normalizeChannel(_all_TB_HIV_label,     "Statistical Population");

        ReportTB::postProcessAccumulatedData();
        normalizeChannel(_HIV_prevalence_label,        "Statistical Population");
        normalizeChannel(_active_TB_CD4_500_label,      _active_TB_HIV_prevalence_label);
        normalizeChannel(_active_TB_CD4_500_350_label, _active_TB_HIV_prevalence_label);
        normalizeChannel(_active_TB_CD4_less_350_label, _active_TB_HIV_prevalence_label);
        normalizeChannel(_active_TB_HIV_prevalence_label, _HIV_prevalence_label);
        normalizeChannel(_latent_TB_HIV_prevalence_label, _HIV_prevalence_label);
    }

    void
        ReportTBHIV::LogNodeData( Kernel::INodeContext * pNC )
    {
        Accumulate(_active_mean_CD4_TB_HIV_label, m_CD4count_TB_and_HIV_pos_persons );
        Accumulate(_all_TB_HIV_label,      m_allTB_HIV_persons );
        Accumulate(_active_TB_HIV_prevalence_label,      m_active_TB_HIV_persons );
        Accumulate(_latent_TB_HIV_prevalence_label, m_latent_TB_HIV_persons );
        Accumulate( _HIV_prevalence_label,         m_HIV_persons );

        Accumulate(_active_TB_CD4_less_350_label,  m_TB_active_below_350);
        Accumulate(_active_TB_CD4_500_350_label,  m_TB_active_500_350);
        Accumulate(_active_TB_CD4_500_label,  m_TB_active_above_500);

        ReportTB::LogNodeData( pNC );
    }

    void
        ReportTBHIV::LogIndividualData( IIndividualHuman* individual )
    {

        // Cast from IndividualHuman to IndividualHumanCoinfection
        float mc_weight = (float) individual->GetMonteCarloWeight();

        IIndividualHumanCoinfection* tb_ind = nullptr;
        if( individual->QueryInterface( GET_IID(IIndividualHumanCoinfection), (void**) &tb_ind ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "individual", "IIndividualHumanCoinfection", "IndividualHuman" );
        } 

        if(tb_ind->HasHIV())
        {
            m_HIV_persons += mc_weight;
        }

        auto suslist = tb_ind->Getsusceptibilitylist();
        for (auto psusceptibility : suslist)
        {
            ISusceptibilityHIV* susHIV = nullptr;
            if( psusceptibility->QueryInterface( GET_IID(ISusceptibilityHIV), (void**) &susHIV ) != s_OK )
            {
                continue;
            }
            float immune_strata = susHIV->GetCD4count();
            if (tb_ind->HasHIV() && individual->IsInfected() )
            {
                m_CD4count_TB_and_HIV_pos_persons += susHIV->GetCD4count() * mc_weight; 

                if (immune_strata > 500.0)
                    m_TB_active_above_500 += mc_weight;
                else if (immune_strata > 350.0 )
                    m_TB_active_500_350 += mc_weight;
                else if (immune_strata < 350.0)
                    m_TB_active_below_350 += mc_weight;
            }
        } 

        if (tb_ind->HasHIV() && individual->IsInfected())
        {
            m_allTB_HIV_persons += mc_weight;

            if ( tb_ind->HasActiveInfection() )
            { m_active_TB_HIV_persons += mc_weight; }
            if ( tb_ind->HasLatentInfection() )
            { m_latent_TB_HIV_persons += mc_weight; }
        }

        ReportTB::LogIndividualData(individual);
    }
}
