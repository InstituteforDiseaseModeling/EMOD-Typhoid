/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include "Vaccine.h"

namespace Kernel
{
    class IPolioVaccine : public ISupports
    {
    public:
        virtual PolioVaccineType::Enum   GetVaccineType()            const = 0;
    };

    class PolioVaccine : public IVaccine, public BaseIntervention, public IPolioVaccine
    {
        DECLARE_FACTORY_REGISTERED(InterventionFactory, PolioVaccine, IDistributableIntervention)

    public:
        static PolioVaccine* CreateVaccine(PolioVaccineType::Enum type, float days_since_vaccine);
        PolioVaccine();
        virtual ~PolioVaccine();
        bool Configure( const Configuration* config );
        virtual int AddRef() { return BaseIntervention::AddRef(); }
        virtual int Release() { return BaseIntervention::Release(); }

        // IDistributableIntervention
        virtual bool Distribute(IIndividualHumanInterventionsContext *context, ICampaignCostObserver * const pCCO );
        virtual void SetContextTo(IIndividualHumanContext *context) { /* not needed for this intervention */ }
        virtual void Update(float dt);

        // ISupports
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);

        // IPolioVaccine
        virtual PolioVaccineType::Enum   GetVaccineType()            const;

        // IVaccine
        virtual void  ApplyVaccineTake() {} // Take is handled in live vaccines
        virtual float GetVaccineReducedAcquire()  const { return 0; }
        virtual float GetVaccineReducedTransmit() const { return 0; }

    protected:
        PolioVaccineType::Enum vaccine_type;
        float time_since_vaccination;
        IVaccineConsumer *ivc;

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        template<class Archive>
        friend void serialize(Archive &ar, PolioVaccine& vacc, const unsigned int v);
#endif    
    };
}
