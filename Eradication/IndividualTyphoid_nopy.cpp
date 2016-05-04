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

///////////////////////////////////////
#ifndef WIN32
#include <sys/time.h> // DLC for gettimeofday
#endif
#define RANDFROMLOGNORMAL(m,s) (Probability::getInstance()->fromDistribution(  DistributionFunction::LOG_NORMAL_DURATION, m, s ))


#include "stdafx.h"

#ifdef ENABLE_TYPHOID
#include "Debug.h"
#include "Contexts.h"
#include "RANDOM.h"
#include "Environment.h"
#include "IndividualTyphoid_nopy.h"
#include "SusceptibilityTyphoid.h"
#include "InfectionTyphoid.h"
#include "IContagionPopulation.h"
#include "TyphoidInterventionsContainer.h"
#include "IdmString.h"
#include "SimulationConfig.h"

#pragma warning(disable: 4244)

static const char * _module = "IndividualTyphoid";

#define UNINIT_TIMER (-100.0f)

namespace Kernel
{
    class Stopwatch
    {
        public:
            Stopwatch( const char* label )
            {
#ifndef WIN32
                gettimeofday( &start_tv, nullptr );
#endif
                _label = label;
            }
            ~Stopwatch()
            {
#ifndef WIN32
                struct timeval stop_tv;
                gettimeofday( &stop_tv, nullptr );
                float timespentinms = ( stop_tv.tv_sec - start_tv.tv_sec ) + 1e-06* ( stop_tv.tv_usec - start_tv.tv_usec );
#endif
                //std::cout << "[" << _label << "] duration = " << timespentinms << std::endl;
            }
#ifndef WIN32
            struct timeval start_tv;
#endif
            std::string _label;
    };

    const double IndividualHumanTyphoid::P1 = 0.1111; // probability that an infection becomes clinical
    const double IndividualHumanTyphoid::P5 = 0.05; // probability of typhoid death
    //////////JG REMOVE double P6 = 0.0f; // probability of sterile immunity after acute infection
    const double IndividualHumanTyphoid::P7 = 0.0f; // probability of clinical immunity after acute infection
    //const //////////JG REMOVE double P8 = 0.0f; // probability of sterile immunity from a subclinical infectin in the clinically immune
    //const //////////JG REMOVE double P9 = 0.0f; // probability of sterile immunity from a subclinical infection
    const double IndividualHumanTyphoid::P10 = 0.0f; // probability of clinical immunity from a subclinical infection

    const int IndividualHumanTyphoid::_chronic_duration = 100000000;
    const int IndividualHumanTyphoid::_clinical_immunity_duration = 160*30;
    //////////JG REMOVE int _sterile_immunity_duration = 800*30;

    // Incubation period by transmission route (taken from Glynn's dose response analysis) assuming low dose for environmental.
    // mean and std dev of log normal distribution
    const double IndividualHumanTyphoid::mpe = 2.23;
    const double IndividualHumanTyphoid::spe = 0.05192995;
    const double IndividualHumanTyphoid::mpf = 1.55; // math.log(4.7)
    const double IndividualHumanTyphoid::spf = 0.0696814;

    // Subclinical infectious duration parameters: mean and standard deviation under and over 30
    const double IndividualHumanTyphoid::mso30=3.430830;
    const double IndividualHumanTyphoid::sso30=0.922945;
    const double IndividualHumanTyphoid::msu30=3.1692211;
    const double IndividualHumanTyphoid::ssu30=0.5385523;

    // Acute infectious duration parameters: mean and standard deviation under and over 30
    const double IndividualHumanTyphoid::mao30=3.430830;
    const double IndividualHumanTyphoid::sao30=0.922945;
    const double IndividualHumanTyphoid::mau30=3.1692211;
    const double IndividualHumanTyphoid::sau30=0.5385523;

    const int IndividualHumanTyphoid::acute_symptoms_duration = 5; // how long people are symptomatic in days
    const double IndividualHumanTyphoid::CFRU = 0.1;   // case fatality rate?
    const double IndividualHumanTyphoid::CFRH = 0.005; // hospitalized case fatality rate?
    const double IndividualHumanTyphoid::treatmentprobability = 0.9;  // probability of treatment

    // typhoid constants from "OutBase.csv" file
    //////////JG REMOVE static double agechronicmale[200]; //probability of becoming chronic carrier, male
    //////////JG REMOVE static double agechronicfemale[200]; //probability of becoming chronic carrier, female

