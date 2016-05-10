/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#include "NChooserEventCoordinator.h"
#include "InterventionFactory.h"
#include "Environment.h"
#include "RANDOM.h"
#include "IIndividualHumanSTI.h"
#include "IIndividualHumanHIV.h"
#include "IHIVInterventionsContainer.h"
//#include "SimulationConfig.h"

static const char * _module = "NChooserEventCoordinator";

namespace Kernel
{
    uint32_t RandInt( RANDOMBASE* pRNG, uint32_t N )
    {
        uint64_t ulA = uint64_t(pRNG->ul());
        uint64_t ulB = uint64_t(pRNG->ul());
        ulB <<= 32;
        ulA += ulB;
        uint64_t ll = (ulA & 0xFFFFFFFFL) * N;
        ll >>= 32;
        return ll;
    }



    std::set< std::string > GetAllowedTargetDiseaseStates()
    {
        std::set< std::string > allowed;
        for( int i = 0 ; i < TargetedDiseaseState::pairs::count() ; ++i )
        {
            allowed.insert( std::string(TargetedDiseaseState::pairs::get_keys()[i]) );
        }

        return allowed;
    }

    std::vector<std::vector<TargetedDiseaseState::Enum>> ConvertStringsToDiseaseState( std::vector<std::vector<std::string>>& rStringMatrix )
    {
        std::vector<std::vector<TargetedDiseaseState::Enum>> enum_matrix;
        for( auto& vec : rStringMatrix )
        {
            std::vector<TargetedDiseaseState::Enum> enum_vec ;
            for( auto& str : vec )
            {
                 int int_state = TargetedDiseaseState::pairs::lookup_value( str.c_str() );
                 if( int_state == -1 )
                 {
                    std::stringstream ss ;
                    ss << "The 'Target_Disease_State' value of '" << str << "' is not a valid enum of TargetedDiseaseState.  Valid values are:\n";
                    for( int i = 0 ; i < TargetedDiseaseState::pairs::count() ; ++i )
                    {
                        ss << TargetedDiseaseState::pairs::get_keys()[i] << "\n" ;
                    }
                    throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
                 }
                 TargetedDiseaseState::Enum state = (TargetedDiseaseState::Enum)int_state;
                 enum_vec.push_back( state );
            }
            enum_matrix.push_back( enum_vec );
        }
        return enum_matrix;
    }

    bool HasDiseaseState( TargetedDiseaseState::Enum state,
                          IIndividualHumanEventContext *pHEC,
                          IIndividualHumanSTI* pSTI,
                          IIndividualHumanHIV *pHIV,
                          IHIVMedicalHistory * pMedHistory)
    {
        switch( state )
        {
            case TargetedDiseaseState::HIV_Positive:
                return pHIV->HasHIV();

            case TargetedDiseaseState::HIV_Negative:
                return !pHIV->HasHIV();

            case TargetedDiseaseState::Tested_Positive:
                return pMedHistory->EverTestedHIVPositive();

            case TargetedDiseaseState::Tested_Negative:
                return (pMedHistory->EverTested() && !pMedHistory->EverTestedHIVPositive());

            case TargetedDiseaseState::Male_Circumcision_Positive:
                return pSTI->IsCircumcised();

            case TargetedDiseaseState::Male_Circumcision_Negative:
                return !pSTI->IsCircumcised();

            case TargetedDiseaseState::Vaccinated_Positive:
                return (pHEC->GetInterventionsContext()->GetInterventionsByType( "class Kernel::RevaccinatableVaccine" ).size() > 0);

            case TargetedDiseaseState::Vaccinated_Negative:
                return !(pHEC->GetInterventionsContext()->GetInterventionsByType( "class Kernel::RevaccinatableVaccine" ).size() > 0);
                break;

            default:
                throw BadEnumInSwitchStatementException( __FILE__, __LINE__, __FUNCTION__, "TargetedDiseaseState", state );
        }
    }

    bool QualifiesByDiseaseState( const std::vector<std::vector<TargetedDiseaseState::Enum>>& diseaseStates,
                                  IIndividualHumanEventContext *pHEC )
    {
        if( diseaseStates.size() == 0 )
        {
            return true;
        }

        IIndividualHumanSTI* p_ind_sti = NULL;
        if( pHEC->QueryInterface( GET_IID( IIndividualHumanSTI ), (void**)&p_ind_sti ) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "ihec", "IIndividualSTI", "IIndividualHumanEventContext" );
        }

        IIndividualHumanHIV * p_ind_hiv = nullptr;
        if( pHEC->QueryInterface(GET_IID(IIndividualHumanHIV), (void**)&p_ind_hiv) != s_OK )
        {
            throw QueryInterfaceException(__FILE__, __LINE__, __FUNCTION__, "ihec", "IIndividualHumanHIV", "IIndividualHumanEventContext");
        }

