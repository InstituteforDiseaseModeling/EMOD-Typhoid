/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <string>

#ifdef ENABLE_POLIO

#include "Debug.h"
#include "MathFunctions.h"
#include "SusceptibilityPolio.h"
#include "Environment.h"
#include "Types.h"
#include "Interventions.h"
#include "NodeDemographics.h"
#include "Common.h"
#include "Exceptions.h"
#include "PolioVaccine.h"
// clorton #include "PolioInterventionsContainer.h"
#include "RANDOM.h"
#include "SimulationConfig.h"
#include "PolioParameters.h"

static const char * _module = "SusceptibilityPolio";

namespace Kernel
{
    float SusceptibilityPolioConfig::x_population_immunity        = 0.0f;
    float SusceptibilityPolioConfig::mucosalImmIPV                = 0.0f;
    float SusceptibilityPolioConfig::mucosalImmIPVOPVExposed      = 0.0f;
    float SusceptibilityPolioConfig::TauNAb                       = 0.0f;
    float SusceptibilityPolioConfig::paralytic_immunity_titer     = 0.0f;
    float SusceptibilityPolioConfig::waning_humoral_rate_fast     = 0.0f;
    float SusceptibilityPolioConfig::waning_humoral_rate_slow     = 0.0f;
    float SusceptibilityPolioConfig::waning_humoral_fast_fraction = 0.0f;
    float SusceptibilityPolioConfig::waning_mucosal_rate_fast     = 0.0f;
    float SusceptibilityPolioConfig::waning_mucosal_rate_slow     = 0.0f;
    float SusceptibilityPolioConfig::waning_mucosal_fast_fraction = 0.0f;
    float SusceptibilityPolioConfig::acquire_rate_default_polio   = 0.0f;
    float SusceptibilityPolioConfig::maternalAbHalfLife           = 0.0f;
    float SusceptibilityPolioConfig::maternal_log2NAb_mean        = 0.0f;
    float SusceptibilityPolioConfig::maternal_log2NAb_std         = 0.0f;
    float SusceptibilityPolioConfig::excrement_load               = 0.0f;

