/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip> //setw(), setfill()

#include "BoostLibWrapper.h"
#include "FileSystem.h"
#include "Debug.h"
#include "Log.h"
#include "suids.hpp"
#include "SimulationConfig.h"
#include "SimulationFactory.h"
#include "SimulationEventContext.h" // compiler forcing me to do this...
#include "Simulation.h"

#ifndef _DLLS_
#include "SimulationMalaria.h"
#ifdef ENABLE_POLIO
#include "SimulationPolio.h"
#endif
#ifdef ENABLE_TB
#include "SimulationTB.h"
#ifdef ENABLE_TBHIV
#include "SimulationTBHIV.h"
#endif // TBHIV
#endif // TB
#endif // _DLLS_
#include "ControllerFactory.h"

#pragma warning(disable : 4244)

using namespace Kernel;

static const char * _module = "Controller";


// more static polymorphism that is temporary until we determine why
// boost refuses to register the sim type with the archive

template<class ControllerExecuteFunctorT>
bool call_templated_functor_with_sim_type_hack(ControllerExecuteFunctorT &cef)
{
#ifdef _DLLS_
    return cef.template call<Simulation>();
#else
    //SimType::Enum sim_type = GET_CONFIGURABLE(SimulationConfig)->sim_type;
    std::string sSimType = GET_CONFIG_STRING(EnvPtr->Config, "Simulation_Type");      
    SimType::Enum sim_type;
    if (sSimType == "GENERIC_SIM")
        sim_type = SimType::GENERIC_SIM;
    else if (sSimType == "MALARIA_SIM")
        sim_type = SimType::MALARIA_SIM;
    else if (sSimType == "VECTOR_SIM")
        sim_type = SimType::VECTOR_SIM;
#ifdef ENABLE_POLIO
    else if (sSimType == "ENVIRONMENTAL_SIM")
        sim_type = SimType::ENVIRONMENTAL_SIM;
    else if (sSimType == "POLIO_SIM")
        sim_type = SimType::POLIO_SIM;
#endif
#ifdef ENABLE_TB
    else if (sSimType == "AIRBORNE_SIM")
        sim_type = SimType::AIRBORNE_SIM;
    else if (sSimType == "TB_SIM")
        sim_type = SimType::TB_SIM;
#ifdef ENABLE_TBHIV
    else if (sSimType == "TBHIV_SIM")
        sim_type = SimType::TBHIV_SIM;
#endif // TBHIV
#endif // TB
    else
    {
        std::string note = "The Simulation_Type (='"+sSimType+"') is unknown.  Please select a valid type." ;
        throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, note.c_str() );
    }

    Simulation * newsim = NULL;
    switch (sim_type)
    {
        case SimType::GENERIC_SIM:       return cef.template call<Simulation>(); break;
        case SimType::VECTOR_SIM:        return cef.template call<SimulationVector>(); break;
        case SimType::MALARIA_SIM:       return cef.template call<SimulationMalaria>(); break;
#ifdef ENABLE_POLIO
        case SimType::ENVIRONMENTAL_SIM: return cef.template call<SimulationEnvironmental>(); break;
        case SimType::POLIO_SIM:         return cef.template call<SimulationPolio>(); break;
#endif
#ifdef ENABLE_TB
        case SimType::AIRBORNE_SIM:      return cef.template call<SimulationAirborne>(); break;
        case SimType::TB_SIM:            return cef.template call<SimulationTB>(); break;
#ifdef ENABLE_TBHIV
        case SimType::TBHIV_SIM:         return cef.template call<SimulationTBHIV>(); break;
#endif // TBHIV
#endif // TB
    default: 
        // ERROR: ("call_templated_functor_with_sim_type_hack(): Error, Sim_Type %d is not implemented.\n", sim_type);
        throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "sim_type", sim_type, SimType::pairs::lookup_key( sim_type ) );
        break;
    }
    return false;
#endif
}



#include "Instrumentation.h"
#include "StatusReporter.h"

#include <functional>
#ifdef WIN32
//#undef UNICODE
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

HANDLE thread_handle;
DWORD start_routine(void* args);
SOCKET ClientSocket;
#else
#include<sys/socket.h>
pthread_t server_thread;
void * start_routine(void* args );
int client_sock;
#endif
//std::stringstream timestep_report_json;

void StepSimulation(ISimulation* sim, float dt);

typedef enum {
    paused,
    stepping,
    stepping_and_reloading,
    playing
} tPlayback;

tPlayback playback = playing;

