/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Vaccine.h"

#include "Common.h"                  // for INFINITE_TIME
#include "Contexts.h"                // for IIndividualHumanContext, IIndividualHumanInterventionsContext
#include "InterventionsContainer.h"  // for IVaccineConsumer methods
#include "InterventionEnums.h"
#include "RANDOM.h"                  // for ApplyVaccineTake random draw

// TBD: currently included for JDeserialize only. Once we figure out how to wrap the deserialize
// into rapidjsonimpl class, then this is not needed
#include "RapidJsonImpl.h"

static const char* _module = "SimpleVaccine";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_BODY(SimpleVaccine)
        HANDLE_INTERFACE(IConfigurable)
        HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_INTERFACE(IVaccine)
        HANDLE_INTERFACE(IBaseIntervention)
        HANDLE_ISUPPORTS_VIA(IDistributableIntervention)
        HANDLE_INTERFACE(IBaseIntervention)
    END_QUERY_INTERFACE_BODY(SimpleVaccine)

    IMPLEMENT_FACTORY_REGISTERED(SimpleVaccine)

    bool
    SimpleVaccine::Configure(
        const Configuration * inputJson
    )
    {
        initConfigTypeMap("Vaccine_Take", &vaccine_take, SV_Vaccine_Take_DESC_TEXT, 0.0, 1.0, 1.0 );
        initConfig( "Durability_Time_Profile", durability_time_profile, inputJson, MetadataDescriptor::Enum("Durability_Time_Profile", SV_Durability_Time_Profile_DESC_TEXT, MDD_ENUM_ARGS(InterventionDurabilityProfile) ) );
        if ( durability_time_profile == InterventionDurabilityProfile::BOXDECAYDURABILITY || JsonConfigurable::_dryrun )
        {
            initConfigTypeMap("Secondary_Decay_Time_Constant", &secondary_decay_time_constant, SV_Secondary_Decay_Time_Constant_DESC_TEXT, 0, INFINITE_TIME);
        }

        initConfig( "Vaccine_Type", vaccine_type, inputJson, MetadataDescriptor::Enum("Vaccine_Type", SV_Vaccine_Type_DESC_TEXT, MDD_ENUM_ARGS(SimpleVaccineType)));

        if ( vaccine_type == SimpleVaccineType::AcquisitionBlocking )
        {
            initConfigTypeMap("Reduced_Acquire", &current_reducedacquire, SV_Reduced_Acquire_DESC_TEXT, 0.0, 1.0, 1.0 );
        }
        else if (vaccine_type == SimpleVaccineType::TransmissionBlocking )
        {
            initConfigTypeMap("Reduced_Transmit", &current_reducedtransmit, SV_Reduced_Transmit_DESC_TEXT, 0.0, 1.0, 1.0 );
        }
        else if (vaccine_type == SimpleVaccineType::MortalityBlocking )
        {
            initConfigTypeMap("Reduced_Mortality", &current_reducedmortality, SV_Reduced_Mortality_DESC_TEXT, 0.0, 1.0, 1.0 );
        }
        else // SimpleVaccineType::Generic
        {
            initConfigTypeMap("Reduced_Acquire", &current_reducedacquire, SV_Reduced_Acquire_DESC_TEXT, 0.0, 1.0, 1.0 );
            initConfigTypeMap("Reduced_Transmit", &current_reducedtransmit, SV_Reduced_Transmit_DESC_TEXT, 0.0, 1.0, 1.0 );
            initConfigTypeMap("Reduced_Mortality", &current_reducedmortality, SV_Reduced_Mortality_DESC_TEXT, 0.0, 1.0, 1.0 );
        }
        return JsonConfigurable::Configure( inputJson );
    }

    SimpleVaccine::SimpleVaccine()
    : parent(nullptr)
    , vaccine_type(SimpleVaccineType::Generic)
    , current_reducedtransmit(0.0)
    , current_reducedacquire(0.0)
    , current_reducedmortality(0.0)
    , secondary_decay_time_constant(0.0)
    {
        initConfigTypeMap("Cost_To_Consumer", &cost_per_unit, SV_Cost_To_Consumer_DESC_TEXT, 0, 999999, 10.0);
        initConfigTypeMap("Primary_Decay_Time_Constant", &primary_decay_time_constant, SV_Primary_Decay_Time_Constant_DESC_TEXT, 0, INFINITE_TIME);
    }

    SimpleVaccine::~SimpleVaccine() { }

    /////////////////////////////////////////////////////////////////////////////////////

    int
    SimpleVaccine::GetVaccineType()
    const
    {
        return vaccine_type;
    }

    // context is nothing more than ISupports really, and it's a pointer to the individual's
    // intervention container, not the individual itself. It was gotten by a call to
    // pIndividual->GetInterventionsContext().

    bool
    SimpleVaccine::Distribute(
        IIndividualHumanInterventionsContext *context,
        ICampaignCostObserver * pCCO
    )
    {
        // store ivc for apply
        LOG_DEBUG("Distributing SimpleVaccine.\n");
        if (s_OK != context->QueryInterface(GET_IID(IVaccineConsumer), (void**)&ivc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IVaccineConsumer", "IIndividualHumanInterventionsContext" );
        }
        bool success = BaseIntervention::Distribute( context, pCCO );
        ApplyVaccineTake();
        return success;
    }


    void SimpleVaccine::Update( float dt )
    {
        if(durability_time_profile == InterventionDurabilityProfile::BOXDECAYDURABILITY)
        {
            if(primary_decay_time_constant > 0)
            {
                primary_decay_time_constant -= dt;
            }
            else
            {
                if(secondary_decay_time_constant > dt)
                {
                    current_reducedacquire   *= (1-dt/secondary_decay_time_constant);
                    current_reducedtransmit  *= (1-dt/secondary_decay_time_constant);
                    current_reducedmortality *= (1-dt/secondary_decay_time_constant);
                }
                else
                {
                    current_reducedacquire   = 0;
                    current_reducedtransmit  = 0;
                    current_reducedmortality = 0;
                }
            }
        }
        else if(durability_time_profile == InterventionDurabilityProfile::DECAYDURABILITY)
        {
            if(primary_decay_time_constant > dt)
            {
                current_reducedacquire   *= (1-dt/primary_decay_time_constant);
                current_reducedtransmit  *= (1-dt/primary_decay_time_constant);
                current_reducedmortality *= (1-dt/primary_decay_time_constant);
            }
            else
            {
                current_reducedacquire   = 0;
                current_reducedtransmit  = 0;
                current_reducedmortality = 0;
            }
        }
        else if(durability_time_profile == InterventionDurabilityProfile::BOXDURABILITY)
        {
            primary_decay_time_constant -= dt;
            if(primary_decay_time_constant < 0)
            {
                current_reducedacquire   = 0;
                current_reducedtransmit  = 0;
                current_reducedmortality = 0;
            }
        }
        assert(ivc);
        ivc->UpdateVaccineAcquireRate( current_reducedacquire );
        ivc->UpdateVaccineTransmitRate( current_reducedtransmit );
        ivc->UpdateVaccineMortalityRate( current_reducedmortality );
    }

/*
    Kernel::QueryResult SimpleVaccine::QueryInterface( iid_t iid, void **ppinstance )
    {
        assert(ppinstance);

        if ( !ppinstance )
            return e_NULL_POINTER;

        ISupports* foundInterface;

        if ( iid == GET_IID(IVaccine))
            foundInterface = static_cast<IVaccine*>(this);
        else if ( iid == GET_IID(ISupports))
            foundInterface = static_cast<ISupports*>(static_cast<IVaccine*>(this));
        else
            foundInterface = 0;

        QueryResult status;
        if ( !foundInterface )
            status = e_NOINTERFACE;
        else
        {
            //foundInterface->AddRef();           // not implementing this yet!
            status = s_OK;
        }

        *ppinstance = foundInterface;
        return status;
    }*/

    void SimpleVaccine::ApplyVaccineTake()
    {
        if(parent)
        {
            if(vaccine_take<1.0)
            {
                if(parent->GetRng()->e()>vaccine_take)
                {
                    LOG_DEBUG("Vaccine did not take.\n");
                    current_reducedacquire = 0.0;
                    current_reducedtransmit = 0.0;
                    current_reducedmortality = 0.0;
                }
            }
        }
    }

    void SimpleVaccine::SetContextTo(
        IIndividualHumanContext *context
    )
    {
        parent = context;
        if (s_OK != parent->GetInterventionsContext()->QueryInterface(GET_IID(IVaccineConsumer), (void**)&ivc) )
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context->GetInterventionsContext()", "IVaccineConsumer", "IIndividualHumanInterventionsContext" );
        }
    } // needed for VaccineTake

    REGISTER_SERIALIZABLE(SimpleVaccine);

    void SimpleVaccine::serialize(IArchive& ar, ISerializable* obj)
    {
        SimpleVaccine& vaccine = *dynamic_cast<SimpleVaccine*>(obj);
        ar.startElement();
        ar.labelElement("vaccine_type")                  & vaccine.vaccine_type;
        ar.labelElement("vaccine_take")                  & vaccine.vaccine_take;
        ar.labelElement("current_reducedacquire")        & vaccine.current_reducedacquire;
        ar.labelElement("current_reducedtransmit")       & vaccine.current_reducedtransmit;
        ar.labelElement("current_reducedmortality")      & vaccine.current_reducedmortality;
        ar.labelElement("durability_time_profile")       & (uint32_t&)vaccine.durability_time_profile;
        ar.labelElement("primary_decay_time_constant")   & vaccine.primary_decay_time_constant;
        ar.labelElement("secondary_decay_time_constant") & vaccine.secondary_decay_time_constant;
        ar.endElement();
    }
}