    // environmental exposure constants
    const int IndividualHumanTyphoid::N50 = 1110000;
    const double IndividualHumanTyphoid::alpha = 0.175;

    GET_SCHEMA_STATIC_WRAPPER_IMPL(Typhoid.Individual,IndividualHumanTyphoid)
    BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanTyphoid, IndividualHumanEnvironmental)
        HANDLE_INTERFACE(IIndividualHumanTyphoid)
    END_QUERY_INTERFACE_DERIVED(IndividualHumanTyphoid, IndividualHumanEnvironmental)

    IndividualHumanTyphoid::IndividualHumanTyphoid(suids::suid _suid, float monte_carlo_weight, float initial_age, int gender, float initial_poverty) :
        IndividualHumanEnvironmental(_suid, monte_carlo_weight, initial_age, gender, initial_poverty)
    {
        last_state_reported = "S";
        _infection_count=0;
        //////////JG REMOVE hasSterileImmunity = false;
        hasClinicalImmunity = false;
        chronic_timer = UNINIT_TIMER;
        subclinical_timer = UNINIT_TIMER;
        acute_timer = UNINIT_TIMER;
        prepatent_timer = UNINIT_TIMER;
        //sterile_immunity_timer = UNINIT_TIMER;
        clinical_immunity_timer =UNINIT_TIMER;
        _subclinical_duration = _prepatent_duration = _acute_duration = 0;
        _routeOfInfection = TransmissionRoute::TRANSMISSIONROUTE_ALL;// IS THIS OK for a default? DLC
        isDead = false;
    }

    bool
    IndividualHumanTyphoid::Configure( const Configuration* config ) // just called once!
    {
        LOG_DEBUG( "Configure\n" );
        // typhoid
        SusceptibilityTyphoidConfig fakeImmunity;
        fakeImmunity.Configure( config );
        InfectionTyphoidConfig fakeInfection;
        fakeInfection.Configure( config );

        //do we need to call initConfigTypeMap? DLC 
        return IndividualHumanEnvironmental::Configure( config );
    }

    IndividualHumanTyphoid *IndividualHumanTyphoid::CreateHuman(INodeContext *context, suids::suid id, float monte_carlo_weight, float initial_age, int gender, float initial_poverty)
    {
        IndividualHumanTyphoid *newhuman = _new_ IndividualHumanTyphoid(id, monte_carlo_weight, initial_age, gender, initial_poverty);
        
        newhuman->SetContextTo(context);
        LOG_DEBUG_F( "Created human with age=%f\n", newhuman->m_age );
        return newhuman;
    }

    void IndividualHumanTyphoid::PropagateContextToDependents()
    {
        IndividualHuman::PropagateContextToDependents();
        typhoid_susceptibility = static_cast<SusceptibilityTyphoid*>(susceptibility);
    }

    void IndividualHumanTyphoid::setupInterventionsContainer()
    {
        interventions = _new_ TyphoidInterventionsContainer();
    }

    void IndividualHumanTyphoid::CreateSusceptibility(float imm_mod, float risk_mod)
    {
        SusceptibilityTyphoid *newsusceptibility = SusceptibilityTyphoid::CreateSusceptibility(this, m_age, imm_mod, risk_mod);
        typhoid_susceptibility = newsusceptibility;
        susceptibility = newsusceptibility;
    }

    void IndividualHumanTyphoid::Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route )
    {
        if( cp->GetTotalContagion() == 0 )
        {
            return;
        }

        if( randgen->e() > GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_fraction )
        {
            return; // what does this mean???????
        }

        ///////////////////
        //        # if random_number_draw < contagion_population * dt * individual_modifiers_based_on_immunity_and_ interventions
        // call AcquireInfection
        if ( chronic_timer>UNINIT_TIMER || subclinical_timer>UNINIT_TIMER || acute_timer>UNINIT_TIMER || prepatent_timer>UNINIT_TIMER || GetAge()<1.0)
        {
            return;// 0;
        }

        if (transmission_route==TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL)
        {
            int SimDay = (int)parent->GetTime().time; // is this the date of the simulated year?
            int nDayOfYear = SimDay % 365;
            double fEnvironment = cp->GetTotalContagion(); //JG: should this be getenvironmentalcontagion? is there such a thing? 
            if (nDayOfYear >=91 && nDayOfYear<=274)
                fEnvironment *= 0.2; // less exposure during the rainy season
            double fExposure = fEnvironment * GET_CONFIGURABLE(SimulationConfig)->typhoid_environmental_amplification;

            double infects = pow(1-(1 + GET_CONFIGURABLE(SimulationConfig)->typhoid_environmental_exposure * (pow(2,(1/alpha)-1))/N50),-alpha); // DOES THIS INFECT INFECTED PEOPLE? ##took out dt
            double immunity= max(0.01f, 1-(_infection_count*GET_CONFIGURABLE(SimulationConfig)->typhoid_protection_per_infection)); //  change this later to distribution
            double prob = min(1.0f, 1.0f- (float) pow(1.0f-(immunity * infects),dt));

            //#add probability reduction according to previous infection. 10% protection per infection. or clincial?

            if (randgen->e() < prob) {
                //#print( "Individual " + str(self._id) + " infected over route " + route )
                _routeOfInfection = transmission_route;

                StrainIdentity strainId; // how to initialize?
                AcquireNewInfection(&strainId);  // do we need this?
                return;// 1;
            } else {
                return;// 0;
            }
        } else if (transmission_route==TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL){
            //JG- I've left this out- we need is to start individuals shedding into both environmental and contact contagions and exposing them at a high dose/super low exposure %

            return;// 0; // no person-to-person transmission?
            //return IndividualHumanEnvironmental::Expose( cp, dt, transmission_route );
        }
    }

    void IndividualHumanTyphoid::ExposeToInfectivity(float dt, const TransmissionGroupMembership_t* transmissionGroupMembership)
    {
        IndividualHumanEnvironmental::ExposeToInfectivity(dt, transmissionGroupMembership);
    }

    void IndividualHumanTyphoid::UpdateInfectiousness(float dt)
    {
        infectiousness = 0.0f;
        // Would be great to optmize this so as only to call into Py layer if individual is infected.
        if( IsInfected() == false && IsChronicCarrier() == false )
        {
            return;
        }
        if (acute_timer>=0)
            infectiousness += GET_CONFIGURABLE(SimulationConfig)->typhoid_acute_infectivity;
        else if (prepatent_timer>=0)
            infectiousness += GET_CONFIGURABLE(SimulationConfig)->typhoid_prepatent_infectivity;
        else if (subclinical_timer>=0)
            infectiousness += GET_CONFIGURABLE(SimulationConfig)->typhoid_subclinical_infectivity;
        else if (chronic_timer>=0)
            infectiousness += GET_CONFIGURABLE(SimulationConfig)->typhoid_chronic_infectivity;

        //parent->GetTransmissionRoutes()
        string route = "environmental"; // DLC - I have no idea if this makes sense!!!!!!!!
        StrainIdentity tmp_strainID;
        release_assert( transmissionGroupMembershipByRoute.find( route ) != transmissionGroupMembershipByRoute.end() );
        if (infectiousness>0.0)
            parent->DepositFromIndividual( &tmp_strainID, (float) infectiousness, &transmissionGroupMembershipByRoute.at( route ) ); // shed to environment

        // note: no "contact" route infectivity
        //        IndividualHumanEnvironmental::UpdateInfectiousness(dt);
    }

    Infection* IndividualHumanTyphoid::createInfection( suids::suid _suid )
    {
        return InfectionTyphoid::CreateInfection(this, _suid);
    }

    std::string IndividualHumanTyphoid::processPrePatent( float dt )
    {
        return state_to_report;
    }

    void IndividualHumanTyphoid::Update( float currenttime, float dt)
    {
        state_to_report = "S"; // default state is susceptible
        if( IsInfected() )
        {
            if (prepatent_timer > UNINIT_TIMER)
            { // pre-patent
                state_to_report="P";
                prepatent_timer -= dt;
                if( UNINIT_TIMER<prepatent_timer<=0 )
                {
                    prepatent_timer=UNINIT_TIMER;
                    if (hasClinicalImmunity) {
                        if (GetAge() < 30.0)
                            _subclinical_duration = int(RANDFROMLOGNORMAL(msu30, ssu30));
                        else
                            _subclinical_duration = int(RANDFROMLOGNORMAL(mso30, sso30));
                        // DLC - Do we need to truncate the duration to 365?
                        subclinical_timer = _subclinical_duration;
                        if (_subclinical_duration > 365)
                            isAmesChronic = true;
                        else
                            isAmesChronic = false;
                    } else if (!hasClinicalImmunity) {
                        if (randgen->e()<P1) {
                            if (GetAge() < 30.0)
                                _acute_duration = int(RANDFROMLOGNORMAL(mau30, sau30));
                            else
                                _acute_duration = int(RANDFROMLOGNORMAL(mao30, sao30));
                            // DLC - Do we need to truncate the duration to 365?
                            acute_timer = _acute_duration;
                            if (_acute_duration > 365)
                                isAmesChronic = true;
                        } else {
                            if (GetAge() < 30.0)
                                _subclinical_duration = int(RANDFROMLOGNORMAL( msu30, ssu30));
                            else
                                _subclinical_duration = int(RANDFROMLOGNORMAL( mso30, sso30));
                            // DLC - Do we need to truncate the duration to 365?
                            subclinical_timer = _subclinical_duration;
                            if (_subclinical_duration > 365)
                                isAmesChronic = true;
                        }
                    }
                }

                if (subclinical_timer > UNINIT_TIMER)
                { // asymptomatic infection
                    state_to_report="SUB";
                    subclinical_timer -= dt;
                    if (UNINIT_TIMER<subclinical_timer<=0)
                    {
                        double p2; // probability of infection turning chronic from subclinical
                        int ageInYrs = (int)GetAge();
                        if (isAmesChronic)
                        {
                            p2 = 1.0;
                        }
                        else
                        {
                            p2 = 0.0;
                        }
                        //#endif

                        if (randgen->e() < p2) {
                            chronic_timer = _chronic_duration;
                        }
                        else
                        {
                            // shift individuals who recovered into immunity states
                            if (hasClinicalImmunity)
                            {
                                clinical_immunity_timer += _clinical_immunity_duration;
                            }
                            else
                            {
                                if (randgen->e() < P10)
                                {
                                    hasClinicalImmunity = true;
                                    clinical_immunity_timer = _clinical_immunity_duration;
                                }
                            }
                        }
                    }
                    subclinical_timer = UNINIT_TIMER;
                }
            }
            if (acute_timer > UNINIT_TIMER)
            {
                // acute infection
                state_to_report = "A";
                acute_timer -= dt;
                if( ( _acute_duration - acute_timer ) <= acute_symptoms_duration &&
                    ( ( _acute_duration - acute_timer + dt ) < acute_symptoms_duration )
                  )
                {
                    if (randgen->e() < treatmentprobability)
                    {
                        //if they don't die and seek treatment, we are assuming they recover
                        if (randgen->e() < CFRH)
                        {
                            isDead = true;
                            // Do we need to set the state to "D"?
                                    acute_timer = UNINIT_TIMER;
                        } else 
                        {
                            acute_timer = UNINIT_TIMER;
                        }
                    }

                    //if they dont seek treatment, just keep them infectious until the end of their acute stage


                    //Assume all acute individuals seek treatment since this is Mike's "reported" fraction
                    //Some fraction are treated effectively, some become chronic carriers, some die, some remain infectious

                    //assume all other risks are dependent on unsuccessful treatment
                    if (UNINIT_TIMER < acute_timer <= 0)
                    {
                        if (randgen->e() < CFRU)
                        {
                            isDead = true;
                        }
                        else
                        {  //if they survived, calculate probability of being a carrier
                            double p3 = 0.0; // P3 is age dependent so is determined below. Probability of becoming a chronic carrier from a CLINICAL infection
                            if (isAmesChronic)
                            {
                                p3 = 1.0;
                            }
                            else 
                            {
                                p3 = 0.0;
                            }
                            if (randgen->e() < p3)
                            {
                                chronic_timer = _chronic_duration;
                            }
                            else
                            {
                                // for other recovereds, calculate probability of becoming immune
                                if (randgen->e() < P7)
                                {
                                    hasClinicalImmunity = true;
                                    clinical_immunity_timer = _clinical_immunity_duration;
                                }
                            }
                        }
                    }
                }
                acute_timer = UNINIT_TIMER;
            }
        }

        if (chronic_timer > UNINIT_TIMER) {
            state_to_report="C";
            chronic_timer -= dt;
            if (UNINIT_TIMER< chronic_timer<=0)
                chronic_timer = UNINIT_TIMER;
        }

        if (hasClinicalImmunity && subclinical_timer ==UNINIT_TIMER& prepatent_timer ==UNINIT_TIMER )
        {
            state_to_report="CI";
            clinical_immunity_timer -= dt;
            if (clinical_immunity_timer <= 0) {
                hasClinicalImmunity = false;
                clinical_immunity_timer = UNINIT_TIMER;
            }
        }

        if (last_state_reported==state_to_report) {
            // typhoid state is the same as before
            state_to_report += ":EXISTING";
        } else {
            // typhoid state changed
            last_state_reported=state_to_report;
            state_to_report += ":NEW";
        }
        // take care of this later! DLC
        //        if self._id in tracking_sample:
        // store this individuals state in report
        //            tracking_report[ self._id ].append( state_to_report )
    
        // Take care of this later!!!!!!!!!!
        LOG_DEBUG_F( "state_to_report for individual %d = %s\n", GetSuid().data, state_to_report.c_str() );

        if( state_to_report == "S:NEW" && GetInfections().size() > 0 )
        {
            // ClearInfection
            auto inf = GetInfections().front();
            IInfectionTyphoid * inf_typhoid  = NULL;
            if (s_OK != inf->QueryInterface(GET_IID(IInfectionTyphoid ), (void**)&inf_typhoid) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "inf", "IInfectionTyphoid ", "Infection" );
            }
            // get InfectionTyphoid pointer
            inf_typhoid->Clear();
        }
        else if( state_to_report == "D:NEW" )
        {
            LOG_INFO_F( "[Update] Somebody died from their infection.\n" );
        }
        IndividualHumanEnvironmental::Update( currenttime, dt );
    }

    void IndividualHumanTyphoid::AcquireNewInfection(StrainIdentity *infstrain, int incubation_period_override )
    {
        IndividualHumanEnvironmental::AcquireNewInfection( infstrain, incubation_period_override );
        infectious_timer = 30; // Do we need this variable?
        if (_routeOfInfection == TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL)
            _prepatent_duration = int(RANDFROMLOGNORMAL(mpe, spe));
        else 
            _prepatent_duration = int(RANDFROMLOGNORMAL(mpf, spf));
        _infection_count ++;
        prepatent_timer=_prepatent_duration;
    }

    HumanStateChange IndividualHumanTyphoid::GetStateChange() const
    {
        HumanStateChange retVal = StateChange;
        auto parsed = IdmString(state_to_report).split();
        if( parsed[0] == "D" )
        {
            LOG_INFO_F( "[GetStateChange] Somebody died from their infection.\n" );
            retVal = HumanStateChange::KilledByInfection;
        }
        return retVal;
    }

    bool IndividualHumanTyphoid::IsChronicCarrier( bool incidence_only ) const
    {
        return false;

        auto parsed = IdmString(state_to_report).split();
        if( parsed[0] == "C" &&
            ( ( incidence_only && parsed[1] == "NEW" ) ||
              ( incidence_only == false )
            )
          )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool IndividualHumanTyphoid::IsSubClinical( bool incidence_only ) const
    {
        return false;

        auto parsed = IdmString(state_to_report).split();
        if( parsed[0] == "SUB" &&
            ( ( incidence_only && parsed[1] == "NEW" ) ||
              ( incidence_only == false )
            )
          )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool IndividualHumanTyphoid::IsAcute( bool incidence_only ) const
    {
        return false;

        auto parsed = IdmString(state_to_report).split();
        if( parsed[0] == "A" &&
            ( ( incidence_only && parsed[1] == "NEW" ) ||
              ( incidence_only == false )
            )
          )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool IndividualHumanTyphoid::IsPrePatent( bool incidence_only ) const
    {
        return false;

        auto parsed = IdmString(state_to_report).split();
        if( parsed[0] == "P" &&
            ( ( incidence_only && parsed[1] == "NEW" ) ||
              ( incidence_only == false )
            )
          )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
#include "InfectionTyphoid.h"
#include "SusceptibilityTyphoid.h"
#include "TyphoidInterventionsContainer.h"

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Kernel::IndividualHumanTyphoid)

/*
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, IndividualHumanTyphoid& human, const unsigned int  file_version )
    {
        LOG_DEBUG("(De)serializing IndividualHumanTyphoid\n");

        ar.template register_type<Kernel::InfectionTyphoid>();
        ar.template register_type<Kernel::SusceptibilityTyphoid>();
        ar.template register_type<Kernel::TyphoidInterventionsContainer>();
            
        // Serialize fields - N/A
        

        // Serialize base class
        ar & boost::serialization::base_object<Kernel::IndividualHumanEnvironmental>(human);
    }
}
*/

#endif

#endif // ENABLE_TYPHOID
