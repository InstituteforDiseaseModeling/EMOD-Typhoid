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

#ifdef ENABLE_PYTHON
#include "Debug.h"
#include "Contexts.h"
#include "RANDOM.h"
#include "Environment.h"
#include "IndividualPyDemo.h"
#include "SusceptibilityPyDemo.h"
#include "InfectionPyDemo.h"
#include "IContagionPopulation.h"
#include "PyDemoInterventionsContainer.h"
#include "IdmString.h"
#include "SimulationConfig.h"

#ifndef WIN32
#include <sys/time.h>
#endif

#define ENABLE_TOYPHOID 1
#ifdef ENABLE_TOYPHOID 
#include "Python.h"
extern PyObject *
IdmPyInit(
    const char * python_script_name,
    const char * python_function_name
);
#endif

#pragma warning(disable: 4244)

static const char * _module = "IndividualPyDemo";

#define UNINIT_TIMER (-100.0f)


namespace Kernel
{
    inline float generateRandFromLogNormal(float m, float s) {
        // inputs: m is mean of underlying distribution, s is std dev
        return (exp((m)+randgen->eGauss()*s));
    }

    GET_SCHEMA_STATIC_WRAPPER_IMPL(PyDemo.Individual,IndividualHumanPyDemo)
    BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanPyDemo, IndividualHuman)
        HANDLE_INTERFACE(IIndividualHumanPyDemo)
    END_QUERY_INTERFACE_DERIVED(IndividualHumanPyDemo, IndividualHuman)

    IndividualHumanPyDemo::IndividualHumanPyDemo(suids::suid _suid, float monte_carlo_weight, float initial_age, int gender, float initial_poverty) :
        IndividualHuman(_suid, monte_carlo_weight, initial_age, gender, initial_poverty)
    {
#ifdef ENABLE_TOYPHOID
        // Call into python script to notify of new individual
        static auto pFunc = IdmPyInit( "dtk_pydemo_individual", "create" );
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
#endif
    }

    IndividualHumanPyDemo::~IndividualHumanPyDemo()
    {
#ifdef ENABLE_TOYPHOID
        // Call into python script to notify of new individual
        static auto pFunc = IdmPyInit( "dtk_pydemo_individual", "destroy" );
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
#endif
    }

    bool
    IndividualHumanPyDemo::Configure( const Configuration* config ) // just called once!
    {
        LOG_DEBUG( "Configure\n" );
        // pydemo
        SusceptibilityPyDemoConfig fakeImmunity;
        fakeImmunity.Configure( config );
        InfectionPyDemoConfig fakeInfection;
        fakeInfection.Configure( config );

        //do we need to call initConfigTypeMap? DLC 
        return IndividualHuman::Configure( config );
    }

    IndividualHumanPyDemo *IndividualHumanPyDemo::CreateHuman(INodeContext *context, suids::suid id, float monte_carlo_weight, float initial_age, int gender, float initial_poverty)
    {
        IndividualHumanPyDemo *newhuman = _new_ IndividualHumanPyDemo(id, monte_carlo_weight, initial_age, gender, initial_poverty);
        
        newhuman->SetContextTo(context);
        LOG_DEBUG_F( "Created human with age=%f\n", newhuman->m_age );
        return newhuman;
    }

    void IndividualHumanPyDemo::PropagateContextToDependents()
    {
        IndividualHuman::PropagateContextToDependents();
        pydemo_susceptibility = static_cast<SusceptibilityPyDemo*>(susceptibility);
    }

    void IndividualHumanPyDemo::setupInterventionsContainer()
    {
        interventions = _new_ PyDemoInterventionsContainer();
    }

    void IndividualHumanPyDemo::CreateSusceptibility(float imm_mod, float risk_mod)
    {
        SusceptibilityPyDemo *newsusceptibility = SusceptibilityPyDemo::CreateSusceptibility(this, m_age, imm_mod, risk_mod);
        pydemo_susceptibility = newsusceptibility;
        susceptibility = newsusceptibility;
    }

    void IndividualHumanPyDemo::Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route )
    { 
#ifdef ENABLE_TOYPHOID
        /*if( randgen->e() > GET_CONFIGURABLE(SimulationConfig)->pydemo_exposure_fraction )
        {
            return;
        }*/
        if( cp->GetTotalContagion() == 0 )
        {
            return;
        }

        LOG_DEBUG_F( "Calling py:expose with contagion pop %f\n", cp->GetTotalContagion() );

        static auto pFunc = IdmPyInit( "dtk_pydemo_individual", "expose" );
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
        return;
#endif
    }

    void IndividualHumanPyDemo::ExposeToInfectivity(float dt, const TransmissionGroupMembership_t* transmissionGroupMembership)
    {
        IndividualHuman::ExposeToInfectivity(dt, transmissionGroupMembership);
    }

    void IndividualHumanPyDemo::UpdateInfectiousness(float dt)
    {
#ifdef ENABLE_TOYPHOID
        for( auto &route: parent->GetTransmissionRoutes() )
        {
            static auto pFunc = IdmPyInit( "dtk_pydemo_individual", "update_and_return_infectiousness" );
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
                if( val > 0 )
                {
                    LOG_DEBUG_F("Depositing %f to route %s: (antigen=%d, substain=%d)\n", val, route.c_str(), tmp_strainID.GetAntigenID(), tmp_strainID.GetGeneticID());
                    parent->DepositFromIndividual( &tmp_strainID, (float) val, &transmissionGroupMembershipByRoute.at( route ) );
                }
                //Py_DECREF( vars );
                //Py_DECREF( py_existing_id_str );
                //Py_DECREF( py_route_str );
                Py_DECREF( retVal );
            }
        }
        return;
