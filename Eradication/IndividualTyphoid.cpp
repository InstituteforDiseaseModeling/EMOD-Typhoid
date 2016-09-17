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
#include "Debug.h"
#include "Contexts.h"
#include "RANDOM.h"
#include "Environment.h"
#include "IndividualTyphoid.h"
#include "SusceptibilityTyphoid.h"
#include "InfectionTyphoid.h"
#include "IContagionPopulation.h"
#include "TyphoidInterventionsContainer.h"
#include "IdmString.h"
#include "SimulationConfig.h"

#ifndef WIN32
#include <sys/time.h>
#endif

#ifdef ENABLE_PYTHOID 
#include "Python.h"
extern PyObject *
IdmPyInit(
        const char * python_script_name,
        const char * python_function_name
        );
#endif

#pragma warning(disable: 4244)

static const char * _module = "IndividualTyphoid";

#define UNINIT_TIMER (-100.0f)


namespace Kernel
{
    inline float generateRandFromLogNormal(float m, float s) {
        // inputs: m is mean of underlying distribution, s is std dev
        return (exp((m)+randgen->eGauss()*s));
    }


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
    const float IndividualHumanTyphoid::P5 = 0.05f; // probability of typhoid death
    const float IndividualHumanTyphoid::P7 = 0.0f; // probability of clinical immunity after acute infection
    const float IndividualHumanTyphoid::P10 = 0.0f; // probability of clinical immunity from a subclinical infection

    const int IndividualHumanTyphoid::_chronic_duration = 100000000;
    const int IndividualHumanTyphoid::_clinical_immunity_duration = 160*30;

    // Incubation period by transmission route (taken from Glynn's dose response analysis) assuming low dose for environmental.
    // mean and std dev of log normal distribution
    const float IndividualHumanTyphoid::mpe = 2.23f;
    const float IndividualHumanTyphoid::spe = 0.05192995f;
    const float IndividualHumanTyphoid::mpf = 1.55f; // math.log(4.7)
    const float IndividualHumanTyphoid::spf = 0.0696814f;

    // Subclinical infectious duration parameters: mean and standard deviation under and over 30 (refitted without carriers)
    //const float IndividualHumanTyphoid::mso30=3.430830f;
    //const float IndividualHumanTyphoid::sso30=0.922945f;
    //const float IndividualHumanTyphoid::msu30=3.1692211f;
    //const float IndividualHumanTyphoid::ssu30=0.5385523f;
    const float IndividualHumanTyphoid::mso30=1.2584990f;
    const float IndividualHumanTyphoid::sso30=0.7883767f;
    const float IndividualHumanTyphoid::msu30=1.171661f;
    const float IndividualHumanTyphoid::ssu30=0.483390f;

    // Acute infectious duration parameters: mean and standard deviation under and over 30
    //const float IndividualHumanTyphoid::mao30=3.430830f;
    //const float IndividualHumanTyphoid::sao30=0.922945f;
    //const float IndividualHumanTyphoid::mau30=3.1692211f;
    //const float IndividualHumanTyphoid::sau30=0.5385523f;
    const float IndividualHumanTyphoid::mao30=1.2584990f;
    const float IndividualHumanTyphoid::sao30=0.7883767f;
    const float IndividualHumanTyphoid::mau30=1.171661f;
    const float IndividualHumanTyphoid::sau30=0.483390f;

    const int IndividualHumanTyphoid::acute_treatment_day = 5; // how many days after infection will people receive treatment
    const float IndividualHumanTyphoid::CFRU = 0.08f;   // case fatality rate?
    const float IndividualHumanTyphoid::CFRH = 0.08f; // hospitalized case fatality rate?
    const float IndividualHumanTyphoid::treatmentprobability = 1.0f;  // probability of treatment seeking for an acute case. we are in santiago so assume 100%

    // environmental exposure constants
    const int IndividualHumanTyphoid::N50 = 1110000;
    const float IndividualHumanTyphoid::alpha = 0.175f;

