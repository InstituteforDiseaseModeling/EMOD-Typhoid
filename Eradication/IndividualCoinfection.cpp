/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include <list>
#include <map>
#include "Exceptions.h"
#include "IndividualCoinfection.h"
#include "IContagionPopulation.h"

#include "InfectionTB.h"
#include "SusceptibilityTB.h"

#include "InfectionHIV.h"
#include "SusceptibilityHIV.h"

#include "MasterInterventionsContainer.h"

#include "Configuration.h"
#include "Infection.h"
#include "SimulationConfig.h"
#include "NodeDemographics.h"

#include "Log.h"

// These includes are only used for serialization
// clorton #include "InfectionTB.h"
// clorton #include "SusceptibilityTB.h"
// clorton #include "TBInterventionsContainer.h"

// GHH added: These includes are only used for serialization
// clorton #include "InfectionHIV.h"
// clorton #include "SusceptibilityHIV.h"
// clorton #include "HIVInterventionsContainer.h"

static const char* _module = "IndividualHumanCoinfection";

namespace Kernel
{
    float IndividualHumanCoinfectionConfig::HIV_coinfection_probability = 0.0f;
    float IndividualHumanCoinfectionConfig::Coinfected_mortality_rate = 0.0f;
    map <float,float> IndividualHumanCoinfectionConfig::CD4_act_map;

    GET_SCHEMA_STATIC_WRAPPER_IMPL(TBHIV.Individual,IndividualHumanCoinfectionConfig)
    BEGIN_QUERY_INTERFACE_BODY(IndividualHumanCoinfectionConfig)
    END_QUERY_INTERFACE_BODY(IndividualHumanCoinfectionConfig)

    bool IndividualHumanCoinfectionConfig::Configure( const Configuration * config )
    {
        LOG_DEBUG( "Configure()\n");

        initConfigTypeMap( "HIV_coinfection_probability", &HIV_coinfection_probability, HIV_Coinfection_Probability_DESC_TEXT, 0.0f, 1.0f, 0.5f );
        initConfigTypeMap( "Coinfected_mortality_rate",   &Coinfected_mortality_rate, Coinfected_Mortality_Rate_DESC_TEXT, 0.0f, 1.0f, 0.1f );

        return JsonConfigurable::Configure( config );
    }

    BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanCoinfection, IndividualHumanAirborne)
        HANDLE_INTERFACE(IIndividualHumanCoinfection)
        HANDLE_INTERFACE(IIndividualHumanTB)
        /*HANDLE_INTERFACE(ISusceptibilityHIV)
        HANDLE_INTERFACE(ISusceptibilityTB)
        HANDLE_INTERFACE(IInfectionTB)
        HANDLE_INTERFACE(IInfectionHIV)*/
        END_QUERY_INTERFACE_DERIVED(IndividualHumanCoinfection, IndividualHumanAirborne)

        IndividualHumanCoinfection::IndividualHumanCoinfection(suids::suid _suid, float monte_carlo_weight, float initial_age, int gender, float initial_poverty) :
        IndividualHumanAirborne(_suid, monte_carlo_weight, initial_age, gender, initial_poverty),
        infectioncount_tb(0),
        infectioncount_hiv(0)
    {

    }

    void IndividualHumanCoinfection::InitializeStaticsCoinfection( const Configuration* config )
    {
        SusceptibilityTBConfig tb_immunity_config;
        tb_immunity_config.Configure( config );
        InfectionTBConfig tb_infection_config;
        tb_infection_config.Configure( config );

        // Now create static, constant map between CD4 and factor for increased reactivation rate. This is only done once.
        IndividualHumanCoinfectionConfig::CD4_act_map = tb_infection_config.GetCD4Map();
        
        SusceptibilityHIVConfig hiv_immunity_config;
        hiv_immunity_config.Configure( config );
        InfectionHIVConfig hiv_infection_config;
        hiv_infection_config.Configure( config );

        IndividualHumanCoinfectionConfig individual_config;
        individual_config.Configure( config );
    }

    IndividualHumanCoinfection *IndividualHumanCoinfection::CreateHuman(INodeContext *context, suids::suid id, float MCweight, float init_age, int gender, float init_poverty)
    {
        IndividualHumanCoinfection *newindividual = _new_ IndividualHumanCoinfection(id, MCweight, init_age, gender, init_poverty);

        newindividual->SetContextTo(context);
        LOG_DEBUG_F( "Created human with age=%f\n", newindividual->m_age );

        return newindividual;
    }

    void IndividualHumanCoinfection::InitializeHuman()
    {
        IndividualHuman::InitializeHuman();

        m_is_on_ART = 0;
        m_has_ever_been_onART= 0;
        m_has_ever_tested_positive_for_HIV = 0;
    }

    void IndividualHumanCoinfection::CreateSusceptibility(float imm_mod, float risk_mod)
    {
        susceptibility_tb = SusceptibilityTB::CreateSusceptibility(this, m_age, imm_mod, risk_mod) ;
        susceptibility_hiv = SusceptibilityHIV::CreateSusceptibility(this, m_age, imm_mod, risk_mod) ;
        susceptibilitylist.push_back( susceptibility_tb);
        susceptibilitylist.push_back( susceptibility_hiv);
    }

    const std::list< Susceptibility* > &
        IndividualHumanCoinfection::Getsusceptibilitylist()
    {
        return susceptibilitylist;
    }
    void IndividualHumanCoinfection::AcquireNewInfection(StrainIdentity *infstrain, int incubation_period_override )
    {
        LOG_DEBUG( "AcquireNewInfection\n" );

        //int numInfs = (int)infectionslist.size(); // note here we use max_ind_inf for the total number of tb + HIV infections
        int numInfs = infectioncount_tb;
        if ((IndividualHumanConfig::superinfection && (numInfs < IndividualHumanConfig::max_ind_inf)) || numInfs == 0)
        {
            cumulativeInfs++;
            m_is_infected = true;

            //First acquire the TB infection (100% prob), acquire the HIV infection (30% fixed prob)
            //for each created infection, set the parameters and the initinfectionimmunology
            infection_list_t newInfectionlist;
            createInfection( parent->GetNextInfectionSuid(), newInfectionlist  );

            for (auto newinf : newInfectionlist)
            {
                IInfectionTB* pinfection = nullptr;
                if (s_OK == newinf->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfection) )
                {
                    static_cast<Infection*>(newinf)->SetParameters(infstrain, incubation_period_override);

                    if (HasHIV() )
                    {
                        Mod_activate();
                    }

                    static_cast<Infection*>(newinf)->InitInfectionImmunology(susceptibility_tb);
                    LOG_DEBUG( "Adding infection to infections list.\n" );
                    infections.push_front(newinf);
                    infection2susceptibilitymap[newinf] = susceptibility_tb;
                    infectioncount_tb++;
                    //infection2interventionsmap[newinf] = interventions_tb;
                    infectiousness += newinf->GetInfectiousness();
                    ReportInfectionState(); // can specify different reporting in derived classes
                }

                IInfectionHIV* pinfectionHIV = nullptr;
                if (s_OK == newinf->QueryInterface(GET_IID( IInfectionHIV ), (void**)&pinfectionHIV) )
                {
                    static_cast<Infection*>(newinf)->SetParameters(infstrain, incubation_period_override);

                    static_cast<Infection*>(newinf)->InitInfectionImmunology(susceptibility_hiv);

                    LOG_DEBUG( "Adding infection to infections list.\n" );
                    infections.push_front(newinf);
                    infection2susceptibilitymap[newinf] = susceptibility_hiv;
                    infectioncount_hiv++;
                    //infection2interventionsmap[newinf] = interventions_hiv;
                    //                
                    //infectiousness += newinf->GetInfectiousness(); //do not count the hiv for now 
                    //                ReportInfectionState(); // can specify different reporting in derived classes, going to need separate counting
                    if ( HasLatentInfection())
                    LifeCourseLatencyTimerUpdate(susceptibility_tb);              
                }
            }

            LOG_VALID_F( " Individual %lu acquired %d new infections \n", suid.data, newInfectionlist.size() );

            // Trigger new infection event observers
            //IIndividualTriggeredInterventionConsumer * pITIC = nullptr;
            //if (s_OK == GetInterventionsContext()->QueryInterface(GET_IID(IIndividualTriggeredInterventionConsumer), (void**)&pITIC) )
            //{
            //    pITIC->TriggerIndividualEventObservers( GetEventContext(), IndividualEventTriggerType::NewInfectionEvent );
            //}
        }
    }

    void IndividualHumanCoinfection::Set_forward_TB_act( std::vector<float> vin)
    {
        for (auto susceptibility : susceptibilitylist)
        {
            ISusceptibilityTB* pTBsus = nullptr;
            if (s_OK == susceptibility->QueryInterface(GET_IID( ISusceptibilityTB ), (void**)&pTBsus) && HasHIV())
            {
                SetTBactivationvector(vin);
                pTBsus->SetCD4ActFlag(true);
                break;
            }
        }
    }

    void IndividualHumanCoinfection::AcquireNewInfectionHIV(StrainIdentity *infstrain, int incubation_period_override )
    {
        LOG_DEBUG( "AcquireNewInfectionHIV\n" );
        //code is nearly duplicate to AcquireNewInfection but only gives new infection to HIV(the non-transmitting infection)
        //below I've commented out stuff that is actually specific to the TB infection

        //cumulativeInfs++; for now cumulativeInfs only counts TB infections
        //m_is_infected = true;

        //Now acquire the HIV infection, this is used for the OutbreakHIV
        //note, use the original createInfection (which does not take the newInfectionlist) for the HIV 
        //for each created infection, set the parameters and the initinfectionimmunology
        IInfection* newinf = createInfection( parent->GetNextInfectionSuid()  );


        IInfectionHIV* pinfectionHIV = nullptr;
        //this should always be true, just use for debugging
        if (s_OK == newinf->QueryInterface(GET_IID( IInfectionHIV ), (void**)&pinfectionHIV) )
        {
            newinf->SetParameters(infstrain, incubation_period_override);
            if (HasLatentInfection())
                newinf->InitInfectionImmunology(susceptibility_hiv);

            LOG_DEBUG( "Adding infection to infections list.\n" );
            infections.push_front(newinf);
            infection2susceptibilitymap[newinf] = susceptibility_hiv;
            infectioncount_hiv++;
        }    

        LOG_VALID_F( " Individual %lu acquired %d new infections \n", suid.data, newInfectionlist.size() );
    }

    void IndividualHumanCoinfection::Update(float currenttime, float dt)
    {
        float infection_timestep = dt;
        int numsteps = 1;

        // eventually need to correct for timestep in case of individuals moving among communities with different adapted time steps

        StateChange = HumanStateChange::None;

        //  Aging
        if (IndividualHumanConfig::aging) { m_age += dt; }

        // Adjust time step for infections as specified by infection_updates_per_tstep.  A value of 0 reverts to a single update per timestep for backward compatibility.
        // There is no special meaning of 1 being hourly.  For hourly infection updates with a tstep of one day, one must now specify 24.
        if (IndividualHumanConfig::infection_updates_per_tstep > 1 )
        {
            // infection_updates_per_tstep is now an integer > 1, so set numsteps equal to it,
            // allowing the subdivision dt into smaller infection_timestep
            numsteps = IndividualHumanConfig::infection_updates_per_tstep;
            infection_timestep = dt / numsteps;
        }

        // Process list of infections
        if (infections.size() == 0) // don't need to process infections or go hour by hour
        {
            for (auto *susceptibility : susceptibilitylist)
            {
                susceptibility->Update(dt); 
            }
            /*for (auto intervention : interventionslist)
            {    
            intervention->Update(dt); 
            }*/

            interventions->Update(dt);
        }
        else
        {
            for (int i = 0; i < numsteps; i++)
            {
                for (infection_list_t::iterator it = infections.begin(); it != infections.end();)
                {
                    auto infection = (*it);

                    IInfectionTB* pinfTB = nullptr;
                    if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
                    {    
                        LOG_DEBUG("This infection is not TB\n");
                    }
                    if (s_OK == infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
                    {    
                        LOG_DEBUG("This infection is  TB\n");
                    }

                    // Update infection (both HIV and TB)
                    infection->Update(infection_timestep, infection2susceptibilitymap[infection] );

                    // Check for a new infection/human state (e.g. Fatal, Cleared)
                    /*if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
                    {    
                    LOG_DEBUG("This infection is HIV, do not do the infection state stuff \n");
                    }*/

                    //if (s_OK == infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
                    //{    
                    //    LOG_DEBUG("This infection is  TB\n");

                    InfectionStateChange::_enum inf_state_change = infection->GetStateChange(); 
                    if (inf_state_change != InfectionStateChange::None && s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) ) 
                    {
                        LOG_DEBUG("InfectionStateChange due to HIV infection \n");
                    }
                    else if (inf_state_change != InfectionStateChange::None && s_OK == infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) ) 
                    {
                        LOG_DEBUG("InfectionStateChange due to TB infection \n");
                    }
                    if (inf_state_change != InfectionStateChange::None) 
                    {
                        //Need to adjust this code when HIV is also using the InfectionStateChange
                        IInfectionTB* pinfTB = nullptr;
                        if (s_OK == infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
                        {
                            SetNewInfectionState(inf_state_change);  //this is ONLY used for REPORTING! so only report for TB
                        }

                        // Notify susceptibility of cleared infection and remove from list
                        if ( inf_state_change == InfectionStateChange::Cleared ) 
                        {
                            if (IndividualHumanConfig::immunity) { static_cast<Susceptibility*>(infection2susceptibilitymap[infection])->UpdateInfectionCleared(); } //Immunity update: survived infection, note the notification of Susceptibility HIV contains only a placeholder now

                            IInfectionTB* pinfTB = nullptr;
                            if (s_OK == infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
                            {
                                infectioncount_tb--;
                                LOG_DEBUG("This deleted infection is TB\n");
                            }
                            IInfectionHIV* pinfHIV = nullptr;
                            if (s_OK == infection->QueryInterface(GET_IID( IInfectionHIV ), (void**)&pinfHIV) )
                            {
                                infectioncount_hiv--;
                                LOG_DEBUG("This deleted infection is HIV\n");
                            }

                            delete *it;
                            it = infections.erase(it); 

                            continue; 
                        }
                    }
                    // Set human state change and stop updating infections if the person has died
                    if ( inf_state_change == InfectionStateChange::Fatal ) 
                    {
                        StateChange = HumanStateChange::KilledByInfection; //this can only happen by TB, HIV deaths are handled as an "other non-disease mortality" so no HumanStateChange::KilledByCoinfection is called from here
                        break; 
                    }
                    //}
                    ++it;
                }

                if (IndividualHumanConfig::immunity) { 
                    for (auto susceptibility : susceptibilitylist)
                    {
                        susceptibility->Update(infection_timestep); 
                    } 
                }      // Immunity update: mainly decay of immunity
                if (StateChange == HumanStateChange::KilledByInfection) { break; } // If individual died, no need to keep simulating infections.
                interventions->Update(dt);
            }
        }

        applyNewInterventionEffects(dt);

        //  Get new infections
        ExposeToInfectivity(dt, &transmissionGroupMembership); // Need to do it even if infectivity==0, because of diseases in which immunity of acquisition depends on challenge (eg malaria)

        //  Is there an active infection for statistical purposes?
        //m_is_infected = (infections.size() > 0);
        m_is_infected = (infectioncount_tb > 0); //only count as infected if they have a tb infection (used for reportTB)

        if ( HasActiveInfection() && HasHIV() ) 
        {
            if ( randgen->e() < IndividualHumanCoinfectionConfig::Coinfected_mortality_rate)
            {
                LOG_DEBUG("I have TB and HIV and now I have another chance to die from my infection, using the probability of death when coinfected\n" );
                StateChange = HumanStateChange::KilledByInfection;
            }
        }


        if (StateChange == HumanStateChange::None && GET_CONFIGURABLE(SimulationConfig)->vital_dynamics) // Individual can't die if they're already dead
        {
            CheckVitalDynamics(currenttime, dt);
            CheckHIVVitalDynamics(dt);
        }

        if (StateChange == HumanStateChange::None && GET_CONFIGURABLE(SimulationConfig)->migration_structure) // Individual can't migrate if they're already dead
            CheckForMigration(currenttime, dt);
    }

    void IndividualHumanCoinfection::CheckHIVVitalDynamics( float dt)
    {
        if (GET_CONFIGURABLE(SimulationConfig)->enable_coinfection_mortality == true)
        {
            // "HIVMortalityDistribution" is added to map in Node::SetParameters if 'enable_coinfection_mortality' flag is set and you add the demographic file for HIV mortality
            //NOTE: HIV Mortality rates are amongst HIV pos people only!
            float hiv_mortality_rate = GetDemographicsDistribution(NodeDemographicsDistribution::HIVMortalityDistribution)->DrawResultValue(GetGender() == Gender::FEMALE, GetTime(), GetAge());

            if ( HasHIV() )
            {
                if(randgen->e() < hiv_mortality_rate * dt)
                {
                    LOG_DEBUG_F("died of HIV at age %f with hiv_mortality_rate = %f\n", GetAge() / DAYSPERYEAR, hiv_mortality_rate);
                    StateChange = HumanStateChange::KilledByCoinfection;
                }
            }
        }
    }

    void IndividualHumanCoinfection::Expose(const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route)
    {
        StrainIdentity strainIDs;

        strainIDs.SetAntigenID(cp->GetAntigenId()); // find antigenID of the strain to get infectivity from, is the individual already infected by the contagion of this antigen type?

        if (!InfectionExistsForThisStrain(&strainIDs)) // no existing infection of this antigenic type, so determine infection from exposure
        {
            //GHH temp changed to susceptibility_tb since this is for pools, which is specific for infectiousness, 
            //deal with HIV later (it has no strain tracking now anyways)

            if (randgen->e() < EXPCDF(-cp->GetTotalContagion()*dt*susceptibility_tb->getModAcquire()*interventions->GetInterventionReducedAcquire())) // infection results from this strain?
            {
                cp->ResolveInfectingStrain(&strainIDs); // get the substrain ID
                AcquireNewInfection(&strainIDs);
            }
            else
            {
                // immune response without infection
            }
        }
        else
        {
            // multiple infections of the same type happen here
            if (randgen->e() < EXPCDF(-cp->GetTotalContagion()*dt * susceptibility_tb->getModAcquire()*interventions->GetInterventionReducedAcquire())) // infection results from this strain?
            {
                cp->ResolveInfectingStrain(&strainIDs);
                AcquireNewInfection(&strainIDs); // superinfection of this antigenic type
            }
            else
            {
                // immune response without super infection
            }
        }
    }

    void IndividualHumanCoinfection::UpdateInfectiousness(float dt)
    {
        // Big simplification for now.  Just binary infectivity depending on latent/active state of infection
        infectiousness = 0;

        for (auto infection : infections)
        {
            IInfectionTB* pinfTB = nullptr;
            if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
            {
                LOG_DEBUG("This infection is not TB, do not add to total infectiousness in IndividualCoinfection (assume HIV is not infectious now) \n");
                continue;
            }
            infectiousness += infection->GetInfectiousness();
            float tmp_infectiousness =  m_mc_weight * infection->GetInfectiousness() * infection2susceptibilitymap[infection]->GetModTransmit() * interventions->GetInterventionReducedTransmit();
            StrainIdentity tmp_strainIDs;
            infection->GetInfectiousStrainID(&tmp_strainIDs);
            if( tmp_infectiousness )
            {
                parent->DepositFromIndividual(&tmp_strainIDs, tmp_infectiousness, &transmissionGroupMembership);
            }

            if(infectiousness > 0) break; // TODO: in IndividualTB we only count FIRST active infection in container, here we comment that out? reconsider only counting FIRST active infection in container
        }

        // Effects of transmission-reducing immunity/interventions.  Can set a maximum individual infectiousness here
        // TODO: if we want to actually truncate infectiousness at some maximum value, then QueueDepositContagion will have to be postponed as in IndividualVector
        if (infectiousness > 1)
        {
            infectiousness *= susceptibility_tb->GetModTransmit() * interventions->GetInterventionReducedTransmit();
        }
        else
        {
            infectiousness *= susceptibility_tb->GetModTransmit() * interventions->GetInterventionReducedTransmit();
        }
    }

    bool IndividualHumanCoinfection::SetNewInfectionState(InfectionStateChange::_enum inf_state_change)
    {
        if ( IndividualHuman::SetNewInfectionState(inf_state_change) )
        {
            // Nothing is currently set in the base function (death and disease clearance are handled directly in the Update function)
        }
        else if ( inf_state_change == InfectionStateChange::Cleared )
        {
            m_new_infection_state = NewInfectionState::NewlyCleared;                  //  Additional reporting of cleared infections
            LOG_VALID_F( " Individual %lu has infectionstatechange Cleared \n", suid.data);
        }
        else if ( inf_state_change == InfectionStateChange::TBActivation )   //  Latent infection that became active
        {
            m_new_infection_state = NewInfectionState::NewlyActive;
            broadcaster->TriggerNodeEventObservers(GetEventContext(), IndividualEventTriggerType::TBActivation);
            LOG_VALID_F( " Individual %lu has infectionstatechange TBActivation \n", suid.data);
        }
        else if ( inf_state_change == InfectionStateChange::TBInactivation ) //  Active infection that became latent
        {
            m_new_infection_state = NewInfectionState::NewlyInactive;
            LOG_VALID_F( " Individual %lu has infectionstatechange TBInactivation \n", suid.data);
            if ( HasHIV() )
                {
                    Mod_activate();
                }
        }
        else
        {
            return false;
        }

        return true;
    }

    bool IndividualHumanCoinfection::HasActiveInfection() const
    {
        bool ret = false;
        for (auto infection : infections)
        {
            IInfectionTB* pinfTB = nullptr;
            if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
            {
                LOG_DEBUG("This infection is not TB, so it cannot contribute to whether the individual has active infection \n");
                continue;
            }

            //must have tb infection to get down here
            if(infection->IsActive())
            {
                ret = true;
                break;
            }
        }
        LOG_DEBUG_F( "Individual %lu reporting %d for HasActiveInfection with %d infections.\n", suid.data, ret, infections.size() );
        return ret;
    }

    float IndividualHumanCoinfection::GetCD4TimeStep() const   /*Performance hit here would be nice to just cache the CD4 time step once */
    {
        float ret = 1.0f;
        
        for( auto sus: susceptibilitylist)
        {
            ISusceptibilityHIV* pSuscHIV = nullptr;
            if (s_OK != sus->QueryInterface(GET_IID( ISusceptibilityHIV ), (void**)&pSuscHIV) )
            {
                LOG_DEBUG("This susceptibility is not TB, we want HIV here\n");
                continue;
            }
            else
            {
                throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__ );
                //ret = pSuscHIV->GetCD4TimeStep();
                return ret;
            }
        }
        return ret;
    }

    int IndividualHumanCoinfection::GetNumCD4TimeSteps() const    /*Performance hit here would be nice to just cache the CD4 time step once */
    {
        int ret = 0;

        for( auto sus : susceptibilitylist)
        {
            ISusceptibilityHIV* pSuscHIV = nullptr;
            if (s_OK != sus->QueryInterface(GET_IID( ISusceptibilityHIV ), (void**)&pSuscHIV) )
            {
                LOG_DEBUG("This infection is not TB we want HIV here\n");
                continue;
            }
            else
            {
                throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__ );
                //ret = pSuscHIV->GetNumCD4TimeSteps();
            }
        }
        return ret;
    }

    bool IndividualHumanCoinfection::HasLatentInfection() const
    {
        LOG_DEBUG_F( "%s: infections.size() = %d.\n", __FUNCTION__, infections.size() );
        for (auto infection : infections)
        {
            IInfectionTB* pinfTB = nullptr;
            if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
            {
                LOG_DEBUG("This infection is not TB, so it cannot contribute to whether the individual has latent infection \n");
                continue;
            }

            //must have tb infection to get down here
            if(!infection->IsActive())
            {
                return true;
            }
        }
        return false;
    }

    bool IndividualHumanCoinfection::HasTB() const
    {
        LOG_DEBUG_F( "%s: infections.size() = %d.\n", __FUNCTION__, infections.size() );
        for (auto infection : infections)
        {
            IInfectionTB* pinfTB = nullptr;
            if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pinfTB) )
            {
                LOG_DEBUG("This infection is not TB, so it cannot contribute to whether the individual has latent infection \n");
                continue;
            }
            //must have tb infection to get down here
            return true;

        }
        return false;
    }

    bool IndividualHumanCoinfection::IsSmearPositive() const
    {
        for (auto infection : infections)
        {
            IInfectionTB* pointerITB = nullptr;
            if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pointerITB) )
            {
                LOG_DEBUG("This infection is not TB, so it cannot contribute to whether the individual has latent infection \n");
                continue;
            }

            if(pointerITB->IsSmearPositive())
            {
                return true;
            }
        }
        return false;
    }

    bool IndividualHumanCoinfection::IsMDR() const
    {
        for (auto infection : infections)
        {
            IInfectionTB* pointerITB = nullptr;
            if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pointerITB) )
            {
                LOG_DEBUG("This infection is not TB, so it cannot contribute to whether the individual has latent infection \n");
                continue;
            }

            if(pointerITB->IsMDR()) 
            {
                return true;
            }
        }
        return false;
    }

    bool IndividualHumanCoinfection::HasExtrapulmonaryInfection() const
    {
        for (auto infection : infections)
        {
            IInfectionTB* pointerITB = nullptr;
            if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pointerITB) )
            {
                LOG_DEBUG("This infection is not TB, so it cannot contribute to whether the individual has latent infection \n");
                continue;
            }

            if(pointerITB->IsExtrapulmonary()) 
            {
                return true;
            }
        }
        return false;
    }

    bool IndividualHumanCoinfection::HasActivePresymptomaticInfection() const
    {
        for (auto infection : infections)
        {
            IInfectionTB* pointerITB = nullptr;
            if (s_OK != infection->QueryInterface(GET_IID( IInfectionTB ), (void**)&pointerITB) )
            {
                LOG_DEBUG("This infection is not TB, so it cannot contribute to whether the individual has latent infection \n");
                continue;
            }

            if(!pointerITB->IsSymptomatic() && pointerITB->IsActive() ) 
            {
                return true;
            }
        }
        return false;
    }

    int IndividualHumanCoinfection::GetTime() const
    {
        return parent->GetTime().time;
    }

    bool IndividualHumanCoinfection::HasHIV() const
    {
        bool ret = false;
        for (auto infection : infections)
        {
            IInfectionHIV* pinfHIV = nullptr;
            if (s_OK == infection->QueryInterface(GET_IID( IInfectionHIV ), (void**)&pinfHIV) )
            {
                ret = true;
                break;
            }
        }
        return ret;
    }

    float IndividualHumanCoinfection::GetCD4() const
    {// return 1000 if healthy (this could be improved by introducing variability in CD4 among healthy individuals)
        float ret = 1000.00;
        for (auto susceptibility : susceptibilitylist)
        {
            ISusceptibilityHIV* pointer_to_HIV_susceptibility = nullptr;
            if (s_OK == susceptibility->QueryInterface(GET_IID( ISusceptibilityHIV ), (void**)&pointer_to_HIV_susceptibility) )
            {
                ret = pointer_to_HIV_susceptibility->GetCD4count();
                break;
            }
        }
        return ret;
    }

    void IndividualHumanCoinfection::InitiateART()
    {
        /* clorton float current_CD4 = */ GetCD4();
        for (auto susceptibility : susceptibilitylist)
        {
            ISusceptibilityHIV* pointer_to_HIV_susceptibility = nullptr;
            if (s_OK == susceptibility->QueryInterface(GET_IID( ISusceptibilityHIV ), (void**)&pointer_to_HIV_susceptibility) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "susceptibility", "ISusceptibilityHIV", "Susceptibility" );
            }

            m_is_on_ART = 1;
            m_has_ever_been_onART = 1;
        }
    }

    NaturalNumber IndividualHumanCoinfection::GetViralLoad() const
    {
        NaturalNumber ret = 0;
        for (auto infection : infections)
        {
            IInfectionHIV* pinfHIV = nullptr;
            if (s_OK == infection->QueryInterface(GET_IID( IInfectionHIV ), (void**)&pinfHIV) )
            {
                int vl = pinfHIV->GetViralLoad();
                ret += vl;
            }
        }
        return ret;
    }

    bool IndividualHumanCoinfection::IsImmune() const
    {
        //this is ONLY used by ReportTB. 
        //People get IsImmune when they get an infection, then with some period of infection-less time, they may lose their immunity

        return susceptibility_tb->IsImmune();
    }

    IInfection* IndividualHumanCoinfection::createInfection( suids::suid _suid )
    {
        InfectionHIV* new_inf = InfectionHIV::CreateInfection(this, _suid);
        return static_cast<IInfection*>(new_inf);
    }

    bool IndividualHumanCoinfection::createInfection( suids::suid _suid, infection_list_t &newInfections )
    {
        InfectionTB* new_inf = InfectionTB::CreateInfection(this, _suid);
        newInfections.push_back( new_inf );
        //30% chance of  getting HIV
        if ( randgen->e() < IndividualHumanCoinfectionConfig::HIV_coinfection_probability)
        {
            InfectionHIV* new_inf2 = InfectionHIV::CreateInfection(this, _suid);
            newInfections.push_back( new_inf2 );
        }
        return true;
    }

    IIndividualHumanInterventionsContext* IndividualHumanCoinfection::GetInterventionsContextbyInfection(Infection* infection) 
    {
        return static_cast<IIndividualHumanInterventionsContext*>(interventions);
    }

    void IndividualHumanCoinfection::setupInterventionsContainer()
    {
        /*interventions_tb = _new_ TBInterventionsContainer();
        interventions_hiv = _new_ HIVInterventionsContainer();
        interventionslist.push_back (interventions_tb);
        interventionslist.push_back (interventions_hiv) ;
        */
        interventions = _new_ MasterInterventionsContainer();
        //IIndividualHumanContext *indcontext = GetContextPointer();
        interventions->SetContextTo(this); //TODO: fix this when init pattern standardized <ERAD-291>  PE: See comment above
    }

    void IndividualHumanCoinfection::Mod_activate()  
    {
        for (auto susceptibility : susceptibilitylist)
        {
            ISusceptibilityHIV* pHIVsus = nullptr;
            if (s_OK == susceptibility->QueryInterface(GET_IID( ISusceptibilityHIV ), (void**)&pHIVsus) )
            {
                pHIVsus->Generate_forward_CD4();

                if (this->HasHIV() )
                {
                    /* clorton float temp_CD4 = */ this->GetCD4();
                    vector <float> CD4_future = CD4_forward_vector;

                    std::vector <float> v_act(CD4_future.size(), 0.0f);

                    std::vector<float>::iterator vin;
                    std::vector<float>::iterator vout;

                    for (vin = CD4_future.begin(), vout = v_act.begin() ; vin != CD4_future.end(); ++vin, ++vout)  //could have  vout! v_act.end() but redundant due to definition above
                    {
                        auto temp = IndividualHumanCoinfectionConfig::CD4_act_map.lower_bound(*vin);
                        *vout = temp->second;
                    }

                    Set_forward_TB_act(v_act);
                }
                else
                {
                    Set_forward_TB_act (std::vector <float>(1.0f) );  //vector of one element equal to one for consistency
                } 
            }
        }
    }

    void IndividualHumanCoinfection::Coinf_Generate_forward_CD4()
    {
        ISusceptibilityHIV* psusHIV = nullptr;

        for (auto susceptibility : susceptibilitylist)
        {
            if( susceptibility->QueryInterface( GET_IID( ISusceptibilityHIV ), (void**)&psusHIV ) == s_OK )
            {
                psusHIV->Generate_forward_CD4();
            }
        }
    }

