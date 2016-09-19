/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include <vector>

#include "StrainIdentity.h"
#include "PolioDefs.h"
#include "EnumSupport.h"
#include "Common.h"


// ENUM defs for evolution_polio_clock_type
ENUM_DEFINE(EvolutionPolioClockType, 
    ENUM_VALUE_SPEC(POLIO_EVOCLOCK_NONE                                 , 0)
    ENUM_VALUE_SPEC(POLIO_EVOCLOCK_LINEAR                               , 1)
    ENUM_VALUE_SPEC(POLIO_EVOCLOCK_IMMUNITY                             , 2)
    ENUM_VALUE_SPEC(POLIO_EVOCLOCK_REVERSION_AND_IMMUNITY               , 3)
    ENUM_VALUE_SPEC(POLIO_EVOCLOCK_REVERSION                            , 4)
    ENUM_VALUE_SPEC(POLIO_EVOCLOCK_POISSONSITES                         , 5))

// ENUM defs for VDPV_virulence_model_type
ENUM_DEFINE(VDPVVirulenceModelType, 
    ENUM_VALUE_SPEC(POLIO_VDPV_NONVIRULENT                              , 0)
    ENUM_VALUE_SPEC(POLIO_VDPV_PARALYSIS                                , 1)
    ENUM_VALUE_SPEC(POLIO_VDPV_PARALYSIS_AND_LOG_INFECTIVITY            , 2)
    ENUM_VALUE_SPEC(POLIO_VDPV_LOG_PARALYSIS_AND_LOG_INFECTIVITY        , 3))


namespace Kernel
{
    struct PolioParameters
    {
        EvolutionPolioClockType::Enum evolution_polio_clock_type; // evolution_polio_clock_type
        VDPVVirulenceModelType::Enum  VDPV_virulence_model_type;  // VDPV_virulence_model_type

        float maternalAbHalfLife;

        // SusceptibilityPolio
        // probability of no infection (1-p0)^events is approximated by Poisson, exp(-p0*events)
        float TauNAb; // (days) neutralization time constant
        float PVinf0[N_POLIO_VIRUS_TYPES]; // (dimensionless) probability of infection from single virion
        float viral_interference[N_POLIO_VIRUS_TYPES]; // (dimensionless) probability that infection will prevent infection of any heterologous serotype
        float vaccine_take_multiplier[N_POLIO_VIRUS_TYPES]; //counts for the host factors of vaccine take
        float mucosalImmIPV; // (dimensionless, OPV mucosal / IPV mucosal) relative mucosal immunogenicity
        float mucosalImmIPVOPVExposed; // (dimensionless, OPV mucosal / IPV mucosal) relative mucosal immunogenicity on OPV-exposed individuals
        float paralysis_base_rate[N_POLIO_VIRUS_TYPES]; // (dimensionless) probability of a fully susceptible individual to be paralyzed by infection, for WPV and fully reverted cVDPV, assuming age = ? years
        float boostLog2NAb_OPV[N_POLIO_SEROTYPES]; // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_IPV[N_POLIO_SEROTYPES]; // (dimensionless) multiplication of antibody titer in response to challenge infection
        float maxLog2NAb_OPV[N_POLIO_SEROTYPES]; // (dimensionless) multiplication of antibody titer in response to challenge infection
        float maxLog2NAb_IPV[N_POLIO_SEROTYPES]; // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_stddev_OPV[N_POLIO_SEROTYPES]; // (dimensionless) multiplication of antibody titer in response to challenge infection
        float boostLog2NAb_stddev_IPV[N_POLIO_SEROTYPES]; // (dimensionless) multiplication of antibody titer in response to challenge infection
        float primeLog2NAb_OPV[N_POLIO_SEROTYPES]; // (log2 reciprocal titer) antibody level at first infection
        float primeLog2NAb_IPV[N_POLIO_SEROTYPES]; // (log2 reciprocal titer) antibody level at first infection
        float primeLog2NAb_stddev_OPV[N_POLIO_SEROTYPES]; // (log2 reciprocal titer) standard deviation antibody level at first infection
        float primeLog2NAb_stddev_IPV[N_POLIO_SEROTYPES]; // (log2 reciprocal titer) standard deviation antibody level at first infection
        float decayRatePassiveImmunity; // (1/days) exponential decay rate
        float maternal_log2NAb_mean;
        float maternal_log2NAb_std;
        float vaccine_titer_tOPV[N_POLIO_SEROTYPES]; // (TCID50) viral dose of each serotype
        float vaccine_titer_bOPV[2]; // (TCID50) viral dose of each serotype
        float vaccine_titer_mOPV[N_POLIO_SEROTYPES]; // (TCID50) viral dose of each serotype
        float vaccine_Dantigen_IPV[N_POLIO_SEROTYPES]; // (D-antigen units) antigen content of each serotype
        float Incubation_Disease_Mu; // paralysis incubation period, lognormal parameter mu
        float Incubation_Disease_Sigma; // paralysis incubation period, lognormal parameter sigma