// Basic simulation main loop with reporting
template <class SimulationT> 
void RunSimulation(SimulationT &sim, int steps, float dt)
{
    bool game_mode = false;
#ifdef WIN32
    DWORD threadid = -1;
    size_t required;
    char buffer[256];
    auto ret = getenv_s( &required, buffer, 256, "GAME_MODE" );
    if( required > 0 )
    {
        game_mode = true;
        // CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
        thread_handle = CreateThread(nullptr, 0 /* default */, &start_routine, nullptr, 0 /* run immediately */, &threadid);
        playback = paused;
    }
#else
    int threadid = -1;
    if( getenv( "GAME_MODE" ) )
    {
        game_mode = true;
        threadid = pthread_create( &server_thread, NULL, &start_routine, NULL );
        playback = paused;
    }
#endif

    LOG_DEBUG( "RunSimulation\n" );

    for (int t = 0; t < steps; t++)
    {
        while( playback == paused )
        {
#ifdef WIN32
            Sleep(0.1);
#else
            sleep(0.1);
#endif
            //if( playback > 0 )
            if( playback == stepping_and_reloading )
            {
                sim.LoadCampaignFile();
                playback = stepping;
            }
        }

        //sleep(1.0);
        StepSimulation(&sim, dt);
        if( game_mode )
        {
            if(playback == stepping )
            {
                playback = paused;
            }

#ifdef WIN32
            size_t required;
            char status_message[1024];
            auto ret = getenv_s( &required, status_message, 1024, "JSON_SER_REPORT" );
            int iSendResult = send(ClientSocket, status_message, required + 1, 0);
            cout << "send() returned " << iSendResult << endl; cout.flush();
#else
            //auto status_message = timestep_report_json.str().c_str();
            auto status_message = getenv( "JSON_SER_REPORT" );
            write(client_sock, status_message, strlen(status_message));
#endif

            //timestep_report_json.str("");
            //timestep_report_json.clear();
        }

        if (EnvPtr->MPI.Rank == 0)
        {
            EnvPtr->getStatusReporter()->ReportProgress(t+1, steps);
        }
    }

    if ( game_mode )
    {
#ifdef WIN32
        WaitForSingleObject( thread_handle, INFINITE );
#else
        pthread_join( threadid, NULL );
#endif
    }
}

// note: this version passes a branch_duration that counts only timesteps taken within itself
// if branches have more complicated logic, we may want to put some of that outside.
template <class SimulationT>
void RunSimulation(SimulationT &sim, std::function<bool(SimulationT &, float)> termination_predicate) // TODO: add support for 'dt' to this version
{
    LOG_DEBUG( "RunSimulation\n" );

    float branch_begin = sim.GetSimulationTime();
    float branch_time  = 0;
    float dt           = (float)(GET_CONFIGURABLE(SimulationConfig)->Sim_Tstep);

    while(!termination_predicate(sim, branch_time))
    {
        StepSimulation(&sim, dt);
        branch_time = sim.GetSimulationTime() - branch_begin;
    }
}

void StepSimulation(ISimulation* sim, float dt)
{
    int currentTimestep = sim->GetSimulationTimestep();

    sim->Update(dt);

    EnvPtr->Log->Flush();
}


// ******** WARNING *********
// ENTERING MASSIVE HACK ZONE 

// this class mimics the interface of scoped_ptr but actually does nothing. 
// its a quick hack to try ducking very expensive and unnecessary object cleanup associated 
// runs that only instantiate one simulation and then exit...while keeping the code formally 
// similar to a 'proper' implementation
// this would all go away with more thoughtful use of allocators, and perhaps custom allocators
// - in particular, scoping the allocation blocks for boost pool allocators around the simulation lifetimes would help

template<class T>
class MassivelyHackedLeakyPointer
{
public:
    MassivelyHackedLeakyPointer(T* _ptr) : px(_ptr) {}

    void reset(T * p = 0) // never throws
    {
        BOOST_ASSERT( p == 0 || p != px ); // catch self-reset errors
        px = p;
    }

    T & operator*() const // never throws
    {
        BOOST_ASSERT( px != 0 );
        return *px;
    }

    T * operator->() const // never throws
    {
        BOOST_ASSERT( px != 0 );
        return px;
    }

    T * get() const // never throws
    {
        return px;
    }

    bool operator==( MassivelyHackedLeakyPointer<T> const& o ) const { return o->px == px; }
    bool operator==( T* const& o ) const { return o == px; }

protected:
    T * px;
};