    const int GallstoneDataLength= 9;
    const double FemaleGallstones[GallstoneDataLength] = {0.0, 0.097, 0.234, 0.431, 0.517, 0.60, 0.692, 0.692, 0.555}; // 10-year age bins
    const double MaleGallstones[GallstoneDataLength] = {0.0, 0.0, 0.045, 0.134, 0.167, 0.198, 0.247, 0.435, 0.4};
    GET_SCHEMA_STATIC_WRAPPER_IMPL(Typhoid.Individual,IndividualHumanTyphoid)
        BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanTyphoid, IndividualHumanEnvironmental)
        HANDLE_INTERFACE(IIndividualHumanTyphoid)
        END_QUERY_INTERFACE_DERIVED(IndividualHumanTyphoid, IndividualHumanEnvironmental)

        IndividualHumanTyphoid::IndividualHumanTyphoid(suids::suid _suid, float monte_carlo_weight, float initial_age, int gender, float initial_poverty) :
            IndividualHumanEnvironmental(_suid, monte_carlo_weight, initial_age, gender, initial_poverty)
    {
#ifdef ENABLE_PYTHOID
        volatile Stopwatch * check = new Stopwatch( __FUNCTION__ );
        // Call into python script to notify of new individual
        static auto pFunc = IdmPyInit( "dtk_typhoid_individual", "create" );
        if( pFunc )
        {
            // pass individual id
            static PyObject * vars = PyTuple_New(4);
            PyObject* py_newid = PyLong_FromLong( _suid.data );
            PyObject* py_newmcweight = PyFloat_FromDouble( monte_carlo_weight );
            PyObject* py_newage = PyFloat_FromDouble( initial_age );
            PyObject* py_newsex_str = PyString_FromFormat( "%s", ( ( gender==0 ) ? "MALE" : "FEMALE" ) );

            PyTuple_SetItem(vars, 0, py_newid );
            PyTuple_SetItem(vars, 1, py_newmcweight );
            PyTuple_SetItem(vars, 2, py_newage );
            PyTuple_SetItem(vars, 3, py_newsex_str );
            PyObject_CallObject( pFunc, vars );

            //Py_DECREF( vars );
            //Py_DECREF( py_newid_str );
            //Py_DECREF( py_newmcweight_str );
            //Py_DECREF( py_newage_str );
            PyErr_Print();
        }
        delete check;
#else 
        last_state_reported = "S";
        _infection_count=0; // should not be necessary
        hasClinicalImmunity = false;
        chronic_timer = UNINIT_TIMER;
        subclinical_timer = UNINIT_TIMER;
        acute_timer = UNINIT_TIMER;
        prepatent_timer = UNINIT_TIMER;
        clinical_immunity_timer =UNINIT_TIMER;
        _subclinical_duration = _prepatent_duration = _acute_duration = 0;
        _routeOfInfection = TransmissionRoute::TRANSMISSIONROUTE_ALL;// IS THIS OK for a default? DLC
        isDead = false;

#endif
    }

    IndividualHumanTyphoid::~IndividualHumanTyphoid()
    {
#ifdef ENABLE_PYTHOID
        volatile Stopwatch * check = new Stopwatch( __FUNCTION__ );
        // Call into python script to notify of new individual
        static auto pFunc = IdmPyInit( "dtk_typhoid_individual", "destroy" );
        if( pFunc )
        {
            static PyObject * vars = PyTuple_New(1);
            PyObject* py_id = PyLong_FromLong( GetSuid().data );
            PyTuple_SetItem(vars, 0, py_id );
            PyObject_CallObject( pFunc, vars );
            //Py_DECREF( vars );
            //Py_DECREF( py_id_str  );
            PyErr_Print();
        }
        delete check;
#endif
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
#ifdef ENABLE_PYTHOID
        if( randgen->e() > GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_fraction )
        {
            return;
        }

        volatile Stopwatch * check = new Stopwatch( __FUNCTION__ );
        static auto pFunc = IdmPyInit( "dtk_typhoid_individual", "expose" );
        if( pFunc )
        {
            // pass individual id AND dt
            static PyObject * vars = PyTuple_New(4);
            PyObject* py_existing_id = PyLong_FromLong( GetSuid().data );

            // silly. can't seem to figure out how to do floats so doing this way for now!
            PyObject* py_contagion_pop = PyLong_FromLong( cp->GetTotalContagion() );

            //PyObject* py_contagion_pop = Py_BuildValue( "%f", 
            PyObject* py_dt = PyLong_FromLong( dt );

            //PyObject* py_tx_route = PyString_FromFormat( "%s", TransmissionRoute::pairs::lookup_key( transmission_route ) );
            PyObject* py_tx_route = PyLong_FromLong( transmission_route == TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL ? 0 : 1 );
            PyTuple_SetItem(vars, 0, py_existing_id );
            PyTuple_SetItem(vars, 1, py_contagion_pop  );
            PyTuple_SetItem(vars, 2, py_dt  );
            PyTuple_SetItem(vars, 3, py_tx_route );
            PyObject * retVal = PyObject_CallObject( pFunc, vars );
            PyErr_Print();
            auto val = (bool) PyInt_AsLong(retVal);
            if( val )
            {
                StrainIdentity strainId;
                AcquireNewInfection(&strainId);
            }
            //Py_DECREF( vars );
            //Py_DECREF( py_existing_id_str );
            //Py_DECREF( py_contagion_pop );
            //Py_DECREF( py_dt );
            //Py_DECREF( py_tx_route );
            Py_DECREF( retVal );
        }
        delete check;
        return;
#else
        LOG_DEBUG_F("Expose route: %d, %f\n", transmission_route, cp->GetTotalContagion());
        if( IsInfected() )
        {
            return;
        }
        if (susceptibility->getModAcquire() == 0)
        {
            return;
        }

        if (transmission_route==TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL) 
        {
            int SimDay = (int)parent->GetTime().time; // is this the date of the simulated year?
            int nDayOfYear = SimDay % 365;
			int SimYear = floor((int)parent->GetTime().Year());

            float fEnvironment = cp->GetTotalContagion();
            if (fEnvironment==0.0)
            {
                return;
            }

            float ramp_days = GET_CONFIGURABLE(SimulationConfig)->typhoid_environmental_ramp_duration;
			float cutoff_days = GET_CONFIGURABLE(SimulationConfig)->typhoid_environmental_cutoff_days;
            float peak_amplification = GET_CONFIGURABLE(SimulationConfig)->typhoid_environmental_peak_multiplier;
            float peak_start_day = floor(GET_CONFIGURABLE(SimulationConfig)->typhoid_environmental_peak_start); 
			if (peak_start_day > 365){
				peak_start_day = peak_start_day - 365;}
		//this is mostly for calibtool purposes
			float peak_days = (365 - cutoff_days) - (2 * ramp_days);
            float peak_end_day = peak_start_day + peak_days;
			if (peak_end_day > 365){
				peak_end_day = peak_end_day - 365;}

            float slope = peak_amplification / ramp_days;
            float amplification = 0;
            //float HarvestDayOfYear = nDayOfYear - GET_CONFIGURABLE(SimulationConfig)->environmental_incubation_period;
            //if( HarvestDayOfYear < 1){
            //    HarvestDayOfYear = 365 - HarvestDayOfYear;
	    //}
			if (peak_start_day - ramp_days > 0){
            if ((nDayOfYear >= peak_start_day-ramp_days) && ( nDayOfYear < peak_start_day)) { // beginning of wastewater irrigation
                amplification=((nDayOfYear- (peak_start_day-ramp_days))+0.5)*(slope);
	    }
			if ((peak_start_day - peak_end_day > 0) && ((nDayOfYear >= peak_start_day)  || (nDayOfYear<=peak_end_day))) { // peak of wastewater irrigation
                amplification= peak_amplification;
	    }
			if ((peak_start_day - peak_end_day < 0) && (nDayOfYear >= peak_start_day) && (nDayOfYear <= peak_end_day)) { // peak of wastewater irrigation
                amplification= peak_amplification;
	    }
            if ((nDayOfYear > peak_end_day) && (nDayOfYear <= (peak_end_day+ramp_days)) ){ // end of wastewater irrigation
                amplification= peak_amplification-(((nDayOfYear-peak_end_day)-0.5)*slope);
		}
			}

			else if (peak_start_day - ramp_days < 1){
			if ((nDayOfYear >= peak_start_day-ramp_days+365) || ( nDayOfYear < peak_start_day)) { // beginning of wastewater irrigation
                if (nDayOfYear >= peak_start_day-ramp_days+365){
					amplification= (nDayOfYear - (peak_start_day-ramp_days+365)+0.5)*(slope);
				}
				else if (nDayOfYear < peak_start_day) {
				amplification= (((ramp_days-peak_start_day) + nDayOfYear)  + 0.5)*(slope);
				}
			}
			if ((peak_start_day - peak_end_day > 0) && ((nDayOfYear >= peak_start_day)  || (nDayOfYear<=peak_end_day))) { // peak of wastewater irrigation
                amplification= peak_amplification;
	    }
			if ((peak_start_day - peak_end_day < 0) && (nDayOfYear >= peak_start_day) && (nDayOfYear <= peak_end_day)) { // peak of wastewater irrigation
                amplification= peak_amplification;
	    }
            if ((nDayOfYear > peak_end_day) && (nDayOfYear <= (peak_end_day+ramp_days)) ){ // end of wastewater irrigation
                amplification= peak_amplification-(((nDayOfYear-peak_end_day)-0.5)*slope);
		}
			
			}


			float intervention_multiplier = 1;
			if (SimYear == 1983){ intervention_multiplier = GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_1983;}
			if (SimYear == 1984){ intervention_multiplier = GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_1984;}
			if (SimYear == 1985){ intervention_multiplier = GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_1985;}
			if (SimYear == 1986){ intervention_multiplier = GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_1986;}
			if (SimYear == 1987){ intervention_multiplier = GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_1987;}
			if (SimYear == 1988){ intervention_multiplier = GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_1988;}
			if (SimYear == 1989){ intervention_multiplier = GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_1989;}
			if (SimYear == 1990){ intervention_multiplier = GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_1990;}
			if (SimYear >= 1991){ intervention_multiplier = GET_CONFIGURABLE(SimulationConfig)->typhoid_exposure_1991;}



            float fExposure = fEnvironment * amplification;
            if (fExposure>0)
            {
                float infects = 1.0f-pow(1.0f + fExposure * (pow(2.0f,(1/alpha)-1.0f)/N50),-alpha); // Dose-response for prob of infection
                float immunity= pow(1-GET_CONFIGURABLE(SimulationConfig)->typhoid_protection_per_infection, _infection_count);
                //float prob = 1.0f- pow(1.0f-(immunity * infects * interventions->GetInterventionReducedAcquire()),dt);
                int number_of_exposures = randgen->Poisson(GET_CONFIGURABLE(SimulationConfig)->typhoid_environmental_exposure_rate * dt * intervention_multiplier);
                //int number_of_exposures = randgen->Poisson(exposure_rate * dt);
                float prob = 0;
                if (number_of_exposures > 0)
                {
                    prob = 1.0f - pow(1.0f - immunity * infects * interventions-> GetInterventionReducedAcquire(), number_of_exposures);
                }
                //LOG_INFO_F("Reduced Acquire multiplier %f\n", interventions->GetInterventionReducedAcquire());
                //LOG_INFO_F("Environ contagion %f amp %f day %f\n", fEnvironment, amplification, HarvestDayOfYear);
                //LOG_DEBUG_F("Expose::TRANSMISSIONROUTE_ENVIRONMENTAL %f, %f, %f, %f, %f\n", prob, infects, immunity, fExposure, fEnvironment);
                if (prob>0.0f && randgen->e() < prob)
                {
                    _routeOfInfection = transmission_route;
                    StrainIdentity strainId;
                    //LOG_DEBUG("INDIVIDUAL INFECTED BY ENVIRONMENT.\n"); // This is for reporting DON'T DELETE :)
                    AcquireNewInfection(&strainId);
                    return;
                } else {
                    return;
                }
            }
        }

            else if (transmission_route==TransmissionRoute::TRANSMISSIONROUTE_CONTACT)
            {
                float fContact=cp->GetTotalContagion();
                if (fContact==0)
                {
                    return;
                }
                //LOG_INFO_F("contact congation %f\n", fContact);
                //float infects = 1-pow(1 + fContact * (pow(2,(1/alpha)-1)/N50),-alpha);
                float infects = 1.0f-pow(1.0f + fContact * (pow(2.0f,(1/alpha)-1.0f)/N50),-alpha);
                //LOG_INFO_F("Environ contagion %f %f\n", fContact, infects);

                float immunity= pow(1-GET_CONFIGURABLE(SimulationConfig)->typhoid_protection_per_infection, _infection_count);             
                //float prob = min(1.0f, 1.0f- (float) pow(1.0f-(immunity * infects * interventions->GetInterventionReducedAcquire()),dt));
                int number_of_exposures = randgen->Poisson(GET_CONFIGURABLE(SimulationConfig)->typhoid_contact_exposure_rate * dt);
                float prob = 0;
                if (number_of_exposures > 0)
                {
                    prob = 1.0f - pow(1.0f - immunity * infects * interventions-> GetInterventionReducedAcquire(), number_of_exposures);
                }
                if (prob>0.0f && randgen->e() < prob) {
                    LOG_DEBUG("INDIVIDUAL INFECTED BY CONTACT.\n"); // FOR REPORTING
                    _routeOfInfection = transmission_route;
                    StrainIdentity strainId;
                    AcquireNewInfection(&strainId);
                    return;
                } else {
                    return;
                }
            }
#endif
        }

        void IndividualHumanTyphoid::ExposeToInfectivity(float dt, const TransmissionGroupMembership_t* transmissionGroupMembership)
        {
            IndividualHumanEnvironmental::ExposeToInfectivity(dt, transmissionGroupMembership);
        }

        void IndividualHumanTyphoid::UpdateInfectiousness(float dt)
        {
#ifdef ENABLE_PYTHOID
            //volatile Stopwatch * check = new Stopwatch( __FUNCTION__ );
            for( auto &route: parent->GetTransmissionRoutes() )
            {
                static auto pFunc = IdmPyInit( "dtk_typhoid_individual", "update_and_return_infectiousness" );
                if( pFunc )
                {
                    // pass individual id ONLY
                    static PyObject * vars = PyTuple_New(2);
                    PyObject* py_existing_id = PyLong_FromLong( GetSuid().data );
                    PyTuple_SetItem( vars, 0, py_existing_id );
                    PyObject* py_route_str = PyString_FromFormat( "%s", route.c_str() );
                    PyTuple_SetItem( vars, 1, py_route_str );
                    PyObject * retVal = PyObject_CallObject( pFunc, vars );
                    PyErr_Print();
                    auto val = PyFloat_AsDouble(retVal);
                    infectiousness += val;
                    StrainIdentity tmp_strainID;
                    release_assert( transmissionGroupMembershipByRoute.find( route ) != transmissionGroupMembershipByRoute.end() );
                    LOG_DEBUG_F("Depositing %f to route %s: (antigen=%d, substain=%d)\n", val, route.c_str(), tmp_strainID.GetAntigenID(), tmp_strainID.GetGeneticID());
                    parent->DepositFromIndividual( &tmp_strainID, (float) val, &transmissionGroupMembershipByRoute.at( route ) );
                    //Py_DECREF( vars );
                    //Py_DECREF( py_existing_id_str );
                    //Py_DECREF( py_route_str );
                    Py_DECREF( retVal );
                }
            }
            //delete check;
            return;
#else
            if( !IsInfected() && !IsChronicCarrier() )
            {
                return;
            }
            infectiousness = 0.0f;
			float base_infectiousness = GET_CONFIGURABLE(SimulationConfig)->typhoid_acute_infectiousness;
            if (acute_timer>=0)
            {
                infectiousness = base_infectiousness*interventions->GetInterventionReducedTransmit();
            }
            else if (prepatent_timer>=0)
            {
                infectiousness = base_infectiousness*GET_CONFIGURABLE(SimulationConfig)->typhoid_prepatent_relative_infectiousness*interventions->GetInterventionReducedTransmit();
            }
            else if (subclinical_timer>=0)
            {
                infectiousness = base_infectiousness*GET_CONFIGURABLE(SimulationConfig)->typhoid_subclinical_relative_infectiousness*interventions->GetInterventionReducedTransmit();
            }
            else if (chronic_timer>=0)
            {
                infectiousness = base_infectiousness*GET_CONFIGURABLE(SimulationConfig)->typhoid_chronic_relative_infectiousness*interventions->GetInterventionReducedTransmit();
            }

            //parent->GetTransmissionRoutes()

            for (auto infection : infections)
            {
                LOG_DEBUG("Getting infectiousness by route.\n");

                StrainIdentity tmp_strainID;
                infection->GetInfectiousStrainID(&tmp_strainID);

                //deposit oral to 'contact', fecal to 'environmental' pool
                LOG_DEBUG("Getting routes.\n");

                for(auto& entry : transmissionGroupMembershipByRoute)
                {
                    LOG_DEBUG_F("Found route:%s.\n",entry.first.c_str());
                    if (entry.first==string("contact"))
                    {
                        //          float tmp_infectiousnessOral = m_mc_weight * infection->GetInfectiousnessByRoute(string("contact"));
                        float tmp_infectiousnessOral = infectiousness;
                        if (tmp_infectiousnessOral > 0.0f)
                        {
                            LOG_DEBUG_F("Depositing %f to route %s: (antigen=%d, substain=%d)\n", tmp_infectiousnessOral, entry.first.c_str(), tmp_strainID.GetAntigenID(), tmp_strainID.GetGeneticID());
                            parent->DepositFromIndividual(&tmp_strainID, tmp_infectiousnessOral, &entry.second);
                        } 
                    }
                    else if (entry.first==string("environmental"))
                    {
                        //                    float tmp_infectiousnessFecal =  m_mc_weight * infection->GetInfectiousnessByRoute(string("environmental"));

                        float tmp_infectiousnessFecal =  infectiousness;
                        if (tmp_infectiousnessFecal > 0.0f)
                        {
                            LOG_DEBUG_F("UpdateInfectiousness::Depositing %f to route %s: (antigen=%d, substain=%d)\n", tmp_infectiousnessFecal, entry.first.c_str(), tmp_strainID.GetAntigenID(), tmp_strainID.GetGeneticID());    
                            parent->DepositFromIndividual(&tmp_strainID, tmp_infectiousnessFecal, &entry.second);
                            ///LOG_DEBUG_F("contagion= %f\n", cp->GetTotalContagion());
                        }
                    }
                    else
                    {
                        LOG_WARN_F("unknown route %s, do not deposit anything.\n", entry.first.c_str());
                    }
                }
            }
#endif
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
#ifdef ENABLE_PYTHOID
            volatile Stopwatch * check = new Stopwatch( __FUNCTION__ );
            static auto pFunc = IdmPyInit( "dtk_typhoid_individual", "update" );
            if( pFunc )
            {
                // pass individual id AND dt
                static PyObject * vars = PyTuple_New(2);
                PyObject* py_existing_id = PyLong_FromLong( GetSuid().data );
                PyObject* py_dt = PyLong_FromLong( (int) dt );
                PyTuple_SetItem(vars, 0, py_existing_id );
                PyTuple_SetItem(vars, 1, py_dt );
                auto pyVal = PyObject_CallObject( pFunc, vars );
                if( pyVal != nullptr )
                {
                    //state_to_report = PyString_AsString(pyVal); 
                    // parse tuple: char, bool
                    //PyObject *ob1,*ob2;
                    char * state = "UNSET";
                    PyArg_ParseTuple(pyVal,"si",&state, &state_changed ); //o-> pyobject |i-> int|s-> char*
                    state_to_report = state;
                    //= PyString_AsString(ob1); 
                    //state_changed = (bool) PyInt_AsLong(ob2); 
                }
                else
                {
                    state_to_report = "D";
                }
                //Py_DECREF( vars );
                //Py_DECREF( py_existing_id_str );
                //Py_DECREF( py_dt_str );
                PyErr_Print();
            }
            delete check;
            LOG_DEBUG_F( "state_to_report for individual %d = %s; Infected = %d.\n", GetSuid().data, state_to_report.c_str(), IsInfected() );

            if( state_to_report == "S" && state_changed && GetInfections().size() > 0 )
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
            else if( state_to_report == "D" && state_changed )
            {
                LOG_INFO_F( "[Update] Somebody died from their infection.\n" );
            }
#else
            state_to_report = "S"; // default state is susceptible
            if( IsInfected() )
            {
                //            LOG_INFO_F("%d INFECTED!!!%d,%d,%d,%d\n", GetSuid().data, prepatent_timer, acute_timer, subclinical_timer,chronic_timer);
                if (prepatent_timer > UNINIT_TIMER)
                { // pre-patent
                    state_to_report="P";
                    prepatent_timer -= dt;
                    if( UNINIT_TIMER<prepatent_timer && prepatent_timer<=0 )
                    {
                        //LOG_INFO_F("hasclin subclinical dur %d, pre %d\n", _subclinical_duration, prepatent_timer);

                        prepatent_timer=UNINIT_TIMER;
                        if (hasClinicalImmunity) {
                            if (getAgeInYears() < 30.0)
                                _subclinical_duration = int(generateRandFromLogNormal(msu30, ssu30)*7);
                            else
                                _subclinical_duration = int(generateRandFromLogNormal(mso30, sso30)*7);
                            subclinical_timer = _subclinical_duration;
                        } else if (!hasClinicalImmunity) {
                            if (randgen->e()<(GET_CONFIGURABLE(SimulationConfig)->typhoid_symptomatic_fraction*interventions->GetInterventionReducedMortality())) { //THIS IS NOT ACTUALLY MORTALITY, I AM JUST USING THE CALL 
                                if (getAgeInYears() < 30.0)
                                    _acute_duration = int(generateRandFromLogNormal(mau30, sau30)*7);
                                else
                                    _acute_duration = int(generateRandFromLogNormal(mao30, sao30)*7);
                                //LOG_INFO_F("acute dur %d\n", _acute_duration);
                                acute_timer = _acute_duration;
                                //if (_acute_duration > 365)
                                //    isChronic = true; // will be a chronic carrier
                            } else {
                                if (getAgeInYears() < 30.0)
                                    _subclinical_duration = int(generateRandFromLogNormal( msu30, ssu30)*7);
                                else
                                    _subclinical_duration = int(generateRandFromLogNormal( mso30, sso30)*7);
                                subclinical_timer = _subclinical_duration;
                                //if (_subclinical_duration > 365)
                                //    isChronic = true;
                                //state_to_report="C";
                            }
                        }
                    }
                }
                if (subclinical_timer > UNINIT_TIMER)
                { // asymptomatic infection
                    //              LOG_INFO_F("is subclinical dur %d, %d, %d\n", _subclinical_duration, subclinical_timer, dt);
                    state_to_report="SUB";
                    subclinical_timer -= dt;
                    if (UNINIT_TIMER<subclinical_timer && subclinical_timer<=0)
                    {
                        //LOG_INFO_F("SOMEONE FINSIHED SUB %d, %d\n", _subclinical_duration, subclinical_timer);
                        subclinical_timer = UNINIT_TIMER;
                        float p2 = 0;
                        float carrier_prob = 0;
                        int agebin = int(floor(getAgeInYears()/10));
                        if (agebin>=GallstoneDataLength)
                            agebin=GallstoneDataLength-1;
                        if (GetGender()==1)
                        {
                            p2=FemaleGallstones[agebin];
                            carrier_prob = GET_CONFIGURABLE(SimulationConfig)->typhoid_carrier_probability_male * 1.3793;
                        } 
                        else if (GetGender()==0)
                        {
                            p2=MaleGallstones[agebin];
                            carrier_prob = GET_CONFIGURABLE(SimulationConfig)->typhoid_carrier_probability_male;

                        }
                        //LOG_INFO_F("Gallstone percentage is %f %f\n", getAgeInYears(), p2);
                        if (randgen->e() < p2*carrier_prob) {
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
                                if (P10>0.0 && randgen->e() < P10)
                                {
                                    hasClinicalImmunity = true;
                                    clinical_immunity_timer = _clinical_immunity_duration;
                                }
                            }
                        }
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
                      ){       //if they seek treatment and don't die, we are assuming they have a probability of becoming a carrier (chloramphenicol treatment does not prevent carriage)
                            if (randgen->e() < CFRH)
                            {
                                isDead = true;
                                state_to_report = "D";
                                acute_timer = UNINIT_TIMER;
                            } else 
                            {
                                acute_timer = UNINIT_TIMER;
								float p3=0.0; // P3 is age dependent so is determined below. Probability of becoming a chronic carrier from a CLINICAL infection
                                float carrier_prob = 0;
                                int agebin = int(floor(getAgeInYears()/10));
                                if (agebin>=GallstoneDataLength)
                                    agebin=GallstoneDataLength-1;
                                if (GetGender()==1)
                                {
                                    p3=FemaleGallstones[agebin];
                                    carrier_prob = GET_CONFIGURABLE(SimulationConfig)->typhoid_carrier_probability_male * 1.3793;
                                } 
                                else if (GetGender()==0)
                                {
                                    p3=MaleGallstones[agebin];
                                    carrier_prob = GET_CONFIGURABLE(SimulationConfig)->typhoid_carrier_probability_male;
                                }
                                if (randgen->e()< p3*carrier_prob)
                                {
                                    chronic_timer = _chronic_duration;
                                }
                            }
                        }

                        //if they dont seek treatment, just keep them infectious until the end of their acute stage


                        //Assume all acute individuals seek treatment since this is Mike's "reported" fraction
                        //Some fraction are treated effectively, some become chronic carriers, some die, some remain infectious

                        //assume all other risks are dependent on unsuccessful treatment
                        if (UNINIT_TIMER < acute_timer && acute_timer<= 0)
                        {
                            acute_timer = UNINIT_TIMER;
                            if (randgen->e() < CFRU)
                            {
                                isDead = true;
                                state_to_report = "D";
                            }
                            else{
                              //if they survived, calculate probability of being a carrier
                                float p3=0.0; // P3 is age dependent so is determined below. Probability of becoming a chronic carrier from a CLINICAL infection
                                float carrier_prob = 0;
                                int agebin = int(floor(getAgeInYears()/10));
                                if (agebin>=GallstoneDataLength)
                                    agebin=GallstoneDataLength-1;
                                if (GetGender()==1)
                                {
                                    p3=FemaleGallstones[agebin];
                                    carrier_prob = GET_CONFIGURABLE(SimulationConfig)->typhoid_carrier_probability_male * 1.3793;
                                } 
                                else if (GetGender()==0)
                                {
                                    p3=MaleGallstones[agebin];
                                    carrier_prob = GET_CONFIGURABLE(SimulationConfig)->typhoid_carrier_probability_male;
                                }
                                if (randgen->e()< p3*carrier_prob)
                                {
                                    chronic_timer = _chronic_duration;
                                }
							}   
                        }
               }


                if (chronic_timer > UNINIT_TIMER) {
                    state_to_report="C";
                    chronic_timer -= dt;
                    if (UNINIT_TIMER< chronic_timer && chronic_timer<=0)
                        chronic_timer = UNINIT_TIMER;
                }
            }
            if (hasClinicalImmunity && subclinical_timer ==UNINIT_TIMER && prepatent_timer ==UNINIT_TIMER )
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
                state_changed = false;
            } else {
                // typhoid state changed
                last_state_reported=state_to_report;
                state_changed = true;
            }
            LOG_DEBUG_F( "state_to_report for individual %d = %s\n", GetSuid().data, state_to_report.c_str() );

            if( state_to_report == "S" && state_changed && GetInfections().size() > 0 )
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
            else if( state_to_report == "D" && state_changed )
            {    
                LOG_INFO_F( "[Update] Somebody died from their infection.\n" );
            }