    GET_SCHEMA_STATIC_WRAPPER_IMPL(Polio.Susceptibility,SusceptibilityPolioConfig)
    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityPolioConfig)
    END_QUERY_INTERFACE_BODY(SusceptibilityPolioConfig)

    bool
    SusceptibilityPolioConfig::Configure(
        const Configuration* config
    )
    {
        LOG_DEBUG( "Configure\n" );
        initConfigTypeMap( "x_Population_Immunity",             &x_population_immunity, x_Population_Immunity_DESC_TEXT, 0.0f, 1.0f, 1.0f );
        initConfigTypeMap( "Mucosal_Immunogenicity_IPV",        &mucosalImmIPV, Mucosal_Immunogenicity_IPV_DESC_TEXT, 0.0f, 100.0f, 3.0f );
        initConfigTypeMap("Mucosal_Immunogenicity_IPV_OPVExposed", &mucosalImmIPVOPVExposed, Mucosal_Immunogenicity_IPV_OPVEXPOSED_DESC_TEXT, 0.0f, 100.0f, 3.0f);
        initConfigTypeMap( "Neutralization_Time_Tau",           &TauNAb, Neutralization_Time_Tau_DESC_TEXT, 0.0f, 1.0f, 0.04f );
        initConfigTypeMap( "Paralytic_Immunity_Titer",          &paralytic_immunity_titer, Paralytic_Immunity_Titer_DESC_TEXT, 0.0f, FLT_MAX, 8.0f );
        initConfigTypeMap( "Waning_Humoral_Rate_Fast",          &waning_humoral_rate_fast, Waning_Humoral_Rate_Fast_DESC_TEXT, 0.0f, 1000.0f, 0.005f );
        initConfigTypeMap( "Waning_Humoral_Rate_Slow",          &waning_humoral_rate_slow, Waning_Humoral_Rate_Slow_DESC_TEXT, 0.0f, 1000.0f, 0.0f );
        initConfigTypeMap( "Waning_Humoral_Fast_Fraction",      &waning_humoral_fast_fraction, Waning_Humoral_Fast_Fraction_DESC_TEXT, 0.0f, 1.0f, 0.5f );
        initConfigTypeMap( "Waning_Mucosal_Rate_Fast",          &waning_mucosal_rate_fast, Waning_Mucosal_Rate_Fast_DESC_TEXT, 0.0f, 1000.0f, 0.005f );
        initConfigTypeMap( "Waning_Mucosal_Rate_Slow",          &waning_mucosal_rate_slow, Waning_Mucosal_Rate_Slow_DESC_TEXT, 0.0f, 1000.0f, 0.0f );
        initConfigTypeMap( "Waning_Mucosal_Fast_Fraction",      &waning_mucosal_fast_fraction, Waning_Mucosal_Fast_Fraction_DESC_TEXT, 0.0f, 1.0f, 0.5f );

        initConfigTypeMap( "Acquire_Rate_Default_Polio", &acquire_rate_default_polio, Acquire_Rate_Default_Polio_DESC_TEXT, 0.0f, 1.0f, 3.7e-006f );

        initConfigTypeMap( "Maternal_log2NAb_mean", &maternal_log2NAb_mean, Maternal_log2NAb_mean_DESC_TEXT,  0.0f, 18.0f, 6.0f );
        initConfigTypeMap( "Maternal_log2NAb_std",  &maternal_log2NAb_std,  Maternal_log2NAb_std_DESC_TEXT,   0.0f, 18.0f, 3.0f );
        initConfigTypeMap( "Maternal_Ab_Halflife",  &maternalAbHalfLife,    Maternal_Ab_Halflife_DESC_TEXT,   0.0f, 1000.0f, 22.0f );

        return JsonConfigurable::Configure( config );
    }

    BEGIN_QUERY_INTERFACE_BODY(SusceptibilityPolio)
        HANDLE_INTERFACE(ISusceptibilityPolio)
        HANDLE_INTERFACE(ISusceptibilityPolioReportable)
    END_QUERY_INTERFACE_BODY(SusceptibilityPolio)

    SusceptibilityPolio::SusceptibilityPolio()
        : SusceptibilityEnvironmental()
        , individual_acquire_risk(1.0f)
    {
        ZERO_ARRAY( shedding_titer );
        ZERO_ARRAY( humoralNAb );
        ZERO_ARRAY( mucosalNAb );
        ZERO_ARRAY( maternalSerumNAb );
        ZERO_ARRAY( humoralMemoryNAb );
        ZERO_ARRAY( mucosalMemoryNAb );
        ZERO_ARRAY( humoral_fastDecayCompartment );
        ZERO_ARRAY( mucosal_fastDecayCompartment );
        ZERO_ARRAY( time_since_last_infection );
        ZERO_ARRAY( time_since_last_IPV );
        ZERO_ARRAY( vaccine_doses_received );
        ZERO_ARRAY( vaccine_doses_received_by_type );
        ZERO_ARRAY( infectionStrains );
        ZERO_ARRAY( newInfectionByStrain );
    }

    SusceptibilityPolio::SusceptibilityPolio(IIndividualHumanContext *context)
        : SusceptibilityEnvironmental(context) 
        , individual_acquire_risk(1.0f)
    {
        ZERO_ARRAY( shedding_titer );
        ZERO_ARRAY( humoralNAb );
        ZERO_ARRAY( mucosalNAb );
        ZERO_ARRAY( maternalSerumNAb );
        ZERO_ARRAY( humoralMemoryNAb );
        ZERO_ARRAY( mucosalMemoryNAb );
        ZERO_ARRAY( humoral_fastDecayCompartment );
        ZERO_ARRAY( mucosal_fastDecayCompartment );
        ZERO_ARRAY( time_since_last_infection );
        ZERO_ARRAY( time_since_last_IPV );
        ZERO_ARRAY( vaccine_doses_received );
        ZERO_ARRAY( vaccine_doses_received_by_type );
        ZERO_ARRAY( infectionStrains );
        ZERO_ARRAY( newInfectionByStrain );
    }

    void SusceptibilityPolio::Initialize(float _age, float _immmod, float _riskmod)
    {
        LOG_DEBUG_F( "Initializing Polio immunity object for new individual: id=%lu, age=%f, immunity modifier=%f, risk modifier=%f\n", parent->GetSuid().data, _age, _immmod, _riskmod );
        SusceptibilityEnvironmental::Initialize(_age, _immmod, _riskmod);

        // throws exception on error, no return type.
        validateStrainSettings();

        age = _age;
        demographic_risk = _riskmod; // takes values from 0 to 1, demographic_risk = 1+sanitation*(minrisk - 1), (sanitation = fraction of households with latrine)

        release_assert( age >= 0);

        if (age == 0)
        {
            if(GET_CONFIGURABLE(SimulationConfig)->immunity_initialization_distribution_type == DistributionType::DISTRIBUTION_COMPLEX )
            {
                float randdraw = randgen->e();
                maternalSerumNAb[0] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::maternal_antibody_distribution1  )->DrawFromDistribution(randdraw);
                maternalSerumNAb[1] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::maternal_antibody_distribution2  )->DrawFromDistribution(randdraw);
                maternalSerumNAb[2] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::maternal_antibody_distribution3  )->DrawFromDistribution(randdraw);
                for(int i_serotype = 0; i_serotype < N_POLIO_SEROTYPES; i_serotype++) 
                {
                    maternalSerumNAb[i_serotype] = pow(2.0f, maternalSerumNAb[i_serotype]);
                }

            }
            else
            {
                for(int i = 0; i < N_POLIO_SEROTYPES; i++)
                {
                    // immune_response_OPV.xls, from Ogra1968, Warren1964, Dexiang1956
                    float exp =  GET_CONFIGURABLE(SimulationConfig)->polio_params->maternal_log2NAb_mean
                              + (GET_CONFIGURABLE(SimulationConfig)->polio_params->maternal_log2NAb_std * GetRandNBounded()) ;
                    maternalSerumNAb[i] = pow( 2.0f, exp );
                }
            }
            return;
        }

        if(GET_CONFIGURABLE(SimulationConfig)->immunity_initialization_distribution_type == DistributionType::DISTRIBUTION_COMPLEX)
        {

            float  randdraw = randgen->e(); // get a new random number for next item. use the same random number for each serotype to assume they all increase in a correlated fashion with increase number of doses
            mucosalMemoryNAb[0] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::mucosal_memory_distribution1  )->DrawFromDistribution(randdraw, age);
            mucosalMemoryNAb[1] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::mucosal_memory_distribution2  )->DrawFromDistribution(randdraw, age);
            mucosalMemoryNAb[2] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::mucosal_memory_distribution3  )->DrawFromDistribution(randdraw, age);

            // randdraw = randgen->e(); // keeping same random number from mucosal draw
            humoralMemoryNAb[0] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::humoral_memory_distribution1  )->DrawFromDistribution(randdraw, age, mucosalMemoryNAb[0]); // obtain the joint distribution for humoral and mucosal by two random draws. one from the marginal and second from the conditional
            humoralMemoryNAb[1] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::humoral_memory_distribution2  )->DrawFromDistribution(randdraw, age, mucosalMemoryNAb[1]);
            humoralMemoryNAb[2] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::humoral_memory_distribution3  )->DrawFromDistribution(randdraw, age, mucosalMemoryNAb[2]);

            randdraw = randgen->e();
            maternalSerumNAb[0] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::maternal_antibody_distribution1  )->DrawFromDistribution(randdraw);
            maternalSerumNAb[1] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::maternal_antibody_distribution2  )->DrawFromDistribution(randdraw);
            maternalSerumNAb[2] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::maternal_antibody_distribution3  )->DrawFromDistribution(randdraw);
            
            
            // assume no correlation of time of infection across serotypes
            for (int i_serotype = 0; i_serotype < N_POLIO_SEROTYPES; i_serotype++)
            {
            time_since_last_infection[i_serotype] = parent->GetDemographicsDistribution(NodeDemographicsDistribution::fake_time_since_last_infection_distribution ) ->DrawFromDistribution(randgen->e(), age);
            NO_LESS_THAN( time_since_last_infection[i_serotype], 0.0f);
            }

            for(int i_serotype = 0; i_serotype < N_POLIO_SEROTYPES; i_serotype++) // convert from log2 NAb to linear NAb
            {
                mucosalMemoryNAb[i_serotype] = pow(2.0f, mucosalMemoryNAb[i_serotype]);
                mucosalNAb[i_serotype] = mucosalMemoryNAb[i_serotype];

                humoralMemoryNAb[i_serotype] = pow(2.0f, humoralMemoryNAb[i_serotype]);
                humoralNAb[i_serotype] = humoralMemoryNAb[i_serotype];

                maternalSerumNAb[i_serotype] = pow(2.0f, maternalSerumNAb[i_serotype]);
                maternalSerumNAb[i_serotype] *= exp(-age * GET_CONFIGURABLE(SimulationConfig)->polio_params->decayRatePassiveImmunity); // get maternal antibody level at present age of individual
            }
             
            waneAntibodyTitersFromLastEvent(); // wane mucosal and humoral from memory based on time since last infection
            ZERO_ARRAY(time_since_last_infection); //zero out so only count "real" infections in DTK not fake initialized ones

        }
        else
        {   /*Setting to have zero initial immunity humoral, mucosal or maternal */
            for(int i = 0; i < N_POLIO_SEROTYPES; i++)
            {
                maternalSerumNAb[i] = pow(2.0f, GetRandNBounded() * GET_CONFIGURABLE(SimulationConfig)->polio_params->maternal_log2NAb_std 
                                                                  + GET_CONFIGURABLE(SimulationConfig)->polio_params->maternal_log2NAb_mean); // immune_response_OPV.xls, from Ogra1968, Warren1964, Dexiang1956
                
                maternalSerumNAb[i] *= exp(-age * GET_CONFIGURABLE(SimulationConfig)->polio_params->decayRatePassiveImmunity); // get maternal antibody level at present age of individual
            }
        }
#if 0
        // This block was actually really useful so leaving it here deliberately as 'dead code'.
        if( getenv( "INIT_HUMORAL_NAB" ) )
        {
            humoralNAb[0] = atof( getenv( "INIT_HUMORAL_NAB" ) );
            mucosalNAb[0] = atof( getenv( "INIT_HUMORAL_NAB" ) );
        }