template <class SimulationT>
bool DefaultController::execute_internal()
{
    using namespace Kernel;
    list<string> serialization_test_state_filenames;
    //typedef Simulation SimulationT ;

    LOG_INFO("DefaultController::Execute<>()...\n");

    // NB: BIG INTENTIONAL HACK
    // the exact nature of pool allocators substantially helps communication performance BUT unwinding them all at the end can double the simulation runtime for a real production scenario.
    // for processes that dont need to have more than one simulation in memory, its faster to just leak the whole object

#ifdef _DEBUG
    boost::scoped_ptr<SimulationT> sim(dynamic_cast<SimulationT*>(SimulationFactory::CreateSimulation())); // 30+ minutes to unwind a ~2gb simulation state if we do this. unacceptable for real work!
#else
#ifdef _DLLS_
    ISimulation * sim = SimulationFactory::CreateSimulation(); 
    release_assert(sim);
#else
    MassivelyHackedLeakyPointer<SimulationT> sim(dynamic_cast<SimulationT*>(SimulationFactory::CreateSimulation())); 

    if (NULL == sim.get())
    {
        throw InitializationException( __FILE__, __LINE__, __FUNCTION__, "sim.get() returned NULL after call to CreateSimulation." );
    }
#endif
#endif

    std::string state_filename = (boost::format("SimulationStateProc%04d.txt") % EnvPtr->MPI.Rank).str();

    if (EnvPtr->MPI.Rank==0) { ostringstream oss; oss << "Beginning Simulation...";  EnvPtr->getStatusReporter()->ReportStatus(oss.str()); }

    // populate it
    LOG_INFO("DefaultController populate simulation...\n");
    if(sim->Populate())
    {
        if( !JsonConfigurable::missing_parameters_set.empty() )
        {
            std::stringstream errMsg;
            errMsg << "The following necessary parameters were not specified" << std::endl;
            for (auto& key : JsonConfigurable::missing_parameters_set)
            {
                errMsg << "\t \"" << key.c_str() << "\"" << std::endl;
            }
            //LOG_ERR( errMsg.str().c_str() );
            throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, errMsg.str().c_str() );
        }
        // now try to run it
        // divide the simulation into stages according to requesting number of serialization test cycles
        float dt = GET_CONFIGURABLE(SimulationConfig)->Sim_Tstep;
        int simulation_steps = (int)(GET_CONFIGURABLE(SimulationConfig)->Sim_Duration)/dt;

#ifndef _DLLS_
        int remaining_steps = simulation_steps;

        int serialization_test_cycles = GET_CONFIGURABLE(SimulationConfig)->serialization_test_cycles;

        int serialization_cycles = 1 + serialization_test_cycles; 
        for (int k= 0; /*k < serialization_cycles*/ remaining_steps > 0; k++)
        {
            int serialization_stress_cycles = serialization_test_cycles > 0 ? 1 : 0;
            for (int j = 0; j < serialization_stress_cycles; j++)
            {
                LOG_INFO("Saving Simulation to disk...\n");

                state_filename = (boost::format("SimulationStateProc%04dCycle%04d.txt") % EnvPtr->MPI.Rank % j).str();

#if USE_BOOST_SERIALIZATION
                REPORT_TIME(false,(boost::format("SaveCycle%d") % k).str(),
                    save_sim<boost::archive::binary_oarchive>(sim.get(), state_filename.c_str()));

                LOG_INFO("Loading Simulation from disk...\n");
                REPORT_TIME(false,(boost::format("LoadCycle%d") % k).str(),
                    sim.reset(dynamic_cast<SimulationT*>(load_sim<boost::archive::binary_iarchive, SimulationT>(state_filename.c_str()))));
                EnvPtr->Log->Flush();

                serialization_test_state_filenames.push_back(state_filename);
#endif
            }

            int cycle_steps = min(remaining_steps, max(1, (int)ceil(float(simulation_steps)/float(serialization_cycles))));
            if (cycle_steps > 0)
                RunSimulation(*sim, cycle_steps, dt);

            remaining_steps -= cycle_steps;
        }
#else
        LOG_INFO( "Execute<> Calling RunSimulation.\n" );
        RunSimulation(*sim, simulation_steps, dt);
#endif
        sim->WriteReportsData();

        if (EnvPtr->MPI.Rank==0)
        {
            LogTimeInfo lti;
            EnvPtr->Log->GetLogInfo(lti);

            ostringstream oss;
            oss << "Done - " << lti.hours << ":" << setw(2) << setfill('0') << lti.mins << ":" << setw(2) << setfill('0') << lti.secs;
            EnvPtr->getStatusReporter()->ReportStatus(oss.str());
        }

        // cleanup serialization test state files
        for (auto& filename : serialization_test_state_filenames)
        {
            if( FileSystem::FileExists( filename ) )
                FileSystem::RemoveFile( filename );
        }

        return true;
    }
    return false;
}


