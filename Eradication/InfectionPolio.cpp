/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "InfectionPolio.h"
#include "SusceptibilityPolio.h"
#include "InterventionsContainer.h"
#include "PolioDefs.h"
#include "Environment.h"
#include "Debug.h"

#include "Common.h"
#include "MathFunctions.h"
#include "RANDOM.h"
#include "Exceptions.h"
#include "SimulationConfig.h"
using namespace std;

static const char* _module = "InfectionPolio";

namespace Kernel
{
    GET_SCHEMA_STATIC_WRAPPER_IMPL(Polio.Infection,InfectionPolioConfig)
    BEGIN_QUERY_INTERFACE_BODY(InfectionPolioConfig)
    END_QUERY_INTERFACE_BODY(InfectionPolioConfig)

    bool           InfectionPolioConfig::tracecontact_mode = 0.0f;          // flag for genome mutation algorithm for serial infection tracing
    int            InfectionPolioConfig::default_antigen = 0.0f;            // default infection antigenID
    int            InfectionPolioConfig::default_genome = 0.0f;             // default infection genomeID
    float          InfectionPolioConfig::paralytic_case_mortality = 0.0f;   // (dimensionless) fraction of paralytic cases resulting in death
    float          InfectionPolioConfig::evolutionWPVLinearRate = 0.0f;     // (bits / day) mutation rate in the model genome
    float          InfectionPolioConfig::evolutionSabinLinearRate[N_POLIO_SEROTYPES];   // (bits / day) maximum mutation rate in model genome, Sabin reversion
    float          InfectionPolioConfig::evolutionImmuneRate = 0.0f;        // (bits / day-log2(antibody)) mutation rate factor for log2(neutralizing antibody)
    float          InfectionPolioConfig::evolutionHalfmaxReversion = 0.0f;  // (reversion degree) point of reversion at which the reversion rate declines to half maximum rate

