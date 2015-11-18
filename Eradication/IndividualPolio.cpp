/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "Debug.h"
#include "Contexts.h"
#include "RANDOM.h"
#include "Environment.h"
#include "IndividualPolio.h"
#include "SusceptibilityPolio.h"
#include "InfectionPolio.h"
#include "IContagionPopulation.h"
#include "PolioInterventionsContainer.h"

#pragma warning(disable: 4244)

static const char * _module = "IndividualPolio";

namespace Kernel
{

    GET_SCHEMA_STATIC_WRAPPER_IMPL(Polio.Individual,IndividualHumanPolio)
    BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanPolio, IndividualHumanEnvironmental)
        HANDLE_INTERFACE(IIndividualHumanPolio)
    END_QUERY_INTERFACE_DERIVED(IndividualHumanPolio, IndividualHumanEnvironmental)

    bool IndividualHumanPolio::ReportSabinWildPhenotypeAsWild = false;

    bool
    IndividualHumanPolio::Configure( const Configuration* config ) // just called once!
    {
        LOG_DEBUG( "Configure\n" );
        // polio
        initConfigTypeMap( "Report_Sabin_Wild_Phenotype_As_Wild", &ReportSabinWildPhenotypeAsWild, Report_Sabin_Wild_Phenotype_As_Wild_DESC_TEXT, false );

        SusceptibilityPolioConfig fakeImmunity;
        fakeImmunity.Configure( config );
        InfectionPolioConfig fakeInfection;
        fakeInfection.Configure( config );
        //return IndividualHuman::Configure( config );
        return JsonConfigurable::Configure( config );
    }

    IndividualHumanPolio::IndividualHumanPolio(suids::suid _suid, float monte_carlo_weight, float initial_age, int gender, float initial_poverty)
        : IndividualHumanEnvironmental(_suid, monte_carlo_weight, initial_age, gender, initial_poverty)
        , age_most_recent_infection(0)
        , paralysisVirusTypeMask(0)
        , polio_susceptibility(nullptr)
    {
    }

    IndividualHumanPolio *IndividualHumanPolio::CreateHuman(INodeContext *context, suids::suid id, float monte_carlo_weight, float initial_age, int gender, float initial_poverty)
    {
        IndividualHumanPolio *newhuman = _new_ IndividualHumanPolio(id, monte_carlo_weight, initial_age, gender, initial_poverty);
        
        newhuman->SetContextTo(context);
        LOG_DEBUG_F( "Created human with age=%f\n", newhuman->m_age );

        return newhuman;
    }

    void IndividualHumanPolio::PropagateContextToDependents()
    {
        IndividualHuman::PropagateContextToDependents();

#if 0
        //if( polio_susceptibility == nullptr && susceptibility != nullptr)
        {
            if ( s_OK != susceptibility->QueryInterface(GET_IID(ISusceptibilityPolio), (void**)&polio_susceptibility) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "susceptibility", "SusceptibilityPolio", "Susceptibility" );
            }
            release_assert(polio_susceptibility);

            // If we don't want to static_cast, the "right way" to fix this is to create an ISusceptibilityPolioReport 
            // containing the methods that use the ptr (e.g. AccumulateInfectionCount(), etc), move usage of those
            // methods from NodePolio::updateNodeStateCounters() to the reporter, and have the reporter get the 
            // ISusceptibilityPolioReport from the individual.  ERAD-769

            //if ( s_OK != susceptibility->QueryInterface(GET_IID(SusceptibilityPolio), (void**)&polio_susceptibility) )
            //{
            //    throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "susceptibility", "SusceptibilityPolio", "Susceptibility" );
            //}
        }
#else
        polio_susceptibility = static_cast<SusceptibilityPolio*>(susceptibility);
#endif
    }

    void IndividualHumanPolio::setupInterventionsContainer()
    {
        interventions = _new_ PolioInterventionsContainer();
    }

    REGISTER_SERIALIZABLE(IndividualHumanPolio);

    void IndividualHumanPolio::serialize(IArchive& ar, IndividualHumanPolio* obj)
    {
        IndividualHumanEnvironmental::serialize(ar, obj);
        IndividualHumanPolio& individual = *obj;
        ar.startObject();
            ar.labelElement("age_most_recent_infection") & individual.age_most_recent_infection;
            ar.labelElement("paralysisVirusTypeMask") & individual.paralysisVirusTypeMask;
        ar.endObject();
    }
}

