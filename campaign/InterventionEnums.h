/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <map>
#include "EnumSupport.h"

namespace Kernel
{
    ENUM_DEFINE(SpaceSprayTarget,
        ENUM_VALUE_SPEC(SpaceSpray_FemalesOnly       , 11)
        ENUM_VALUE_SPEC(SpaceSpray_MalesOnly         , 12)
        ENUM_VALUE_SPEC(SpaceSpray_FemalesAndMales   , 13))

    ENUM_DEFINE(ArtificialDietTarget,
        //ENUM_VALUE_SPEC(AD_WithinHouse             , 20) // to be handled as individual rather than node-targeted intervention
        ENUM_VALUE_SPEC(AD_WithinVillage             , 21)
        ENUM_VALUE_SPEC(AD_OutsideVillage            , 22))
   
    ENUM_DEFINE(InterventionDurabilityProfile,
        ENUM_VALUE_SPEC(BOXDURABILITY       , 1)
        ENUM_VALUE_SPEC(DECAYDURABILITY     , 2)
        ENUM_VALUE_SPEC(BOXDECAYDURABILITY  , 3))

    ENUM_DEFINE(MalariaDrugType,
        ENUM_VALUE_SPEC(Artemisinin             , 1)
        ENUM_VALUE_SPEC(Chloroquine             , 2)
        ENUM_VALUE_SPEC(Quinine                 , 3)
        ENUM_VALUE_SPEC(SP                      , 4)
        ENUM_VALUE_SPEC(Primaquine              , 5)
        ENUM_VALUE_SPEC(Artemether_Lumefantrine , 6)
        ENUM_VALUE_SPEC(GenTransBlocking        , 7)
        ENUM_VALUE_SPEC(GenPreerythrocytic      , 8)
        ENUM_VALUE_SPEC(Tafenoquine             , 9))

    ENUM_DEFINE(MalariaChallengeType,
        ENUM_VALUE_SPEC(InfectiousBites         , 1)
        ENUM_VALUE_SPEC(Sporozoites             , 2))

    ENUM_DEFINE(TBDrugType,
        ENUM_VALUE_SPEC(DOTS                    , 1)
        ENUM_VALUE_SPEC(DOTSImproved            , 2)
        ENUM_VALUE_SPEC(EmpiricTreatment        , 3)
        ENUM_VALUE_SPEC(FirstLineCombo          , 4)
        ENUM_VALUE_SPEC(SecondLineCombo         , 5)
        ENUM_VALUE_SPEC(ThirdLineCombo          , 6)
        ENUM_VALUE_SPEC(LatentTreatment         , 7))

    ENUM_DEFINE(DrugUsageType,
        ENUM_VALUE_SPEC(SingleDose                    , 1)
        ENUM_VALUE_SPEC(FullTreatmentCourse           , 2)
        ENUM_VALUE_SPEC(Prophylaxis                   , 3)
        ENUM_VALUE_SPEC(SingleDoseWhenSymptom         , 4)
        ENUM_VALUE_SPEC(FullTreatmentWhenSymptom      , 5)
        ENUM_VALUE_SPEC(SingleDoseParasiteDetect      , 6)
        ENUM_VALUE_SPEC(FullTreatmentParasiteDetect   , 7)
        ENUM_VALUE_SPEC(SingleDoseNewDetectionTech    , 8)
        ENUM_VALUE_SPEC(FullTreatmentNewDetectionTech , 9))

    ENUM_DEFINE(BednetType,
        ENUM_VALUE_SPEC(Barrier     , 1)
        ENUM_VALUE_SPEC(ITN         , 2)
        ENUM_VALUE_SPEC(LLIN        , 3)
        ENUM_VALUE_SPEC(Retreatment , 4))

    ENUM_DEFINE(SimpleVaccineType,
        ENUM_VALUE_SPEC(Generic              , 1)
        ENUM_VALUE_SPEC(TransmissionBlocking , 2)
        ENUM_VALUE_SPEC(AcquisitionBlocking  , 3)
        ENUM_VALUE_SPEC(MortalityBlocking    , 4))

    ENUM_DEFINE(PolioVaccineType,
        ENUM_VALUE_SPEC(tOPV   , 0)
        ENUM_VALUE_SPEC(bOPV   , 1)
        ENUM_VALUE_SPEC(mOPV_1 , 2)
        ENUM_VALUE_SPEC(mOPV_2 , 3)
        ENUM_VALUE_SPEC(mOPV_3 , 4)
        ENUM_VALUE_SPEC(eIPV   , 5))

    ENUM_DEFINE(PolioDrugType,
        ENUM_VALUE_SPEC(V073                    , 1)
        ENUM_VALUE_SPEC(Combo                   , 2))