#endif
        for( int i=0; i<N_POLIO_SEROTYPES; i++ )
        {
            LOG_DEBUG_F( "Individual %lu initialized with humoralNAb[%d] = %f\n", parent->GetSuid().data, i, humoralNAb[i] );
            LOG_DEBUG_F( "Individual %lu initialized with mucosalNAb[%d] = %f\n", parent->GetSuid().data, i, mucosalNAb[i] );
        }
    }

    SusceptibilityPolio::~SusceptibilityPolio(void)
    {
    }

    SusceptibilityPolio *SusceptibilityPolio::CreateSusceptibility(IIndividualHumanContext *context, float age, float immmod, float riskmod)
    {
        //LOG_DEBUG_F( "Creating Polio immunity object for new individual: age=%f, immunity modifier=%f, risk modifier=%f\n", age, immmod, riskmod );
        SusceptibilityPolio *newsusceptibility = _new_ SusceptibilityPolio(context);
        newsusceptibility->Initialize(age, immmod, riskmod);

        return newsusceptibility;
    }

    void SusceptibilityPolio::GetSheddingTiter(float sheddingTiters[]) const
    {
        memcpy(sheddingTiters, shedding_titer, sizeof(shedding_titer));
    }

    void SusceptibilityPolio::GetHumoralNAb(float receiveHumoralNAb[]) const
    {
        memcpy(receiveHumoralNAb, humoralNAb, sizeof(humoralNAb));
    }

    void SusceptibilityPolio::GetMucosalNAb(float receiveMucosalNAb[]) const
    {
        memcpy(receiveMucosalNAb, mucosalNAb, sizeof(mucosalNAb));
    }

    void SusceptibilityPolio::GetMaternalSerumNAb(float receiveMaternalSerumNAb[]) const
    {
        memcpy(receiveMaternalSerumNAb, maternalSerumNAb, sizeof(maternalSerumNAb));
    }

    void SusceptibilityPolio::GetHumoralMemoryNAb(float receiveHumoralMemoryNAb[]) const
    {
        memcpy(receiveHumoralMemoryNAb, humoralMemoryNAb, sizeof(humoralMemoryNAb));
    }

    void SusceptibilityPolio::GetMucosalMemoryNAb(float receiveMucosalMemoryNAb[]) const
    {
        memcpy(receiveMucosalMemoryNAb, mucosalMemoryNAb, sizeof(mucosalMemoryNAb));
    }

    void SusceptibilityPolio::GetTimeSinceLastInfection(float times[]) const
    {
        memcpy(times, time_since_last_infection, sizeof(time_since_last_infection));
    }

    void SusceptibilityPolio::GetTimeSinceLastIPV(float times[]) const
    {
        memcpy(times, time_since_last_IPV, sizeof(time_since_last_IPV));
    }

    void SusceptibilityPolio::GetVaccineDosesReceived(int vaccineDosesReceived[]) const
    {
        memcpy(vaccineDosesReceived, vaccine_doses_received, sizeof(vaccine_doses_received));
    }

    float SusceptibilityPolio:: GetIndividualAcquireRisk() const
    {
        return individual_acquire_risk;
    }

    void SusceptibilityPolio::Update(float dt)
    {
        age += dt; // tracks age for immune purposes

        updateMemoryImmunity();

        float delta_slow_Ab, delta_fast_Ab;

        for(int i = 0; i < N_POLIO_SEROTYPES; i++)
        {
            maternalSerumNAb[i] -= maternalSerumNAb[i] * dt * GET_CONFIGURABLE(SimulationConfig)->polio_params->decayRatePassiveImmunity;
            
            delta_slow_Ab                   = (humoralNAb[i] - humoral_fastDecayCompartment[i]) * dt * SusceptibilityPolioConfig::waning_humoral_rate_slow;
            delta_fast_Ab                   = humoral_fastDecayCompartment[i] * dt * SusceptibilityPolioConfig::waning_humoral_rate_fast;
            humoral_fastDecayCompartment[i]    -= delta_fast_Ab;
            humoralNAb[i]                      -= delta_slow_Ab + delta_fast_Ab;

            delta_slow_Ab                   = (mucosalNAb[i] - mucosal_fastDecayCompartment[i]) * dt * SusceptibilityPolioConfig::waning_mucosal_rate_slow;
            delta_fast_Ab                   = mucosal_fastDecayCompartment[i] * dt * SusceptibilityPolioConfig::waning_mucosal_rate_fast;
            mucosal_fastDecayCompartment[i]    -= delta_fast_Ab;
            mucosalNAb[i]                      -= delta_slow_Ab + delta_fast_Ab;
            
            time_since_last_infection[i]    += dt;
            time_since_last_IPV[i]          += dt;
        }

        /*if(GET_CONFIGURABLE(SimulationConfig)->track_interventions)
        {
        // update_intervention_status(dt); // Polio uses ApplyInterventionsToIndividual instead
        //L_update_intervention_effects();
        }*/
    }

    void SusceptibilityPolio::UpdateInfectionCleared()
    {
        // nothing special to do in polio immune model when infections are cleared
    }

    void SusceptibilityPolio::updateMemoryImmunity(void)
    {
        for(int i_serotype = 0; i_serotype < N_POLIO_SEROTYPES; i_serotype++)
        {
            NO_LESS_THAN( humoralMemoryNAb[i_serotype], humoralNAb[i_serotype] )
            NO_LESS_THAN( mucosalMemoryNAb[i_serotype], mucosalNAb[i_serotype] )
        }
    }

    void SusceptibilityPolio::waneAntibodyTitersFromLastEvent()
    {
        NonNegativeFloat tmp_time;
        NonNegativeFloat tmp_slow_Ab;
        NonNegativeFloat k_hum_fast = SusceptibilityPolioConfig::waning_humoral_fast_fraction;
        NonNegativeFloat k_muc_fast = SusceptibilityPolioConfig::waning_mucosal_fast_fraction;

        for(int i_serotype = 0; i_serotype < N_POLIO_SEROTYPES; i_serotype++)
        {
            tmp_time                                 = time_since_last_infection[i_serotype];
            humoral_fastDecayCompartment[i_serotype] = k_hum_fast * humoralMemoryNAb[i_serotype];
            mucosal_fastDecayCompartment[i_serotype] = k_muc_fast * mucosalMemoryNAb[i_serotype];

            tmp_slow_Ab                                 = (humoralMemoryNAb[i_serotype] - humoral_fastDecayCompartment[i_serotype]) * exp(-tmp_time * SusceptibilityPolioConfig::waning_humoral_rate_slow);
            humoral_fastDecayCompartment[i_serotype]   *= exp(-tmp_time * SusceptibilityPolioConfig::waning_humoral_rate_fast);
            humoralNAb[i_serotype]                      = tmp_slow_Ab + humoral_fastDecayCompartment[i_serotype];

            tmp_slow_Ab                                 = (mucosalMemoryNAb[i_serotype] - mucosal_fastDecayCompartment[i_serotype]) * exp(-tmp_time * SusceptibilityPolioConfig::waning_mucosal_rate_slow);
            mucosal_fastDecayCompartment[i_serotype]   *= exp(-tmp_time * SusceptibilityPolioConfig::waning_mucosal_rate_fast);
            mucosalNAb[i_serotype]                      = tmp_slow_Ab + mucosal_fastDecayCompartment[i_serotype];
        }
    }

    void SusceptibilityPolio::GetProbabilityInfected(StrainIdentity* strain_id, float* acquired_virus, int n_challenge_strains, float* probability_infected)
    {
        if( !n_challenge_strains )
        {
            return;
        }

        float challenge_dose[N_POLIO_VIRUS_TYPES] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // range WPV1 - VRPV3
        float heterotypic_take_rates[N_POLIO_VIRUS_TYPES];
        float vaccine_take_multiplier[N_POLIO_VIRUS_TYPES];
        StrainIdentity acquired_strains[N_POLIO_VIRUS_TYPES] = {
                StrainIdentity(PolioVirusTypes::WPV1, 0),
                StrainIdentity(PolioVirusTypes::WPV2, 0),
                StrainIdentity(PolioVirusTypes::WPV3, 0),
                GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_strains[0],
                GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_strains[1],
                GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_strains[2]
            };

        // set up array of all challenge viruses on this exposure
        for(int i_challenge = 0; i_challenge < n_challenge_strains; i_challenge++)
        {
            int virustype = strain_id[i_challenge].GetAntigenID();
            challenge_dose[virustype] = acquired_virus[i_challenge];
            LOG_DEBUG_F( "challenge_dose[%d] = (acquired_virions) = %e\n", virustype, challenge_dose[virustype] );
            acquired_strains[virustype] = strain_id[i_challenge];
        }
        // determine cross serotype interferences and the multiplier for vaccine take (by sabin virus type)
        for(int virustype = 0; virustype < N_POLIO_VIRUS_TYPES; virustype++)
        {
            // ---------------------------------------------------------------
            // --- This is the old equation dealing with vaccine interference
            // --- between different types, prior to Fall 2014
            // ---------------------------------------------------------------
            // --- float prob_infec = getProbabilityInfectionSingleStrain(&acquired_strains[virustype], challenge_dose[virustype])
            // --- heterotypic_take_rates[virustype] = (shedding_titer[virustype] > 0.0f) ||
            // ---                                     ( (challenge_dose[virustype] > 0) && (randgen->e() < prob_infec) );
            // ---------------------------------------------------------------
            // boolean factor in interference, will an infection of this virus type exist to provide interference?
            heterotypic_take_rates[virustype] = (shedding_titer[virustype] > 0.0f) || (challenge_dose[virustype] > 0 );

            LOG_DEBUG_F( "shedding_titer[%d] = %f, heterotypic_take_rates[%d] = %f\n", virustype, shedding_titer[virustype], virustype, heterotypic_take_rates[virustype] );

            heterotypic_take_rates[virustype] *= GET_CONFIGURABLE(SimulationConfig)->polio_params->viral_interference[virustype]; // virus-specific interference rate contribution

            LOG_DEBUG_F( "(*viral_int) heterotypic_take_rates[%d] = %f\n", virustype, heterotypic_take_rates[virustype] );

            // if the geneticID affects viral interference rates, then it modulates them here!
            heterotypic_take_rates[virustype] = 1.0f - heterotypic_take_rates[virustype]; // change from interference rate to take rate

            LOG_DEBUG_F( "(1-x) heterotypic_take_rates[%d] = %f\n", virustype, heterotypic_take_rates[virustype] );

            //load the vaccine take multiplier [0,1], fixed to 1 for all wild virus
            vaccine_take_multiplier[virustype] = GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_take_multiplier[virustype];
        }
        // determine take rate for each challenge virus contributed by viral interference
        for(int i_challenge = 0; i_challenge < n_challenge_strains; i_challenge++)
        {
            int virustype = strain_id[i_challenge].GetAntigenID();

            // viral interference component of take rate, this is the non-interference rate, not the interference rate
            NonNegativeFloat vi_take_rate = 1.0f;

            LOG_DEBUG_F("vi_take_rate for type %d before = %f\n", virustype, (float) vi_take_rate);
            for(int i_type = 0; i_type < N_POLIO_VIRUS_TYPES; i_type++)
            {
                if(i_type != virustype)
                {
                    LOG_DEBUG_F("vi_take_rate * %f for type %d = %f\n", heterotypic_take_rates[i_type], i_type, (float) vi_take_rate);
                    vi_take_rate *= heterotypic_take_rates[i_type];
                }
            }
            LOG_DEBUG_F("vi_take_rate for type %d after = %f\n", virustype, (float) vi_take_rate);

            probability_infected[i_challenge] = vaccine_take_multiplier[virustype] *  vi_take_rate * getProbabilityInfectionSingleStrain(&strain_id[i_challenge], challenge_dose[virustype]);
            LOG_DEBUG_F( "vi_take_rate = %f, challenge dose = %f, probability_infected[%d] = %f\n", float(vi_take_rate), challenge_dose[virustype], i_challenge, probability_infected[i_challenge] );
            /*if( probability_infected[i_challenge] == 0 )
            {
                LOG_DEBUG("prob[i] = 0: vi_take_rate = %f, probInfSingleStrain = %f\n", vi_take_rate, getProbabilityInfectionSingleStrain(&strain_id[i_challenge], challenge_dose[virustype]));
            }*/
        }
    }

    float SusceptibilityPolio::getProbabilityInfectionSingleStrain(StrainIdentity* strain_id, float challenge_dose)
    {
        float specificInfectivity = 0.0f;
        int virus_type = strain_id->GetAntigenID();
        if( (virus_type < 0) || (N_POLIO_VIRUS_TYPES <= virus_type) )
        {
            throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "virus_type", virus_type, PolioVirusTypes::pairs::lookup_key( virus_type ) );
        }
        else if( IS_SABIN_TYPE( virus_type ) && GET_CONFIGURABLE(SimulationConfig)->polio_params->VDPV_virulence_model_type >= VDPVVirulenceModelType::POLIO_VDPV_PARALYSIS_AND_LOG_INFECTIVITY) // vaccine-related virus
        {
            // convert sabin strain id to wild strain id equivalent without using fragile math
            // doing this "inline" to save overhead of function call, and coz this is the only place we do this.
            int wild_strain = 0;
            switch( virus_type)
            {
                case PolioVirusTypes::VRPV1:
                wild_strain = PolioVirusTypes::WPV1; // look up wild strain of the same serotype
                break;

                case PolioVirusTypes::VRPV2:
                wild_strain = PolioVirusTypes::WPV2; // look up wild strain of the same serotype
                break; 
            
                case PolioVirusTypes::VRPV3:
                wild_strain = PolioVirusTypes::WPV3; // look up wild strain of the same serotype
                break; 

                default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "virus_type", virus_type, PolioVirusTypes::pairs::lookup_key( virus_type ) );
            }
            
            specificInfectivity = GET_CONFIGURABLE(SimulationConfig)->polio_params->PVinf0[virus_type] * pow( GET_CONFIGURABLE(SimulationConfig)->polio_params->PVinf0[wild_strain] / GET_CONFIGURABLE(SimulationConfig)->polio_params->PVinf0[virus_type], GetReversionDegree(strain_id) );
        }
        else if( (0 <= virus_type) && (virus_type < N_POLIO_VIRUS_TYPES) )  // redundant check here to get rid of code analysis warning C638
        {
            specificInfectivity = GET_CONFIGURABLE(SimulationConfig)->polio_params->PVinf0[virus_type];
        }

        float NAb               = GetMucosalImmunity(strain_id);
        float neutralization    = (1.0f + GET_CONFIGURABLE(SimulationConfig)->polio_params->TauNAb * (NAb - 1.0f) * (1.0f - exp(-1.0f / GET_CONFIGURABLE(SimulationConfig)->polio_params->TauNAb))) / NAb;
        //float inf_beta          = GET_CONFIGURABLE(SimulationConfig)->InfectBeta;
        ProbabilityNumber ret = 1.0f - pow( 1.0f + (challenge_dose), -specificInfectivity * neutralization ); // Beta-Poisson infection model, where alpha is specificInfectivity and reduced by the factor of neutralizing antibodies
        LOG_DEBUG_F("challengedose=%f, specificInfectivity=%f, neutralization=%f, probability of infection = %f.\n", challenge_dose, specificInfectivity, neutralization, (float) ret);
        return ret;
    }

    void SusceptibilityPolio::GetNewInterventionsForIndividual(float dt, int* n_vaccine_strains, StrainIdentity* strain_id, float* dose_titer) // n_strains
    {
        using namespace std;
        // Polio does interventions here instead of update_intervention_status()
        // dose titer must be a 3-element array
        // strain_id must be a 3-element array, this argument always returns the same thing - the OPV (Sabin) vacine_strain IDs
        /*float drug_vaccine_mod_acquire = 1.0f;
        float drug_vaccine_mod_transmit = 1.0f;*/
        *n_vaccine_strains = 3;
        strain_id[0] = GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_strains[PolioSerotypes::PV1];
        strain_id[1] = GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_strains[PolioSerotypes::PV2];
        strain_id[2] = GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_strains[PolioSerotypes::PV3];
        dose_titer[0] = 0.0f;
        dose_titer[1] = 0.0f;
        dose_titer[2] = 0.0f;


        float temp_IPV_Dantigen[N_POLIO_SEROTYPES] = {0.0f, 0.0f, 0.0f};
        // assume bOPV has equal efficacy to the sum of mOPV1 and mOPV3
        // Vaccines
        // Polio only cares when a new vaccine dose has been given - durability does not apply to Polio vaccines
        // Polio should set the durability of all vaccines to 1 day. This will eliminate unnecessary iteration through the following loops.
        // Use GetParent, and then intervention interfaces here.
        IIndividualHumanContext* vaccinee = nullptr;
        IIndividualHumanInterventionsContext* context = nullptr;
        IPolioVaccineEffects* ipve = nullptr;

        vaccinee = GetParent();

        context = vaccinee->GetInterventionsContext();
        if (s_OK == context->QueryInterface(GET_IID(IPolioVaccineEffects), (void**)&ipve) )
        {
            int new_vaccine_index=0;
            std::list<IPolioVaccine*> new_vaccines = ipve->GetNewVaccines();
            for (auto vaccine : new_vaccines)   // add up vaccine strength in case multiple vaccines are delivered on same day
            {
                new_vaccine_index = vaccine->GetVaccineType();
                vaccine_doses_received[new_vaccine_index]++;
                // This is a duplicate block of code. Also has some magic number issues, but presumably will be blown away/consolidated soon.
                switch (new_vaccine_index)
                {
                    case PolioVaccines::TOPV:
                        dose_titer[PolioSerotypes::PV1] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_titer_tOPV[0];
                        dose_titer[PolioSerotypes::PV2] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_titer_tOPV[1];
                        dose_titer[PolioSerotypes::PV3] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_titer_tOPV[2];
                        vaccine_doses_received_by_type[PolioSerotypes::PV1]++;
                        vaccine_doses_received_by_type[PolioSerotypes::PV2]++;
                        vaccine_doses_received_by_type[PolioSerotypes::PV3]++;
                        break;

                    case PolioVaccines::BOPV:
                        dose_titer[PolioSerotypes::PV1] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_titer_bOPV[0];
                        dose_titer[PolioSerotypes::PV3] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_titer_bOPV[1];
                        vaccine_doses_received_by_type[PolioSerotypes::PV1]++;
                        vaccine_doses_received_by_type[PolioSerotypes::PV3]++;
                    break;

                    case PolioVaccines::MOPV1:
                        dose_titer[PolioSerotypes::PV1] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_titer_mOPV[0];
                        vaccine_doses_received_by_type[PolioSerotypes::PV1]++;
                        break;

                    case PolioVaccines::MOPV2:
                        dose_titer[PolioSerotypes::PV2] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_titer_mOPV[1];
                        vaccine_doses_received_by_type[PolioSerotypes::PV2]++;
                        break;

                    case PolioVaccines::MOPV3:
                        dose_titer[PolioSerotypes::PV3] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_titer_mOPV[2];
                        vaccine_doses_received_by_type[PolioSerotypes::PV3]++;
                        break;

                    case PolioVaccines::IPV:
                        temp_IPV_Dantigen[PolioSerotypes::PV1] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_Dantigen_IPV[0];
                        temp_IPV_Dantigen[PolioSerotypes::PV2] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_Dantigen_IPV[1];
                        temp_IPV_Dantigen[PolioSerotypes::PV3] += GET_CONFIGURABLE(SimulationConfig)->polio_params->vaccine_Dantigen_IPV[2];
                        break;

                    default:
                        //ERROR: ("Invalid Polio vaccine type %d\n", new_vaccine_index);
                        throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "new_vaccine_index", new_vaccine_index, PolioVaccineType::pairs::lookup_key( new_vaccine_index ) );
                }
            }

            if ( !new_vaccines.empty() ) 
            {
                challengeIPV(temp_IPV_Dantigen);

                // now that the titers etc. have been calculated, the list of new vaccines can be reset
                ipve->ClearNewVaccines();
            }
        }
        else
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IPolioVaccineEffects", "IIndividualHumanInterventionsContext" );
        }
    }