// TODO: want to get these out of standalone (back with serialization bundle) but in polymorphic
// cases, only 1 can exist with multiple includes.
#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI

// SERIALIZATION BLOCK
BOOST_CLASS_EXPORT_IMPLEMENT(Kernel::SimpleVaccine)

namespace Kernel {

/*    template<class Archive>
    void serialize(Archive &ar, SimpleVaccine& vacc, const unsigned int v)
    {
      LOG_ERR("(De)serializing SimpleVaccine\n");

        boost::serialization::void_cast_register<SimpleVaccine, IDistributableIntervention>();
        ar & boost::serialization::base_object<BaseIntervention>(vacc);
        ar& vacc.vaccine_type;
        ar& vacc.vaccine_take;
        ar& vacc.current_reducedacquire;
        ar& vacc.current_reducedtransmit;
        ar& vacc.current_reducedmortality;
        ar& vacc.durability_time_profile;
        ar& vacc.primary_decay_time_constant;
        ar& vacc.secondary_decay_time_constant;
    }
    template void serialize( boost::mpi::detail::mpi_datatype_oarchive&, Kernel::SimpleVaccine&, unsigned int);
    template void serialize( boost::mpi::detail::content_oarchive&, Kernel::SimpleVaccine&, unsigned int);
    template void serialize( boost::mpi::packed_skeleton_oarchive&, Kernel::SimpleVaccine&, unsigned int);
    template void serialize( boost::mpi::packed_iarchive&, Kernel::SimpleVaccine&, unsigned int);
    template void serialize( boost::archive::binary_iarchive&, Kernel::SimpleVaccine&, unsigned int);
    template void serialize( boost::mpi::packed_skeleton_iarchive&, Kernel::SimpleVaccine&, unsigned int);
*/
        template<typename Archive>
        void serialize( Archive &ar, SimpleVaccine& vaccine, const unsigned int version )
        {
            LOG_DEBUG("serialize(Archive&, SimpleVaccine&, unsigned int)\n");
            boost::serialization::split_member(ar, vaccine, version);
        }

/*        template void serialize(boost::mpi::packed_oarchive&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::mpi::detail::content_oarchive&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::mpi::packed_skeleton_oarchive&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::archive::binary_oarchive&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::mpi::detail::mpi_datatype_oarchive&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::mpi::packed_iarchive&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::mpi::packed_skeleton_iarchive&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::archive::binary_iarchive&, Kernel::SimpleVaccine&, unsigned int);

        template void serialize(boost::mpi::detail::ignore_skeleton_oarchive<boost::mpi::detail::mpi_datatype_oarchive>&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::mpi::detail::forward_skeleton_iarchive<boost::mpi::packed_skeleton_iarchive, boost::mpi::packed_iarchive>&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::mpi::detail::ignore_skeleton_oarchive<boost::mpi::detail::content_oarchive>&, Kernel::SimpleVaccine&, unsigned int);
        template void serialize(boost::mpi::detail::forward_skeleton_oarchive<boost::mpi::packed_skeleton_oarchive, boost::mpi::packed_oarchive>&, Kernel::SimpleVaccine&, unsigned int);
*/
#include "Templates.h"
DECLARE_TEMPLATES(Kernel::SimpleVaccine)

