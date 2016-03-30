/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <map>
#include <stdint.h>

#include "EnumSupport.h"

namespace Kernel
{
    typedef std::map<std::string, float> vector_float_map_t;
    typedef std::map<std::string, int32_t> vector_int_map_t;

    // ENUM defs for Egg_Hatch_Delay_Distribution
    ENUM_DEFINE(EggHatchDelayDist, 
        ENUM_VALUE_SPEC(NO_DELAY                                            , 0)
        ENUM_VALUE_SPEC(EXPONENTIAL_DURATION                                , 1))

    // ENUM defs for Egg_Saturation_At_Oviposition
    ENUM_DEFINE(EggSaturation, 
        ENUM_VALUE_SPEC(NO_SATURATION                                       , 0)
        ENUM_VALUE_SPEC(SATURATION_AT_OVIPOSITION                           , 1)
        ENUM_VALUE_SPEC(SIGMOIDAL_SATURATION                                , 2))

    // ENUM defs for Larval_Density_Dependence
    ENUM_DEFINE(LarvalDensityDependence, 
        ENUM_VALUE_SPEC(UNIFORM_WHEN_OVERPOPULATION                         , 0)
        ENUM_VALUE_SPEC(GRADUAL_INSTAR_SPECIFIC                             , 1)
        ENUM_VALUE_SPEC(LARVAL_AGE_DENSITY_DEPENDENT_MORTALITY_ONLY         , 2)
        ENUM_VALUE_SPEC(NO_DENSITY_DEPENDENCE                               , 3)
        ENUM_VALUE_SPEC(DENSITY_DELAYED_GROWTH_NOT_MORTALITY                , 4))

    // ENUM defs for Vector_Larval_Rainfall_Mortality
    ENUM_DEFINE(VectorRainfallMortality,
        ENUM_VALUE_SPEC(NONE                                                , 0)
        ENUM_VALUE_SPEC(SIGMOID                                             , 1)
        ENUM_VALUE_SPEC(SIGMOID_HABITAT_SHIFTING                            , 2))

    // ENUM defs for Vector_Sampling_Type
    ENUM_DEFINE(VectorSamplingType, 
        ENUM_VALUE_SPEC(TRACK_ALL_VECTORS                                   , 1)
        ENUM_VALUE_SPEC(SAMPLE_IND_VECTORS                                  , 2)
        ENUM_VALUE_SPEC(VECTOR_COMPARTMENTS_NUMBER                          , 3)
        ENUM_VALUE_SPEC(VECTOR_COMPARTMENTS_PERCENT                         , 4))

    // ENUM defs for Vector_Sugar_Feeding_Frequency
    ENUM_DEFINE(VectorSugarFeeding, 
        ENUM_VALUE_SPEC(VECTOR_SUGAR_FEEDING_NONE                           , 0)
        ENUM_VALUE_SPEC(VECTOR_SUGAR_FEEDING_ON_EMERGENCE_ONLY              , 1)
        ENUM_VALUE_SPEC(VECTOR_SUGAR_FEEDING_EVERY_FEED                     , 2)
        ENUM_VALUE_SPEC(VECTOR_SUGAR_FEEDING_EVERY_DAY                      , 3))

    // ENUM defs for Habitat_Type in VectorSpeciesParameters
    ENUM_DEFINE(VectorHabitatType, 
        ENUM_VALUE_SPEC(TEMPORARY_RAINFALL                                  , 1)
        ENUM_VALUE_SPEC(WATER_VEGETATION                                    , 2)
        ENUM_VALUE_SPEC(HUMAN_POPULATION                                    , 3)
        ENUM_VALUE_SPEC(CONSTANT                                            , 4)
        ENUM_VALUE_SPEC(BRACKISH_SWAMP                                      , 5)
        ENUM_VALUE_SPEC(ALL_HABITATS                                        , 100))

    ENUM_DEFINE(HEGModel,
        ENUM_VALUE_SPEC(OFF                                                 , 0)
        ENUM_VALUE_SPEC(GERMLINE_HOMING                                     , 1)
        ENUM_VALUE_SPEC(EGG_HOMING                                          , 2)
        ENUM_VALUE_SPEC(DUAL_GERMLINE_HOMING                                , 3)
        ENUM_VALUE_SPEC(DRIVING_Y                                           , 4))

    ENUM_DEFINE(AgeDependentBitingRisk, 
        ENUM_VALUE_SPEC(OFF                     , 0)
        ENUM_VALUE_SPEC(LINEAR                  , 1)
        ENUM_VALUE_SPEC(SURFACE_AREA_DEPENDENT  , 2))

    ENUM_DEFINE(VectorPoolIdEnum,
        ENUM_VALUE_SPEC(INDOOR_VECTOR_POOL,    0)
        ENUM_VALUE_SPEC(OUTDOOR_VECTOR_POOL,   1)
        ENUM_VALUE_SPEC(BOTH_VECTOR_POOLS,     2))

    // State counters for feeding cycle, reporting, etc.
    ENUM_DEFINE(VectorStateEnum, 
        ENUM_VALUE_SPEC(STATE_ADULT              , 1)
        ENUM_VALUE_SPEC(STATE_INFECTED           , 2)
        ENUM_VALUE_SPEC(STATE_INFECTIOUS         , 3)
        ENUM_VALUE_SPEC(STATE_MALE               , 4)
        ENUM_VALUE_SPEC(STATE_EGG                , 5)
        ENUM_VALUE_SPEC(STATE_LARVA              , 6)
        ENUM_VALUE_SPEC(STATE_IMMATURE           , 7))

    // USE EXTREME CAUTION in changing the values of the following genetic characteristics
    // They are used in a bit-packing scheme in the VectorMatingStructure
    // 1-bit for gender, 1-bit for sterility, 2-bits for Wolbachia, etc.
    ENUM_DEFINE(VectorGender,
        ENUM_VALUE_SPEC(VECTOR_FEMALE           , 0)
        ENUM_VALUE_SPEC(VECTOR_MALE             , 1))

    ENUM_DEFINE(VectorSterility,
        ENUM_VALUE_SPEC(VECTOR_FERTILE          , 0)
        ENUM_VALUE_SPEC(VECTOR_STERILE          , 1))

    ENUM_DEFINE(VectorWolbachia,
        ENUM_VALUE_SPEC(WOLBACHIA_FREE          , 0)
        ENUM_VALUE_SPEC(VECTOR_WOLBACHIA_A      , 1)
        ENUM_VALUE_SPEC(VECTOR_WOLBACHIA_B      , 2)
        ENUM_VALUE_SPEC(VECTOR_WOLBACHIA_AB     , 3))

    ENUM_DEFINE(VectorAllele,
        ENUM_VALUE_SPEC(WILD      , 0)
        ENUM_VALUE_SPEC(HALF      , 1)
        ENUM_VALUE_SPEC(FULL      , 2)
        ENUM_VALUE_SPEC(NotMated  , 3))
}