bool DefaultController::execute_internal()
{

    using namespace Kernel;
    list<string> serialization_test_state_filenames;

    LOG_INFO("DefaultController::execute_internal()...\n");

#ifdef _DLLS_
    ISimulation * sim = SimulationFactory::CreateSimulation(); 
    release_assert(sim);
#else

    boost::scoped_ptr<ISimulation> sim((SimulationFactory::CreateSimulation()));

    if (NULL == sim.get())
    {
        throw InitializationException( __FILE__, __LINE__, __FUNCTION__, "sim.get() returned NULL after call to CreateSimulation.\n" );
    }

#endif // End of _DLLS_

    std::string state_filename = (boost::format("SimulationStateProc%04d.txt") % EnvPtr->MPI.Rank).str();

    if (EnvPtr->MPI.Rank==0) { ostringstream oss; oss << "Beginning Simulation...";  EnvPtr->getStatusReporter()->ReportStatus(oss.str()); }

    // populate it
    LOG_INFO("DefaultController::execute_internal() populate simulation...\n");
    // Confusing variable name (JC::useDefaults); we want to collect all defaults for reporting. It's up to us as the calling function
    JsonConfigurable::_useDefaults = true;
    if(sim->Populate())
    {
        // Need to reset back to false; will be set as appropriate by campaign related code after this based on
        // "Use_Defaults" in campaign.json.
        JsonConfigurable::_useDefaults = false;
        if( !JsonConfigurable::missing_parameters_set.empty() )
        {
            std::stringstream errMsg;
            errMsg << "The following necessary parameters were not specified" << std::endl;
            for (auto& key : JsonConfigurable::missing_parameters_set)
            {
                errMsg << "\t \"" << key.c_str() << "\"" << std::endl;
            }
            //LOG_ERR( errMsg.str().c_str() );
            throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, errMsg.str().c_str() );
        }
        // now try to run it
        // divide the simulation into stages according to requesting number of serialization test cycles
        float dt = GET_CONFIGURABLE(SimulationConfig)->Sim_Tstep;
        int simulation_steps = (int)(GET_CONFIGURABLE(SimulationConfig)->Sim_Duration)/dt;

#ifndef _DLLS_
        int remaining_steps = simulation_steps;

        int serialization_test_cycles = GET_CONFIGURABLE(SimulationConfig)->serialization_test_cycles;

        int serialization_cycles = 1 + serialization_test_cycles; 
        for (int k= 0; /*k < serialization_cycles*/ remaining_steps > 0; k++)
        {
            int serialization_stress_cycles = serialization_test_cycles > 0 ? 1 : 0;
            for (int j = 0; j < serialization_stress_cycles; j++)
            {
                LOG_INFO("Saving Simulation to disk...\n");

                state_filename = (boost::format("SimulationStateProc%04dCycle%04d.txt") % EnvPtr->MPI.Rank % j).str();

#if USE_BOOST_SERIALIZATION
                REPORT_TIME(false,(boost::format("SaveCycle%d") % k).str(),
                    save_sim<boost::archive::binary_oarchive>(sim.get(), state_filename.c_str()));

                LOG_INFO("Loading Simulation from disk...\n");
                REPORT_TIME(false,(boost::format("LoadCycle%d") % k).str(),
                    sim.reset(dynamic_cast<SimulationT*>(load_sim<boost::archive::binary_iarchive, SimulationT>(state_filename.c_str()))));
                EnvPtr->Log->Flush();

                serialization_test_state_filenames.push_back(state_filename);
#endif
            }

            int cycle_steps = min(remaining_steps, max(1, (int)ceil(float(simulation_steps)/float(serialization_cycles))));
            if (cycle_steps > 0)
                RunSimulation(*sim, cycle_steps, dt);

            remaining_steps -= cycle_steps;
        }
#else // _DLLS_
        LOG_INFO( "Execute_internal(): Calling RunSimulation.\n" );
        RunSimulation(*sim, simulation_steps, dt);
#endif
        sim->WriteReportsData();

        if (EnvPtr->MPI.Rank==0)
        {
            LogTimeInfo lti;
            EnvPtr->Log->GetLogInfo(lti);

            ostringstream oss;
            oss << "Done - " << lti.hours << ":" << setw(2) << setfill('0') << lti.mins << ":" << setw(2) << setfill('0') << lti.secs;
            EnvPtr->getStatusReporter()->ReportStatus(oss.str());
        }

        // cleanup serialization test state files
        for (auto& filename : serialization_test_state_filenames)
        {
            if( FileSystem::FileExists( filename ) )
                FileSystem::RemoveFile( filename );
        }

        return true;
    }
    return false;
}