    bool
    InfectionPolioConfig::Configure(
        const Configuration * config
    )
    {
//        LOG_DEBUG( "Configure\n" );
        initConfigTypeMap( "Enable_Contact_Tracing", &tracecontact_mode, Enable_Contact_Tracing_DESC_TEXT, false ); // polio
        initConfigTypeMap( "Default_Antigen", &default_antigen, Default_Antigen_DESC_TEXT, 0, 5, 0 ); // polio
        initConfigTypeMap( "Default_Genome", &default_genome, Default_Genome_DESC_TEXT, 0, 1000, 0 ); // polio
        initConfigTypeMap( "Paralytic_Case_Mortality", &paralytic_case_mortality, Paralytic_Case_Mortality_DESC_TEXT, 0.0f, 1.0f, 0.05f ); // polio
        initConfigTypeMap( "Evolution_Polio_WPV_Linear_Rate", &evolutionWPVLinearRate, Evolution_Polio_WPV_Linear_Rate_DESC_TEXT, 0.0f, 1.0f, 0.0219f ); // polio
        initConfigTypeMap( "Evolution_Polio_Sabin1_Linear_Rate", &evolutionSabinLinearRate[0], Evolution_Polio_Sabin1_Linear_Rate_DESC_TEXT, 0.0f, 1.0f, 0.022f ); // polio
        initConfigTypeMap( "Evolution_Polio_Sabin2_Linear_Rate", &evolutionSabinLinearRate[1], Evolution_Polio_Sabin2_Linear_Rate_DESC_TEXT, 0.0f, 1.0f, 0.022f ); // polio
        initConfigTypeMap( "Evolution_Polio_Sabin3_Linear_Rate", &evolutionSabinLinearRate[2], Evolution_Polio_Sabin3_Linear_Rate_DESC_TEXT, 0.0f, 1.0f, 0.022f ); // polio
        initConfigTypeMap( "Evolution_Polio_Immune_Rate", &evolutionImmuneRate, Evolution_Polio_Immune_Rate_DESC_TEXT, 0.0f, 1.0f, 0.0f ); // polio
        initConfigTypeMap( "Evolution_Polio_Halfmax_Reversion", &evolutionHalfmaxReversion, Evolution_Polio_Halfmax_Reversion_DESC_TEXT, 0.0f, 1.0f, 0.5f ); // polio
        bool bRet = JsonConfigurable::Configure( config );
#if !defined(_DLLS_)
        if (!JsonConfigurable::_dryrun && !(default_genome < GET_CONFIGURABLE(SimulationConfig)->number_substrains))
        {
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "default_genome", default_genome, "GET_CONFIGURABLE(SimulationConfig)->number_substrains-1", GET_CONFIGURABLE(SimulationConfig)->number_substrains-1 );
        }
#endif
        return bRet;
    }

    BEGIN_QUERY_INTERFACE_BODY(InfectionPolio)
        HANDLE_INTERFACE(IInfectionPolioReportable) 
    END_QUERY_INTERFACE_BODY(InfectionPolio)

    InfectionPolio::InfectionPolio()
    {
    }

    const SimulationConfig*
    InfectionPolio::params()
    {
        return GET_CONFIGURABLE(SimulationConfig);
    }

    InfectionPolio::InfectionPolio(IIndividualHumanContext *context) : InfectionEnvironmental(context)
    {
        duration = 0.0; // time counter of infection
        immunity_updated = 0;
        paralysis_reported = 0;
        paralysis_time = -1.0f;
        host_mc_weight = 0.0f;
        peakFecalLog10VirusTiter = 0.0f;
        peakOralLog10VirusTiter = 0.0f;
        durationFecalInfection = 0.0f;
        durationOralInfection = 0.0f;
        shed_genome_traceContactMode = -1; // invalid value (-1) indicates the shed genome has not been set
        paralysis_probability = -1.0f;
        drug_titer_reduction = 0.0f;
        drug_infection_duration_reduction = 0.0f;

        drug_flag = false;
    }

    void InfectionPolio::Initialize(suids::suid _suid)
    {
        InfectionEnvironmental::Initialize(_suid);
    }

    InfectionPolio *InfectionPolio::CreateInfection(IIndividualHumanContext *context, suids::suid _suid)
    {
        //VALIDATE(boost::format(">InfPolio::CreateInfection(%1%, %2%)") % context->GetSuid().data % _suid.data );

        InfectionPolio *newinfection = _new_ InfectionPolio(context);
        newinfection->Initialize(_suid);

        return newinfection;
    }

    InfectionPolio::~InfectionPolio()
    {
    }

    REGISTER_SERIALIZABLE(InfectionPolio, IInfection);

    void InfectionPolio::serialize(IArchive& ar, IInfection* obj)
    {
        InfectionEnvironmental::serialize(ar, obj);
        InfectionPolio& infection = *dynamic_cast<InfectionPolio*>(obj);
        ar.startElement();
            ar.labelElement("shed_genome_traceContactMode") & infection.shed_genome_traceContactMode;
            ar.labelElement("immunity_updated") & infection.immunity_updated;
            ar.labelElement("paralysis_reported") & infection.paralysis_reported;
            ar.labelElement("paralysis_time") & infection.paralysis_time;
            ar.labelElement("initial_infectiousness") & infection.initial_infectiousness;
            ar.labelElement("cached_mucosal_immunity") & infection.cached_mucosal_immunity;
            ar.labelElement("cached_humoral_immunity") & infection.cached_humoral_immunity;
            ar.labelElement("host_mc_weight") & infection.host_mc_weight;
            ar.labelElement("peakFecalLog10VirusTiter") & infection.peakFecalLog10VirusTiter;
            ar.labelElement("peakOralLog10VirusTiter") & infection.peakOralLog10VirusTiter;
            ar.labelElement("durationFecalInfection") & infection.durationFecalInfection;
            ar.labelElement("durationOralInfection") & infection.durationOralInfection;
            ar.labelElement("paralysis_probability") & infection.paralysis_probability;
            ar.labelElement("drug_titer_reduction") & infection.drug_titer_reduction;
            ar.labelElement("drug_infection_duration_reduction") & infection.drug_infection_duration_reduction;
            ar.labelElement("drug_flag") & infection.drug_flag;
        ar.endElement();
    }
}

