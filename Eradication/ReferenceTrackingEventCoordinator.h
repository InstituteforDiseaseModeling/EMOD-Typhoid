/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "StandardEventCoordinator.h"
#include "InterpolatedValueMap.h"
#include "Types.h"

namespace Kernel
{
    class ReferenceTrackingEventCoordinator : public StandardInterventionDistributionEventCoordinator 
    {
        DECLARE_FACTORY_REGISTERED_EXPORT(EventCoordinatorFactory, ReferenceTrackingEventCoordinator, IEventCoordinator)    

    public:
        DECLARE_CONFIGURED(ReferenceTrackingEventCoordinator)
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        ReferenceTrackingEventCoordinator();
        virtual ~ReferenceTrackingEventCoordinator() { } 

        virtual void Update(float dt);
        virtual void preDistribute();

#if USE_JSON_SERIALIZATION
    public:
        // IJsonSerializable Interfaces
        virtual void JSerialize( IJsonObjectAdapter* root, JSerializer* helper ) const;
        virtual void JDeserialize( IJsonObjectAdapter* root, JSerializer* helper );
#endif    
    protected:
        InterpolatedValueMap year2ValueMap;
        Fraction target_coverage;
        float end_year;

#if USE_BOOST_SERIALIZATION
    private:
        friend class ::boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive &ar, StandardInterventionDistributionEventCoordinator &ec, const unsigned int v);
#endif
    };
}
