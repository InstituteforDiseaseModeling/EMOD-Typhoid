/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "IndividualAirborne.h"
#include <list>
#include <vector>
#include "Exceptions.h"

#ifdef ENABLE_TB
#include "IndividualTB.h"

namespace Kernel
{
    // This class has no need yet for flags beyond those in the base class
    class IIndividualHumanCoinfection : public ISupports
    {
    //friend class IInfection;
    //friend class ISusceptibilityHIV;
    public:
        virtual bool HasActiveInfection() const = 0;
        virtual bool HasLatentInfection() const = 0;
        virtual bool IsImmune() const = 0;
        virtual NewInfectionState::_enum GetNewInfectionState() const = 0;
        virtual    bool HasHIV() const = 0;
        virtual bool HasTB() const = 0;
        virtual NaturalNumber GetViralLoad() const = 0;
        virtual const std::list< Susceptibility* > &Getsusceptibilitylist() = 0;
        virtual bool IsMDR() const = 0; 
        virtual bool IsSmearPositive() const = 0;
        virtual float GetCD4() const = 0;
        virtual void Mod_activate()  = 0;
        virtual void Set_forward_CD4(std::vector<float> )  = 0;
        virtual void Set_forward_TB_act( std::vector<float> ) = 0;
        virtual vector <float> Get_forward_CD4_act() = 0;
        virtual map <float,float> Get_CD4_Map() = 0;
        virtual void Coinf_Generate_forward_CD4() = 0;
        /*virtual bool IsOnART() const = 0;
        virtual bool HasEvenBeenOnART() const = 0;
        virtual bool HasEverTestedPositiveForHIV() const = 0;
        virtual void TakeHIVDiagnosticTest() = 0;*/
        virtual void InitiateART() = 0;
        virtual void AcquireNewInfectionHIV(StrainIdentity *infstrain = nullptr, int incubation_period_override = -1) = 0;
        virtual bool HasActivePresymptomaticInfection() const = 0;
        virtual bool HasExtrapulmonaryInfection() const = 0;
        virtual void SetTBactivationvector( const std::vector<float>& ) = 0;
        virtual const std::vector<float>& GetTBactivationvector() const = 0;
        virtual float GetNextLatentActivation(float time) const = 0;
        virtual float GetCD4TimeStep() const = 0;
        virtual int   GetNumCD4TimeSteps() const = 0;
    };

    class IndividualHumanCoinfectionConfig : public IndividualHumanAirborneConfig
    {
        GET_SCHEMA_STATIC_WRAPPER( IndividualHumanCoinfectionConfig )
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        virtual bool Configure( const Configuration* config ) override;

    protected:
        friend class IndividualHumanCoinfection;

        static map <float,float> CD4_act_map;
        static float HIV_coinfection_probability;
        static float Coinfected_mortality_rate;
    };

    class IndividualHumanCoinfection : public IIndividualHumanCoinfection, public IIndividualHumanTB, public IndividualHumanAirborne
    {
        friend class SimulationTBHIV;
        friend class Node;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING();
        DECLARE_QUERY_INTERFACE()

    public:
        virtual ~IndividualHumanCoinfection(void) { }
        static   IndividualHumanCoinfection *CreateHuman(INodeContext *context, suids::suid id, float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);
        virtual void InitializeHuman();

        // Infections and Susceptibility
        virtual void AcquireNewInfection(StrainIdentity *infstrain = nullptr, int incubation_period_override = -1);
        virtual void AcquireNewInfectionHIV(StrainIdentity *infstrain = nullptr, int incubation_period_override = -1);
        virtual void CreateSusceptibility(float=1.0, float=1.0);
        virtual void UpdateInfectiousness(float dt);
        virtual void Update(float currenttime, float dt);
        //virtual void Expose(const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route);
        virtual void Expose( const IContagionPopulation* cp, float dt, TransmissionRoute::Enum transmission_route = TransmissionRoute::TRANSMISSIONROUTE_ALL );
        virtual void CheckHIVVitalDynamics(float=1.0); // non-disease mortality
        