void Kernel::IndividualHumanPolio::CreateSusceptibility(float imm_mod, float risk_mod)
{
    SusceptibilityPolio *newsusceptibility = SusceptibilityPolio::CreateSusceptibility(this, m_age, imm_mod, risk_mod);
    polio_susceptibility = newsusceptibility;
    susceptibility = newsusceptibility;
}

int Kernel::IndividualHumanPolio::CountInfections(void) const
{
    return int(infections.size());
}

void Kernel::IndividualHumanPolio::ClearAllNewInfectionByStrain(void)
{
    polio_susceptibility->ClearAllNewInfectionByStrain();
}

bool Kernel::IndividualHumanPolio::GetSusceptibleStatus(int pvType)
{
    return (polio_susceptibility->GetSusceptibleStatus(pvType));
}


int Kernel::IndividualHumanPolio::GetParalysisVirusTypeMask() const
{
    return paralysisVirusTypeMask;
}

// Should this be part of QI?
Kernel::ISusceptibilityPolioReportable *
Kernel::IndividualHumanPolio::GetSusceptibilityReporting()
const
{
    return (static_cast<ISusceptibilityPolioReportable*>(polio_susceptibility)); //->GetReporting();
}

const Kernel::infection_polio_reportable_list_t *Kernel::IndividualHumanPolio::GetInfectionReportables(bool getNewInfectionsOnly) const
{
    infection_polio_reportable_list_t *reportable_infections = new infection_polio_reportable_list_t;
    // Loop over infections, find new ones - push on new_reportable_infections
    for (auto infection : infections)
    {
        IInfectionPolioReportable *ipr = nullptr;
        if( infection->QueryInterface( GET_IID( IInfectionPolioReportable ), (void**)&ipr ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "Infection", "IInfectionPolio", "*it" );
        }
        if( !getNewInfectionsOnly || infection->GetStateChange() == InfectionStateChange::New )
        {
            // Note, infection can be deleted, so consume new_reportable_infections immediately
            reportable_infections->push_back( ipr );
        }
    }

    return reportable_infections;
}

const Kernel::infection_polio_reportable_list_t *Kernel::IndividualHumanPolio::GetNewInfectionReportables() const
{
    return GetInfectionReportables(true);
}

void Kernel::IndividualHumanPolio::applyNewInterventionEffects(float dt)
{
    StrainIdentity tmp_vaccine_strains[N_POLIO_SEROTYPES];
    float tmp_vaccine_dose_titer[N_POLIO_SEROTYPES];
    float prob_vaccine_take[N_POLIO_SEROTYPES];
    int tmp_n_vaccines;
    polio_susceptibility->GetNewInterventionsForIndividual(dt, &tmp_n_vaccines, tmp_vaccine_strains, tmp_vaccine_dose_titer);

    polio_susceptibility->GetProbabilityInfected(tmp_vaccine_strains, tmp_vaccine_dose_titer, tmp_n_vaccines, prob_vaccine_take);
    for(int i_vaccine = 0; i_vaccine < N_POLIO_SEROTYPES; i_vaccine++) // iterate over all members
    {
        if( prob_vaccine_take[i_vaccine] > 0 )
        {
            LOG_DEBUG_F("prob_vaccine_take[%d] = %f\n", i_vaccine, prob_vaccine_take[i_vaccine]);
            if(randgen->e() < prob_vaccine_take[i_vaccine])
            {
                AcquireNewInfection(&tmp_vaccine_strains[i_vaccine]);
            }
        }
    }
}

