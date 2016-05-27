/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "EventCoordinator.h"
#include "Configure.h"
#include "PropertyRestrictions.h"
#include "NodeEventContext.h"
#include "SimulationEnums.h"

namespace Kernel
{
    ENUM_DEFINE(TargetedDiseaseState,
        ENUM_VALUE_SPEC( HIV_Positive               , 1) 
        ENUM_VALUE_SPEC( HIV_Negative               , 2) 
        ENUM_VALUE_SPEC( Tested_Positive            , 3) 
        ENUM_VALUE_SPEC( Tested_Negative            , 4)
        ENUM_VALUE_SPEC( Male_Circumcision_Positive , 5)
        ENUM_VALUE_SPEC( Male_Circumcision_Negative , 6)
        ENUM_VALUE_SPEC( Has_Intervention           , 7)
        ENUM_VALUE_SPEC( Not_Have_Intervention      , 8))

    // ------------------------------------------------------------------------
    // --- AgeRange
    // ------------------------------------------------------------------------

    class IDMAPI AgeRange : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
    public:
        AgeRange( float minYears=0.0, float maxYears=MAX_HUMAN_AGE);
        virtual ~AgeRange();

        bool operator<( const AgeRange& rThat ) const;

        virtual bool Configure( const Configuration * inputJson ) override;

        float GetMinYear() const;
        float GetMaxYear() const;

        bool IsInRange( float ageYears ) const;

    private:
        float m_MinYears;
        float m_MaxYears;
    };

    // ------------------------------------------------------------------------
    // --- AgeRangeList
    // ------------------------------------------------------------------------

    class IDMAPI AgeRangeList : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
    public:
        AgeRangeList();
        virtual ~AgeRangeList();

        const AgeRange& operator[]( int index ) const;

        virtual void ConfigureFromJsonAndKey( const Configuration* inputJson, const std::string& key );
        virtual json::QuickBuilder GetSchema();

        void Add( const AgeRange& rar );
        int Size() const;
        void CheckForOverlap();

    private:
#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        std::vector<AgeRange> m_AgeRanges;
#pragma warning( pop )
    };

    // ------------------------------------------------------------------------
    // --- TargetedByAgeAndGender
    // ------------------------------------------------------------------------

    class IDMAPI TargetedByAgeAndGender
    {
    public:
        TargetedByAgeAndGender( const AgeRange& rar, 
                                Gender::Enum gender, 
                                int numTargeted, 
                                int numTimeSteps, 
                                int initialTimeStep );
        ~TargetedByAgeAndGender();

        void IncrementNextNumTargets();

        int GetNumTargeted() const;

        void FindQualifyingIndividuals( INodeEventContext* pNEC, 
                                        const std::vector<std::vector<TargetedDiseaseState::Enum>>& diseaseStates,
                                        const std::string& rHasInterventionName,
                                        PropertyRestrictions& rPropertyRestrictions );

        std::vector<IIndividualHumanEventContext*> SelectIndividuals();

        bool IsFinished() const;

    private:
#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        AgeRange     m_AgeRange;
        Gender::Enum m_Gender;
        int          m_NumTargeted;
        int          m_TimeStep;
        std::vector<int> m_NumTargetedPerTimeStep;
        std::vector<IIndividualHumanEventContext*> m_QualifyingIndividuals;
#pragma warning( pop )
    };

    // ------------------------------------------------------------------------
    // --- TargetedDistribution
    // ------------------------------------------------------------------------

    class IDMAPI TargetedDistribution : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
    public:
        TargetedDistribution();
        virtual ~TargetedDistribution();

        bool operator<( const TargetedDistribution& rThat ) const;

        virtual bool Configure( const Configuration * inputJson ) override;

        bool IsInRange( float currentYear ) const;

        bool IsFinished() const;
        float GetStartYear() const;
        float GetEndYear() const;

        void UpdateTargeting( const IdmDateTime& rDateTime, float dt );
        std::vector< IIndividualHumanEventContext* > DetermineWhoGetsIntervention( const std::vector<INodeEventContext*> nodeList );

        void CreateAgeAndGenderList( const IdmDateTime& rDateTime, float dt );

        void ScaleTargets( float popScaleFactor );

    private:
        void CheckForZeroTargeted();

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        float m_StartYear;
        float m_EndYear;
        std::vector<std::vector<TargetedDiseaseState::Enum>> m_DiseaseStates;
        std::string m_HasInterventionName;
        PropertyRestrictions m_PropertyRestrictions;
        AgeRangeList m_AgeRangeList;
        std::vector<int> m_NumTargeted;
        std::vector<int> m_NumTargetedMales;
        std::vector<int> m_NumTargetedFemales;
        std::vector<TargetedByAgeAndGender> m_AgeAndGenderList;
