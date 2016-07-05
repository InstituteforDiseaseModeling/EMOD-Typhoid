/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "stdafx.h"
#include <vector>
#include <string>
#include <fstream>

#ifdef __GNUC__
#include <ext/hash_map>
namespace std
{
     using namespace __gnu_cxx;
}
#else
#include <hash_map>
#endif

#include "IMigrationInfo.h"
#include "InterpolatedValueMap.h"

#define MAX_LOCAL_MIGRATION_DESTINATIONS    (8)
#define MAX_AIR_MIGRATION_DESTINATIONS      (60)
#define MAX_REGIONAL_MIGRATION_DESTINATIONS (30)
#define MAX_SEA_MIGRATION_DESTINATIONS      (5)



namespace Kernel
{
    ENUM_DEFINE(GenderDataType,
        ENUM_VALUE_SPEC(SAME_FOR_BOTH_GENDERS , 0)  // The one set of the data is used for both genders
        ENUM_VALUE_SPEC(ONE_FOR_EACH_GENDER   , 1)) // There are two sets of data - one for each gender

    ENUM_DEFINE(InterpolationType,
        ENUM_VALUE_SPEC(LINEAR_INTERPOLATION , 0)  // Interpolate between ages - no extrapolation
        ENUM_VALUE_SPEC(PIECEWISE_CONSTANT   , 1)) // Use the value if the age is greater than the current and less than the next

    // ---------------------------
    // --- MigrationRateData
    // ---------------------------

    // MigrationRateData contains data about migrating to a particular node.
    // The rates can be age dependent.
    class IDMAPI MigrationRateData
    {
    public:
        MigrationRateData();
        MigrationRateData( suids::suid to_node_suid, MigrationType::Enum migType, InterpolationType::Enum interpType );

        const suids::suid       GetToNodeSuid()           const;
        MigrationType::Enum     GetMigrationType()        const;
        InterpolationType::Enum GetInterpolationType()    const;
        int                     GetNumRates()             const;
        float                   GetRate( float ageYears ) const;

        void AddRate( float ageYears, float rate );
    private:
        suids::suid             m_ToNodeSuid ;
        MigrationType::Enum     m_MigType ;
        InterpolationType::Enum m_InterpType ;
        InterpolatedValueMap    m_InterpMap;
    };

    // ---------------------------
    // --- MigrationInfoNull
    // ---------------------------

    // MigrationInfoNull is the null object in the Null Object Pattern.
    // Essentially, this object implements the IMigrationInfo interface
    // but doesn't do anything.  This object is given to nodes when migration
    // is on but the node does not have any migration away from it.
    class IDMAPI MigrationInfoNull : virtual public IMigrationInfo
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
    public:
        virtual ~MigrationInfoNull();

        // IMigrationInfo methods
        virtual void PickMigrationStep( IIndividualHumanContext * traveler, 
                                        float migration_rate_modifier, 
                                        suids::suid &destination, 
                                        MigrationType::Enum &migration_type,
                                        float &time ) override;
        virtual void SetContextTo(INodeContext* _parent) override;
        virtual const std::vector<suids::suid>& GetReachableNodes() const override;
        virtual const std::vector<MigrationType::Enum>& GetMigrationTypes() const override;
        virtual bool IsHeterogeneityEnabled() const override;

    protected:
        friend class MigrationInfoFactoryFile;