void Kernel::InfectionPolio::SetParameters(StrainIdentity* infstrain, int incubation_period_override)
{
    InfectionEnvironmental::SetParameters(infstrain, incubation_period_override); // setup infection timers and infection state
    if(infstrain == nullptr)
    {
        // using default strainIDs
        infection_strain->SetAntigenID(default_antigen);
        infection_strain->SetGeneticID(default_genome);
    }
    else
    {
        *infection_strain = *infstrain;
    }
}

void Kernel::InfectionPolio::InitInfectionImmunology(Susceptibility* _immunity)
{
    ISusceptibilityPolio* immunity = nullptr;
    if( _immunity->QueryInterface( GET_IID( ISusceptibilityPolio ), (void**)&immunity ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "_immunity", "ISusceptibilityPolio", "Susceptibility" );
    }
    immunity->IncrementInfection(infection_strain);

    int serotype = immunity->GetSerotype(infection_strain);
    immunity->ResetTimeSinceLastInfection(serotype);

    immunity->SetNewInfectionByStrain(infection_strain);

    peakFecalLog10VirusTiter    = immunity->GetPeakFecalLog10VirusTiter(infection_strain);
    peakOralLog10VirusTiter        =  immunity->GetPeakOralLog10VirusTiter(infection_strain);
    durationFecalInfection        = immunity->GetFecalInfectiousDuration(infection_strain);
    durationOralInfection        =  immunity->GetOralInfectiousDuration(infection_strain);

    if(durationFecalInfection > durationOralInfection)
        infectious_timer = durationFecalInfection;
    else
        infectious_timer = durationOralInfection;
    LOG_DEBUG_F( "infectious_timer initialized to %f\n", infectious_timer );
    
    total_duration = incubation_timer + infectious_timer;
    StateChange = InfectionStateChange::New;

    cached_mucosal_immunity = immunity->GetMucosalImmunity(infection_strain);
    //cached_humoral_immunity = immunity->GetHumoralImmunity(infection_strain);
}