bool DefaultController::Execute()
{

#if USE_BOOST_SERIALIZATION
    ControllerExecuteFunctor<DefaultController> cef(this);
    return call_templated_functor_with_sim_type_hack(cef);
#else
    return execute_internal();
#endif

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//

template <class SimulationT>
bool BurnInCacheTestController::execute_internal()
{
    /*
    Next steps on burn-in caching controller

    - update schema with new parameters
    - matlab scripts to test locally
    - write a test case that caches the burn-in for a trivial polio sim
    - write a test case that caches the burn-in for a nontrivial polio sim
    */
    using namespace Kernel;
#ifdef BURNIN_CACHE_TEST_CONTROLLER
    LOG_INFO("BurnInCacheTestController::Execute()\n");

    std::string burnin_cache_mode = GET_CONFIGURABLE(SimulationConfig)->burnin_cache_mode;
    int burnin_period = GET_CONFIGURABLE(SimulationConfig)->burnin_period;
    std::string burnin_name = GET_CONFIGURABLE(SimulationConfig)->burnin_name;

    std::string state_filename = (boost::format("%s.Proc%04d.state") % burnin_name % EnvPtr->MPI.Rank).str();

    if (burnin_cache_mode=="write")
    {
        boost::scoped_ptr<SimulationT> sim((SimulationT*)SimulationFactory::CreateSimulation());

        if (NULL == sim)
            return false;

        if (EnvPtr->MPI.Rank==0) { ostringstream oss; oss << "Beginning Simulation..."; EnvPtr->getStatusReporter()->ReportStatus(oss.str()); }

        // run up until burnin_period, save, and quit

        // populate it
        if(sim->Populate())
        {
            float dt = (float)GET_CONFIGURABLE(SimulationConfig)->Sim_Tstep;
            int simulation_steps = (int)((float)burnin_period/dt);

            // now try to run it
            RunSimulation(*sim, simulation_steps, dt);

            LOG_INFO_F("Saving Simulation State to %s after %d steps...\n", state_filename.c_str(), simulation_steps);
            EnvPtr->Log->Flush();

            REPORT_TIME(false, (boost::format("SaveState") ).str(),
            {
                if (!save_sim<boost::archive::binary_oarchive>(
                        sim.get(),
                        (FileSystem::Concat( EnvPtr->StatePath, state_filename).c_str()))
                {
                    // ERROR: ("Failed to save simulation state.");
                    throw SerializationException( __FILE__, __LINE__, __FUNCTION__, "Saving" );
                    return false;
                }
            });

            sim->WriteReportsData();

            if (EnvPtr->MPI.Rank==0)
            {
                LogTimeInfo lti;
                EnvPtr->Log->GetLogInfo(lti);

                ostringstream oss;
                oss << "Done - " << lti.hours << ":" << setw(2) << setfill('0') << lti.mins << ":" << setw(2) << setfill('0') << lti.secs;
                EnvPtr->getStatusReporter()->ReportStatus(oss.str());
            }

            return true;
        }
        return false;

    }
    else if (burnin_cache_mode=="read")
    {
        // load simulation and then run it for remaining steps specified (Sim_Duration - burnin_period)
        boost::scoped_ptr<SimulationT> sim(NULL);

        if (EnvPtr->MPI.Rank==0) { ostringstream oss; oss << "Beginning Simulation..."; EnvPtr->getStatusReporter()->ReportStatus(oss.str()); }

        LOG_INFO_F("Loading Simulation State from %s ...\n", state_filename.c_str()); EnvPtr->Log->Flush();

        REPORT_TIME(false, (boost::format("LoadState") ).str(),  
            sim.reset((SimulationT*)load_sim<boost::archive::binary_iarchive, SimulationT>(
                (FileSystem::Concat( EnvPtr->StatePath, state_filename ).c_str())));

        if (sim == NULL)
        {
            //ERROR: ("Failed to load saved state.\n");
            throw SerializationException( __FILE__, __LINE__, __FUNCTION__, "Loading" );
            return false;
        }

        float dt = GET_CONFIGURABLE(SimulationConfig)->Sim_Tstep;
        int simulation_steps = (int)((float)(GET_CONFIGURABLE(SimulationConfig)->Sim_Duration) - burnin_period)/dt);

        // apply a new random number seed from the config so that the evolved trajectory has a chance to differ
        // HACK JUST COMMENTING OUT COZ ResetRng not in ISimulation. sim->ResetRng(EnvPtr->Config);

        // now try to run it
        RunSimulation(*sim, simulation_steps, dt);

        sim->WriteReportsData();

        if (EnvPtr->MPI.Rank==0)
        {
            LogTimeInfo lti;
            EnvPtr->Log->GetLogInfo(lti);

            ostringstream oss;
            oss << "Done - " << lti.hours << ":" << setw(2) << setfill('0') << lti.mins << ":" << setw(2) << setfill('0') << lti.secs;
            EnvPtr->getStatusReporter()->ReportStatus(oss.str());
        }

        return true;
    }
    else
    {
        // default case not implemented for this test controller. in a real controller, the burn-in wrapper would just pass control to some normal inner process
        // ERROR: ("Burnin mode \"%s\" not implemented for this test controller; must be \"read\" or \"write\".\n", burnin_cache_mode.c_str());
        throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Burnin" );
        return false;
    }
#endif
    return false;
}

bool BurnInCacheTestController::Execute()
{
    ControllerExecuteFunctor<BurnInCacheTestController> cef(this);
    return call_templated_functor_with_sim_type_hack(cef);
}

//////////////////////////////////////////////////////////////////////////
// SimpleBranchController
// optionally loads a given state from the shared state cache, runs it for Sim_Duration steps, and optionally writes a new state

template <class SimulationT>
bool SimpleBranchController::execute_internal()
{
#ifdef BRANCH_CONTROLLER
    using namespace Kernel;

    LOG_INFO("SimpleBranchController::Execute()\n");

    std::string branch_start_state = GET_CONFIGURABLE(SimulationConfig)->branch_start_state;
    std::string branch_end_state = GET_CONFIGURABLE(SimulationConfig)->branch_end_state;
    int branch_duration = GET_CONFIGURABLE(SimulationConfig)->branch_duration;

    std::string start_state_filename = (boost::format("%s.Proc%04d.state") % branch_start_state % EnvPtr->MPI.Rank).str();
    std::string end_state_filename = (boost::format("%s.Proc%04d.state") % branch_end_state % EnvPtr->MPI.Rank).str();

    SimulationT *sim;

    if (EnvPtr->MPI.Rank==0) { ostringstream oss; oss << "Beginning Simulation..."; EnvPtr->getStatusReporter()->ReportStatus(oss.str()); }

    // init sim by reading a state
    if (branch_start_state.length() > 0)
    {
        LOG_INFO_F("Loading Simulation State from %s ...\n", start_state_filename.c_str()); EnvPtr->Log->Flush();

        BEGIN_REPORT_TIME(false)
        {
            sim = (SimulationT*)load_sim<boost::archive::binary_iarchive, SimulationT>(
                (FileSystem::Concat( EnvPtr->StatePath, start_state_filename ).c_str());
        }
        END_REPORT_TIME(false,"LoadState")

        if (sim == NULL)
        {
            // ERROR ("Failed to load saved state.\n");
            throw SerializationException( __FILE__, __LINE__, __FUNCTION__, "Loading" );
            return false;
        }

        // apply a new random number seed from the config so that the evolved trajectory has a chance to differ
        // this is done instead of after populate for consistency with the behavior of the default controller when starting from scratch
        // HACK TO GET DLL CONCEPT WORKING sim->ResetRng(EnvPtr->Config);
    }
    else // initialize via populate
    {
        sim = (SimulationT*)SimulationFactory::CreateSimulation();

        if(!sim->Populate())
        {
            //delete sim; LEAK ME!!
            throw InitializationException( __FILE__, __LINE__, __FUNCTION__, "Failed to populate simulation." );
        }
    }

    auto termination_predicate =  SimpleTerminationPredicateFactory::CreatePredicate(*sim, EnvPtr->Config);

    // now run
    RunSimulation(*sim, termination_predicate);

    // write final state if desired

    if (branch_end_state.length() > 0)
    {
        LOG_INFO_F("Saving Simulation State to %s after %d steps...\n", end_state_filename.c_str(), branch_duration);
        EnvPtr->Log->Flush();

        REPORT_TIME(false,(boost::format("SaveState")).str(),
        {
            if (!save_sim<boost::archive::binary_oarchive>(
                sim,
                (FileSystem::Concat( EnvPtr->StatePath, end_state_filename ).c_str()))
            {
                throw SerializationException( __FILE__, __LINE__, __FUNCTION__, "Saving" );
            }
        });
    }

    sim->WriteReportsData();

    if (EnvPtr->MPI.Rank==0)
    {
        LogTimeInfo lti;
        EnvPtr->Log->GetLogInfo(lti);

        ostringstream oss;
        oss << "Done - " << lti.hours << ":" << setw(2) << setfill('0') << lti.mins << ":" << setw(2) << setfill('0') << lti.secs;
        EnvPtr->getStatusReporter()->ReportStatus(oss.str());
    }

    //delete sim;  LEAK ME!! no need to clean up
#endif
    return true;
}

bool SimpleBranchController::Execute()
{
    ControllerExecuteFunctor<SimpleBranchController> cef(this);
    return call_templated_functor_with_sim_type_hack(cef);
}

template <class SimulationT>
static std::function<bool(SimulationT&,float)>
    SimpleTerminationPredicateFactory::CreatePredicate(SimulationT &sim_context,  const Configuration *config)
{
    string TerminationPredicate;
    if (!CONFIG_PARAMETER_EXISTS(config, "TerminationPredicate"))
        TerminationPredicate = "SimDuration";
#ifdef FUTURE
    else
        TerminationPredicate = GET_CONFIGURABLE(SimulationConfig)->TerminationPredicate;
#endif
    LOG_INFO( "And now you are here...\n" );

    if ("SimDuration" == TerminationPredicate)
    {
        int simulation_duration = GET_CONFIGURABLE(SimulationConfig)->Sim_Duration;
        return [=](SimulationT& sim,float) { return sim.GetSimulationTime() >= simulation_duration; };
    }

    else if ("ZeroHumanInfectionsOrSimDuration" == TerminationPredicate)
    {
        int simulation_steps = GET_CONFIGURABLE(SimulationConfig)->Sim_Duration;

        int holdoff_period = 100; // cold make this smarter even, but meh

        return [=](SimulationT& sim, float branch_time)->bool
        {
            bool terminate = false;

            // have to evaluate this condition on every rank because it involves a reduce, but the result is only valid on rank 0
            bool infections_reached_zero_after_holdoff = 
                (sim.GetSimulationTime() > holdoff_period) &&
                (0 == false/*sim.GetProcessedSummaryData("Infected", sim.GetSimulationTimestep()-1)*/); // only returns a valid number to rank 0, but must be invoked on all processes. call is short circuited to prevent t=-1 being passed.

            // !!!!!!!!!!!!!!!!!!!!!!!!
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Querying simulation to get info for termination predicate." );
            // !!!!!!!!!!!!!!!!!!!!!!!!

            if (EnvPtr->MPI.Rank == 0) // only rank zero gets a valid value for infected
            {
                // assuming 1day/timestep
                if (infections_reached_zero_after_holdoff)
                {
                    LOG_INFO("Infected fraction reached zero, terminating.\n");
                    terminate = true;
                }
                else
                {
                    terminate = (sim.GetSimulationTimestep() >= simulation_steps);
                }
            }

            boost::mpi::broadcast(*EnvPtr->MPI.World, terminate, 0); // all processes need to return the result of the decision on rank zero
            return terminate;
        };
    }
    else if ("VDPVOutbreakOrSimDuration" == TerminationPredicate)
    {
        int simulation_steps = GET_CONFIGURABLE(SimulationConfig)->Sim_Duration;

        int holdoff_period = 180; // could be grabbed from config as well

        return [=](SimulationT& sim, float branch_time)->bool
        {
            // assuming 1day/timestep
            bool terminate = false;
            float cumulative_infections_at_holdoff = 
                sim.GetSimulationTimestep() > holdoff_period ? 0/*sim.GetProcessedSummaryData("New Reported Infections", holdoff_period)*/ : 0;  // assuming 1 day timesteps // TODO: this isnt the right channel name, obviously!  <ERAD-201>
            float cumulative_infections_current = 
                sim.GetSimulationTimestep() > 0 ? 0/*sim.GetProcessedSummaryData("New Reported Infections", sim.GetSimulationTimestep()-1)*/ : 0;

            // !!!!!!!!!!!!!!!!!!!!!!!!
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Querying simulation to get info for termination predicate." );
            // !!!!!!!!!!!!!!!!!!!!!!!!

            if (EnvPtr->MPI.Rank == 0)
            {
                if ( cumulative_infections_at_holdoff < cumulative_infections_current )
                {
                    LOG_INFO_F("New paralytic case detected after day %d, terminating.", holdoff_period);
                    terminate = true;
                }
                else
                {
                    terminate = (sim.GetSimulationTimestep() >= simulation_steps);
                }
            }
            boost::mpi::broadcast(*EnvPtr->MPI.World, terminate, 0);
            return terminate;
        };
    }
    else
    {
        throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "Burnin" );
    }
}


#if USE_BOOST_SERIALIZATION

// R: Note: this could probably be obviated with use of BOOST_CLASS_EXPORT for these top level classes
template <class Archive>
void registerSimTypes(Archive &ar)
{
#ifndef _DLLS_
    ar.template register_type<Simulation>();
    ar.template register_type<SimulationEnvironmental>();  // IF ONLY THIS WORKED
    ar.template register_type<SimulationPolio>();       // IF ONLY THIS WORKED
    ar.template register_type<SimulationVector>();
    ar.template register_type<SimulationMalaria>();
#ifdef ENABLE_TB
    ar.template register_type<SimulationAirborne>();
    ar.template register_type<SimulationTB>();
#endif
#endif
}

template<class OArchiveT, class SimulationT>
bool save_sim(/*const breaks if I cant make serialize() const?*/ SimulationT *sim, const char * filename)
{
    // make an archive
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs.is_open()) 
    {
        // ERROR ("save_sim() failed to open file %s for writing.\n", filename);
        throw SerializationException( __FILE__, __LINE__, __FUNCTION__, "Saving" );
    }

    OArchiveT oa(ofs);
    registerSimTypes(oa);
    oa << (*sim);
    return true;
}


/* R: TODO: experiment with better boost class registration to clean up serialization mess <ERAD-326>

Noticed this gem in the boost docs:
(see: http://www.boost.org/doc/libs/1_42_0/libs/serialization/doc/special.html#plugins)

Plugins

In order to implement the library, various facilities for runtime manipulation of types are runtime were required. These are extended_type_info for associating classes with external identifying strings (GUID) and void_cast for casting between pointers of related types. To complete the functionality of extended_type_info the ability to construct and destroy corresponding types has been added. In order to use this functionality, one must specify how each type is created. This should be done at the time a class is exported. So, a more complete example of the code above would be:

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
... // other archives

#include "a.hpp" // header declaration for class a

// this class has a default constructor
BOOST_SERIALIZATION_FACTORY_0(a)
// as well as one that takes one integer argument
BOOST_SERIALIZATION_FACTORY_1(a, int)

// specify the GUID for this class
BOOST_CLASS_EXPORT(a)
... // other class headers and exports

---> With this in place, one can construct, serialize and destroy about which only is know the GUID and a base class. <---

If this actually works, a lot of the templates in this file can go away!

*/

template<class IArchiveT, class SimulationT>
ISimulation* load_sim(const char * filename)
{
    // open the archive
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs.is_open())
    {
        // ERROR: ("load_sim() failed to open file %s for reading.\n", filename);
        throw SerializationException( __FILE__, __LINE__, __FUNCTION__, "Loading" );
        return NULL;
    }

    IArchiveT ia(ifs);
    registerSimTypes(ia);

    // restore the schedule from the archive
#ifdef _DLLS_
    return NULL; // just testing
#else
    return SimulationFactory::CreateSimulationFromArchive<IArchiveT, SimulationT>(ia);
#endif
}

#endif // end of USE_BOOST_SERIALIZATION



// for DLL build
#if USE_BOOST_SERIALIZATION
template void Kernel::serialize( boost::archive::binary_oarchive & ar, Simulation& sim, const unsigned int file_version);
template void Kernel::serialize( boost::archive::binary_iarchive & ar, Simulation& sim, const unsigned int file_version);
#endif

#ifdef WIN32

DWORD start_routine(void* /*arg*/)
{
cout << "Starting server thread..." << endl; cout.flush();

    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
    {
        printf("Could not initialize WinSock.\n");
    }
    cout << "WSAStartup() returned " << iResult << endl; cout.flush();

#define DEFAULT_PORT    "8887"

    struct addrinfo *result = nullptr, *ptr = nullptr, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the local address and port to be used by the server
    size_t required;
    char port_string[16];
    auto ret = getenv_s( &required, port_string, 16, "GAME_PORT" );
    if ( required == 0)
    {
        strcpy_s( port_string, 16, DEFAULT_PORT );
    }
    cout << "Using port " << port_string << endl; cout.flush();

    iResult = getaddrinfo(nullptr, port_string, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return -1;
    }
    cout << "getaddrinfo() returned " << iResult << endl; cout.flush();

    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return -1;
    }
    cout << "socket() returned " << ListenSocket << endl; cout.flush();

    // Setup the TCP listening socket
    iResult = ::bind( ListenSocket, result->ai_addr, int(result->ai_addrlen));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }
    cout << "bind() returned " << iResult << endl; cout.flush();

    freeaddrinfo(result);

    if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
        printf( "Listen failed with error: %ld\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }
    cout << "listen()-ing" << endl; cout.flush();

    // Accept a client socket
    /* SOCKET */ ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }
    cout << "accept() returned " << ClientSocket << endl; cout.flush();