void Kernel::IndividualHumanPolio::Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route )
{
    //note: dt includes pool acquisition scaling factor
    int n_infections = CountInfections();
    StrainIdentity strain_id;
    strain_id.SetAntigenID(cp->GetAntigenId()); // antigenID of the strain to get infectivity from, is the individual already infected by the strain of this antigen type?
    if(n_infections == 0 || !InfectionExistsForThisStrain(&strain_id)) // no existing infection of this antigenic type, so determine infection from exposure
    {
        float acquired_virus;
        float infection_prob;
        float acquisition_rate = polio_susceptibility->GetDefaultAcquireRate();

        //dt contains x_acquisition_scaling so we only need one acquisition rate, see pool.cpp
        acquired_virus = cp->GetTotalContagion() * acquisition_rate;
        if( dt != 1.0f )
        {
           acquired_virus *= dt;
        }
        LOG_DEBUG_F( "acquired_virus = contagion (%f) * acquisition_rate (%e) * dt (%f) = %f\n", cp->GetTotalContagion(), acquisition_rate, dt, acquired_virus );
        if (acquired_virus == 0.0f ) return;

        float exponent = GetParent()->GetSusceptDynamicScaling();
        if( exponent != 1.0 )
        {
            float base = 1.0f + acquired_virus;
            acquired_virus = pow( base, exponent ) - 1.0f; // infectivity scales with log of virus, so modify susceptibility by exponentiating virus with the factor
            LOG_DEBUG_F( "acquired_virus = %f to the power %f - 1 = %f\n", base, exponent, acquired_virus );
        }
        
        // the acquire scaling cannot go in GetProbabilityInfected because vaccine interventions also call that. Only virus acquired from the community is to be modified by susceptibility scaling.

        cp->ResolveInfectingStrain(&strain_id); // get the substrain ID, does two things 1) samples genetic identity from the distribution important for reversion-dependent infectivity 2) gets identity of potential infection
        polio_susceptibility->GetProbabilityInfected(&strain_id, &acquired_virus, 1, &infection_prob); // strains are applied sequentially - eventually need a unified process for continuous dosing effects and interference among multiple serotypes
        LOG_DEBUG_F( "contagion = %f => infection_prob = %f\n", cp->GetTotalContagion(), infection_prob );

        if(infection_prob>0 && randgen->e() < infection_prob)
        {
            if(ReportSabinWildPhenotypeAsWild && (polio_susceptibility->GetReversionDegree(&strain_id) >= 1.0f))
            {
                strain_id.SetAntigenID( polio_susceptibility->GetSerotype(&strain_id) );
            }
            AcquireNewInfection(&strain_id);
        }
    }
    else
    {
        // polio does not respond to challenge when currently infected by same type
    } // superinfection of this antigenic type
}

void Kernel::IndividualHumanPolio::ExposeToInfectivity(float dt, const TransmissionGroupMembership_t* transmissionGroupMembership)
{
    IndividualHumanEnvironmental::ExposeToInfectivity(dt, transmissionGroupMembership);
}

void Kernel::IndividualHumanPolio::ReportInfectionState()
{
    IndividualHumanEnvironmental::ReportInfectionState();
    age_most_recent_infection = float(m_age);
}

bool Kernel::IndividualHumanPolio::SetNewInfectionState(InfectionStateChange::_enum inf_state_change)
{
    if ( IndividualHuman::SetNewInfectionState(inf_state_change) )
    {
        // Nothing is currently set in the base function (death and disease clearance are handled directly in the Update function)
    }
    else if (   inf_state_change == InfectionStateChange::PolioParalysis_WPV1  ||
                inf_state_change == InfectionStateChange::PolioParalysis_WPV2  ||
                inf_state_change == InfectionStateChange::PolioParalysis_WPV3  ||
                inf_state_change == InfectionStateChange::PolioParalysis_VDPV1  ||
                inf_state_change == InfectionStateChange::PolioParalysis_VDPV2  ||
                inf_state_change == InfectionStateChange::PolioParalysis_VDPV3  ) 
    {
        // Previously existing infection is now detected
        m_new_infection_state = NewInfectionState::NewlyDetected;

        paralysisVirusTypeMask += 1 << (inf_state_change - InfectionStateChange::PolioParalysis_WPV1);
    }
    else
    {
        return false;
    }
    
    return true;
}

//update infectiousness is handled at Environmental level
//void Kernel::IndividualHumanPolio::UpdateInfectiousness(float dt)
//{
//    IndividualHumanEnvironmental::UpdateInfectiousness(dt);
//}


void Kernel::IndividualHumanPolio::ClearNewInfectionState()
{
    IndividualHumanEnvironmental::ClearNewInfectionState();
    paralysisVirusTypeMask = 0; // Clear the paralysis mask
}

float Kernel::IndividualHumanPolio::GetAgeOfMostRecentInfection(void)
{
    return age_most_recent_infection;
}

Kernel::Infection* Kernel::IndividualHumanPolio::createInfection( suids::suid _suid )
{
    InfectionPolio *ip = InfectionPolio::CreateInfection(this, _suid);
    ip->CacheMCWeightOfHost(float(m_mc_weight));
    paralysisVirusTypeMask = 0; // Clear the paralysis mask
    return ip;
}

#endif // ENABLE_POLIO