void Kernel::InfectionPolio::Update(float dt, ISusceptibilityContext* _immunity)
{
    StateChange = InfectionStateChange::None;
    ISusceptibilityPolio* immunity = nullptr;
    if( _immunity->QueryInterface( GET_IID( ISusceptibilityPolio ), (void**)&immunity ) != s_OK )
    {
        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "_immunity", "Susceptibility", "SusceptibilityPolio" );
    }

    // Update cached variables from immunity for InfectionPolioReportable
    //cached_mucosal_immunity = immunity->GetMucosalImmunity(infection_strain);
    //cached_humoral_immunity = immunity->GetHumoralImmunity(infection_strain);

    duration += dt;

    // Reset drug effects each time step
    drug_titer_reduction = 0.0f;
    drug_infection_duration_reduction = 0.0f;

    // Query for drug present effects
    IIndividualHumanContext *patient = GetParent();
    IIndividualHumanInterventionsContext *context = patient->GetInterventionsContext();
    IPolioDrugEffects *ipde = nullptr;

    if (s_OK ==  context->QueryInterface(GET_IID(IPolioDrugEffects), (void **)&ipde))
    {
        drug_titer_reduction                = ipde->get_titer_efficacy();
        drug_infection_duration_reduction   = ipde->get_infection_duration_efficacy();

        if(drug_titer_reduction > 0.99f)
            drug_flag = true;

        if(drug_flag && drug_titer_reduction < 0.99f)
        {
            LOG_DEBUG_F("ErrorEfficacy - infTime=%f,   InfectionDrug:  kTiter= %f,  kDuration= %f\n", duration, drug_titer_reduction, drug_infection_duration_reduction);
            drug_flag = false;
        }
    }


    float infection_end_time = incubation_timer + (pow(1.0f + infectious_timer, 1.0f - drug_infection_duration_reduction) - 1.0f); // multiplication by 1-duration_reduction is in log time
    LOG_DEBUG_F( "calc infection_end_time from incubation_timer(%f), infectious_timer(%f), and drug_infection_duration_reduction(%f)\n",
                 incubation_timer,
                 infectious_timer,
                 drug_infection_duration_reduction );
    
    // assign and change infectiousness for both WPV and VRPV
    if(duration >= incubation_timer)
    {
        release_assert(infection_strain);
        assert(immunity);
        float tmp_relative_infectivity;

        if( IS_WILD_TYPE( infection_strain->GetAntigenID() ) )
        {
            tmp_relative_infectivity = 1.0f;
        }
        else // vaccine-related
        {
            tmp_relative_infectivity = params()->substrainRelativeInfectivity[immunity->GetSerotype(infection_strain)][infection_strain->GetGeneticID()];
        }

        // viral excretion units are TCID50/day
        setCurrentInfectivity(tmp_relative_infectivity, duration-incubation_timer, duration-incubation_timer);

        // We only want to do this ONCE for each Infection.
        if( paralysis_probability == -1.0f && infectiousness )
        {
            paralysis_probability = -0.5f;
            paralysis_time = immunity->GetParalysisTime(infection_strain, total_duration);
        }
    }
    
    // Check for paralysis after we (possibly) set a new value for paralysis_time.
    if(paralysis_time > 0 && duration > paralysis_time) // infection has passed the date of paralysis. report the paralytic case.
    {
        switch (infection_strain->GetAntigenID())
        {
            case PolioVirusTypes::WPV1:  
                StateChange = InfectionStateChange::PolioParalysis_WPV1;
                break;

            case PolioVirusTypes::WPV2:  
                StateChange = InfectionStateChange::PolioParalysis_WPV2;
                break;

            case PolioVirusTypes::WPV3:  
                StateChange = InfectionStateChange::PolioParalysis_WPV3;
                break;

            case PolioVirusTypes::VRPV1:  
                StateChange = InfectionStateChange::PolioParalysis_VDPV1;
                break;

            case PolioVirusTypes::VRPV2:  
                StateChange = InfectionStateChange::PolioParalysis_VDPV2;
                break;

            case PolioVirusTypes::VRPV3:  
                StateChange = InfectionStateChange::PolioParalysis_VDPV3;
                break;

            default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "infection_strain->GetAntigenID()", infection_strain->GetAntigenID(), std::to_string(infection_strain->GetAntigenID()).c_str());
        }
        paralysis_time = -1;
        paralysis_reported = 1;
    } // skip infection clearance and mortality until the next time step to make sure paralysis is reported, otherwise it would under-report paralysis at the maximum incubation time
    else if(duration > infection_end_time)
    {
        LOG_DEBUG_F( "duration = %f, infection_end_time = %f\n", duration, infection_end_time );
        if(paralysis_reported && params()->vital_disease_mortality)
        {
            // To query for mortality-reducing effects of drugs or vaccines
            IDrugVaccineInterventionEffects* idvie = nullptr;
            if ( s_OK != parent->GetInterventionsContext()->QueryInterface(GET_IID(IDrugVaccineInterventionEffects), (void**)&idvie) )
            {
                throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "parent->GetInterventionsContext()", "IDrugVaccineInterventionEffects", "IIndividualHumanEventContext" );
            }

            if( ( paralytic_case_mortality * idvie->GetInterventionReducedMortality() ) &&
                randgen->e() < paralytic_case_mortality * idvie->GetInterventionReducedMortality() )
            {
                StateChange = InfectionStateChange::Fatal;
            }
            else
            {
                StateChange = InfectionStateChange::Cleared;
            }
        }
        else // 1, individual has killed infection, 2 infection has killed individual, 3 for polio paralysis
        {
            StateChange = InfectionStateChange::Cleared;
        }
    }

    // Do this at the end. If we updated immunity at beginning of Update, paralysis calc is wrong (too unlikely).
    if(immunity_updated == 0 && duration > 0) // active immune response
    {
        immunity->ApplyImmumeResponseToInfection(infection_strain);
        immunity_updated = 1;
    }

    // only reduce the infection counter if the individual has survived an infection.
    // Death will remove all infections along with the individual before the next time step.
    if (StateChange == InfectionStateChange::Cleared)
    {
        immunity->DecrementInfection(infection_strain);
    }

    evolveStrain(immunity, dt); // genetic modifications
}