        // For reporting of individual's infection status to NodeTB
        // TODO: Consider giving the individual an infection count to avoid repeated function calls
        virtual bool HasActiveInfection() const;
        virtual bool HasLatentInfection() const;
        virtual bool HasTB() const;
        virtual bool IsImmune() const;
        virtual inline NewInfectionState::_enum GetNewInfectionState() const { return m_new_infection_state; }
        virtual bool IsMDR() const;
        virtual int GetTime() const;
        virtual bool IsSmearPositive() const;
        virtual float GetCD4() const;
        virtual void Set_forward_CD4(vector <float> vin)  {CD4_forward_vector = vin; };
        virtual void Set_forward_TB_act( std::vector<float> vin);
        virtual vector <float> Get_forward_CD4_act(){return CD4_forward_vector;};
        virtual map <float,float> Get_CD4_Map(){return IndividualHumanCoinfectionConfig::CD4_act_map;};
        virtual void LifeCourseLatencyTimerUpdate( Susceptibility*);
        virtual void Coinf_Generate_forward_CD4();
        virtual bool HasActivePresymptomaticInfection() const;
        virtual bool HasExtrapulmonaryInfection() const;
        virtual float GetCD4TimeStep() const;
        virtual int   GetNumCD4TimeSteps() const;
        /*virtual bool IsOnART() const;
        virtual bool HasEvenBeenOnART() const;
        virtual bool HasEverTestedPositiveForHIV() const;
        virtual void TakeHIVDiagnosticTest();*/
        virtual void InitiateART();

        //Hooks for changing infectiousness, activation, mortality based on CD4 count and age (maybe there is a more natural spot for this)
        //virtual float  Mod_mort(float CD4, float age);
        virtual void  Mod_activate() ;
        //virtual float  Mod_infectiousness(float CD4, float age);

        //used for ReportTBHIV
        virtual    bool HasHIV() const;
        virtual NaturalNumber GetViralLoad() const;
        virtual const std::list< Susceptibility* > &Getsusceptibilitylist();

        //IIndividualHumanContext
        virtual IIndividualHumanInterventionsContext* GetInterventionsContextbyInfection(Infection* infection);
        virtual void SetTBactivationvector(const std::vector <float>& vin);
        virtual const std::vector <float>& GetTBactivationvector() const ;
        virtual float GetNextLatentActivation(float time) const;

        // Coinfection reactivation
        std::vector <float> CD4_forward_vector;
        std::vector<float> TB_activation_vector;

        // Not implemented for Co-Inf functions inherited from IIndividualHumanTB
        /*virtual bool IndividualHumanCoinfection::HasPendingRelapseInfection() const
        {
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "IIndividualHumanTB function not yet used in IndividualHumanCoInfection." );
        }
        virtual bool IndividualHumanCoinfection::IsTreatmentNaive() const
        {
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "IIndividualHumanTB function not yet used in IndividualHumanCoInfection." );
        }
        virtual bool IndividualHumanCoinfection::HasFailedTreatment() const
        {
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "IIndividualHumanTB function not yet used in IndividualHumanCoInfection." );
        }
        virtual bool IndividualHumanCoinfection::IsOnTreatment() const
        {
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "IIndividualHumanTB function not yet used in IndividualHumanCoInfection." );
        }
        virtual bool IndividualHumanCoinfection::IsEvolvedMDR() const
        {
            throw NotYetImplementedException( __FILE__, __LINE__, __FUNCTION__, "IIndividualHumanTB function not yet used in IndividualHumanCoInfection." );
        }*/

    protected:
        IndividualHumanCoinfection(suids::suid _suid = suids::nil_suid(), float monte_carlo_weight = 1.0f, float initial_age = 0.0f, int gender = 0, float initial_poverty = 0.5f);

        // Factory methods
        virtual IInfection* createInfection(suids::suid _suid);
        virtual bool createInfection( suids::suid _suid, infection_list_t &newInfections );
        virtual void setupInterventionsContainer();
        virtual bool SetNewInfectionState(InfectionStateChange::_enum inf_state_change);

        infection_list_t newInfectionlist;
        std::list< Susceptibility* > susceptibilitylist;
        //std::list< InterventionsContainer* > interventionslist;

        std::map< IInfection*, ISusceptibilityContext*> infection2susceptibilitymap;
        std::map< IInfection*, InterventionsContainer*> infection2interventionsmap;

        //future, please use the list not the individual ones
        Susceptibility* susceptibility_tb;
        Susceptibility* susceptibility_hiv;
        //InterventionsContainer* interventions_tb;
        //InterventionsContainer* interventions_hiv;


        infection_list_t infectionslist;
        int infectioncount_tb;
        int infectioncount_hiv;

        bool m_is_on_ART;
        bool m_has_ever_been_onART;
        bool m_has_ever_tested_positive_for_HIV;

        static void InitializeStaticsCoinfection( const Configuration* config );
    };
}

#if 0
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, IndividualHumanCoinfection& human, const unsigned int  file_version )
    {
        // Serialize fields - N/A

        ar & boost::serialization::base_object<IndividualHumanAirborne>(human);
    }
}
#endif

#endif