        ENUM_DEFINE(ImmunoglobulinType,
        ENUM_VALUE_SPEC(StrainSpecific      , 1)
        ENUM_VALUE_SPEC(BroadlyNeutralizing , 2))

    ENUM_DEFINE(OutdoorSprayingTarget,
        ENUM_VALUE_SPEC(FemalesOnly     , 1)
        ENUM_VALUE_SPEC(MalesOnly       , 2)
        ENUM_VALUE_SPEC(FemalesAndMales , 3))

    ENUM_DEFINE(OutbreakType, 
        ENUM_VALUE_SPEC(PrevalenceIncrease , 1)
        ENUM_VALUE_SPEC(ImportCases        , 2))

    ENUM_DEFINE(GlobalVaccineType,
        ENUM_VALUE_SPEC(VACCINE_0     , 0)
        ENUM_VALUE_SPEC(VACCINE_1     , 1) 
        ENUM_VALUE_SPEC(VACCINE_TOPV  , 2)
        ENUM_VALUE_SPEC(VACCINE_BOPV  , 3) 
        ENUM_VALUE_SPEC(VACCINE_MOPV1 , 4) 
        ENUM_VALUE_SPEC(VACCINE_MOPV2 , 5) 
        ENUM_VALUE_SPEC(VACCINE_MOPV3 , 6) 
        ENUM_VALUE_SPEC(VACCINE_IPV   , 7))

    ENUM_DEFINE(TargetDemographicType,
        ENUM_VALUE_SPEC(Everyone                    , 1) 
        ENUM_VALUE_SPEC(ExplicitAgeRanges           , 2) 
        ENUM_VALUE_SPEC(ExplicitAgeRangesAndGender  , 3) 
        ENUM_VALUE_SPEC(ExplicitGender              , 4) 
        ENUM_VALUE_SPEC(PossibleMothers             , 5) 
        ENUM_VALUE_SPEC(ArrivingAirTravellers       , 6)
        ENUM_VALUE_SPEC(DepartingAirTravellers      , 7)
        ENUM_VALUE_SPEC(ArrivingRoadTravellers      , 8)
        ENUM_VALUE_SPEC(DepartingRoadTravellers     , 9)
        ENUM_VALUE_SPEC(AllArrivingTravellers       , 10)
        ENUM_VALUE_SPEC(AllDepartingTravellers      , 11)
        ENUM_VALUE_SPEC(ExplicitDiseaseState        , 12))

    ENUM_DEFINE(TargetGroupType,
        ENUM_VALUE_SPEC(Everyone                , 1) 
        ENUM_VALUE_SPEC(Infected                , 2) 
        ENUM_VALUE_SPEC(ActiveInfection         , 3) 
        ENUM_VALUE_SPEC(LatentInfection         , 4) 
        ENUM_VALUE_SPEC(MDR                     , 5)
        ENUM_VALUE_SPEC(TreatmentNaive          , 6)
        ENUM_VALUE_SPEC(HasFailedTreatment      , 7)
        ENUM_VALUE_SPEC(HIVNegative             , 8)
        ENUM_VALUE_SPEC(ActiveHadTreatment      , 9))
        
    ENUM_DEFINE(TargetGender,
        ENUM_VALUE_SPEC(All     , 0)
        ENUM_VALUE_SPEC(Male    , 1)
        ENUM_VALUE_SPEC(Female  , 2))

    ENUM_DEFINE(InfectionType,
        ENUM_VALUE_SPEC(TB, 0)
        ENUM_VALUE_SPEC(HIV, 1))

    ENUM_DEFINE(PolygonFormatType,
        ENUM_VALUE_SPEC(SHAPE      , 1) 
        //ENUM_VALUE_SPEC(GEOJSON    , 2)
        )


