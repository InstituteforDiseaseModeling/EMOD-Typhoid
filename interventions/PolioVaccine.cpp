/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "PolioVaccine.h"

#include "Common.h"
#include "Contexts.h"
#include "InterventionEnums.h"
#include "RANDOM.h"

static const char * _module = "PolioVaccine";

namespace Kernel
{

    ///////////////////////////////////////////////////////////////////////////////////////
    IMPLEMENT_FACTORY_REGISTERED(PolioVaccine)

    bool
    PolioVaccine::Configure(
        const Configuration * inputJson
    )
    {
        initConfigTypeMap( "Cost_To_Consumer", &cost_per_unit, PV_Cost_To_Consumer_DESC_TEXT, 0.0, 100.0, 10.0 );
        initConfigTypeMap( "Time_Since_Vaccination", &time_since_vaccination, PV_Time_Since_Vaccination_DESC_TEXT, 0.0, 100.0, 0.0 );
        JsonConfigurable::Configure( inputJson );
        initConfig( "Vaccine_Type", vaccine_type, inputJson, MetadataDescriptor::Enum("vaccine_type", PV_Vaccine_Type_DESC_TEXT, MDD_ENUM_ARGS(PolioVaccineType)) );
        return true;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    PolioVaccine::PolioVaccine()
    : time_since_vaccination(0)
    {
        LOG_DEBUG("PolioVaccine ctor\n");
        initSimTypes( 1, "POLIO_SIM" );

        // TODO: Hack to stop PV object getting destructed by Configure codepath.
        AddRef();
    }

    PolioVaccine::~PolioVaccine()
    {
        LOG_DEBUG("PolioVaccine dtor\n");
    }

    // For retroactive creation of vaccine history of initial population
    PolioVaccine* PolioVaccine::CreateVaccine(PolioVaccineType::Enum type, float days_since_vaccine)
    {
        PolioVaccine *vac = _new_ PolioVaccine();
        vac->vaccine_type = type;
        vac->time_since_vaccination = days_since_vaccine;

        return vac;
    }


    bool PolioVaccine::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * const pCCO
    )
    {
        if (s_OK != context->QueryInterface(GET_IID(IVaccineConsumer), (void**)&ivc) )
        {
            // just add in another vaccine to list, can later check the person's records and apply accordingly (TODO)
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IVaccineConsumer", "IIndividualHumanInterventionsContext" );
        } 

        return BaseIntervention::Distribute( context, pCCO );
    }

    // IPolioVaccine
    PolioVaccineType::Enum
    PolioVaccine::GetVaccineType()
    const
    {
        return vaccine_type;
    }

    void PolioVaccine::Update( float dt )
    {
        time_since_vaccination += dt;
    } 

    Kernel::QueryResult PolioVaccine::QueryInterface( iid_t iid, void **ppinstance )
    {
        assert(ppinstance);

        if ( !ppinstance )
            return e_NULL_POINTER;

        ISupports* foundInterface;

        /*if ( iid == GET_IID(IVaccine)) 
            foundInterface = static_cast<IVaccine*>(this); */
        if ( iid == GET_IID(IPolioVaccine))
            foundInterface = static_cast<IPolioVaccine*>(this);
        /*else if ( iid == GET_IID(ISupports)) 
            foundInterface = static_cast<ISupports*>(static_cast<IVaccine*>(this));*/
        else if ( iid == GET_IID(IBaseIntervention)) 
            foundInterface = static_cast<ISupports*>(static_cast<IBaseIntervention*>(this));
        else if ( iid == GET_IID(IConfigurable)) 
            foundInterface = static_cast<ISupports*>(static_cast<IConfigurable*>(this));
        else if ( iid == GET_IID(IDistributableIntervention)) 
            foundInterface = static_cast<IDistributableIntervention*>(this); 
        else
            foundInterface = 0;

        QueryResult status;
        if ( !foundInterface )
        {
            status = e_NOINTERFACE;
        }
        else
        {
            foundInterface->AddRef();           // not implementing this yet!
            status = s_OK;
        }

        *ppinstance = foundInterface;
        return status;
    }

    REGISTER_SERIALIZABLE(PolioVaccine, IDistributableIntervention);

    void PolioVaccine::serialize(IArchive& ar, IDistributableIntervention* obj)
    {
        PolioVaccine& vaccine = *dynamic_cast<PolioVaccine*>(obj);
        ar.startElement();
        ar.labelElement("vaccine_type") & (uint32_t&)vaccine.vaccine_type;
        ar.labelElement("time_since_vaccination") & vaccine.time_since_vaccination;
        ar.endElement();
    }
}

// TODO: Consolidate with main serialization block
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::PolioVaccine)

namespace Kernel {
    REGISTER_SERIALIZATION_VOID_CAST(PolioVaccine, IDistributableIntervention)
    template<class Archive>
    void serialize(Archive &ar, PolioVaccine& vacc, const unsigned int v)
    {
        boost::serialization::void_cast_register<PolioVaccine, IDistributableIntervention>();
        ar & vacc.vaccine_type;
        ar & vacc.time_since_vaccination;
        ar & boost::serialization::base_object<BaseIntervention>(vacc);
    }
}

#endif

#endif // ENABLE_POLIO