#define REFERENCE_EIPV_DANTIGEN_PV1 (40.0f)
#define REFERENCE_EIPV_DANTIGEN_PV2 (8.0f)
#define REFERENCE_EIPV_DANTIGEN_PV3 (32.0f)

    float SusceptibilityPolio::ReferenceEIPV_Dantigen[] = {
        REFERENCE_EIPV_DANTIGEN_PV1,
        REFERENCE_EIPV_DANTIGEN_PV2,
        REFERENCE_EIPV_DANTIGEN_PV3
    }; // D-antigen units of enhanced IPV. Reference antigen content for immunogenicity parameter estimation.

    void SusceptibilityPolio::challengeIPV(float* dose_Dantigen_content)
    {
        float log2NAb_boost, NAb_saturation, log2NAb_prime;
        float delta_humoral, delta_mucosal,  new_fast_Ab;

        for(int i_serotype = 0; i_serotype < N_POLIO_SEROTYPES; i_serotype++)
        {
            float mucosalImmIPVmultiplier;
            if (isOPVExposed(i_serotype))
            {
                mucosalImmIPVmultiplier = GET_CONFIGURABLE(SimulationConfig)->polio_params->mucosalImmIPVOPVExposed;
            }
            else
            {
                mucosalImmIPVmultiplier = GET_CONFIGURABLE(SimulationConfig)->polio_params->mucosalImmIPV;
            }

            LOG_DEBUG_F("mucosal immunity IPV multiplier = %f\n.", mucosalImmIPVmultiplier);

            if(dose_Dantigen_content[i_serotype] > 0.0f)
            {
                if(humoralMemoryNAb[i_serotype] > 1.0f ||  isOPVExposed(i_serotype)) //including individuals who failed to seroconvert due to OPV
                {
                    log2NAb_boost   = GET_CONFIGURABLE(SimulationConfig)->polio_params->boostLog2NAb_IPV[i_serotype] + (GetRandNBounded() * GET_CONFIGURABLE(SimulationConfig)->polio_params->boostLog2NAb_stddev_IPV[i_serotype]);
                    log2NAb_boost  *= log(dose_Dantigen_content[i_serotype] + 1.0f) / log(ReferenceEIPV_Dantigen[i_serotype] + 1.0f);
                    
                    NAb_saturation          = LOG_2 - log(mucosalMemoryNAb[i_serotype]) / GET_CONFIGURABLE(SimulationConfig)->polio_params->maxLog2NAb_IPV[i_serotype];
                    NO_LESS_THAN(NAb_saturation, 0.0f);
                    mucosalNAb[i_serotype] = mucosalMemoryNAb[i_serotype] * exp(log2NAb_boost * NAb_saturation * mucosalImmIPVmultiplier);

                    delta_mucosal           = mucosalMemoryNAb[i_serotype] - mucosalNAb[i_serotype];

                    NAb_saturation          = LOG_2 - log(humoralMemoryNAb[i_serotype]) / GET_CONFIGURABLE(SimulationConfig)->polio_params->maxLog2NAb_IPV[i_serotype];
                    NO_LESS_THAN( NAb_saturation, 0.0f )
                    humoralNAb[i_serotype]  = humoralMemoryNAb[i_serotype] * exp( log2NAb_boost * NAb_saturation );
                    delta_humoral           = humoralMemoryNAb[i_serotype] - humoralNAb[i_serotype];
                }
                else
                {
                    log2NAb_prime  = GET_CONFIGURABLE(SimulationConfig)->polio_params->primeLog2NAb_IPV[i_serotype] + (GetRandNBounded() * GET_CONFIGURABLE(SimulationConfig)->polio_params->primeLog2NAb_stddev_IPV[i_serotype]);
                    log2NAb_prime *= log(dose_Dantigen_content[i_serotype] + 1.0f) / log(ReferenceEIPV_Dantigen[i_serotype] + 1.0f);

                    mucosalNAb[i_serotype] = pow(2.0f, mucosalImmIPVmultiplier * log2NAb_prime);
                    humoralNAb[i_serotype] = pow( 2.0f, log2NAb_prime );

                    delta_mucosal = mucosalNAb[i_serotype];
                    delta_humoral = humoralNAb[i_serotype];
                }

                //    if the increase in NAb is greater than the NAb that would be assigned to fast compartment, then assign to fast compartment, else leave fast compartment as is
                new_fast_Ab     = mucosalNAb[i_serotype] * SusceptibilityPolioConfig::waning_mucosal_fast_fraction;
                if( delta_mucosal > new_fast_Ab )
                    mucosal_fastDecayCompartment[i_serotype]  = new_fast_Ab;

                 new_fast_Ab    = humoralNAb[i_serotype] * SusceptibilityPolioConfig::waning_humoral_fast_fraction;
                if( delta_humoral > new_fast_Ab )
                    humoral_fastDecayCompartment[i_serotype]  = new_fast_Ab;
           }
        }
    }

    bool SusceptibilityPolio::isOPVExposed(int serotype)
    {
        LOG_DEBUG_F("For serotype %d, the number of exposed doses is %d.\n", serotype, vaccine_doses_received_by_type[serotype]);
        if (vaccine_doses_received_by_type[serotype] > 0)
            return true;
        else
            return false;
    }

    void SusceptibilityPolio::ApplyImmumeResponseToInfection(StrainIdentity* infection_strain)
    {
        int serotype = GetSerotype(infection_strain);

        if(mucosalMemoryNAb[serotype] > 1.0f) // boosting
        {
            float log2NAb_boost = GET_CONFIGURABLE(SimulationConfig)->polio_params->boostLog2NAb_OPV[serotype] + (GetRandNBounded() * GET_CONFIGURABLE(SimulationConfig)->polio_params->boostLog2NAb_stddev_OPV[serotype]);

            float NAb_saturation = LOG_2 - log(mucosalMemoryNAb[serotype]) / GET_CONFIGURABLE(SimulationConfig)->polio_params->maxLog2NAb_OPV[serotype];
            NO_LESS_THAN( NAb_saturation, 0.0f ) // antibody saturation scaling of immune response. scaling = 0 if antibodies are greater than maximum antibody titer
            mucosalNAb[serotype] = mucosalMemoryNAb[serotype] * exp( log2NAb_boost * NAb_saturation );

            NAb_saturation       = LOG_2 - log(humoralMemoryNAb[serotype]) / GET_CONFIGURABLE(SimulationConfig)->polio_params->maxLog2NAb_OPV[serotype];
            NO_LESS_THAN( NAb_saturation, 0.0f ) // antibody saturation scaling of immune response. scaling = 0 if antibodies are greater than maximum antibody titer

            humoralNAb[serotype] = humoralMemoryNAb[serotype] * exp( log2NAb_boost * NAb_saturation );
            LOG_DEBUG_F("boosting for type %d, final humoral titer is %f\n.", serotype, humoralNAb[serotype]);

            // log(NAb2) = log(NAb1) + boost*(log(2) - log(NAb)/maxLog2NAb)  is equivalent to log2(NAb2) = log2(NAb1) + boost*(1 - log2(NAb)/maxLog2NAb) 
        }
        else // priming
        {
            mucosalNAb[serotype] = pow (2.0f, GET_CONFIGURABLE(SimulationConfig)->polio_params->primeLog2NAb_OPV[serotype] + (GetRandNBounded() * GET_CONFIGURABLE(SimulationConfig)->polio_params->primeLog2NAb_stddev_OPV[serotype]) );
            NO_LESS_THAN( mucosalNAb[serotype], 1.0f ) // check result, minimum titer is 1 
                // check we haven't exceeded the maximum titer this is a config parameter use thing so only throw warning
            if ( log(mucosalNAb[serotype])/LOG_2 > GET_CONFIGURABLE(SimulationConfig)->polio_params->maxLog2NAb_OPV[serotype] )
            {
                //float tmp_max_titer = exp(LOG_2*GET_CONFIGURABLE(SimulationConfig)->maxLog2NAb_OPV[serotype]);
                throw CalculatedValueOutOfRangeException( __FILE__, __LINE__, __FUNCTION__, "log(mucosalNAb[serotype])/LOG_2", log(mucosalNAb[serotype])/LOG_2, GET_CONFIGURABLE(SimulationConfig)->polio_params->maxLog2NAb_OPV[serotype] );
            }

            NO_LESS_THAN(humoralNAb[serotype], mucosalNAb[serotype]);
            LOG_DEBUG_F("priming for type %d, final humoral titer is %f\n.", serotype, humoralNAb[serotype]);
        }
        
        humoral_fastDecayCompartment[serotype]  = humoralNAb[serotype] * SusceptibilityPolioConfig::waning_humoral_fast_fraction; // reset fast decay compartment
        mucosal_fastDecayCompartment[serotype]  = mucosalNAb[serotype] * SusceptibilityPolioConfig::waning_mucosal_fast_fraction; // reset fast decay compartment
    }

    float SusceptibilityPolio::GetFecalInfectiousDuration(StrainIdentity* strain_id)
    {
        float shed_LnDuration = (GetRandNBounded() * GET_CONFIGURABLE(SimulationConfig)->polio_params->shedFecalMaxLnDuration_Stddev) + GET_CONFIGURABLE(SimulationConfig)->polio_params->shedFecalMaxLnDuration * (1.0f - log(GetMucosalImmunity(strain_id)) / (GET_CONFIGURABLE(SimulationConfig)->polio_params->shedFecalDurationBlockLog2NAb * LOG_2));
        return exp(shed_LnDuration);
    }

    float SusceptibilityPolio::GetOralInfectiousDuration(StrainIdentity* strain_id)
    {
        float shed_LnDuration = (GetRandNBounded() * GET_CONFIGURABLE(SimulationConfig)->polio_params->shedOralMaxLnDuration_Stddev) + GET_CONFIGURABLE(SimulationConfig)->polio_params->shedOralMaxLnDuration * (1.0f - log(GetMucosalImmunity(strain_id)) / (GET_CONFIGURABLE(SimulationConfig)->polio_params->shedOralDurationBlockLog2NAb * LOG_2));
        return exp(shed_LnDuration);
    }

    float SusceptibilityPolio::GetPeakFecalLog10VirusTiter(StrainIdentity* strain_id)
    {
        float rand = GetRandNBounded();
        float stddev = GET_CONFIGURABLE(SimulationConfig)->polio_params->shedFecalMaxLog10PeakTiter_Stddev;
        float numerator = GET_CONFIGURABLE(SimulationConfig)->polio_params->shedFecalMaxLog10PeakTiter;
        float denom = GET_CONFIGURABLE(SimulationConfig)->polio_params->shedFecalTiterBlockLog2NAb * LOG_2;
        float immun_factor = log(GetMucosalImmunity(strain_id));

        float meanlog10titer = ( numerator * (1.0f - immun_factor / denom) ); 
        NO_LESS_THAN(meanlog10titer, 0);
        float randomdraw = (rand * stddev);
        NO_LESS_THAN(randomdraw, -1*meanlog10titer);
        NonNegativeFloat log10titer = randomdraw + meanlog10titer;

        LOG_DEBUG_F( "log10titer = %f = rand(%f) + stddev(%f) + ( %f * ( 1.0f - %f / %f ) )\n", float(log10titer), rand, stddev, numerator, immun_factor, denom );
        return float(log10titer);
    }

    float SusceptibilityPolio::GetPeakOralLog10VirusTiter(StrainIdentity* strain_id)
    {
        float log10titer = (GetRandNBounded() * GET_CONFIGURABLE(SimulationConfig)->polio_params->shedOralMaxLog10PeakTiter_Stddev) + GET_CONFIGURABLE(SimulationConfig)->polio_params->shedOralMaxLog10PeakTiter * (1.0f - log(GetMucosalImmunity(strain_id)) / (GET_CONFIGURABLE(SimulationConfig)->polio_params->shedOralTiterBlockLog2NAb * LOG_2));
        return log10titer;
    }

    float SusceptibilityPolio::GetRandNBounded()
    {
        float rn = randgen->eGauss();

        // Would use NO_LESS_THAN/UPPER_THRESH but want the 'else' in here for optimization
        if(rn < -GET_CONFIGURABLE(SimulationConfig)->polio_params->MaxRandStdDev)
        {
            rn = -GET_CONFIGURABLE(SimulationConfig)->polio_params->MaxRandStdDev;
        }
        else if(rn >  GET_CONFIGURABLE(SimulationConfig)->polio_params->MaxRandStdDev)
        {
            rn = GET_CONFIGURABLE(SimulationConfig)->polio_params->MaxRandStdDev;
        }

        return rn;    
    }

    float SusceptibilityPolio::GetParalysisTime(StrainIdentity* strain_id, float infect_duration)
    {
        int serotype = GetSerotype(strain_id);

        float revDegree = GetReversionDegree(strain_id);
        float seronegativeParalysisRate;
        if( IS_SABIN_TYPE( strain_id->GetAntigenID() ) && GET_CONFIGURABLE(SimulationConfig)->polio_params->VDPV_virulence_model_type == VDPVVirulenceModelType::POLIO_VDPV_LOG_PARALYSIS_AND_LOG_INFECTIVITY) // reversion-dependent log(paralysis rate)
        {
            seronegativeParalysisRate = pow( GET_CONFIGURABLE(SimulationConfig)->polio_params->paralysis_base_rate[serotype], revDegree ) * pow( GET_CONFIGURABLE(SimulationConfig)->polio_params->paralysis_base_rate[strain_id->GetAntigenID()], 1.0f - revDegree ); // paralytic range is recipient VAPP to WPV rates
        }
        else
        {
            seronegativeParalysisRate = revDegree * GET_CONFIGURABLE(SimulationConfig)->polio_params->paralysis_base_rate[serotype]; // linear reversion-dependent paralysis rate
        }

        float p_paralysis = seronegativeParalysisRate * float(GetHumoralImmunity(strain_id) < SusceptibilityPolioConfig::paralytic_immunity_titer);
        if(p_paralysis && randgen->e() < p_paralysis)
        {
            float t_paral = exp( GET_CONFIGURABLE(SimulationConfig)->polio_params->Incubation_Disease_Mu + GetRandNBounded() * GET_CONFIGURABLE(SimulationConfig)->polio_params->Incubation_Disease_Sigma ); // lognormal distributed incubation period, from Casey 1942, mu=2.3893 sigma=0.4558
            
            if(t_paral >= infect_duration) // paralysis must occur before the infection is cleared
            {
                t_paral = 0.99f * infect_duration;
            }
            
            return t_paral;
        }
        return -1.0f; // not paralytic polio
    }

    float SusceptibilityPolio::GetReversionDegree(StrainIdentity* strain_id)
    {
        float reversion_degree = 0.0f;
        if(GET_CONFIGURABLE(SimulationConfig)->number_substrains > 1 && GET_CONFIGURABLE(SimulationConfig)->polio_params->VDPV_virulence_model_type > VDPVVirulenceModelType::POLIO_VDPV_NONVIRULENT)
        {
            if( IS_SABIN_TYPE( strain_id->GetAntigenID() ) ) // vaccine strain
            {
                if(strain_id->GetGeneticID() == 0)
                {
                    reversion_degree = 0.0f; // Sabin
                }
                else // VRPV
                {
                    int serotype        = GetSerotype(strain_id);
                    reversion_degree    = (1.0f + floor(log(float(strain_id->GetGeneticID())) / LOG_2)) / GET_CONFIGURABLE(SimulationConfig)->polio_params->reversionSteps_cVDPV[serotype]; // log(2.0)=0.6931472
                    
                    if(reversion_degree > 1.0f)
                    {
                        reversion_degree = 1.0f;
                    }
                }
            }
            else // WPV
            {
                reversion_degree = 1.0f;
            }
        }
        else // not modeling reversion process
        {
            if( IS_SABIN_TYPE( strain_id->GetAntigenID() ) ) // OPV
            {
                reversion_degree = 0.0f;
            }
            else // WPV
            {
                reversion_degree = 1.0f;
            }
        }
        return reversion_degree;
    }

    void SusceptibilityPolio::AddVaccineToInterventionsContainer( int type, float time_since_vaccination)
    {
        PolioVaccineType::Enum vac_type = PolioVaccineType::Enum(type); // not great, but better
        PolioVaccine* vac = PolioVaccine::CreateVaccine(vac_type, time_since_vaccination);
        IIndividualHumanInterventionsContext* context = parent->GetInterventionsContext();
        IInterventionConsumer* iic = nullptr;
        if (s_OK ==  context->QueryInterface(GET_IID(IInterventionConsumer), (void**)&iic) )
        {
            iic->GiveIntervention(vac);
        }
        else
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IInterventionConsumer", "IIndividualHumanInterventionsContext" );
        }
    }

  
    void SusceptibilityPolio::DecrementInfection(StrainIdentity* infection_strain)
    {
        infectionStrains[infection_strain->GetAntigenID()]--;
    }

    void SusceptibilityPolio::IncrementInfection(StrainIdentity* infection_strain)
    {
        infectionStrains[infection_strain->GetAntigenID()]++;
    }

    void SusceptibilityPolio::ClearAllNewInfectionByStrain(void)
    {
        for(int i = 0; i < N_POLIO_VIRUS_TYPES; i++)
        {
            newInfectionByStrain[i] = 0;
        }
    }

    void SusceptibilityPolio::SetNewInfectionByStrain(StrainIdentity* infection_strain)
    {
        newInfectionByStrain[infection_strain->GetAntigenID()]++;
    }

    const int*
    SusceptibilityPolio::GetInfectionStrains()
    const
    {
        return infectionStrains;
    }

    const int*
    SusceptibilityPolio::GetNewInfectionsByStrain() const
    {
        return newInfectionByStrain;
    }

    void SusceptibilityPolio::AccumulateNewInfectionCount(float ind_MCweight, float* inf_count)
    {
        for(int i = 0; i < N_POLIO_VIRUS_TYPES; i++)
        {
            inf_count[i] += ind_MCweight * newInfectionByStrain[i];
        }
    }

    bool SusceptibilityPolio::GetSusceptibleStatus(int pvType)
    {
        auto strain_identity = StrainIdentity(pvType, 0);
        LOG_DEBUG_F( "Individual has humoral immunity of %f for antigen %d compared to config param for paralytic immunity_titer of %f.\n",
                     GetHumoralImmunity(&strain_identity),
                     pvType,
                     SusceptibilityPolioConfig::paralytic_immunity_titer );

        bool is_susceptible_to_paralysis = GetHumoralImmunity(&strain_identity) < SusceptibilityPolioConfig::paralytic_immunity_titer;

        if ( is_susceptible_to_paralysis )
        {
            LOG_DEBUG_F( "Is susceptible to paralysis from this infection.\n" );
        }
        return is_susceptible_to_paralysis;
    }

    int SusceptibilityPolio::GetSerotype(StrainIdentity* strain_id)
    {
        int serotype = strain_id->GetAntigenID();

        if( IS_SABIN_TYPE( serotype ) )
        {
            serotype -= PolioVirusTypes::VRPV1;
        }

        return serotype;
    }

    float SusceptibilityPolio::GetMucosalImmunity(StrainIdentity* strain_id)
    {
        int serotype = GetSerotype(strain_id);

        float effective_NAb = mucosalNAb[serotype] + pow( maternalSerumNAb[serotype] + 1.0f, GET_CONFIGURABLE(SimulationConfig)->polio_params->mucosalImmIPV ) - 1.0f; // any antigenic cross reactivity would also go here
        NO_LESS_THAN( effective_NAb, 1.0f )
        return effective_NAb;
    }

    float SusceptibilityPolio::GetHumoralImmunity(StrainIdentity* strain_id)
    {
        int serotype = GetSerotype(strain_id);

        float effective_NAb = humoralNAb[serotype] + maternalSerumNAb[serotype]; // any antigenic cross reactivity would also go here
        NO_LESS_THAN( effective_NAb, 1.0f )
        return effective_NAb;
    }

    float SusceptibilityPolio::GetDefaultAcquireRate(void)
    {
        return SusceptibilityPolioConfig::acquire_rate_default_polio * individual_acquire_risk;
    }

    void SusceptibilityPolio::ResetTimeSinceLastInfection(int serotype)
    {
        time_since_last_infection[serotype] = 0.0f;
    }

    void SusceptibilityPolio::ResetTimeSinceLastIPV(int serotype)
    {
        time_since_last_IPV[serotype] = 0.0f;
    }

    void SusceptibilityPolio::validateStrainSettings(void)
    {
        if( GET_CONFIGURABLE(SimulationConfig)->number_basestrains < 0 )
        {
            throw ConfigurationRangeException( __FILE__, __LINE__, __FUNCTION__, "Number_Basestrains", GET_CONFIGURABLE(SimulationConfig)->number_basestrains, 0 ); 
        }
        else if( GET_CONFIGURABLE(SimulationConfig)->number_basestrains > N_POLIO_VIRUS_TYPES )
        {
            throw ConfigurationRangeException( __FILE__, __LINE__, __FUNCTION__, "Number_Basestrains", GET_CONFIGURABLE(SimulationConfig)->number_basestrains, 6 ); 
        }

        int n_substrain = GET_CONFIGURABLE(SimulationConfig)->number_substrains;
        // h/t to Brian Kernighan
        unsigned int v = n_substrain; // count the number of bits set in v
        unsigned int c; // c accumulates the total bits set in v
        for (c = 0; v; c++)
        {
            v &= v - 1; // clear the least significant bit set
        }

        if(n_substrain < 0 || n_substrain > N_MAX_POLIO_GENOMES || c != 1 )
        {
            throw ConfigurationRangeException( __FILE__, __LINE__, __FUNCTION__, "Number_Substrains", n_substrain, ( n_substrain<0?0:N_MAX_POLIO_GENOMES ) ); 
        }

        if(EvolutionPolioClockType::POLIO_EVOCLOCK_POISSONSITES)
        {
            int n_bits = int(log(float(n_substrain)) / LOG_2);

            if(     n_bits < ((GET_CONFIGURABLE(SimulationConfig)->polio_params->Sabin1_Site_Rates).size() - 1)
                ||  n_bits < ((GET_CONFIGURABLE(SimulationConfig)->polio_params->Sabin2_Site_Rates).size() - 1)
                ||  n_bits < ((GET_CONFIGURABLE(SimulationConfig)->polio_params->Sabin3_Site_Rates).size() - 1))
            {
                throw ConfigurationRangeException( __FILE__, __LINE__, __FUNCTION__, "The number of Sabin reversion site rates may not be greater than the number of bits in the genome." ); 
            }
        }
    }

    bool SusceptibilityPolio::IsSeropositive( unsigned char serotype )
    const
    {
        float receiveHumoralMemoryNAb[N_POLIO_SEROTYPES];
        GetHumoralMemoryNAb( receiveHumoralMemoryNAb );
        auto pit = SusceptibilityPolioConfig::paralytic_immunity_titer;
        return( receiveHumoralMemoryNAb[ serotype ] > pit );
    }

    REGISTER_SERIALIZABLE(SusceptibilityPolio);

    void SusceptibilityPolio::serialize(IArchive& ar, SusceptibilityPolio* obj)
    {
        SusceptibilityEnvironmental::serialize(ar, obj);
        SusceptibilityPolio& susceptibility = *obj;
        ar.labelElement("shedding_titer"); ar.serialize(susceptibility.shedding_titer, N_POLIO_VIRUS_TYPES);
        ar.labelElement("humoralNAb"); ar.serialize(susceptibility.humoralNAb, N_POLIO_SEROTYPES);
        ar.labelElement("mucosalNAb"); ar.serialize(susceptibility.mucosalNAb, N_POLIO_SEROTYPES);
        ar.labelElement("maternalSerumNAb"); ar.serialize(susceptibility.maternalSerumNAb, N_POLIO_SEROTYPES);
        ar.labelElement("humoralMemoryNAb"); ar.serialize(susceptibility.humoralMemoryNAb, N_POLIO_SEROTYPES);
        ar.labelElement("mucosalMemoryNAb"); ar.serialize(susceptibility.mucosalMemoryNAb, N_POLIO_SEROTYPES);
        ar.labelElement("humoral_fastDecayCompartment"); ar.serialize(susceptibility.humoral_fastDecayCompartment, N_POLIO_SEROTYPES);
        ar.labelElement("mucosal_fastDecayCompartment"); ar.serialize(susceptibility.mucosal_fastDecayCompartment, N_POLIO_SEROTYPES);
        ar.labelElement("time_since_last_infection"); ar.serialize(susceptibility.time_since_last_infection, N_POLIO_SEROTYPES);
        ar.labelElement("time_since_last_IPV"); ar.serialize(susceptibility.time_since_last_IPV, N_POLIO_SEROTYPES);
        ar.labelElement("vaccine_doses_received"); ar.serialize(susceptibility.vaccine_doses_received, N_POLIO_VACCINES);
        ar.labelElement("vaccine_doses_received_by_type"); ar.serialize(susceptibility.vaccine_doses_received_by_type, N_POLIO_VIRUS_TYPES);
        ar.labelElement("infectionStrains"); ar.serialize(susceptibility.infectionStrains, N_POLIO_VIRUS_TYPES);
// Boost implementation didn't serialize this            ar.labelElement("newInfectionByStrain"); ar.serialize(susceptibility.newInfectionByStrain, N_POLIO_VIRUS_TYPES);
        ar.labelElement("individual_acquire_risk") & susceptibility.individual_acquire_risk;
    }
}

#endif // ENABLE_POLIO
