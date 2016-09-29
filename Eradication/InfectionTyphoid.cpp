/*****************************************************************************

Copyright (c) 2015 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_TYPHOID

#include "InfectionTyphoid.h"
#include "SusceptibilityTyphoid.h"
#include "IndividualTyphoid.h"
#include "InterventionsContainer.h"
#include "TyphoidDefs.h"
#include "Environment.h"
#include "Debug.h"

#include "Common.h"
#include "MathFunctions.h"
#include "RANDOM.h"
#include "Exceptions.h"
#include "SimulationConfig.h"
using namespace std;

static const char* _module = "InfectionTyphoid";

namespace Kernel
{
#define UNINIT_TIMER (-100.0f)
    GET_SCHEMA_STATIC_WRAPPER_IMPL(Typhoid.Infection,InfectionTyphoidConfig)
    BEGIN_QUERY_INTERFACE_BODY(InfectionTyphoidConfig)
    END_QUERY_INTERFACE_BODY(InfectionTyphoidConfig)

    // Incubation period by transmission route (taken from Glynn's dose response analysis) assuming low dose for environmental.
    // mean and std dev of log normal distribution
    //
    const float ageThresholdInYears = 30.0f;

    const float InfectionTyphoid::mpl = 2.2350f;
    const float InfectionTyphoid::spl = 0.4964f;
    const float InfectionTyphoid::mpm = 2.0026f;
    const float InfectionTyphoid::spm = 0.7604f;
    const float InfectionTyphoid::mph = 1.5487f; // math.log(4.7)
    const float InfectionTyphoid::sph = 0.3442f;

    // Subclinical infectious duration parameters: mean and standard deviation under and over 30 (refitted without carriers) 
    const float InfectionTyphoid::mso30=1.2584990f;
    const float InfectionTyphoid::sso30=0.7883767f;
    const float InfectionTyphoid::msu30=1.171661f;
    const float InfectionTyphoid::ssu30=0.483390f;

    // Acute infectious duration parameters: mean and standard deviation under and over 30 
    const float InfectionTyphoid::mao30=1.2584990f;
    const float InfectionTyphoid::sao30=0.7883767f;
    const float InfectionTyphoid::mau30=1.171661f;
    const float InfectionTyphoid::sau30=0.483390f;

    const int GallstoneDataLength= 9;
    const double FemaleGallstones[GallstoneDataLength] = {0.0, 0.097, 0.234, 0.431, 0.517, 0.60, 0.692, 0.692, 0.555}; // 10-year age bins
    const double MaleGallstones[GallstoneDataLength] = {0.0, 0.0, 0.045, 0.134, 0.167, 0.198, 0.247, 0.435, 0.4};

    const float InfectionTyphoid::P10 = 0.0f; // probability of clinical immunity from a subclinical infection

    const int InfectionTyphoid::_chronic_duration = 100000000;

    const int InfectionTyphoid::acute_treatment_day = 5; // how many days after infection will people receive treatment
    const float InfectionTyphoid::CFRU = 0.00f;   // case fatality rate?
    const float InfectionTyphoid::CFRH = 0.00f; // hospitalized case fatality rate?
    const float InfectionTyphoid::treatmentprobability = 1.0f;  // probability of treatment seeking for an acute case. we are in santiago so assume 100%

    inline float generateRandFromLogNormal(float m, float s) {
        // inputs: m is mean of underlying distribution, s is std dev
        return (exp((m)+randgen->eGauss()*s));
    }

    bool
    InfectionTyphoidConfig::Configure(
        const Configuration * config
    )
    {
        LOG_DEBUG( "Configure\n" );
        //initConfigTypeMap( "Enable_Contact_Tracing", &tracecontact_mode, Enable_Contact_Tracing_DESC_TEXT, false ); // polio
        bool bRet = JsonConfigurable::Configure( config );
        return bRet;
    }

    BEGIN_QUERY_INTERFACE_BODY(InfectionTyphoid)
        HANDLE_INTERFACE(IInfectionTyphoid)
    END_QUERY_INTERFACE_BODY(InfectionTyphoid)

    InfectionTyphoid::InfectionTyphoid()
    {
    }

    const SimulationConfig*
    InfectionTyphoid::params()
    {
        return GET_CONFIGURABLE(SimulationConfig);
    }

    InfectionTyphoid::InfectionTyphoid(IIndividualHumanContext *context) : InfectionEnvironmental(context)
    {
        auto doseTracking = ((IndividualHumanTyphoid*)context)->getDoseTracking();
        if (doseTracking == "High")
        {
            _prepatent_duration = (int)(generateRandFromLogNormal(mph, sph));

        }
        else if (doseTracking == "Medium")
        {
            _prepatent_duration = (int)(generateRandFromLogNormal(mpm, spm));
        }	
        else if (doseTracking == "Low")
        {
            _prepatent_duration = (int)(generateRandFromLogNormal(mpl, spl));
        }
        else
        {
            // neither environmental nor contact source. probably from initial seeding
            _prepatent_duration = (int)(generateRandFromLogNormal(mpl, spl));
        }
        prepatent_timer=_prepatent_duration;
    }

    void InfectionTyphoid::Initialize(suids::suid _suid)
    {
        treatment_multiplier = 1;
        chronic_timer = UNINIT_TIMER;
        subclinical_timer = UNINIT_TIMER;
        acute_timer = UNINIT_TIMER;
        prepatent_timer = UNINIT_TIMER;
        _subclinical_duration = _prepatent_duration = _acute_duration = 0;
        isDead = false;
        InfectionEnvironmental::Initialize(_suid);
        last_state_reported = "S";
    }

    InfectionTyphoid *InfectionTyphoid::CreateInfection(IIndividualHumanContext *context, suids::suid _suid)
    {
        //VALIDATE(boost::format(">InfTyphoid::CreateInfection(%1%, %2%)") % context->GetSuid().data % _suid.data );

        InfectionTyphoid *newinfection = _new_ InfectionTyphoid(context);
        newinfection->Initialize(_suid);

        return newinfection;
    }

    InfectionTyphoid::~InfectionTyphoid()
    {
    }

    void InfectionTyphoid::SetParameters(StrainIdentity* infstrain, int incubation_period_override)
    {
        InfectionEnvironmental::SetParameters(infstrain, incubation_period_override); // setup infection timers and infection state
        if(infstrain == NULL)
        {
            // using default strainIDs
            //infection_strain->SetAntigenID(default_antigen);
        }
        else
        {
            *infection_strain = *infstrain;
        }
    }

    void InfectionTyphoid::InitInfectionImmunology(ISusceptibilityContext* _immunity)
    {
        ISusceptibilityTyphoid* immunity = NULL;
        if( _immunity->QueryInterface( GET_IID( ISusceptibilityTyphoid ), (void**)&immunity ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "_immunity", "ISusceptibilityTyphoid", "Susceptibility" );
        }

        StateChange = InfectionStateChange::New;
        return InfectionEnvironmental::InitInfectionImmunology( _immunity );
    }

    void InfectionTyphoid::handlePrepatentExpiry()
    {
        auto age = dynamic_cast<IIndividualHuman*>(parent)->GetAge() / DAYSPERYEAR;
        auto sex = dynamic_cast<IIndividualHuman*>(parent)->GetGender();
        auto mort = dynamic_cast<IDrugVaccineInterventionEffects*>(parent->GetInterventionsContext())->GetInterventionReducedMortality();
        //state_to_report="P";
        //LOG_INFO_F("hasclin subclinical dur %d, pre %d\n", _subclinical_duration, prepatent_timer); 
        prepatent_timer=UNINIT_TIMER; 
        if (randgen->e()<(IndividualHumanTyphoidConfig::typhoid_symptomatic_fraction*mort))
        { //THIS IS NOT ACTUALLY MORTALITY, I AM JUST USING THE CALL 
            if (age < ageThresholdInYears)
                _acute_duration = int(generateRandFromLogNormal(mau30, sau30)*7);
            else
                _acute_duration = int(generateRandFromLogNormal(mao30, sao30)*7);
            //LOG_INFO_F("acute dur %d\n", _acute_duration);
            acute_timer = _acute_duration;
            //if (_acute_duration > 365)
            //    isChronic = true; // will be a chronic carrier
        } else {
            if (age <= ageThresholdInYears)
                _subclinical_duration = int(generateRandFromLogNormal( msu30, ssu30)*7);
            else
                _subclinical_duration = int(generateRandFromLogNormal( mso30, sso30)*7);
            subclinical_timer = _subclinical_duration;
            //if (_subclinical_duration > 365)
            //    isChronic = true;
            //state_to_report="C";
        }
        //return state_to_report;
    }

    void InfectionTyphoid::handleAcuteExpiry()
    {
        auto age = dynamic_cast<IIndividualHuman*>(parent)->GetAge() / DAYSPERYEAR;
        auto sex = dynamic_cast<IIndividualHuman*>(parent)->GetGender();

        if ((randgen->e() < CFRU) & (treatment_multiplier < 1)) // untreated at end of period has higher fatality rate
        {
            isDead = true;
            //state_to_report = "D";
            LOG_INFO_F( "[Update] Somebody died from their infection.\n" );
        }
        else
        {
            //if they survived, calculate probability of being a carrier
            float p3=0.0; // P3 is age dependent so is determined below. Probability of becoming a chronic carrier from a CLINICAL infection
            float carrier_prob = 0;
            int agebin = int(floor(age/10));
            if (agebin>=GallstoneDataLength)
            {
                agebin=GallstoneDataLength-1;
            }
            if (sex==1)
            {
                p3=FemaleGallstones[agebin];
                carrier_prob = IndividualHumanTyphoidConfig::typhoid_carrier_probability_female;
            } 
            else if (sex==0)
            {
                p3=MaleGallstones[agebin];
                carrier_prob = IndividualHumanTyphoidConfig::typhoid_carrier_probability_male;
            }
            if (randgen->e()< p3*carrier_prob)
            {
                chronic_timer = _chronic_duration;
            }
        } 
        acute_timer = UNINIT_TIMER;
        treatment_multiplier = 1;
    }

    void InfectionTyphoid::handleSubclinicalExpiry()
    {
        auto age = dynamic_cast<IIndividualHuman*>(parent)->GetAge() / DAYSPERYEAR;
        auto sex = dynamic_cast<IIndividualHuman*>(parent)->GetGender();

        //LOG_INFO_F("SOMEONE FINSIHED SUB %d, %d\n", _subclinical_duration, subclinical_timer);
        subclinical_timer = UNINIT_TIMER;
        float p2 = 0;
        float carrier_prob = 0;
        int agebin = int(floor(age/10));
        if (agebin>=GallstoneDataLength)
            agebin=GallstoneDataLength-1;
        if (sex==1)
        {
            p2=FemaleGallstones[agebin];
            carrier_prob = IndividualHumanTyphoidConfig::typhoid_carrier_probability_female ;
        } 
        else if (sex==0)
        {
            p2=MaleGallstones[agebin];
            carrier_prob = IndividualHumanTyphoidConfig::typhoid_carrier_probability_male;
        }
        //LOG_INFO_F("Gallstone percentage is %f %f\n", getAgeInYears(), p2);
        if (randgen->e() < p2*carrier_prob)
        {
            chronic_timer = _chronic_duration;
        }
    }

    void InfectionTyphoid::Update(float dt, ISusceptibilityContext* _immunity)
    {
        bool state_changed = false;
        std::string state_to_report = "S"; // default state is susceptible

        LOG_DEBUG_F("%d INFECTED!!!%d,%d,%d,%d\n", GetSuid().data, prepatent_timer, acute_timer, subclinical_timer,chronic_timer);
        if (prepatent_timer > UNINIT_TIMER)
        { // pre-patent

            state_to_report="P";
            prepatent_timer -= dt;
            if( UNINIT_TIMER<prepatent_timer && prepatent_timer<=0 )
            {
                handlePrepatentExpiry();
            }
        }
        if (subclinical_timer > UNINIT_TIMER)
        { // asymptomatic infection
            //              LOG_INFO_F("is subclinical dur %d, %d, %d\n", _subclinical_duration, subclinical_timer, dt);
            state_to_report="SUB";
            subclinical_timer -= dt;
            if (UNINIT_TIMER<subclinical_timer && subclinical_timer<=0)
            {
                handleSubclinicalExpiry();
            }
        }
        if (acute_timer > UNINIT_TIMER)
        {
            // acute infection
            state_to_report = "A";
            acute_timer -= dt;
            if( ( _acute_duration - acute_timer ) >= acute_treatment_day &&
                    ( ( _acute_duration - acute_timer - dt ) < acute_treatment_day ) && 
                    (randgen->e() < treatmentprobability)
              )
            {       //if they seek treatment and don't die, we are assuming they have a probability of becoming a carrier (chloramphenicol treatment does not prevent carriage)
                if (randgen->e() < CFRH)
                {
                    isDead = true;
                    state_to_report = "D";
                    acute_timer = UNINIT_TIMER;
                }
                else
                {
                    treatment_multiplier = 0.5;
                }
            }

            if (acute_timer<= 0)
            {
                handleAcuteExpiry();
            }
        }

        //Assume all acute individuals seek treatment since this is Mike's "reported" fraction
        //Some fraction are treated effectively, some become chronic carriers, some die, some remain infectious


        if (chronic_timer > UNINIT_TIMER)
        {
            state_to_report="C";
            chronic_timer -= dt;
            if (UNINIT_TIMER< chronic_timer && chronic_timer<=0)
                chronic_timer = UNINIT_TIMER;
        }

        if (last_state_reported==state_to_report)
        {
            // typhoid state is the same as before
            state_changed = false;
        }
        else
        {
            // typhoid state changed
            last_state_reported=state_to_report;
            state_changed = true;
        }
        LOG_DEBUG_F( "state_to_report for individual %d = %s\n", GetSuid().data, state_to_report.c_str() );

        if( state_to_report == "S" && state_changed ) // && GetInfections().size() > 0 )
        {
            Clear();
        }
        else if( state_to_report == "D" && state_changed )
        {    
            LOG_INFO_F( "[Update] Somebody died from their infection.\n" );
        }
        return;
        /*
        StateChange = InfectionStateChange::None;
        ISusceptibilityTyphoid* immunity = NULL;
        if( _immunity->QueryInterface( GET_IID( ISusceptibilityTyphoid ), (void**)&immunity ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "_immunity", "Susceptibility", "SusceptibilityTyphoid" );
        } */
        //return InfectionEnvironmental::Update( dt, _immunity );
    }

    float InfectionTyphoid::GetInfectiousness() const
    {
        float infectiousness = 0.0f;
        float base_infectiousness = IndividualHumanTyphoidConfig::typhoid_acute_infectiousness;
        auto irt = dynamic_cast<IDrugVaccineInterventionEffects*>(parent->GetInterventionsContext())->GetInterventionReducedTransmit();
        if (acute_timer>=0)
        {
            infectiousness = treatment_multiplier*base_infectiousness*irt;
        }
        else if (prepatent_timer>=0)
        {
            infectiousness = base_infectiousness*IndividualHumanTyphoidConfig::typhoid_prepatent_relative_infectiousness*irt;
        }
        else if (subclinical_timer>=0)
        {
            infectiousness = base_infectiousness*IndividualHumanTyphoidConfig::typhoid_subclinical_relative_infectiousness*irt;
        }
        else if (chronic_timer>=0)
        {
            infectiousness = base_infectiousness*IndividualHumanTyphoidConfig::typhoid_chronic_relative_infectiousness*irt;
        }
        return infectiousness;
    }

    void InfectionTyphoid::Clear()
    {
        LOG_DEBUG_F( "Infection cleared.\n" );
        StateChange = InfectionStateChange::Cleared;
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::InfectionTyphoid)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, InfectionTyphoid& inf, const unsigned int file_version )
    {
        ar & boost::serialization::base_object<Kernel::InfectionEnvironmental>(inf);
    }
    template void serialize( boost::mpi::packed_skeleton_iarchive&, Kernel::InfectionTyphoid&, unsigned int);
}
#endif

#endif // ENABLE_TYPHOID
