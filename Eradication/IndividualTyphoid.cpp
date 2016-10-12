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



namespace Kernel
{
//#define LOG_INFO_F printf
//#define LOG_DEBUG_F printf
    float IndividualHumanTyphoidConfig::environmental_incubation_period = 0.0f; // NaturalNumber please
    float IndividualHumanTyphoidConfig::typhoid_acute_infectiousness = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_chronic_relative_infectiousness = 0.0f;

    //        float IndividualHumanTyphoidConfig::typhoid_environmental_exposure = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_prepatent_relative_infectiousness = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_protection_per_infection = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_subclinical_relative_infectiousness = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_carrier_probability_male = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_carrier_probability_female = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_3year_susceptible_fraction = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_6month_susceptible_fraction = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_6year_susceptible_fraction = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_symptomatic_fraction = 0.0f;

    float IndividualHumanTyphoidConfig::typhoid_environmental_exposure_rate = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_contact_exposure_rate = 0.0f;
    //float IndividualHumanTyphoidConfig::typhoid_environmental_exposure_rate_seasonal_max = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_environmental_ramp_up_duration = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_environmental_ramp_down_duration = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_environmental_peak_start = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_environmental_cutoff_days = 0.0f;
    //float IndividualHumanTyphoidConfig::typhoid_environmental_amplification = 0.0f;
    float IndividualHumanTyphoidConfig::typhoid_environmental_peak_multiplier = 0.0f;

    GET_SCHEMA_STATIC_WRAPPER_IMPL(Individual,IndividualHumanTyphoidConfig)
    BEGIN_QUERY_INTERFACE_BODY(IndividualHumanTyphoidConfig)
    END_QUERY_INTERFACE_BODY(IndividualHumanTyphoidConfig)

    bool
    IndividualHumanTyphoidConfig::Configure( const Configuration* config ) // just called once!
    {
        LOG_DEBUG( "Configure\n" );
        // 
        // typhoid
        //
        //initConfigTypeMap( "Environmental_Incubation_Period", &environmental_incubation_period, "So-called Waiting Period for environmental contagion, time between deposit and exposure.", 0, 100, 30 );
        initConfigTypeMap( "Typhoid_Acute_Infectiousness", &typhoid_acute_infectiousness, "Typhoid_Acute_Infectiousness.", 0, 1e7, 4000 );
        initConfigTypeMap( "Typhoid_Chronic_Relative_Infectiousness", &typhoid_chronic_relative_infectiousness, "Typhoid_Chronic_Relative_Infectiousness.", 0, 1e7, 1000 ); 
        initConfigTypeMap( "Typhoid_Prepatent_Relative_Infectiousness", &typhoid_prepatent_relative_infectiousness, "Typhoid_Prepatent_Relative_Infectiousness.", 0, 1e7, 3e3 ); 
        initConfigTypeMap( "Typhoid_Protection_Per_Infection", &typhoid_protection_per_infection, "Typhoid_Protection_Per_Infection.", 0, 1, 0.1f ); 
        initConfigTypeMap( "Typhoid_Subclinical_Relative_Infectiousness", &typhoid_subclinical_relative_infectiousness, "Typhoid_Subclinical_Relative_Infectiousness.", 0, 1e7, 2000 );
        initConfigTypeMap( "Typhoid_Carrier_Probability_Male", &typhoid_carrier_probability_male, "Typhoid_Carrier_Probability_Male.", 0, 1, 0.25 );
        initConfigTypeMap( "Typhoid_Carrier_Probability_Female", &typhoid_carrier_probability_female, "Typhoid_Carrier_Probability_Female.", 0, 1, 0.25 );
        initConfigTypeMap( "Typhoid_6month_Susceptible_Fraction", &typhoid_6month_susceptible_fraction, "Typhoid_6month_Susceptible_Fraction.", 0, 1, 0.5);
        initConfigTypeMap( "Typhoid_3year_Susceptible_Fraction", &typhoid_3year_susceptible_fraction, "Typhoid_3year_Susceptible_Fraction.", 0, 1, 0.5);
        initConfigTypeMap( "Typhoid_Environmental_Exposure_Rate", &typhoid_environmental_exposure_rate, "Typhoid_Environmental_Exposure_Rate.", 0, 10, 0.5);
        initConfigTypeMap( "Typhoid_Contact_Exposure_Rate", &typhoid_contact_exposure_rate, "Typhoid_Contact_Exposure_Rate.", 0, 100, 0.5);
        initConfigTypeMap( "Typhoid_Environmental_Ramp_Up_Duration", &typhoid_environmental_ramp_up_duration, "Typhoid_Environmental_Ramp_Up_Duration.", 0, 200, 2);
        initConfigTypeMap( "Typhoid_Environmental_Ramp_Down_Duration", &typhoid_environmental_ramp_down_duration, "Typhoid_Environmental_Ramp_Down_Duration.", 0, 270, 2);
        initConfigTypeMap( "Typhoid_Environmental_Cutoff_Days", &typhoid_environmental_cutoff_days, "Typhoid_Environmental_Cutoff_Days.", 0, DAYSPERYEAR, 2);
        initConfigTypeMap( "Typhoid_Environmental_Peak_Start", &typhoid_environmental_peak_start, "Typhoid_Environmental_Peak_Start.", 0, 500, 2);
        initConfigTypeMap( "Typhoid_Environmental_Peak_Multiplier", &typhoid_environmental_peak_multiplier, "Typhoid_Environmental_Peak_Multiplier.", 0, 10000, 3 );
        initConfigTypeMap( "Typhoid_6year_Susceptible_Fraction", &typhoid_6year_susceptible_fraction, "Typhoid_6year_Susceptible_Fraction.", 0, 1, 0.5);
        initConfigTypeMap( "Typhoid_Symptomatic_Fraction", &typhoid_symptomatic_fraction, "Typhoid_Symptomatic_Fraction.", 0, 1, 0.5);    

        SusceptibilityTyphoidConfig fakeImmunity;
        fakeImmunity.Configure( config );
        InfectionTyphoidConfig fakeInfection;
        fakeInfection.Configure( config );

        //do we need to call initConfigTypeMap? DLC 
        return JsonConfigurable::Configure( config );
    }

