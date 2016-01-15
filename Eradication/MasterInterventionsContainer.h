/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <string>
#include <list>
#include <vector>

#include "Drugs.h"
#include "Interventions.h"
#include "InterventionsContainer.h"

namespace Kernel
{
    // this container has the necessary other interventionscontainers

    class IMasterInterventionsContainer : public ISupports
    {
        virtual bool GiveIntervention( IDistributableIntervention * pIV ) = 0;
    };

    class MasterInterventionsContainer : public InterventionsContainer,
        public IMasterInterventionsContainer

    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        MasterInterventionsContainer();
        virtual ~MasterInterventionsContainer();

        // IIndividualHumanInterventionsContext
        virtual void SetContextTo(IIndividualHumanContext* context);
        virtual IIndividualHumanContext* GetParent();
        virtual std::list<IDistributableIntervention*> GetInterventionsByType(const std::string& type_name);
        virtual void PurgeExisting( const std::string& iv_name );

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void** pinstance);

        // IVaccineConsumer
        virtual void UpdateVaccineAcquireRate( float acq );
        virtual void UpdateVaccineTransmitRate( float xmit );
        virtual void UpdateVaccineMortalityRate( float mort );

        // IDrugVaccineInterventionEffects
        virtual float GetInterventionReducedAcquire();
        virtual float GetInterventionReducedTransmit();
        virtual float GetInterventionReducedMortality();

 
        virtual bool GiveIntervention( IDistributableIntervention * pIV );

        virtual void Update(float dt);
    private:
        std::list <InterventionsContainer* > InterventionsContainerList;
    };
}

#if 0
namespace Kernel {
    template<class Archive>
    void serialize(Archive &ar, MasterInterventionsContainer& container, const unsigned int v)
    {
        ar & boost::serialization::base_object<InterventionsContainer>(container);
    }
}
#endif