#endif
    }

    Infection* IndividualHumanPyDemo::createInfection( suids::suid _suid )
    {
        return InfectionPyDemo::CreateInfection(this, _suid);
    }

    std::string IndividualHumanPyDemo::processPrePatent( float dt )
    {
        return state_to_report;
    }

    void IndividualHumanPyDemo::Update( float currenttime, float dt)
    {
#ifdef ENABLE_TOYPHOID
        static auto pFunc = IdmPyInit( "dtk_pydemo_individual", "update" );
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
        LOG_DEBUG_F( "state_to_report for individual %d = %s; Infected = %d.\n", GetSuid().data, state_to_report.c_str(), IsInfected() );

        if( state_to_report == "S" && state_changed && GetInfections().size() > 0 )
        {
            // ClearInfection
            auto inf = GetInfections().front();
            IInfectionPyDemo * inf_pydemo  = NULL;
            if (s_OK != inf->QueryInterface(GET_IID(IInfectionPyDemo ), (void**)&inf_pydemo) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "inf", "IInfectionPyDemo ", "Infection" );
            }
            // get InfectionPyDemo pointer
            inf_pydemo->Clear();
        }
        else if( state_to_report == "D" && state_changed )
        {
            LOG_INFO_F( "[Update] Somebody died from their infection.\n" );
        }
#endif
        return IndividualHuman::Update( currenttime, dt);
    }

    void IndividualHumanPyDemo::AcquireNewInfection(StrainIdentity *infstrain, int incubation_period_override )
    {
        LOG_DEBUG_F("AcquireNewInfection: route %d\n", _routeOfInfection);
        IndividualHuman::AcquireNewInfection( infstrain, incubation_period_override );
#ifdef ENABLE_TOYPHOID
        static auto pFunc = IdmPyInit( "dtk_pydemo_individual", "acquire_infection" );
        if( pFunc )
        {
            // pass individual id ONLY
            static PyObject * vars = PyTuple_New(1);
            PyObject* py_existing_id = PyLong_FromLong( GetSuid().data );
            PyTuple_SetItem(vars, 0, py_existing_id );
            PyObject_CallObject( pFunc, vars );
            //Py_DECREF( vars );
        }
#endif
    }

    HumanStateChange IndividualHumanPyDemo::GetStateChange() const
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

    bool IndividualHumanPyDemo::IsChronicCarrier( bool incidence_only ) const
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

    bool IndividualHumanPyDemo::IsSubClinical( bool incidence_only ) const
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

    bool IndividualHumanPyDemo::IsAcute( bool incidence_only ) const
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

    bool IndividualHumanPyDemo::IsPrePatent( bool incidence_only ) const
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
#include "InfectionPyDemo.h"
#include "SusceptibilityPyDemo.h"
#include "PyDemoInterventionsContainer.h"

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Kernel::IndividualHumanPyDemo)

/*
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, IndividualHumanPyDemo& human, const unsigned int  file_version )
    {
        LOG_DEBUG("(De)serializing IndividualHumanPyDemo\n");

        ar.template register_type<Kernel::InfectionPyDemo>();
        ar.template register_type<Kernel::SusceptibilityPyDemo>();
        ar.template register_type<Kernel::PyDemoInterventionsContainer>();
            
        // Serialize fields - N/A
        

        // Serialize base class
        ar & boost::serialization::base_object<Kernel::IndividualHumanEnvironmental>(human);
    }
}
*/

#endif

#endif // ENABLE_PYTHON