void Kernel::InfectionPolio::evolveStrain(ISusceptibilityPolio* immunity, float dt)
{
    if(params()->number_substrains > 1)
    {
        int serotype = infection_strain->GetAntigenID();
        if(serotype > PolioVirusTypes::WPV3) {serotype -= PolioVirusTypes::VRPV1;}

        uint16_t bin_genome = uint16_t(infection_strain->GetGeneticID());
        uint16_t vdpv_state = 0;

        if(params()->reversionSteps_cVDPV[serotype] > 0)
        {
            vdpv_state = uint16_t(1) << uint16_t(params()->reversionSteps_cVDPV[serotype] - 1);
        } // minimum genome value for fully reverted VDPV
        
        float prob_mutation;
        if( IS_WILD_TYPE( infection_strain->GetAntigenID() ) )
        {
            prob_mutation = evolutionWPVLinearRate * dt; // wild type only uses linear mutation
        }
        else // VRPV (vaccine-related poliovirus)
        {
            switch(GET_CONFIGURABLE(SimulationConfig)->evolution_polio_clock_type)
            {
                case EvolutionPolioClockType::POLIO_EVOCLOCK_NONE:
                    prob_mutation  = 0.0f;
                    break;

                case EvolutionPolioClockType::POLIO_EVOCLOCK_LINEAR:
                    prob_mutation = dt * evolutionSabinLinearRate[serotype];
                    break;

                case EvolutionPolioClockType::POLIO_EVOCLOCK_IMMUNITY:
                    prob_mutation = float((dt * evolutionSabinLinearRate[serotype]) + (dt * evolutionImmuneRate * log(immunity->GetMucosalImmunity(infection_strain)) / LOG_2));
                    break;

                case EvolutionPolioClockType::POLIO_EVOCLOCK_REVERSION_AND_IMMUNITY:
                    prob_mutation = float((evolutionHalfmaxReversion / (immunity->GetReversionDegree(infection_strain) + evolutionHalfmaxReversion)) * ((dt * evolutionSabinLinearRate[serotype]) + (dt * evolutionImmuneRate * log(immunity->GetMucosalImmunity(infection_strain)) / LOG_2)));
                    break;

                case EvolutionPolioClockType::POLIO_EVOCLOCK_REVERSION:
                    prob_mutation = (evolutionHalfmaxReversion / (immunity->GetReversionDegree(infection_strain) + evolutionHalfmaxReversion)) * (dt * evolutionSabinLinearRate[serotype]);
                    break;

                case EvolutionPolioClockType::POLIO_EVOCLOCK_POISSONSITES:
                    {
                        // independent Poisson rates for each genetic site
                        prob_mutation = -1.0f; // do not execute any of the other evolution models that use a single mutation process
                        vector<float> site_rates;

                        switch(infection_strain->GetAntigenID())
                        {
                            case PolioVirusTypes::VRPV1:
                                site_rates = params()->Sabin1_Site_Rates;
                                break;

                            case PolioVirusTypes::VRPV2:
                                site_rates = params()->Sabin2_Site_Rates;
                                break;

                            case PolioVirusTypes::VRPV3:
                                site_rates = params()->Sabin3_Site_Rates;
                                break;
                        }

                        for(int i_site = 0; i_site < site_rates.size(); i_site++)
                        {
                            uint16_t mutate_bit;
                            if(site_rates[i_site] && randgen->e() < (dt * site_rates[i_site]))
                            {
                                mutate_bit = 1;
                                mutate_bit = mutate_bit << i_site;
                                bin_genome = bin_genome | mutate_bit; // irreversible reversion of genetic site at bit position = i_site
                            }
                        }
                        break;
                    }
                default:
                    throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "evolution_polio_clock_type", GET_CONFIGURABLE(SimulationConfig)->evolution_polio_clock_type, "(n/a)" ); // ConfigurationRangeException better?
            }
        }

        if(prob_mutation>0 && randgen->e() < prob_mutation)
        {
            if( IS_WILD_TYPE( infection_strain->GetAntigenID() ) ) // WPV, use random bit mutation
            {
                uint16_t mutate_bit = 1;
                mutate_bit = mutate_bit << uint16_t(floor(0.99999f * randgen->e() * log(float(params()->number_substrains)) / LOG_2));
                bin_genome = bin_genome ^ mutate_bit; // apply point mutation in random position
            }
            else if(bin_genome >= vdpv_state) // cVDPV, use random bit mutation
            {
                uint16_t mutate_bit = 1;
                mutate_bit = mutate_bit << uint16_t(floor(0.99999f * randgen->e() * (log(float(params()->number_substrains)) / LOG_2 - 1)));
                bin_genome = bin_genome ^ mutate_bit; // apply point mutation in random position
                bin_genome = bin_genome | (uint16_t(1) << uint16_t(floor(0.99999f *log(float(params()->number_substrains)) / LOG_2))); // set MSB to signify full reversion
            }
            else // not yet cVDPV, use bit push mutation
            {
                if(bin_genome == 0) {bin_genome = 1;} // first reverting mutation sets reversion marker bit
                else
                {
                    bin_genome = bin_genome << 1;
                    bin_genome += (randgen->e() < 0.5);
                }
            }
            infection_strain->SetGeneticID(int(bin_genome)); // new genome selected under host pressures, now ready to compete for new hosts in the community
        }
        // override mutation results in trace contact mode
        if(tracecontact_mode)
        {
            if(shed_genome_traceContactMode < 0)
            {
                shed_genome_traceContactMode = parent->GetNextInfectionSuid().data;
                if(shed_genome_traceContactMode > (params()->number_substrains - 1)) {shed_genome_traceContactMode = params()->number_substrains - 1;}
            }
            infection_strain->SetGeneticID(shed_genome_traceContactMode);
        }
    }
}

