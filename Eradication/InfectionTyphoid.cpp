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
        //return (exp((m)+randgen->eGauss()*s));
        return Probability::getInstance()->fromDistribution( DistributionFunction::LOG_NORMAL_DURATION, exp(m), s );
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

#define SUSCEPT_STATE_LABEL "SUS"
#define PREPAT_STATE_LABEL "PRE"
#define ACUTE_STATE_LABEL "ACU"
#define SUBCLINICAL_STATE_LABEL "SUB"
#define CHRONIC_STATE_LABEL "CHR"
#define DEAD_STATE_LABEL "DED"

    InfectionTyphoid::InfectionTyphoid(IIndividualHumanContext *context) : InfectionEnvironmental(context)
    {
        treatment_multiplier = 1;
        chronic_timer = UNINIT_TIMER;
        subclinical_timer = UNINIT_TIMER;
        acute_timer = UNINIT_TIMER;
        //prepatent_timer = UNINIT_TIMER;
        _subclinical_duration = _prepatent_duration = _acute_duration = 0;
        isDead = false;
        last_state_reported = SUSCEPT_STATE_LABEL;
        // TBD: Nasty cast: prefer QI.
        auto doseTracking = ((IndividualHumanTyphoid*)context)->getDoseTracking();

        float mu = mpl;
        float sigma = spl;
        if (doseTracking == "High")
        {
            mu = mph; sigma = sph;
        }
        else if (doseTracking == "Medium")
        {
            mu = mpm; sigma = spm;
        }
        else if (doseTracking == "Low")
        {
            mu = mpl; sigma = spl;
        }
        else
        {
            LOG_WARN_F( "doseTracking not set. This will lead to bad prepatent durations.\n" );
        }

        _prepatent_duration = (int)(generateRandFromLogNormal(mu, sigma));
        LOG_VALID_F( "Calculated prepatent duration = %d using Log-Normal draw; mu = %f, sigma = %f, doseTracking = %s.\n",
                     _prepatent_duration, mu, sigma, doseTracking.c_str() );
        prepatent_timer =_prepatent_duration;
        prepatent_timer.handle = std::bind( &InfectionTyphoid::handlePrepatentExpiry, this );
        state_to_report=PREPAT_STATE_LABEL;

        //std::cout << "Initialized prepatent_timer to " << prepatent_timer << " using doseTracking value of " << doseTracking << std::endl;
    }

    void InfectionTyphoid::Initialize(suids::suid _suid)
    {
        InfectionEnvironmental::Initialize(_suid);
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
        //LOG_DEBUG_F("hasclin subclinical dur %d, pre %d\n", _subclinical_duration, prepatent_timer); 
        //prepatent_timer=UNINIT_TIMER; 
        LOG_DEBUG_F( "Deciding post-prepatent tx using typhoid_symptomatic_fraction=%f.\n", IndividualHumanTyphoidConfig::typhoid_symptomatic_fraction );
        float mu = 0.0f;
        float sigma = 0.0f;
        if (randgen->e()<(IndividualHumanTyphoidConfig::typhoid_symptomatic_fraction*mort)) //THIS IS NOT ACTUALLY MORTALITY, I AM JUST USING THE CALL 
        {
            if (age < ageThresholdInYears)
            {
                mu = mau30; sigma = sau30;
            } else {
                mu = mao30; sigma = sao30;
            }
            LOG_DEBUG_F("Infection stage transition: Individual=%d, Age=%f, Prepatent->Acute: acute dur=%d\n", parent->GetSuid().data, age, _acute_duration);
            _acute_duration = int(generateRandFromLogNormal( mu, sigma ) * DAYSPERWEEK );
            acute_timer = _acute_duration;
            state_to_report=ACUTE_STATE_LABEL;
        }
        else
        {
            if (age <= ageThresholdInYears)
            {
                mu = msu30; sigma = ssu30;
            } else {
                mu = mso30; sigma = sso30;
            }
            _subclinical_duration = int(generateRandFromLogNormal( mu, sigma ) * DAYSPERWEEK );
            subclinical_timer = _subclinical_duration;
            LOG_DEBUG_F("Infection stage transition: Individual=%d, Age=%f, Prepatent->SubClinical: subc dur=%d\n", parent->GetSuid().data, age, _subclinical_duration );
            state_to_report=SUBCLINICAL_STATE_LABEL;
        }
    }

    void InfectionTyphoid::handleAcuteExpiry()
    {
        auto age = dynamic_cast<IIndividualHuman*>(parent)->GetAge() / DAYSPERYEAR;
        auto sex = dynamic_cast<IIndividualHuman*>(parent)->GetGender();

        if ((randgen->e() < CFRU) & (treatment_multiplier < 1)) // untreated at end of period has higher fatality rate
        {
            isDead = true;
            state_to_report = DEAD_STATE_LABEL;
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
            if (sex==Gender::FEMALE)
            {
                p3=FemaleGallstones[agebin];
                carrier_prob = IndividualHumanTyphoidConfig::typhoid_carrier_probability_female;
            } 
            else // if (sex==Gender::MALE)
            {
                p3=MaleGallstones[agebin];
                carrier_prob = IndividualHumanTyphoidConfig::typhoid_carrier_probability_male;
            }
            LOG_DEBUG_F( "Deciding whether to go from acute->chronic based on probability=%f.\n", p3*carrier_prob);
            if (randgen->e()< p3*carrier_prob)
            {
                chronic_timer = _chronic_duration;
                LOG_DEBUG_F( "Individual %d age %f, sex %d, just went chronic (from acute) with timer %f based on gallstone probability of %f and carrier probability of %f.\n",
                             GetSuid().data, age, sex, chronic_timer, p3, carrier_prob
                           );
                state_to_report = CHRONIC_STATE_LABEL;
            }
            else
            {
                LOG_VALID_F( "Individual %d age %f, sex %d, just recovered (from acute).\n", GetSuid().data, age, sex );
                state_to_report = SUSCEPT_STATE_LABEL;
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
        if( sex == Gender::FEMALE )
        {
            p2=FemaleGallstones[agebin];
            carrier_prob = IndividualHumanTyphoidConfig::typhoid_carrier_probability_female ;
        } 
        else // if (sex==0)
        {
            p2=MaleGallstones[agebin];
            carrier_prob = IndividualHumanTyphoidConfig::typhoid_carrier_probability_male;
        } 

        if (randgen->e() < p2*carrier_prob)
        {
            chronic_timer = _chronic_duration;
            LOG_DEBUG_F( "Individual age %f, sex %d, just went chronic (from subclinical) with timer %f based on gallstone probability of %f and carrier probability of %f.\n",
                         age, sex, chronic_timer, p2, carrier_prob
                       );
            state_to_report = CHRONIC_STATE_LABEL;
        }
        else
        {
            state_to_report = SUSCEPT_STATE_LABEL;
        }
    }

    void InfectionTyphoid::Update(float dt, ISusceptibilityContext* _immunity)
    {
        bool state_changed = false;

        LOG_DEBUG_F("%d INFECTED! prepat=%d,acute=%d,subclin=%d,chronic=%d\n", parent->GetSuid().data, (int) prepatent_timer, acute_timer, subclinical_timer,chronic_timer);
        prepatent_timer.Decrement( dt );
        if (subclinical_timer > UNINIT_TIMER)
        { // asymptomatic infection
            //              LOG_INFO_F("is subclinical dur %d, %d, %d\n", _subclinical_duration, subclinical_timer, dt);
            state_to_report=SUBCLINICAL_STATE_LABEL;
            subclinical_timer -= dt;
            if (UNINIT_TIMER<subclinical_timer && subclinical_timer<=0)
            {
                handleSubclinicalExpiry();
            }
        }
        if (acute_timer > UNINIT_TIMER)
        {
            // acute infection
            state_to_report = ACUTE_STATE_LABEL;
            acute_timer -= dt;
            if( ( _acute_duration - acute_timer ) >= acute_treatment_day &&
                    ( ( _acute_duration - acute_timer - dt ) < acute_treatment_day ) && 
                    (randgen->e() < treatmentprobability)
              )
            {       //if they seek treatment and don't die, we are assuming they have a probability of becoming a carrier (chloramphenicol treatment does not prevent carriage)
                // so they either get treatment or die?
                if (randgen->e() < CFRH)
                {
                    isDead = true;
                    state_to_report = DEAD_STATE_LABEL;
                    acute_timer = UNINIT_TIMER;
                }
                else
                {
                    treatment_multiplier = 0.5;
                    LOG_VALID_F( "Individual ID: %d, State: Acute, GetTreatment: True.\n", parent->GetSuid().data );
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
            state_to_report=CHRONIC_STATE_LABEL;
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

        if( state_to_report == SUSCEPT_STATE_LABEL && state_changed ) // && GetInfections().size() > 0 )
        {
            Clear();
        }
        else if( state_to_report == DEAD_STATE_LABEL && state_changed )
        {    
            LOG_INFO_F( "[Update] Somebody died from their infection.\n" );
        }
        return;
    }

    float InfectionTyphoid::GetInfectiousness() const
    {
        float infectiousness = 0.0f;
        float base_infectiousness = IndividualHumanTyphoidConfig::typhoid_acute_infectiousness;
        auto irt = dynamic_cast<IDrugVaccineInterventionEffects*>(parent->GetInterventionsContext())->GetInterventionReducedTransmit();
        if (acute_timer>0)
        {
            infectiousness = treatment_multiplier*base_infectiousness*irt;
            LOG_VALID_F( "ACUTE infectiousness calculated as %f\n", infectiousness );
        }
        else if (prepatent_timer>0)
        {
            infectiousness = base_infectiousness*IndividualHumanTyphoidConfig::typhoid_prepatent_relative_infectiousness*irt;
            LOG_VALID_F( "PREPATENT infectiousness calculated as %f\n", infectiousness );
        }
        else if (subclinical_timer>0)
        {
            infectiousness = base_infectiousness*IndividualHumanTyphoidConfig::typhoid_subclinical_relative_infectiousness*irt;
            LOG_VALID_F( "SUBCLINICAL infectiousness calculated as %f\n", infectiousness );
        }
        else if (chronic_timer>0)
        {
            infectiousness = base_infectiousness*IndividualHumanTyphoidConfig::typhoid_chronic_relative_infectiousness*irt;
            LOG_VALID_F( "CHRONIC infectiousness calculated as %f\n", infectiousness );
        }
        return infectiousness;
    }

    void InfectionTyphoid::Clear()
    {
        LOG_DEBUG_F( "Infection cleared.\n" );
        StateChange = InfectionStateChange::Cleared;
    }

    REGISTER_SERIALIZABLE(InfectionTyphoid);

    void InfectionTyphoid::serialize(IArchive& ar, InfectionTyphoid* obj)
    {
        InfectionTyphoid& infection = *obj;
        ar.labelElement("prepatent_timer") & infection.prepatent_timer;
        ar.labelElement("prepatent_duration") & infection._prepatent_duration;
        ar.labelElement("acute_timer") & infection.acute_timer;
        ar.labelElement("acute_duration") & infection._acute_duration;
        ar.labelElement("subclinical_timer") & infection.subclinical_timer;
        ar.labelElement("subclinical_duration") & infection._subclinical_duration;
        ar.labelElement("chronic_timer") & infection.chronic_timer;
        //ar.labelElement("chronic_duration") & infection._chronic_duration;
        ar.labelElement("state_to_report") & infection.state_to_report;
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