void IndividualHumanCoinfection::LifeCourseLatencyTimerUpdate( Susceptibility* immunity)
    {
          ISusceptibilityTB* immunityTB = nullptr;
        if( immunity->QueryInterface( GET_IID( ISusceptibilityTB ), (void**)&immunityTB ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "immunity", "Susceptibility", "SusceptibilityTB" );
        }

        Mod_activate();

        for (auto infectionTB : infections)
        {
            IInfectionTB* pTBinf = nullptr;
            if (s_OK == infectionTB->QueryInterface(GET_IID( IInfectionTB ), (void**)&pTBinf) )
            { 
                float temp_latent_cure_rate = pTBinf->GetLatentCureRate();
                float temp_incubation_timer = randgen->time_varying_rate_dist( GetTBactivationvector(), GetCD4TimeStep(), temp_latent_cure_rate);

#ifdef TEST_RNG

                ofstream ofile;

                ofile.open("test.txt");

                for(int ii = 1; ii< 1000; ii++)
                {
                    float temp_incubation_timer = randgen->time_varying_rate_dist( GetTBactivationvector(), GetCD4TimeStep(), 0.0f);
                    ofile << temp_incubation_timer;
                    ofile << "\n";
                }  
                ofile.close();

#endif 
                pTBinf ->SetIncubationTimer(temp_incubation_timer);
                float total_rate = GetNextLatentActivation(temp_incubation_timer) + temp_latent_cure_rate;
                float temp_recover_fraction = (total_rate > 0 ?  temp_latent_cure_rate/total_rate : 0);
                pTBinf->ResetRecoverFraction(temp_recover_fraction);
                pTBinf->ResetDuration();
            }
        }
}

    void
    IndividualHumanCoinfection::SetTBactivationvector(
        const std::vector<float>& vin
    )
    {
        TB_activation_vector = vin;
    }

    const std::vector<float>&
    IndividualHumanCoinfection::GetTBactivationvector()
    const 
    {
        return TB_activation_vector;
    }

    float
    IndividualHumanCoinfection::GetNextLatentActivation(
        float time_incr
    )
    const
    {
        vector <float> forward_act = GetTBactivationvector();
        int v_size = forward_act.size();

        if (int(time_incr)/GetCD4TimeStep() > v_size)
        {
            return forward_act.back();
        }
        else
        {
            return forward_act.at(int(time_incr/GetCD4TimeStep()));
        }
    }
}