        IHIVMedicalHistory * p_med_history = nullptr;
        if( p_ind_hiv->GetHIVInterventionsContainer()->QueryInterface(GET_IID(IHIVMedicalHistory), (void**)&p_med_history) != s_OK )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "p_ind_hiv", "IHIVMedicalHistory", "IHIVInterventionsContainer" );
        }

        for( auto& states_to_and : diseaseStates )
        {
            bool qualifies = true;
            for( int i = 0 ; qualifies && (i < states_to_and.size()) ; ++i )
            {
                auto state = states_to_and[i];
                qualifies = HasDiseaseState( state, pHEC, p_ind_sti, p_ind_hiv, p_med_history );
            }
            if( qualifies )
            {
                return true;
            }
        }
        return false;
    }

    // ------------------------------------------------------------------------
    // --- AgeRange
    // ------------------------------------------------------------------------
#define NC_AR_Min_DESC_TEXT  "Minimum age in years of range - greater than or equal to"
#define NC_AR_Max_DESC_TEXT  "Maximum age in years of range - less than but not equal to"

    BEGIN_QUERY_INTERFACE_BODY(AgeRange)
    END_QUERY_INTERFACE_BODY(AgeRange)


    AgeRange::AgeRange( float minYears, float maxYears )
    : JsonConfigurable()
    , m_MinYears(minYears)
    , m_MaxYears(maxYears)
    {
    }

    AgeRange::~AgeRange()
    {
    }

    bool AgeRange::operator<( const AgeRange& rThat ) const
    {
        return (this->m_MinYears < rThat.m_MinYears);
    }

    bool AgeRange::Configure( const Configuration * inputJson )
    {
        initConfigTypeMap("Min", &m_MinYears, NC_AR_Min_DESC_TEXT, 0.0, MAX_HUMAN_AGE, 0.0 );
        initConfigTypeMap("Max", &m_MaxYears, NC_AR_Max_DESC_TEXT, 0.0, MAX_HUMAN_AGE, MAX_HUMAN_AGE );

        bool ret = JsonConfigurable::Configure( inputJson );
        if( ret && !JsonConfigurable::_dryrun )
        {
            if( m_MinYears >= m_MaxYears )
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Min", m_MinYears, "Max", m_MaxYears, "Min must be < Max" );
            }
        }
        return ret;
    }

    float AgeRange::GetMinYear() const
    {
        return m_MinYears;
    }

    float AgeRange::GetMaxYear() const
    {
        return m_MaxYears;
    }

    bool AgeRange::IsInRange( float ageYears ) const
    {
        return ((m_MinYears <= ageYears) && (ageYears < m_MaxYears));
    }

    // ------------------------------------------------------------------------
    // --- AgeRangeList
    // ------------------------------------------------------------------------

    BEGIN_QUERY_INTERFACE_BODY(AgeRangeList)
    END_QUERY_INTERFACE_BODY(AgeRangeList)

    AgeRangeList::AgeRangeList()
    : JsonConfigurable()
    , m_AgeRanges()
    {
    }

    AgeRangeList::~AgeRangeList()
    {
    }

    const AgeRange& AgeRangeList::operator[]( int index ) const
    {
        release_assert( (0 <= index) && (index < m_AgeRanges.size()) );
        return m_AgeRanges[index];
    }

    void AgeRangeList::ConfigureFromJsonAndKey( const Configuration* inputJson, const std::string& key )
    {
        // Temporary object created so we can 'operate' on json with the desired tools
        auto p_config = Configuration::CopyFromElement( (*inputJson)[key] );

        const auto& json_array = json_cast<const json::Array&>( (*p_config) );
        for( auto data = json_array.Begin(); data != json_array.End(); ++data )
        {
            Configuration* p_element_config = Configuration::CopyFromElement( *data );

            AgeRange ar;
            ar.Configure( p_element_config );

            Add( ar );

            delete p_element_config ;
        }
        delete p_config;
    }

    json::QuickBuilder AgeRangeList::GetSchema()
    {
        AgeRange ar;
        if( JsonConfigurable::_dryrun )
        {
            ar.Configure( nullptr );
        }

        json::QuickBuilder schema( jsonSchemaBase );
        auto tn = JsonConfigurable::_typename_label();
        auto ts = JsonConfigurable::_typeschema_label();
        schema[ tn ] = json::String( "idmType:AgeRangeList" );

        schema[ts] = json::Object();
        schema[ts]["<AgeRange Value>"] = ar.GetSchema().As<Object>();

        return schema;
    }

    void AgeRangeList::Add( const AgeRange& rar )
    {
        m_AgeRanges.push_back( rar );
        std::sort( m_AgeRanges.begin(), m_AgeRanges.end() );
    }

    int AgeRangeList::Size() const
    {
        return m_AgeRanges.size();
    }

    void AgeRangeList::CheckForOverlap()
    {
        if( m_AgeRanges.size() == 0 )
        {
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, "'Age_Ranges_Years' cannot have zero elements." );
        }

        float prev_min = m_AgeRanges[0].GetMinYear();
        float prev_max = m_AgeRanges[0].GetMaxYear();
        for( int i = 1 ; i < m_AgeRanges.size() ; ++i )
        {
            float this_min = m_AgeRanges[i].GetMinYear();
            float this_max = m_AgeRanges[i].GetMaxYear();

            if( prev_max > this_min )
            {
                std::stringstream ss;
                ss << "'Age_Ranges_Years' cannot have age ranges that overlap.  ";
                ss << "(" << prev_min << ", " << prev_max << ") vs (" << this_min << ", " << this_max << ")";
                throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }
        }
    }

    // ------------------------------------------------------------------------
    // --- TargetedByAgeAndGender
    // ------------------------------------------------------------------------

    TargetedByAgeAndGender::TargetedByAgeAndGender( const AgeRange& rar, 
                                                    Gender::Enum gender, 
                                                    int numTargeted, 
                                                    int numTimeSteps, 
                                                    int initialTimeStep )
    : m_AgeRange( rar )
    , m_Gender( gender )
    , m_NumTargeted( numTargeted )
    , m_TimeStep( initialTimeStep )
    , m_NumTargetedPerTimeStep()
    , m_QualifyingIndividuals()
    {
        release_assert( initialTimeStep < numTimeSteps );

        int avg_tgt = m_NumTargeted / numTimeSteps;
        int remainder = m_NumTargeted - (avg_tgt * numTimeSteps);

        for( int i = 0 ; i < numTimeSteps; ++i )
        {
            m_NumTargetedPerTimeStep.push_back( avg_tgt );
        }
        for( int i = 0 ; i < remainder; ++i )
        {
            m_NumTargetedPerTimeStep[i] += 1;
        }
    }

    TargetedByAgeAndGender::~TargetedByAgeAndGender()
    {
    }

    void TargetedByAgeAndGender::IncrementNextNumTargets()
    {
        ++m_TimeStep;
    }

    int TargetedByAgeAndGender::GetNumTargeted() const
    {
        release_assert( m_TimeStep < m_NumTargetedPerTimeStep.size() );
        return m_NumTargetedPerTimeStep[ m_TimeStep ];
    }

    void TargetedByAgeAndGender::FindQualifyingIndividuals( INodeEventContext* pNEC, 
                                                            const std::vector<std::vector<TargetedDiseaseState::Enum>>& diseaseStates,
                                                            PropertyRestrictions& rPropertyRestrictions )
    {
        m_QualifyingIndividuals.clear();
        m_QualifyingIndividuals.reserve( pNEC->GetIndividualHumanCount() );

        INodeEventContext::individual_visit_function_t fn = 
            [ this, &diseaseStates, &rPropertyRestrictions ](IIndividualHumanEventContext *ihec)
        {
            if( !m_AgeRange.IsInRange( ihec->GetAge()/DAYSPERYEAR ) ) return;

            if( (m_Gender != Gender::COUNT) && (m_Gender != ihec->GetGender()) ) return;

            if( !rPropertyRestrictions.Qualifies( ihec ) ) return;

            if( !QualifiesByDiseaseState( diseaseStates, ihec ) ) return;

            m_QualifyingIndividuals.push_back( ihec );
        };

        pNEC->VisitIndividuals( fn );
    }

    std::vector<IIndividualHumanEventContext*> TargetedByAgeAndGender::SelectIndividuals()
    {
        if( GetNumTargeted() >= m_QualifyingIndividuals.size() )
        {
            return m_QualifyingIndividuals;
        }

        // ----------------------------------------------------------------------------------
        // --- Robert Floyd's Algorithm for Sampling without Replacement
        // --- http://www.nowherenearithaca.com/2013/05/robert-floyds-tiny-and-beautiful.html
        // ----------------------------------------------------------------------------------

        std::set<int> selected_indexes;
        int N = m_QualifyingIndividuals.size();
        int M = GetNumTargeted();

        for( int j = (N - M) ; j < N ; j++ )
        {
            int index = RandInt( EnvPtr->RNG, j+1 );
            release_assert( index < N );
            if( selected_indexes.find( index ) == selected_indexes.end() )
            {
                selected_indexes.insert( index );
            }
            else
            {
                selected_indexes.insert( j );
            }
        }

        std::vector<IIndividualHumanEventContext*> selected_individuals;
        for( auto index : selected_indexes )
        {
            selected_individuals.push_back( m_QualifyingIndividuals[ index ] );
        }

        return selected_individuals;
    }

    bool TargetedByAgeAndGender::IsFinished() const
    {
        if( (m_TimeStep != 0) && (m_TimeStep >= (m_NumTargetedPerTimeStep.size()-1)) )
        {
            return true;
        }
        else
        {
            return false;
        }
    }


    // ------------------------------------------------------------------------
    // --- TargetedDistribution
    // ------------------------------------------------------------------------