#define DEFAULT_BUFLEN 4096

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Receive until the peer shuts down the connection
    do {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
//            printf("Bytes received: %d\n", iResult);
//
//            // Echo the buffer back to the sender
//            int iSendResult = send(ClientSocket, recvbuf, iResult, 0);
//            if (iSendResult == SOCKET_ERROR) {
//                printf("send failed: %d\n", WSAGetLastError());
//                closesocket(ClientSocket);
//                WSACleanup();
//                return ;
//            }
//            printf("Bytes sent: %d\n", iSendResult);

            printf("msg = %s\n", recvbuf);
            if( strcmp( recvbuf, "PAUSE" ) == 0 )
            {
                printf( "Pausing...\n" );
                playback = paused;
            }
            else if( strcmp( recvbuf, "PLAY" ) == 0 )
            {
                printf( "Playing...\n" );
                playback = playing;
            }
            else if( strcmp( recvbuf, "STEP" ) == 0 )
            {
                printf( "Playing 1 timestep...\n" );
                playback = stepping;
            }
            else if( strcmp( recvbuf, "STEP_RELOAD" ) == 0 )
            {
                printf( "Playing 1 timestep and re-upping campaign...\n" );
                playback = stepping_and_reloading;
            }

            memset( recvbuf, 0, DEFAULT_BUFLEN );

        } else if (iResult == 0) {
            printf("Connection closing...\n");
        }
        else {
            printf("recv failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return -1;
        }
    } while (iResult > 0);

    // shutdown the send half of the connection since no more data will be sent
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return -1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}