        MigrationInfoNull();

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        // We need these member variables so that GetReachableNodes() and GetMigrationTypes()
        // can retrun references to objects that exist.
        std::vector<suids::suid>         m_EmptyListNodes;
        std::vector<MigrationType::Enum> m_EmptyListTypes;
#pragma warning( pop )
    };

    // ---------------------------
    // --- MigrationInfoFixedRate
    // ---------------------------

    // MigrationInfoFixedRate is an IMigrationInfo object that has fixed/constant rates.
    class IDMAPI MigrationInfoFixedRate : virtual public IMigrationInfo
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
    public:
        virtual ~MigrationInfoFixedRate();

        // IMigrationInfo methods
        virtual void PickMigrationStep( IIndividualHumanContext * traveler, 
                                        float migration_rate_modifier, 
                                        suids::suid &destination, 
                                        MigrationType::Enum &migration_type,
                                        float &time ) override;
        virtual void SetContextTo(INodeContext* _parent) override;
        virtual const std::vector<suids::suid>& GetReachableNodes() const override;
        virtual const std::vector<MigrationType::Enum>& GetMigrationTypes() const override;
        virtual bool IsHeterogeneityEnabled() const override;

    protected:
        friend class MigrationInfoFactoryFile;
        friend class MigrationInfoFactoryDefault;

        MigrationInfoFixedRate( INodeContext* _parent,
                                bool isHeterogeneityEnabled );

        virtual void Initialize( const std::vector<std::vector<MigrationRateData>>& rRateData );
        virtual void CalculateRates( Gender::Enum gender, float ageYears );
        virtual const std::vector<float>& GetRates() const;
        virtual float GetTotalRate() const;
        virtual void NormalizeRates( std::vector<float>& r_rate_cdf, float& r_total_rate );
        virtual void SaveRawRates( std::vector<float>& r_rate_cdf ) {}

        virtual const std::vector<suids::suid>& GetReachableNodes( Gender::Enum gender ) const;
        virtual const std::vector<MigrationType::Enum>& GetMigrationTypes( Gender::Enum gender ) const;

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        INodeContext * m_Parent;
        bool m_IsHeterogeneityEnabled;
        std::vector<suids::suid>         m_ReachableNodes;
        std::vector<MigrationType::Enum> m_MigrationTypes;
        std::vector<float>               m_RateCDF;
        float                            m_TotalRate;
#pragma warning( pop )
    };

    // -----------------------------
    // --- MigrationInfoAgeAndGender
    // -----------------------------

    // MigrationInfoAgeAndGender is an IMigrationInfo object that is used when the rates
    // between nodes is dependent on gender and age.  The reachable/"to nodes" can be different
    // for each gender.
    class IDMAPI MigrationInfoAgeAndGender : public MigrationInfoFixedRate
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
    public:
        virtual ~MigrationInfoAgeAndGender();

    protected:
        friend class MigrationInfoFactoryFile;

        MigrationInfoAgeAndGender( INodeContext* _parent,
                                   bool isHeterogeneityEnabled );

        virtual void Initialize( const std::vector<std::vector<MigrationRateData>>& rRateData );
        virtual void CalculateRates( Gender::Enum gender, float ageYears );

        virtual const std::vector<suids::suid>& GetReachableNodes( Gender::Enum gender ) const override;
        virtual const std::vector<MigrationType::Enum>& GetMigrationTypes( Gender::Enum gender ) const override;


#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        std::vector<std::vector<MigrationRateData>> m_RateData;
        std::vector<suids::suid>         m_ReachableNodesFemale;
        std::vector<MigrationType::Enum> m_MigrationTypesFemale;
