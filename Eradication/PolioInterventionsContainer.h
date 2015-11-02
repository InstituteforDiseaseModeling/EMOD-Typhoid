/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/
#pragma once

#ifdef ENABLE_POLIO
#include <string>
#include <list>
#include <vector>

#include "Drugs.h"
#include "Interventions.h"
#include "InterventionsContainer.h"
#include "SimpleTypemapRegistration.h"

namespace Kernel
{
    // this container becomes a help implementation member of the relevant IndividualHuman class 
    // it needs to implement consumer interfaces for all the relevant intervention types
    struct IPolioDrugEffectsApply : public ISupports
    {
        virtual void ApplyDrugVaccineReducedAcquireEffect( float rate ) = 0;
        virtual void ApplyDrugVaccineReducedTransmitEffect( float rate ) = 0;
        virtual void ApplyDrugTiterEffect( float rate ) = 0;
        virtual void ApplyDrugDurationEffect( float rate ) = 0;
    };

    struct IPolioVaccine;

    class PolioInterventionsContainer : public InterventionsContainer,
                                        public IPolioVaccineEffects,
                                        public IPolioDrugEffects,
                                        IPolioDrugEffectsApply
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        PolioInterventionsContainer();
        virtual ~PolioInterventionsContainer();

        virtual QueryResult QueryInterface(iid_t iid, void** pinstance);

        // IVaccineConsumer: not any more!
        virtual bool GiveIntervention( IDistributableIntervention * pIV );

        // IPolioVaccineEffects
        virtual std::list<IPolioVaccine*>& GetNewVaccines();
        virtual void ClearNewVaccines();

        // IPolioDrugEffectsApply
        virtual void ApplyDrugTiterEffect( float rate );
        virtual void ApplyDrugDurationEffect( float rate );
        virtual void ApplyDrugVaccineReducedAcquireEffect( float rate ); // not used for anything
        virtual void ApplyDrugVaccineReducedTransmitEffect( float rate ); // not used for anything

        //IPolioDrugEffects(Get)
        virtual float get_titer_efficacy() const;
        virtual float get_infection_duration_efficacy() const;

        virtual void Update(float dt); // example of intervention timestep update

    protected:
        std::list<IPolioVaccine*> new_vaccines;
        void GiveDrug(IDrug* drug);
        float titer_efficacy;
        float infection_duration_efficacy;

        DECLARE_SERIALIZABLE(PolioInterventionsContainer, IIndividualHumanInterventionsContext);

    private:
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
        friend class ::boost::serialization::access;
        template<class Archive>
        void serialize(Archive &ar, PolioInterventionsContainer& cont, const unsigned int v);
#endif
    };
}
#endif