    ENUM_DEFINE(IndividualEventTriggerType,
        ENUM_VALUE_SPEC(NoTrigger                 ,  0)
        ENUM_VALUE_SPEC(Births                    ,  1)
        ENUM_VALUE_SPEC(EveryUpdate               ,  2)
        ENUM_VALUE_SPEC(EveryTimeStep             ,  3)
        ENUM_VALUE_SPEC(NewInfectionEvent         ,  4)
        ENUM_VALUE_SPEC(TBActivation              ,  5)
        ENUM_VALUE_SPEC(NewClinicalCase           ,  6)
        ENUM_VALUE_SPEC(NewSevereCase             ,  7)
        ENUM_VALUE_SPEC(DiseaseDeaths             ,  8)
        ENUM_VALUE_SPEC(NonDiseaseDeaths          ,  9)
        ENUM_VALUE_SPEC(TBActivationSmearPos      , 10)
        ENUM_VALUE_SPEC(TBActivationSmearNeg      , 11)
        ENUM_VALUE_SPEC(TBActivationExtrapulm     , 12)
        ENUM_VALUE_SPEC(TBActivationPostRelapse   , 13)
        ENUM_VALUE_SPEC(TBPendingRelapse          , 14)
        ENUM_VALUE_SPEC(TBActivationPresymptomatic, 15)
        ENUM_VALUE_SPEC(TestPositiveOnSmear       , 16)
        ENUM_VALUE_SPEC(ProviderOrdersTBTest      , 17)
        ENUM_VALUE_SPEC(TBTestPositive            , 18)
        ENUM_VALUE_SPEC(TBTestNegative            , 19)
        ENUM_VALUE_SPEC(TBTestDefault             , 20)
        ENUM_VALUE_SPEC(TBRestartHSB              , 21)
        ENUM_VALUE_SPEC(TBMDRTestPositive         , 22)
        ENUM_VALUE_SPEC(TBMDRTestNegative         , 23)
        ENUM_VALUE_SPEC(TBMDRTestDefault          , 24)
        ENUM_VALUE_SPEC(TBFailedDrugRegimen       , 25)
        ENUM_VALUE_SPEC(TBRelapseAfterDrugRegimen , 26)
        ENUM_VALUE_SPEC(TBStartDrugRegimen        , 27)
        ENUM_VALUE_SPEC(TBStopDrugRegimen         , 28)
        ENUM_VALUE_SPEC(PropertyChange            , 29)
        ENUM_VALUE_SPEC(STIDebut                  , 30)
        ENUM_VALUE_SPEC(StartedART                , 31)
        ENUM_VALUE_SPEC(StoppedART                , 32)
        ENUM_VALUE_SPEC(CascadeStateAborted       , 33)
        ENUM_VALUE_SPEC(HIVNewlyDiagnosed         , 34)
        ENUM_VALUE_SPEC(GaveBirth                 , 35)
        ENUM_VALUE_SPEC(Pregnant                  , 36)
        ENUM_VALUE_SPEC(Emigrating                , 37)
        ENUM_VALUE_SPEC(Immigrating               , 38)
        ENUM_VALUE_SPEC(HIVTestedNegative         , 39)
        ENUM_VALUE_SPEC(HIVTestedPositive         , 40)
        ENUM_VALUE_SPEC(HIVSymptomatic            , 41)
        ENUM_VALUE_SPEC(HIVPreARTToART            , 42)
        ENUM_VALUE_SPEC(HIVNonPreARTToART         , 43)
        ENUM_VALUE_SPEC(TwelveWeeksPregnant       , 44)
        ENUM_VALUE_SPEC(FourteenWeeksPregnant     , 45)
        ENUM_VALUE_SPEC(SixWeeksOld               , 46)
        ENUM_VALUE_SPEC(EighteenMonthsOld         , 47)
        ENUM_VALUE_SPEC(STIPreEmigrating          , 48)
        ENUM_VALUE_SPEC(STIPostImmigrating        , 49)
        
        ENUM_VALUE_SPEC(TriggerString          ,  999)
        ENUM_VALUE_SPEC(TriggerList            , 1000)
    )

#define NO_TRIGGER_STR ("NoTrigger")

    ENUM_DEFINE(BssTargetingType,
        ENUM_VALUE_SPEC(TargetBss               , 0)
        ENUM_VALUE_SPEC(IgnoreBss               , 1)
        ENUM_VALUE_SPEC(NeutralBss              , 2))


    ENUM_DEFINE(ScaleUpProfile,
        ENUM_VALUE_SPEC(Immediate       , 1)
        ENUM_VALUE_SPEC(Linear          , 2)
        ENUM_VALUE_SPEC(Sigmoid         , 3))

    ENUM_DEFINE(EventOrConfig,
        ENUM_VALUE_SPEC(Config          , 1)
        ENUM_VALUE_SPEC(Event           , 2))

    ENUM_DEFINE(BinaryBooleanOperator,
        ENUM_VALUE_SPEC(AND, 0)
        ENUM_VALUE_SPEC(OR,  1))

    ENUM_DEFINE(NATrueFalse,
        ENUM_VALUE_SPEC(NA, 0)
        ENUM_VALUE_SPEC(True,  1)
        ENUM_VALUE_SPEC(False, 2))

#define NATrueFalseSwitch(x,t) (x == NATrueFalse::Enum::True ? t : !t)

}

// TBD: I ripped out grace's AntiTB drug serialization solution. Needs alternate solution.