#endif
            return IndividualHumanEnvironmental::Update( currenttime, dt);
        }

        void IndividualHumanTyphoid::AcquireNewInfection(StrainIdentity *infstrain, int incubation_period_override )
        {
            LOG_DEBUG_F("AcquireNewInfection: route %d\n", _routeOfInfection);
            if (_routeOfInfection == TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL)
            {
                infstrain->SetGeneticID( 0 );
            }
            else if (_routeOfInfection == TransmissionRoute::TRANSMISSIONROUTE_CONTACT)
            {
                infstrain->SetGeneticID( 1 );
            }
            else
            {
                infstrain->SetGeneticID( 2 );
            }
                // neither environmental nor contact source. probably from initial seeding
            IndividualHumanEnvironmental::AcquireNewInfection( infstrain, incubation_period_override );
#ifdef ENABLE_PYTHOID
            volatile Stopwatch * check = new Stopwatch( __FUNCTION__ );
            static auto pFunc = IdmPyInit( "dtk_typhoid_individual", "acquire_infection" );
            if( pFunc )
            {
                // pass individual id ONLY
                static PyObject * vars = PyTuple_New(1);
                PyObject* py_existing_id = PyLong_FromLong( GetSuid().data );
                PyTuple_SetItem(vars, 0, py_existing_id );
                PyObject_CallObject( pFunc, vars );
                //Py_DECREF( vars );
            }
            delete check;
#else
            if (_routeOfInfection == TransmissionRoute::TRANSMISSIONROUTE_ENVIRONMENTAL) {
                _prepatent_duration = (int)(generateRandFromLogNormal(mpe, spe));

            } else if (_routeOfInfection == TransmissionRoute::TRANSMISSIONROUTE_CONTACT) {
                _prepatent_duration = (int)(generateRandFromLogNormal(mpf, spf));
            } else {
                // neither environmental nor contact source. probably from initial seeding
                _prepatent_duration = (int)(generateRandFromLogNormal(mpf, spf));
            }
            //		LOG_INFO_F("Prepatent %d.\n", _prepatent_duration);
            _infection_count ++;
            prepatent_timer=_prepatent_duration;
#endif
        }

        HumanStateChange IndividualHumanTyphoid::GetStateChange() const
        {
            HumanStateChange retVal = StateChange;
            //auto parsed = IdmString(state_to_report).split();
            if( state_to_report == "D" )
            {
                LOG_INFO_F( "[GetStateChange] Somebody died from their infection.\n" );
                retVal = HumanStateChange::KilledByInfection;
            }
            return retVal;
        }

        bool IndividualHumanTyphoid::IsChronicCarrier( bool incidence_only ) const
        {
            if( state_to_report == "C" &&
                    ( ( incidence_only && state_changed ) ||
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
            if( state_to_report == "SUB" &&
                    ( ( incidence_only && state_changed ) ||
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
            if( state_to_report == "A" &&
                    ( ( incidence_only && state_changed ) ||
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
            if( state_to_report == "P" &&
                    ( ( incidence_only && state_changed ) ||
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