        template<class Archive>
        void SimpleVaccine::save(Archive& ar, const unsigned int version) const
        {
            LOG_DEBUG("Serializing SimpleVaccine\n");

// clorton www.boost.org/doc/libs/1_58_0/libs/serialization/doc/serializatin.html says we only need void_cast_register
// clorton if we _aren't_ calling into the base class serializer.
            boost::serialization::void_cast_register<SimpleVaccine, IDistributableIntervention>();
            ar & boost::serialization::base_object<BaseIntervention>(*this);
            ar & vaccine_type;
            ar & vaccine_take;
            ar & current_reducedacquire;
            ar & current_reducedtransmit;
            ar & current_reducedmortality;
            ar & durability_time_profile;
            ar & primary_decay_time_constant;
            ar & secondary_decay_time_constant;
        }

        template<class Archive>
        void SimpleVaccine::load(Archive& ar, const unsigned int version)
        {
          LOG_DEBUG("De-serializing SimpleVaccine\n");

// clorton www.boost.org/doc/libs/1_58_0/libs/serialization/doc/serializatin.html says we only need void_cast_register
// clorton if we _aren't_ calling into the base class serializer.
          boost::serialization::void_cast_register<SimpleVaccine, IDistributableIntervention>();
          ar & boost::serialization::base_object<BaseIntervention>(*this);
          ar & vaccine_type;
          ar & vaccine_take;
          ar & current_reducedacquire;
          ar & current_reducedtransmit;
          ar & current_reducedmortality;
          ar & durability_time_profile;
          ar & primary_decay_time_constant;
          ar & secondary_decay_time_constant;
        }
}
#endif
