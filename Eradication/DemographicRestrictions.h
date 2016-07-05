/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <map>
#include <vector>

#include "BoostLibWrapper.h"
#include "Configure.h"
#include "InterventionEnums.h"
#include "PropertyRestrictions.h"

namespace Kernel
{
    struct IIndividualHumanEventContext;

    class DemographicRestrictions
    {
    public:
        DemographicRestrictions();
        DemographicRestrictions( bool age_restrictions, TargetDemographicType::Enum defaultTargetDemographic );
        virtual ~DemographicRestrictions() { } 

        void ConfigureRestrictions( JsonConfigurable* pParent, const Configuration * inputJson );
        void CheckConfiguration();
        bool HasDefaultRestrictions() const;
        bool IsQualified( const IIndividualHumanEventContext* pIndividual );

        void SetDemographicCoverage( float coverage );
        float GetDemographicCoverage() const;
        TargetDemographicType::Enum GetTargetDemographic() const;
        TargetGender::Enum GetTargetGender() const;
        float GetMinimumAge() const;
        float GetMaximumAge() const;

        std::string GetPropertyRestrictionsAsString() const;

    protected:
        bool allow_age_restrictions;
        float demographic_coverage;
        TargetDemographicType::Enum default_target_demographic;
        TargetDemographicType::Enum target_demographic;
        float target_age_min;
        float target_age_max;
        TargetGender::Enum target_gender; 
        jsonConfigurable::tDynamicStringSet property_restrictions_set;
        PropertyRestrictions property_restrictions;
        bool target_residents_only;

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
        friend void serialize(Archive &ar, DemographicRestrictions &ec, const unsigned int v);
#endif
    };
}