#define NC_TD_Start_Year_DESC_TEXT            "The year to start distributing the intervention - January 1st of year"
#define NC_TD_End_Year_DESC_TEXT              "The year to stop distributing the intervention - January 1st of year"
#define NC_TD_Num_Targeted_DESC_TEXT          "The number of individuals to distribute interventions to during this time period (both genders randomly); Num_Targeted_Males/Females must be empty if using this"
#define NC_TD_Num_Targeted_Males_DESC_TEXT    "The number of individuals to distribute interventions to during this time period (males only); Num_Targeted must be empty if using this and Num_Targeted_Females must be same length"
#define NC_TD_Num_Targeted_Females_DESC_TEXT  "The number of individuals to distribute interventions to during this time period (females only); Num_Targeted must be empty if using this and Num_Targeted_Males must be same length"
#define NC_TD_Age_Ranges_Years_DESC_TEXT      "An array of age bins, where a person can be selected if min <= age < max.  It must have the same number of objects as Num_Targeted_XXX has elements."
#define NC_TD_Target_Disease_State_DESC_TEXT  "If not empty, a targeted individual is expected to have a particular disease specific state.  To be flexible, this is a two-dimensional array of TargetDiseaseState where the elements of the inner array are AND'd and these arrays are OR'd. "
#define NC_TD_Property_Restrictions_Within_Node_DESC_TEXT "TBD"

    BEGIN_QUERY_INTERFACE_BODY(TargetedDistribution)
    END_QUERY_INTERFACE_BODY(TargetedDistribution)


    TargetedDistribution::TargetedDistribution()
    : JsonConfigurable()
    , m_StartYear(1900.0)
    , m_EndYear(2200.0)
    , m_DiseaseStates()
    , m_PropertyRestrictions()
    , m_AgeRangeList()
    , m_NumTargeted()
    , m_NumTargetedMales()
    , m_NumTargetedFemales()
    {
    }

    TargetedDistribution::~TargetedDistribution()
    {
    }

    bool TargetedDistribution::operator<( const TargetedDistribution& rThat ) const
    {
        return (this->m_StartYear < rThat.m_StartYear);
    }

    bool TargetedDistribution::Configure( const Configuration * inputJson )
    {
        std::vector<std::vector<std::string>> vector2d_string_disease_states;
        std::set< std::string > allowed_states = GetAllowedTargetDiseaseStates();

        initConfigTypeMap("Start_Year",           &m_StartYear,          NC_TD_Start_Year_DESC_TEXT,           1900.0,  2200.0, 1900.0 );
        initConfigTypeMap("End_Year",             &m_EndYear,            NC_TD_End_Year_DESC_TEXT,             1900.0,  2200.0, 2200.0 );
        initConfigTypeMap("Num_Targeted",         &m_NumTargeted,        NC_TD_Num_Targeted_DESC_TEXT,              0, INT_MAX,      0 );
        initConfigTypeMap("Num_Targeted_Males",   &m_NumTargetedMales,   NC_TD_Num_Targeted_Males_DESC_TEXT,        0, INT_MAX,      0 );
        initConfigTypeMap("Num_Targeted_Females", &m_NumTargetedFemales, NC_TD_Num_Targeted_Females_DESC_TEXT,      0, INT_MAX,      0 );

        initConfigTypeMap("Target_Disease_State", &vector2d_string_disease_states, NC_TD_Target_Disease_State_DESC_TEXT, nullptr, allowed_states );

        initConfigComplexType("Age_Ranges_Years",                  &m_AgeRangeList,         NC_TD_Age_Ranges_Years_DESC_TEXT );
        initConfigComplexType("Property_Restrictions_Within_Node", &m_PropertyRestrictions, NC_TD_Property_Restrictions_Within_Node_DESC_TEXT );

        bool ret = JsonConfigurable::Configure( inputJson );
        if( ret && !JsonConfigurable::_dryrun )
        {
            if( m_StartYear >= m_EndYear )
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                    "Start_Year", m_StartYear,
                    "End_Year", m_EndYear,
                    "Start_Year must be < End_Year" );
            }

            if( (m_NumTargeted.size() > 0) && ((m_NumTargetedMales.size() > 0) || (m_NumTargetedFemales.size() > 0)) )
            {
                int max = m_NumTargetedMales.size();
                if( max < m_NumTargetedFemales.size() ) max = m_NumTargetedFemales.size();

                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                    "Num_Targeted", m_NumTargeted.size(),
                    "Num_Targeted_Males/Num_Targeted_Females", max, 
                    "If using Num_Targeted, then Num_Targeted_Males and Num_Targeted_Females must be empty.\nIf using Num_Targeted_Males and Num_Targeted_Females, then Num_Targeted must be empty." );
            }

            if( (m_NumTargeted.size() > 0) && (m_AgeRangeList.Size() != m_NumTargeted.size()) )
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                    "Num_Targeted", m_NumTargeted.size(),
                    "Age_Ranges_Years", m_AgeRangeList.Size(), 
                    "Num_Targeted and Age_Range_Years must have the same number of elements, but not zero.  There should be one age range for each number targeted." );
            }

            if( ((m_NumTargetedMales.size() > 0) && ((m_AgeRangeList.Size() != m_NumTargetedMales.size()) || (m_AgeRangeList.Size() != m_NumTargetedFemales.size()))) ||
                ((m_AgeRangeList.Size() == 0) && (m_NumTargeted.size() == 0) &&  (m_NumTargetedMales.size() == 0) &&  (m_NumTargetedFemales.size() == 0)) )
            {
                throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, 
                    "Num_Targeted_Males", m_NumTargetedMales.size(),
                    "Age_Ranges_Years", m_AgeRangeList.Size(), 
                    "Num_Targeted_Males, Num_Targeted_Females, and Age_Range_Years must have the same number of elements, but not zero.  There should be one age range for each number targeted." );
            }

            CheckForZeroTargeted();

            m_AgeRangeList.CheckForOverlap();

            m_DiseaseStates = ConvertStringsToDiseaseState( vector2d_string_disease_states );
        }
        return ret;
    }


    void TargetedDistribution::ScaleTargets( float popScaleFactor )
    {
        if( popScaleFactor == 1.0 )
        {
            return;
        }
        if( m_NumTargeted.size() > 0 )
        {
            int num_total = 0;
            for( int i = 0 ; i < m_NumTargeted.size() ; ++i )
            {
                m_NumTargeted[i] *= popScaleFactor;
                num_total += m_NumTargeted[i];
            }
            if( num_total == 0 )
            {
                std::stringstream msg;
                msg << "The Base_Population_Scale_Factor (" << popScaleFactor << ") has scaled the values of Num_Targets all to zero so won't target anyone.";
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }
        }
        else
        {
            int num_total = 0;
            for( int i = 0 ; i < m_NumTargetedMales.size() ; ++i )
            {
                m_NumTargetedMales[i] *= popScaleFactor;
                m_NumTargetedFemales[i] *= popScaleFactor;
                num_total += m_NumTargetedMales[i];
                num_total += m_NumTargetedFemales[i];
            }
            if( num_total == 0 )
            {
                std::stringstream msg;
                msg << "The Base_Population_Scale_Factor (" << popScaleFactor << ") has scaled the values of Num_Targets_Males and Num_Target_Femailes all to zero so won't target anyone.";
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, msg.str().c_str() );
            }
        }
    }

    void TargetedDistribution::CheckForZeroTargeted()
    {
        if( m_NumTargeted.size() > 0 )
        {
            int total = 0;
            for( auto num : m_NumTargeted )
            {
                total += num;
            }
            if( total == 0 )
            {
                throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, "'Num_Targeted' has zero values and won't target anyone." );
            }
        }
        else
        {
            release_assert( m_NumTargetedMales.size() == m_NumTargetedFemales.size() );

            int total_males = 0;
            int total_females = 0;
            for( int i = 0 ; i < m_NumTargetedMales.size() ; ++i )
            {
                total_males   += m_NumTargetedMales[i];
                total_females += m_NumTargetedFemales[i];
            }
            if( (total_males + total_females) == 0 )
            {
                throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, "'Num_Targeted_Males' and 'Num_Targeted_Females' have zero values and won't target anyone." );
            }
        }
    }

    bool TargetedDistribution::IsInRange( float currentYear ) const
    {
        return ((m_StartYear <= currentYear) && (currentYear < m_EndYear));
    }

    float TargetedDistribution::GetStartYear() const
    {
        return m_StartYear;
    }

    float TargetedDistribution::GetEndYear() const
    {
        return m_EndYear;
    }

    void TargetedDistribution::CreateAgeAndGenderList( const IdmDateTime& rDateTime, float dt )
    {
        float start_days = m_StartYear * DAYSPERYEAR;
        float end_days   = m_EndYear   * DAYSPERYEAR;
        int num_time_steps = int( (end_days - start_days) / dt );

        float current_days = rDateTime.Year() * DAYSPERYEAR;
        int current_time_step = int( (current_days - start_days) / dt );

        for( int i = 0 ; i < m_AgeRangeList.Size(); ++i )
        {
            if( m_NumTargeted.size() > 0 )
            {
                if( m_NumTargeted[i] > 0 )
                {
                    TargetedByAgeAndGender tbag( m_AgeRangeList[i], Gender::COUNT, m_NumTargeted[i], num_time_steps, current_time_step );
                    m_AgeAndGenderList.push_back( tbag );
                }
            }
            else
            {
                if( m_NumTargetedMales[i] > 0 )
                {
                    TargetedByAgeAndGender tbag_males(   m_AgeRangeList[i], Gender::MALE,   m_NumTargetedMales[i],  num_time_steps, current_time_step  );
                    m_AgeAndGenderList.push_back( tbag_males );
                }
                if( m_NumTargetedFemales[i] > 0 )
                {
                    TargetedByAgeAndGender tbag_females( m_AgeRangeList[i], Gender::FEMALE, m_NumTargetedFemales[i], num_time_steps, current_time_step );
                    m_AgeAndGenderList.push_back( tbag_females );
                }
            }
        }
        release_assert( m_AgeAndGenderList.size() > 0 );
    }

    void TargetedDistribution::UpdateTargeting( const IdmDateTime& rDateTime, float dt )
    {
        if( m_AgeAndGenderList.size() == 0 )
        {
            CreateAgeAndGenderList( rDateTime, dt );
        }
        else
        {
            for( auto& r_ag : m_AgeAndGenderList )
            {
                r_ag.IncrementNextNumTargets();
            }
        }
    }

    std::vector< IIndividualHumanEventContext* >
    TargetedDistribution::DetermineWhoGetsIntervention( const std::vector<INodeEventContext*> nodeList )
    {
        // ---------------------------------------------------------
        // --- Find the individuals for each age and gender
        // --- that meet the demographic restrictions
        // ---------------------------------------------------------

        for( auto& r_ag : m_AgeAndGenderList )
        {
            for( auto p_nec : nodeList )
            {
                r_ag.FindQualifyingIndividuals( p_nec, m_DiseaseStates, m_PropertyRestrictions );
            }
        }

        // ----------------------------------------------------------------------
        // --- Of those that qualify, pick those that should get the intervention
        // ----------------------------------------------------------------------

        // Determine the total number of individuals that can receive the intervention
        int num_total = 0;
        for( auto& r_ag : m_AgeAndGenderList )
        {
            num_total += r_ag.GetNumTargeted();
        }

        // Create the vector to return and allocate space for the individuals
        std::vector< IIndividualHumanEventContext* > distribute_to;
        distribute_to.reserve( num_total );

        for( auto& r_ag : m_AgeAndGenderList )
        {
            std::vector< IIndividualHumanEventContext* > selected = r_ag.SelectIndividuals();
            distribute_to.insert( distribute_to.end(), selected.begin(), selected.end() );
        }
        return distribute_to;
    }

    bool TargetedDistribution::IsFinished() const
    {
        if( m_AgeAndGenderList.size() == 0 )
        {
            // haven't started yet
            return false;
        }
        bool is_finished = true;

        for( int i = 0 ; is_finished && (i < m_AgeAndGenderList.size()) ; ++i )
        {
            is_finished = m_AgeAndGenderList[i].IsFinished();
        }
        return is_finished;
    }

    // ------------------------------------------------------------------------
    // --- TargetedDistribution
    // ------------------------------------------------------------------------

    BEGIN_QUERY_INTERFACE_BODY(TargetedDistributionList)
    END_QUERY_INTERFACE_BODY(TargetedDistributionList)

    TargetedDistributionList::TargetedDistributionList()
    : JsonConfigurable()
    , m_CurrentIndex(0)
    , m_pCurrentTargets(nullptr)
    , m_TargetedDistributions()
    {
    }

    TargetedDistributionList::~TargetedDistributionList()
    {
    }

    void TargetedDistributionList::ConfigureFromJsonAndKey( const Configuration* inputJson, const std::string& key )
    {
        // Temporary object created so we can 'operate' on json with the desired tools
        auto p_config = Configuration::CopyFromElement( (*inputJson)[key] );

        const auto& json_array = json_cast<const json::Array&>( (*p_config) );
        for( auto data = json_array.Begin(); data != json_array.End(); ++data )
        {
            Configuration* p_element_config = Configuration::CopyFromElement( *data );

            TargetedDistribution td;
            td.Configure( p_element_config );

            Add( td );

            delete p_element_config ;
        }
        delete p_config;
    }

    json::QuickBuilder TargetedDistributionList::GetSchema()
    {
        TargetedDistribution td;
        if( JsonConfigurable::_dryrun )
        {
            td.Configure( nullptr );
        }

        json::QuickBuilder schema( jsonSchemaBase );
        auto tn = JsonConfigurable::_typename_label();
        auto ts = JsonConfigurable::_typeschema_label();
        schema[ tn ] = json::String( "idmType:TargetedDistributionList" );

        schema[ts] = json::Object();
        schema[ts]["<TargetedDistribution Value>"] = td.GetSchema().As<Object>();

        return schema;
    }

    void TargetedDistributionList::CheckForOverlap()
    {
        if( m_TargetedDistributions.size() == 0 )
        {
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, "'Distributions' cannot have zero elements." );
        }

        float prev_start = m_TargetedDistributions[0].GetStartYear();
        float prev_end   = m_TargetedDistributions[0].GetEndYear();
        for( int i = 1 ; i < m_TargetedDistributions.size() ; ++i )
        {
            float this_start = m_TargetedDistributions[i].GetStartYear();
            float this_end   = m_TargetedDistributions[i].GetEndYear();

            if( prev_end > this_start )
            {
                std::stringstream ss;
                ss << "'Distributions' cannot have time periods that overlap.  ";
                ss << "(" << prev_start << ", " << prev_end << ") vs (" << this_start << ", " << this_end << ")";
                throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }
        }
    }

    void TargetedDistributionList::Add( const TargetedDistribution& rtd )
    {
        m_TargetedDistributions.push_back( rtd );
        std::sort( m_TargetedDistributions.begin(), m_TargetedDistributions.end() );
    }

    void TargetedDistributionList::UpdateTargeting( const IdmDateTime& rDateTime, float dt )
    {
        // ----------------------------------------------------
        // --- Update where we are in the list of distributions
        // ----------------------------------------------------
        while( m_TargetedDistributions[ m_CurrentIndex ].GetEndYear() <= rDateTime.Year() )
        {
            ++m_CurrentIndex;
        }

        // -----------------------------------------
        // --- Find the current distributino period
        // -----------------------------------------
        m_pCurrentTargets = nullptr;
        if( (m_CurrentIndex < m_TargetedDistributions.size()) &&
            (m_TargetedDistributions[ m_CurrentIndex ].GetStartYear() <= rDateTime.Year()) )
        {
            m_pCurrentTargets = &m_TargetedDistributions[ m_CurrentIndex ];

            m_pCurrentTargets->UpdateTargeting( rDateTime, dt );
        }
    }

    TargetedDistribution* TargetedDistributionList::GetCurrentTargets()
    {
        return m_pCurrentTargets;
    }

    bool TargetedDistributionList::IsFinished( const IdmDateTime& rDateTime, float dt )
    {
        bool is_finished = false;
        while( (m_CurrentIndex < m_TargetedDistributions.size()) &&
               ( (m_TargetedDistributions[ m_CurrentIndex ].GetEndYear() <= rDateTime.Year()) ||
                  m_TargetedDistributions[ m_CurrentIndex ].IsFinished() ) )
        {
            ++m_CurrentIndex;
        }

        if( m_CurrentIndex >= m_TargetedDistributions.size() )
        {
            is_finished = true;
        }
        return is_finished;
    }

    void TargetedDistributionList::ScaleTargets( float popScaleFactor )
    {
        for( auto& rtd : m_TargetedDistributions )
        {
            rtd.ScaleTargets( popScaleFactor );
        }
    }

    // ------------------------------------------------------------------------
    // --- NChooserEventCoordinator
    // ------------------------------------------------------------------------


    IMPLEMENT_FACTORY_REGISTERED(NChooserEventCoordinator)
    IMPL_QUERY_INTERFACE2(NChooserEventCoordinator, IEventCoordinator, IConfigurable)

    NChooserEventCoordinator::NChooserEventCoordinator()
    : m_Parent( nullptr )
    , m_CachedNodes()
    , m_InterventionName()
    , m_pIntervention( nullptr )
    , m_InterventionConfig()
    , m_TargetedDistributionList()
    , m_DistributionIndex(0)
    , m_IsFinished(false)
    , m_HasBeenScaled(false)
    {
    }

    NChooserEventCoordinator::~NChooserEventCoordinator()
    {
    }


    QuickBuilder NChooserEventCoordinator::GetSchema()
    {
        return QuickBuilder( jsonSchemaBase );
    }

    bool NChooserEventCoordinator::Configure( const Configuration * inputJson )
    {
        initConfigComplexType(     "Distributions",   &m_TargetedDistributionList, "TBD" );
        initConfigComplexType( "Intervention_Config", &m_InterventionConfig,       "TBD" );

        bool retValue = JsonConfigurable::Configure( inputJson );

        if( retValue && !JsonConfigurable::_dryrun)
        {
            m_TargetedDistributionList.CheckForOverlap();

            InterventionValidator::ValidateIntervention( m_InterventionConfig._json );
        }

        return retValue;
    }

    void NChooserEventCoordinator::SetContextTo( ISimulationEventContext *isec )
    {
        m_Parent = isec;
    }

    void NChooserEventCoordinator::AddNode( const suids::suid& node_suid )
    {
        INodeEventContext* pNEC = m_Parent->GetNodeEventContext( node_suid );
        if( !m_HasBeenScaled )
        {
            m_TargetedDistributionList.ScaleTargets( pNEC->GetNodeContext()->GetBasePopulationScaleFactor() );
        }
        m_CachedNodes.push_back( pNEC );
    }

    void NChooserEventCoordinator::UpdateInterventionToBeDistributed( const IdmDateTime& rDateTime, float dt )
    {
        if( m_pIntervention == nullptr )
        {
            // intervention class names for informative logging
            std::ostringstream intervention_name;
            intervention_name << std::string( json::QuickInterpreter(m_InterventionConfig._json)["class"].As<json::String>() );
            m_InterventionName = intervention_name.str();

            auto qi_as_config = Configuration::CopyFromElement( m_InterventionConfig._json );
            m_pIntervention = InterventionFactory::getInstance()->CreateIntervention( qi_as_config );
            delete qi_as_config;
            qi_as_config = nullptr;
        }
    }

    void NChooserEventCoordinator::Update( float dt )
    {
        // --------------------------------------------------------------------------------
        // --- Update the intervention to be distributed.  This is probably only done once.
        // --------------------------------------------------------------------------------
        UpdateInterventionToBeDistributed( m_Parent->GetSimulationTime(), dt );

        // --------------------------------------------------------------------------------------
        // --- Update who is to be targeted
        // --- NOTE: We can't determine who gets the intervention because things could change
        // --- before UpdateNodes() is called.  However, we can determine what we are targeting.
        // --------------------------------------------------------------------------------------
        m_TargetedDistributionList.UpdateTargeting( m_Parent->GetSimulationTime(), dt );
    }

    void NChooserEventCoordinator::UpdateNodes( float dt )
    {
        // ---------------------------------------
        // --- Determine who gets the intervention
        // ---------------------------------------
        TargetedDistribution* p_current_targets = m_TargetedDistributionList.GetCurrentTargets();

        if( p_current_targets != nullptr ) // can be nullptr if in between periods
        {
            std::vector< IIndividualHumanEventContext *> individual_list = p_current_targets->DetermineWhoGetsIntervention( m_CachedNodes );

            // ------------------------------------------------------------------------------------------------------
            // --- Distribute the intervention
            // --- WARNING: This does not use the IVisitIndividual interface method of distributing the intervention
            // --- like StandardInterventionDistributionEventCoordinator.  This was done for performance reasons.
            // ------------------------------------------------------------------------------------------------------
            for( auto pIHEC : individual_list )
            {
                IDistributableIntervention *di = m_pIntervention->Clone();
                release_assert(di);
                if (di)
                {
                    ICampaignCostObserver* p_icco = nullptr;
                    if (s_OK != pIHEC->GetNodeEventContext()->QueryInterface(GET_IID(ICampaignCostObserver), (void**)&p_icco))
                    {
                        throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "pIHEC->GetNodeEventContext()", "ICampaignCostObserver", "INodeEventContext" );
                    }

                    di->AddRef();
                    di->Distribute( pIHEC->GetInterventionsContext(), p_icco );
                    di->Release();
                }
            }

            std::stringstream ss;
            ss << "UpdateNodes() gave out " << individual_list.size() << " '" << m_InterventionName.c_str() << "' interventions\n";
            LOG_INFO( ss.str().c_str() );
        }

        // ----------------------------------------------------
        // --- Determine if finished distributing interventions
        // ----------------------------------------------------
        m_IsFinished = m_TargetedDistributionList.IsFinished(  m_Parent->GetSimulationTime(), dt );
    }

    bool NChooserEventCoordinator::IsFinished()
    {
        return m_IsFinished;
    }
}