        int reversionSteps_cVDPV[N_POLIO_SEROTYPES]; // (bits) number of mutation steps to revert from Sabin to cVDPV, must be <= number_substrains
        float excrement_load; // (grams/day) feces
        float MaxRandStdDev; // (dimensionless) limit to effect of a random normal

        float shedFecalMaxLog10PeakTiter; // (log10 TCID50)
        float shedFecalMaxLog10PeakTiter_Stddev; // (log10 TCID50)
        float shedFecalTiterBlockLog2NAb; // (log2 NAb)
        float shedFecalTiterProfile_mu; // (Ln time)
        float shedFecalTiterProfile_sigma; // (Ln time)
        float shedFecalMaxLnDuration; // (Ln time)
        float shedFecalMaxLnDuration_Stddev; // (Ln time)
        float shedFecalDurationBlockLog2NAb; // (log2 Nab)
        float shedOralMaxLog10PeakTiter; // (log10 TCID50)
        float shedOralMaxLog10PeakTiter_Stddev; // (log10 TCID50)
        float shedOralTiterBlockLog2NAb; // (log2 NAb)
        float shedOralTiterProfile_mu; // (Ln time)
        float shedOralTiterProfile_sigma; // (Ln time)
        float shedOralMaxLnDuration; // (Ln time)
        float shedOralMaxLnDuration_Stddev; // (Ln time)
        float shedOralDurationBlockLog2NAb; // (log2 NAb)

        int vaccine_genome_OPV1;
        int vaccine_genome_OPV2;
        int vaccine_genome_OPV3;

        StrainIdentity vaccine_strains[3]; // (StrainIdentity) strainIDs for vaccine virus out of the vial, types 1-3 Sabin strains

        std::vector<std::vector<float> >   substrainRelativeInfectivity;
        std::vector<float> Sabin1_Site_Rates;
        std::vector<float> Sabin2_Site_Rates;
        std::vector<float> Sabin3_Site_Rates;