#pragma warning( pop )
    };

    // ----------------------
    // --- MigrationInfoFile
    // ----------------------

    // MigrationInfoFile is responsible for reading the migration data from files.
    // This object assumes that for one file name there is one binary file containing the
    // "to-node" and rate data while json-formatted metadata file contains extra information
    // about the data in the binary file.  The factory uses this object to create
    // the IMigrationInfo objects.
    class IDMAPI MigrationInfoFile
    {
    public:
#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        // These are public so that the factory can put these variables into initConfig() statements
        std::string m_Filename ;
        bool m_IsEnabled ;
        float m_xModifier ;
#pragma warning( pop )

        MigrationInfoFile( MigrationType::Enum migType, 
                           int defaultDestinationsPerNode );
        virtual ~MigrationInfoFile();

        virtual void Initialize( const std::string& idreference );

        virtual void SetEnableParameterName( const std::string& rName );
        virtual void SetFilenameParameterName( const std::string& rName );

        virtual bool ReadData( ExternalNodeId_t fromNodeID, 
                               const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeidSuidMap,
                               std::vector<std::vector<MigrationRateData>>& rRateData );

        MigrationType::Enum GetMigrationType() const { return m_MigrationType; }

    protected:
        // Returns the expected size of the binary file
        virtual uint32_t ParseMetadataForFile( const std::string& data_filepath, const std::string& idreference );
        virtual void OpenMigrationFile( const std::string& filepath, uint32_t expected_binary_file_size );
        virtual uint32_t GetNumGenderDataChunks() const;

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        std::string             m_ParameterNameEnable ;
        std::string             m_ParameterNameFilename ;
        int                     m_DestinationsPerNode ;
        MigrationType::Enum     m_MigrationType ;
        GenderDataType::Enum    m_GenderDataType;
        InterpolationType::Enum m_InterpolationType;
        std::vector<float>      m_AgesYears;
        uint32_t                m_GenderDataSize;
        uint32_t                m_AgeDataSize;
        std::ifstream           m_FileStream;

        std::hash_map< ExternalNodeId_t, uint32_t > m_Offsets;
#pragma warning( pop )
    };

    // ----------------------------------
    // --- MigrationInfoFactoryFile
    // ----------------------------------

    // MigrationInfoFactoryFile is an IMigrationInfoFactory that creates IMigrationInfo objects based
    // on data found in migration input files.  It can create one IMigrationInfo object for each node
    // in the simulation.
    class IDMAPI MigrationInfoFactoryFile : public JsonConfigurable, virtual public IMigrationInfoFactory
    {
    public:
        // for JsonConfigurable stuff...
        GET_SCHEMA_STATIC_WRAPPER(MigrationInfoFactoryFile)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()

        MigrationInfoFactoryFile( bool enableHumanMigration );
        MigrationInfoFactoryFile();
        virtual ~MigrationInfoFactoryFile();

        // JsonConfigurable methods
        virtual bool Configure( const Configuration* config ) override;

        // IMigrationInfoFactory methods
        virtual void Initialize( const ::Configuration *config, const std::string& idreference ) override;
        virtual bool IsAtLeastOneTypeConfiguredForIndividuals() const override;
        virtual bool IsEnabled( MigrationType::Enum mt ) const override;

        virtual IMigrationInfo* CreateMigrationInfo( INodeContext *parent_node, 
                                                     const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeIdSuidMap ) override;
    protected:
        virtual void CreateInfoFileList();
        virtual void InitializeInfoFileList( bool enableHumanMigration, const Configuration* config );
        static std::vector<std::vector<MigrationRateData>> GetRateData( INodeContext *parent_node, 
                                                                        const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeIdSuidMap,
                                                                        std::vector<MigrationInfoFile*>& infoFileList,
                                                                        bool* pIsFixedRate );

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        std::vector<MigrationInfoFile*> m_InfoFileList ;
        bool m_IsHeterogeneityEnabled;
        bool m_EnableHumanMigration;
#pragma warning( pop )
    private:
    };

    // ----------------------------------
    // --- MigrationInfoFactoryDefault
    // ----------------------------------

    // MigrationInfoFactoryDefault is used when the user is running the default/internal scenario.
    // This assumes that there are at least 3-rows and 3-columns of nodes and that the set of nodes is square.
    class IDMAPI MigrationInfoFactoryDefault : public JsonConfigurable, virtual public IMigrationInfoFactory
    {
    public:
        // for JsonConfigurable stuff...
        GET_SCHEMA_STATIC_WRAPPER(MigrationInfoFactoryDefault)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()

        MigrationInfoFactoryDefault( bool enableHumanMigration, int torusSize );
        MigrationInfoFactoryDefault();
        virtual ~MigrationInfoFactoryDefault();

        // JsonConfigurable methods
        virtual bool Configure( const Configuration* config ) override;

        // IMigrationInfoFactory methods
        virtual void Initialize( const ::Configuration *config, const std::string& idreference ) override;
        virtual IMigrationInfo* CreateMigrationInfo( INodeContext *parent_node, 
                                                     const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeIdSuidMap ) override;
        virtual bool IsAtLeastOneTypeConfiguredForIndividuals() const override;
        virtual bool IsEnabled( MigrationType::Enum mt ) const override;
    protected:
        std::vector<std::vector<MigrationRateData>> GetRateData( INodeContext *parent_node, 
                                                                 const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeIdSuidMap,
                                                                 float modifier );

#pragma warning( push )
#pragma warning( disable: 4251 ) // See IdmApi.h for details
        bool  m_IsHeterogeneityEnabled;
        float m_xLocalModifier;
        int   m_TorusSize;
#pragma warning( pop )
    private:
        void InitializeParameters( bool enableHumanMigration ); // just used in multiple constructors
    };
}