#pragma warning( pop )
    };

    // ------------------------------------------------------------------------
    // --- TargetedDistributionList
    // ------------------------------------------------------------------------

    class IDMAPI TargetedDistributionList : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()
    public:
        TargetedDistributionList();
        virtual ~TargetedDistributionList();

        virtual void ConfigureFromJsonAndKey( const Configuration* inputJson, const std::string& key );
        virtual json::QuickBuilder GetSchema();

        void CheckForOverlap();
        void Add( const TargetedDistribution& rtd );

        void UpdateTargeting( const IdmDateTime& rDateTime, float dt );
        TargetedDistribution* GetCurrentTargets();
        bool IsFinished( const IdmDateTime& rDateTime, float dt );

        void ScaleTargets( float popScaleFactor );

    private:
#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        int m_CurrentIndex;
        TargetedDistribution* m_pCurrentTargets;
        std::vector<TargetedDistribution> m_TargetedDistributions;
#pragma warning( pop )
    };

    // ------------------------------------------------------------------------
    // --- NChooserEventCoordinator
    // ------------------------------------------------------------------------

    class IDMAPI NChooserEventCoordinator : public IEventCoordinator, /*public IVisitIndividual,*/ public JsonConfigurable
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(EventCoordinatorFactory, NChooserEventCoordinator, IEventCoordinator)    

    public:
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        NChooserEventCoordinator();
        virtual ~NChooserEventCoordinator();

        virtual bool Configure( const Configuration * inputJson ) override;
        virtual QuickBuilder GetSchema() override;

        // IEventCoordinator methods
        virtual void SetContextTo(ISimulationEventContext *isec) override;
        virtual void AddNode( const suids::suid& suid) override;
        virtual void Update(float dt) override;
        virtual void UpdateNodes(float dt) override;
        virtual bool IsFinished() override; // returns false when the EC requires no further updates and can be disposed of

        // IVisitIndividual methods
        //virtual bool visitIndividualCallback( IIndividualHumanEventContext *ihec, float & incrementalCostOut, ICampaignCostObserver * pICCO ) override;

    protected:

        virtual void UpdateInterventionToBeDistributed( const IdmDateTime& rDateTime, float dt );


#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        ISimulationEventContext*        m_Parent;
        std::vector<INodeEventContext*> m_CachedNodes;
        std::string                     m_InterventionName;
        IDistributableIntervention*     m_pIntervention;
        InterventionConfig              m_InterventionConfig;
        TargetedDistributionList        m_TargetedDistributionList;
        uint32_t                        m_DistributionIndex;
        bool                            m_IsFinished;
        bool                            m_HasBeenScaled;
#pragma warning( pop )


#if USE_JSON_SERIALIZATION
    public:
        // IJsonSerializable Interfaces
        virtual void JSerialize( IJsonObjectAdapter* root, JSerializer* helper ) const;
        virtual void JDeserialize( IJsonObjectAdapter* root, JSerializer* helper );
#endif    
#if USE_BOOST_SERIALIZATION
    private:
        friend class ::boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive &ar, StandardInterventionDistributionEventCoordinator &ec, const unsigned int v);
#endif
    };
}