float Kernel::InfectionPolio::getCurrentTiterFromProfile(float peak_Log10Titer, float infectiousTime, float mu, float sigma)
{
    // normalize the profile function to have a maximum of 1
    // the mode of the lognormal PDF is x' = exp(mu - sigma^2);  fmax = lognpdf(x', mu, sigma)
    /*
        float f_max            = exp(0.5f * pow(sigma, 2.0f) - mu) / (sqrt(2.0f * 3.141593f) * sigma);
        float titer_f_time    = exp(-pow( (log(infectiousTime) - mu) / sigma, 2.0f) / 2.0f) / (infectiousTime * sigma * sqrt(2.0f * 3.141593f));
        peakTiter * titer_f_time / f_max;
    */

    // Drug effects
    peak_Log10Titer *= 1.0f - drug_titer_reduction;

    // determine present time point titer from individuals titer profile
    infectiousTime += 1.0; // make sure time > 0
    float log10_titer = peak_Log10Titer * exp(-0.5f * pow( (log(infectiousTime) - mu) / sigma, 2.0f )) / (infectiousTime * exp(0.5f * sigma * sigma - mu));
    return pow(10.0f, log10_titer) - 1.0f;
}

//   keeping shedding titer calculations inside of infection. Call susceptibility functions only for calculations directly using immunity variables
void Kernel::InfectionPolio::setCurrentInfectivity(float relative_infectivity, float infectionTimeFecal, float InfectionTimeOral)
{
    // Fecal route is being labeled "environmental"
    // Oral route is being labeled "contact"

    float titerFecal = 0.0f;
    float titerOral = 0.0f;

    if(infectionTimeFecal <= durationFecalInfection)
    {
        titerFecal += getCurrentTiterFromProfile(peakFecalLog10VirusTiter, infectionTimeFecal, GET_CONFIGURABLE(SimulationConfig)->shedFecalTiterProfile_mu, GET_CONFIGURABLE(SimulationConfig)->shedFecalTiterProfile_sigma);
    }

    if( InfectionTimeOral <=  durationOralInfection)
    {
        titerOral += getCurrentTiterFromProfile( peakOralLog10VirusTiter,  InfectionTimeOral,  GET_CONFIGURABLE(SimulationConfig)->shedOralTiterProfile_mu,  GET_CONFIGURABLE(SimulationConfig)->shedOralTiterProfile_sigma);
    }

    titerFecal *= GET_CONFIGURABLE(SimulationConfig)->excrement_load;
    titerOral *= GET_CONFIGURABLE(SimulationConfig)->excrement_load;

    // HINT off, fecal only
    if (GET_CONFIGURABLE(SimulationConfig)->heterogeneous_intranode_transmission_enabled == false)
    {
        titerOral = 0.0f;
    }

    titerFecal *= relative_infectivity;
    titerOral *= relative_infectivity;

    infectiousnessByRoute["environmental"] = titerFecal;
    infectiousnessByRoute["contact"]  = titerOral;

    infectiousness = titerFecal + titerOral;
    LOG_DEBUG_F("Titer fecal = %f, oral = %f.\n", titerFecal, titerOral);
}