#else

/*
    C socket server example
*/
 
#include<stdio.h>
#include<string.h>    //strlen
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<semaphore.h>    //write
#include<pthread.h>    //write
#include<errno.h>    //write

sem_t mysem;

void * start_routine(void* args )
{
    int socket_desc, c, read_size, new_sock;
    struct sockaddr_in server , client;
    char client_message[4000];
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    int game_port = 8887;
    if( getenv( "GAME_PORT" ) )
    {
        game_port = atoi( getenv( "GAME_PORT" ) );
    }
    std::cout << "Listening for controller socket connections on port " << game_port << std::endl;
    server.sin_port = htons( game_port );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        //playback = playing;
        return; // 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return; // 1;
    }
    puts("Connection accepted");
     
    //Receive a message from client
    while( (read_size = recv(client_sock, client_message, 4000, 0)) > 0 )
    {
        //Send the message back to client
        printf( "msg = %s\n", client_message );
        int sem_value = 0;
        if( strcmp( client_message, "PAUSE" ) == 0 )
        {
            printf( "Pausing...\n" );
            playback = paused;
        }
        else if( strcmp( client_message, "PLAY" ) == 0 )
        {
            printf( "Playing...\n" );
            playback = playing;
        }
        else if( strcmp( client_message, "STEP" ) == 0 )
        {
            printf( "Playing 1 timestep...\n" );
            playback = stepping;
        }
        else if( strcmp( client_message, "STEP_RELOAD" ) == 0 )
        {
            printf( "Playing 1 timestep and re-upping campaign...\n" );
            playback = stepping_and_reloading;
        }
        bzero( client_message, 4000 );
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
     
    return; // 0
}

#endif