    void IndividualHumanTyphoid::InitializeStatics( const Configuration * config )
    {
        IndividualHumanTyphoidConfig human_config;
        human_config.Configure( config );
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

    // environmental exposure constants
    const int IndividualHumanTyphoid::N50 = 1110000;
    const float IndividualHumanTyphoid::alpha = 0.175f;


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
        _infection_count=0; // should not be necessary
        _routeOfInfection = TransmissionRoute::TRANSMISSIONROUTE_ALL;// IS THIS OK for a default? DLC
        doseTracking = "None";
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

    // I think I want to move this function to NodeTyphoid
    float IndividualHumanTyphoid::getSeasonalAmplitude() const
    {
        float amplification = 0.0f;

        float ramp_down_days = IndividualHumanTyphoidConfig::typhoid_environmental_ramp_down_duration;
        float ramp_up_days = IndividualHumanTyphoidConfig::typhoid_environmental_ramp_up_duration;
        float cutoff_days = IndividualHumanTyphoidConfig::typhoid_environmental_cutoff_days;
        float peak_amplification = IndividualHumanTyphoidConfig::typhoid_environmental_peak_multiplier;
        float peak_start_day = floor(IndividualHumanTyphoidConfig::typhoid_environmental_peak_start); 
        if (peak_start_day > DAYSPERYEAR)
        {
            peak_start_day = peak_start_day - DAYSPERYEAR;
        }
        //this is mostly for calibtool purposes
        float peak_days = (DAYSPERYEAR - cutoff_days) - (ramp_down_days + ramp_up_days);
        float peak_end_day = peak_start_day + peak_days;
        if (peak_end_day > DAYSPERYEAR)
        {
            peak_end_day = peak_end_day - DAYSPERYEAR;
        }

        float slope_up = peak_amplification / ramp_up_days;
        float slope_down = peak_amplification / ramp_down_days;

        //float HarvestDayOfYear = nDayOfYear - IndividualHumanTyphoidConfig::environmental_incubation_period;
        //if( HarvestDayOfYear < 1){
        //    HarvestDayOfYear = DAYSPERYEAR - HarvestDayOfYear;
        //}
        int SimDay = (int)parent->GetTime().time; // is this the date of the simulated year?
        int nDayOfYear = SimDay % DAYSPERYEAR;
#define HALF (0.5f)
        if (peak_start_day - ramp_up_days > 0)
        {
            if ((nDayOfYear >= peak_start_day-ramp_up_days) && ( nDayOfYear < peak_start_day))
            { // beginning of wastewater irrigation
                amplification=((nDayOfYear- (peak_start_day-ramp_up_days))+ HALF )*(slope_up);
            }
            if ((peak_start_day - peak_end_day > 0) && ((nDayOfYear >= peak_start_day)  || (nDayOfYear<=peak_end_day)))
            { // peak of wastewater irrigation
                amplification= peak_amplification;
            }
            if ((peak_start_day - peak_end_day < 0) && (nDayOfYear >= peak_start_day) && (nDayOfYear <= peak_end_day))
            { // peak of wastewater irrigation
                amplification= peak_amplification;
            }
            if ((peak_end_day + ramp_down_days < DAYSPERYEAR) && (nDayOfYear > peak_end_day) && (nDayOfYear <= (peak_end_day+ramp_down_days)))
            {                
                amplification= peak_amplification-(((nDayOfYear-peak_end_day)- HALF )*slope_down);
            }
            if ((peak_end_day + ramp_down_days >= DAYSPERYEAR) && ((nDayOfYear > peak_end_day) || (nDayOfYear < ramp_down_days - (DAYSPERYEAR- peak_end_day))))
            {
                // end of wastewater irrigation
                if (nDayOfYear > peak_end_day)
                {
                    amplification = peak_amplification-(((nDayOfYear-peak_end_day)- HALF )*slope_down);
                }
                if (nDayOfYear < ramp_down_days - (DAYSPERYEAR - peak_end_day))
                {
                    amplification = peak_amplification - (((DAYSPERYEAR-peak_end_day)+nDayOfYear- HALF )*slope_down);
                }
            }
        }
        else if (peak_start_day - ramp_up_days < 0)
        {
            if ((nDayOfYear >= peak_start_day-ramp_up_days+DAYSPERYEAR) || ( nDayOfYear < peak_start_day))
            { // beginning of wastewater irrigation
                if (nDayOfYear >= peak_start_day-ramp_up_days+DAYSPERYEAR)
                {
                    amplification= (nDayOfYear - (peak_start_day-ramp_up_days+DAYSPERYEAR)+ HALF )*(slope_up);
                }
                else if (nDayOfYear < peak_start_day) 
                {
                    amplification= (((ramp_up_days-peak_start_day) + nDayOfYear)  +  HALF )*(slope_up);
                }
            }
            if ((peak_start_day - peak_end_day > 0) && ((nDayOfYear >= peak_start_day)  || (nDayOfYear<=peak_end_day)))
            { // peak of wastewater irrigation
                amplification= peak_amplification;
            }
            if ((peak_start_day - peak_end_day < 0) && (nDayOfYear >= peak_start_day) && (nDayOfYear <= peak_end_day))
            { // peak of wastewater irrigation
                amplification= peak_amplification;
            }
            if ((nDayOfYear > peak_end_day) && (nDayOfYear <= (peak_end_day+ramp_down_days)) )
            { // end of wastewater irrigation
                amplification= peak_amplification-(((nDayOfYear-peak_end_day)- HALF )*slope_down);
            }

        }
        LOG_DEBUG_F("day of year %i amplification %f start %f end %f \n", nDayOfYear, amplification, peak_start_day, peak_end_day); 
        return amplification;
    }

#define HIGH_ENVIRO_DOSE_THRESHOLD (55000000)
#define LOW_ENVIRO_DOSE_THRESHOLD (5050000)
    void IndividualHumanTyphoid::quantizeEnvironmentalDoseTracking( float fEnvironment )
    {
        if (fEnvironment <= LOW_ENVIRO_DOSE_THRESHOLD )
        { 
            doseTracking = "Low";
        }
        else if( fEnvironment > LOW_ENVIRO_DOSE_THRESHOLD && fEnvironment <= HIGH_ENVIRO_DOSE_THRESHOLD )
        {
            doseTracking = "Medium";
        }
        else if( fEnvironment > HIGH_ENVIRO_DOSE_THRESHOLD ) // just else should do
        {
            doseTracking = "High";
        }
    }

    void IndividualHumanTyphoid::quantizeContactDoseTracking( float fContact )
    {
        
        doseTracking = "High";
    }

    void IndividualHumanTyphoid::Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route )
    { 
#ifdef ENABLE_PYTHOID
        if( randgen->e() > IndividualHumanTyphoidConfig::typhoid_exposure_fraction )
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
        //LOG_DEBUG_F("Expose route: %d, %f\n", transmission_route, cp->GetTotalContagion());
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

            float fEnvironment = cp->GetTotalContagion();
            if (fEnvironment==0.0)
            {
                return;
            }
            float amplification = getSeasonalAmplitude();

            float intervention_multiplier = 1;
            float fExposure = fEnvironment * amplification;
            if (fExposure>0)
            {
                float infects = 1.0f-pow(1.0f + fExposure * (pow(2.0f,(1/alpha)-1.0f)/N50),-alpha); // Dose-response for prob of infection
                float immunity= pow(1-IndividualHumanTyphoidConfig::typhoid_protection_per_infection, _infection_count);
                //float prob = 1.0f- pow(1.0f-(immunity * infects * interventions->GetInterventionReducedAcquire()),dt);
                int number_of_exposures = randgen->Poisson(IndividualHumanTyphoidConfig::typhoid_environmental_exposure_rate * dt * intervention_multiplier);
                //int number_of_exposures = randgen->Poisson(exposure_rate * dt);
                float prob = 0;
                if (number_of_exposures > 0)
                {
                    prob = 1.0f - pow(1.0f - immunity * infects * interventions-> GetInterventionReducedAcquire(), number_of_exposures);
                }
                //LOG_INFO_F("Reduced Acquire multiplier %f\n", interventions->GetInterventionReducedAcquire());
                //LOG_DEBUG_F("Environ contagion %f amp %f day %f\n", fEnvironment, amplification, HarvestDayOfYear);
                //LOG_DEBUG_F("Expose::TRANSMISSIONROUTE_ENVIRONMENTAL %f, %f, %f, %f, %f\n", prob, infects, immunity, fExposure, fEnvironment);
                //if (prob>0.0f && randgen->e() < prob)
                if( SMART_DRAW( prob ) )
                {
                    _routeOfInfection = transmission_route;
                    StrainIdentity strainId;
                    //LOG_DEBUG("INDIVIDUAL INFECTED BY ENVIRONMENT.\n"); // This is for reporting DON'T DELETE :)
                    quantizeEnvironmentalDoseTracking( fEnvironment );
                    //LOG_INFO_F("dose %f, tracking %s", fEnvironment, doseTracking);
                    AcquireNewInfection(&strainId);
                    return;
                }
                else
                {
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

            float immunity= pow(1-IndividualHumanTyphoidConfig::typhoid_protection_per_infection, _infection_count);             
            //float prob = min(1.0f, 1.0f- (float) pow(1.0f-(immunity * infects * interventions->GetInterventionReducedAcquire()),dt));
            int number_of_exposures = randgen->Poisson(IndividualHumanTyphoidConfig::typhoid_contact_exposure_rate * dt);
            float prob = 0;
            if (number_of_exposures > 0)
            {
                prob = 1.0f - pow(1.0f - immunity * infects * interventions-> GetInterventionReducedAcquire(), number_of_exposures);
            }
            if (prob>0.0f && randgen->e() < prob)
            {
                LOG_DEBUG("INDIVIDUAL INFECTED BY CONTACT.\n"); // FOR REPORTING
                _routeOfInfection = transmission_route;
                StrainIdentity strainId;
                quantizeContactDoseTracking( fContact );
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
                    float tmp_infectiousnessOral = m_mc_weight * infection->GetInfectiousness(); //ByRoute(string("contact"));
                    //float tmp_infectiousnessOral = infectiousness;
                    if (tmp_infectiousnessOral > 0.0f)
                    {
                        LOG_DEBUG_F("Depositing %f to route %s: (antigen=%d, substain=%d)\n", tmp_infectiousnessOral, entry.first.c_str(), tmp_strainID.GetAntigenID(), tmp_strainID.GetGeneticID());
                        parent->DepositFromIndividual(&tmp_strainID, tmp_infectiousnessOral, &entry.second);
                    } 
                }
                else if (entry.first==string("environmental"))
                {
                    float tmp_infectiousnessFecal =  m_mc_weight * infection->GetInfectiousness(); // ByRoute(string("environmental"));
                    //float tmp_infectiousnessFecal =  infectiousness;
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

#if 0
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
#endif
        return IndividualHumanEnvironmental::Update( currenttime, dt);
    }

    void IndividualHumanTyphoid::AcquireNewInfection(StrainIdentity *infstrain, int incubation_period_override )
    {
        LOG_DEBUG_F("AcquireNewInfection: route %d\n", _routeOfInfection);
        if( infstrain )
        {
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
        //LOG_INFO_F("Prepatent %d. dose %s \n", _prepatent_duration, doseTracking);
        _infection_count ++;
#endif
    }

    const std::string IndividualHumanTyphoid::getDoseTracking() const
    {
        return doseTracking;
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
    REGISTER_SERIALIZABLE(IndividualHumanTyphoid);
    //template PoolManager<IndividualHumanTyphoid> IndividualHumanTyphoid::_pool;
    //template<> std::stack<IndividualHumanTyphoid*> PoolManager<IndividualHumanTyphoid>::_pool;                                   
    void IndividualHumanTyphoid::serialize(IArchive& ar, IndividualHumanTyphoid* obj)
    {
        IndividualHumanTyphoid& individual = *obj;
        //ar.labelElement("P1") & individual.P1;
        ar.labelElement("state_to_report") & individual.state_to_report;
        ar.labelElement("isChronic") & individual.isChronic;
        ar.labelElement("_infection_count") & individual._infection_count;
        //ar.labelElement("_routeOfInfection") & individual._routeOfInfection;
        ar.labelElement("state_changed") & individual.state_changed;
        ar.labelElement("doseTracking") & individual.doseTracking;
        ar.labelElement("_infection_count") & individual._infection_count;
        IndividualHumanEnvironmental::serialize(ar, obj);
    }
}

//#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
#include "InfectionTyphoid.h"
#include "SusceptibilityTyphoid.h"
#include "TyphoidInterventionsContainer.h"

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Kernel::IndividualHumanTyphoid)

   

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

//#endif

#endif // ENABLE_TYPHOID