void Kernel::InfectionPolio::CacheMCWeightOfHost(float ind_mc_weight)
{
    host_mc_weight = ind_mc_weight;
}

float Kernel::InfectionPolio::GetMusocalImmunity() const
{
    LOG_DEBUG_F( "Individual %lu returning %f for (cached) mucosal immunity.\n", parent->GetSuid().data, cached_mucosal_immunity );
    return cached_mucosal_immunity;
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::InfectionPolio)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, InfectionPolio& inf, const unsigned int file_version )
    {
        ar & inf.shed_genome_traceContactMode; // genomeID that is shed for contact tracing purposes
        ar & inf.immunity_updated; // takes values 0,1
        ar & inf.paralysis_reported; // takes values 0,1
        ar & inf.paralysis_time; // delay from infection to paralysis. individual will be paralyzed if this value is greater than 0
        ar & inf.initial_infectiousness; // base value of infectiousness based on antibody titer before infection
        ar & inf.host_mc_weight; // (days) stores the MC weight of the individual, use for reporting infections only
        ar & inf.peakFecalLog10VirusTiter; // (TCID50 at the base infectivity pinf of the virus type) the true TCID50 is found by multiplying by relative infectivity for the genome being shed
        ar & inf.peakOralLog10VirusTiter; //
        ar & inf.durationFecalInfection; //
        ar & inf.durationOralInfection; //
        ar & inf.paralysis_probability; // Probability of paralysis. This was in InitInfectionImmunology, moved to Update, but still only want to calc one time.
        ar & boost::serialization::base_object<Kernel::InfectionEnvironmental>(inf);
    }
    template void serialize( boost::mpi::packed_skeleton_iarchive&, Kernel::InfectionPolio&, unsigned int);
}
#endif

#endif // ENABLE_POLIO