        PolioParameters()
        : evolution_polio_clock_type(EvolutionPolioClockType::POLIO_EVOCLOCK_NONE)
        , VDPV_virulence_model_type(VDPVVirulenceModelType::POLIO_VDPV_NONVIRULENT)
        , maternalAbHalfLife(-42.0f)
        , TauNAb(-42.0f)
//        , PVinf0( { -42.0f, -42.0f, -42.0f, -42.0f, -42.0f, -42.0f } )
//        , viral_interference( { -42.0f, -42.0f, -42.0f, -42.0f, -42.0f, -42.0f } )
//        , vaccine_take_multiplier( { -42.0f, -42.0f, -42.0f, -42.0f, -42.0f, -42.0f } )
        , mucosalImmIPV(-42.0f)
        , mucosalImmIPVOPVExposed(-42.0f)
//        , paralysis_base_rate( { -42.0f, -42.0f, -42.0f, -42.0f, -42.0f, -42.0f } )
//        , boostLog2NAb_OPV( { -42.0f, -42.0f, -42.0f } )
//        , boostLog2NAb_IPV( { -42.0f, -42.0f, -42.0f } )
//        , maxLog2NAb_OPV( { -42.0f, -42.0f, -42.0f } )
//        , maxLog2NAb_IPV( { -42.0f, -42.0f, -42.0f } )
//        , boostLog2NAb_stddev_OPV( { -42.0f, -42.0f, -42.0f } )
//        , boostLog2NAb_stddev_IPV( { -42.0f, -42.0f, -42.0f } )
//        , primeLog2NAb_OPV( { -42.0f, -42.0f, -42.0f } )
//        , primeLog2NAb_IPV( { -42.0f, -42.0f, -42.0f } )
//        , primeLog2NAb_stddev_OPV( { -42.0f, -42.0f, -42.0f } )
//        , primeLog2NAb_stddev_IPV( { -42.0f, -42.0f, -42.0f } )
        , decayRatePassiveImmunity(-42.0f)
        , maternal_log2NAb_mean(-42.0f)
        , maternal_log2NAb_std(-42.0f)
//        , vaccine_titer_tOPV( { -42.0f, -42.0f, -42.0f } )
//        , vaccine_titer_bOPV( { -42.0f, -42.0f } )
//        , vaccine_titer_mOPV( { -42.0f, -42.0f, -42.0f } )
//        , vaccine_Dantigen_IPV( { -42.0f, -42.0f, -42.0f } )
        , Incubation_Disease_Mu(-42.0f)
        , Incubation_Disease_Sigma(-42.0f)
//        , reversionSteps_cVDPV( { -42.0f, -42.0f, -42.0f } )
        , excrement_load(-42.0f)
        , MaxRandStdDev(-42.0f)
        , shedFecalMaxLog10PeakTiter(-42.0f)
        , shedFecalMaxLog10PeakTiter_Stddev(-42.0f)
        , shedFecalTiterBlockLog2NAb(-42.0f)
        , shedFecalTiterProfile_mu(-42.0f)
        , shedFecalTiterProfile_sigma(-42.0f)
        , shedFecalMaxLnDuration(-42.0f)
        , shedFecalMaxLnDuration_Stddev(-42.0f)
        , shedFecalDurationBlockLog2NAb(-42.0f)
        , shedOralMaxLog10PeakTiter(-42.0f)
        , shedOralMaxLog10PeakTiter_Stddev(-42.0f)
        , shedOralTiterBlockLog2NAb(-42.0f)
        , shedOralTiterProfile_mu(-42.0f)
        , shedOralTiterProfile_sigma(-42.0f)
        , shedOralMaxLnDuration(-42.0f)
        , shedOralMaxLnDuration_Stddev(-42.0f)
        , shedOralDurationBlockLog2NAb(-42.0f)
        , vaccine_genome_OPV1(1)
        , vaccine_genome_OPV2(1)
        , vaccine_genome_OPV3(1)
//        , vaccine_strains( { 0, 0, 0 } )
        , substrainRelativeInfectivity()
        , Sabin1_Site_Rates()
        , Sabin2_Site_Rates()
        , Sabin3_Site_Rates()
        {
            ZERO_ARRAY(PVinf0);
            ZERO_ARRAY(viral_interference);
            ZERO_ARRAY(vaccine_take_multiplier);
            ZERO_ARRAY(paralysis_base_rate);
            ZERO_ARRAY(boostLog2NAb_OPV);
            ZERO_ARRAY(boostLog2NAb_IPV);
            ZERO_ARRAY(maxLog2NAb_OPV);
            ZERO_ARRAY(maxLog2NAb_IPV);
            ZERO_ARRAY(boostLog2NAb_stddev_OPV);
            ZERO_ARRAY(boostLog2NAb_stddev_IPV);
            ZERO_ARRAY(primeLog2NAb_OPV);
            ZERO_ARRAY(primeLog2NAb_IPV);
            ZERO_ARRAY(primeLog2NAb_stddev_OPV);
            ZERO_ARRAY(primeLog2NAb_stddev_IPV);
            ZERO_ARRAY(vaccine_titer_tOPV);
            ZERO_ARRAY(vaccine_titer_bOPV);
            ZERO_ARRAY(vaccine_titer_mOPV);
            ZERO_ARRAY(vaccine_Dantigen_IPV);
            ZERO_ARRAY(reversionSteps_cVDPV);
            ZERO_ARRAY(vaccine_strains);
        }
    };
}
