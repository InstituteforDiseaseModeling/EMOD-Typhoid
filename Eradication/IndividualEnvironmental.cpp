/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"

#ifdef ENABLE_POLIO

#include "NodeEnvironmental.h"
#include "IndividualEnvironmental.h"
#include "InfectionEnvironmental.h"
#include "SusceptibilityEnvironmental.h"

#pragma warning(disable: 4244)

static const char* _module = "IndividualEnvironmental";

namespace Kernel
{
    BEGIN_QUERY_INTERFACE_DERIVED(IndividualHumanEnvironmental, IndividualHuman)
    END_QUERY_INTERFACE_DERIVED(IndividualHumanEnvironmental, IndividualHuman)

    IndividualHumanEnvironmental::IndividualHumanEnvironmental( suids::suid _suid, float monte_carlo_weight, float initial_age, int gender, float initial_poverty) :
        IndividualHuman( _suid, monte_carlo_weight, initial_age, gender, initial_poverty)
    {
    }

    IndividualHumanEnvironmental *
    IndividualHumanEnvironmental::CreateHuman(INodeContext *context, suids::suid id, float MCweight, float init_age, int gender, float init_poverty)
    {
        IndividualHumanEnvironmental *newindividual = _new_ IndividualHumanEnvironmental( id, MCweight, init_age, gender, init_poverty);

        newindividual->SetContextTo(context);
        LOG_DEBUG_F( "Created human with age=%f\n", newindividual->m_age );

        return newindividual;
    }

    IndividualHumanEnvironmental::~IndividualHumanEnvironmental()
    {
    }
    
    void IndividualHumanEnvironmental::CreateSusceptibility(float imm_mod, float risk_mod)
    {
        susceptibility = SusceptibilityEnvironmental::CreateSusceptibility(this, m_age, imm_mod, risk_mod);
    }

    void IndividualHumanEnvironmental::ExposeToInfectivity(float dt, const TransmissionGroupMembership_t* transmissionGroupMembership)
    {
        IndividualHuman::ExposeToInfectivity(dt, transmissionGroupMembership);
    }

    void IndividualHumanEnvironmental::ReportInfectionState()
    {
        m_new_infection_state = NewInfectionState::NewInfection;
    }

    void IndividualHumanEnvironmental::UpdateInfectiousness(float dt)
    {
        infectiousness = 0;
        for (auto infection : infections)
        {
            LOG_DEBUG("Getting infectiousness by route.\n");
            float tmp_infectiousnessFecal =  m_mc_weight * infection->GetInfectiousnessByRoute(string("environmental"));
            float tmp_infectiousnessOral = m_mc_weight * infection->GetInfectiousnessByRoute(string("contact"));

            StrainIdentity tmp_strainID;
            infection->GetInfectiousStrainID(&tmp_strainID);
            LOG_DEBUG_F("UpdateInfectiousness: InfectiousnessFecal = %f, InfectiousnessOral = %f.\n", tmp_infectiousnessFecal, tmp_infectiousnessOral);

            //deposit oral to 'contact', fecal to 'environmental' pool
            LOG_DEBUG("Getting routes.\n");

            for(auto& entry : transmissionGroupMembershipByRoute)
            {
                LOG_DEBUG_F("Found route:%s.\n",entry.first.c_str());
                if (entry.first==string("contact"))
                {
                    if (tmp_infectiousnessOral > 0.0f)
                    {
                        LOG_DEBUG_F("Depositing %f to route %s: (antigen=%d, substain=%d)\n", tmp_infectiousnessOral, entry.first.c_str(), tmp_strainID.GetAntigenID(), tmp_strainID.GetGeneticID());
                        parent->DepositFromIndividual(&tmp_strainID, tmp_infectiousnessOral, &entry.second);
                        infectiousness += infection->GetInfectiousnessByRoute(string("contact"));
                    }
                }
                else if (entry.first==string("environmental"))
                {
                    if (tmp_infectiousnessFecal > 0.0f)
                    {
                        LOG_DEBUG_F("Depositing %f to route %s: (antigen=%d, substain=%d)\n", tmp_infectiousnessFecal, entry.first.c_str(), tmp_strainID.GetAntigenID(), tmp_strainID.GetGeneticID());    
                        parent->DepositFromIndividual(&tmp_strainID, tmp_infectiousnessFecal, &entry.second);
                        infectiousness += infection->GetInfectiousnessByRoute(string("environmental"));
                    }
                }
                else
                {
                    LOG_WARN_F("unknown route %s, do not deposit anything.\n", entry.first.c_str());
                }
           }

        }
    }

    Infection* IndividualHumanEnvironmental::createInfection( suids::suid _suid )
    {
        return InfectionEnvironmental::CreateInfection(this, _suid);
    }

    REGISTER_SERIALIZABLE(IndividualHumanEnvironmental);

    void IndividualHumanEnvironmental::serialize(IArchive& ar, IndividualHumanEnvironmental* obj)
    {
        IndividualHuman::serialize(ar, obj);
        /* IndividualHumanEnvironmental doesn't (yet) have any member fields.
        IndividualHumanEnvironmental& individual = *dynamic_cast<IndividualHumanEnvironmental*>(obj);
        ar.startObject();
        ar.endObject();
        */
    }
}

#if USE_BOOST_SERIALIZATION || USE_BOOST_MPI
BOOST_CLASS_EXPORT(Kernel::IndividualHumanEnvironmental)
namespace Kernel
{
    template<class Archive>
    void serialize(Archive & ar, IndividualHumanEnvironmental& human, const unsigned int  file_version )
    {
        static const char * _module = "IndividualHumanEnvironmental";
        LOG_DEBUG("(De)serializing IndividualHumanEnvironmental\n");

        // Register derived types
        ar.template register_type<InfectionEnvironmental>();
        ar.template register_type<SusceptibilityEnvironmental>();

        // Serialize fields - N/A

        // Serialize base class
        ar & boost::serialization::base_object<Kernel::IndividualHuman>(human);
    }
    template void serialize( boost::mpi::packed_skeleton_oarchive&, Kernel::IndividualHumanEnvironmental&, unsigned int);
    template void serialize( boost::archive::binary_oarchive&, Kernel::IndividualHumanEnvironmental&, unsigned int);
    template void serialize( boost::mpi::detail::content_oarchive&, Kernel::IndividualHumanEnvironmental&, unsigned int);
    template void serialize( boost::mpi::packed_oarchive&, Kernel::IndividualHumanEnvironmental&, unsigned int);
    template void serialize( boost::mpi::detail::mpi_datatype_oarchive&, Kernel::IndividualHumanEnvironmental&, unsigned int);
    template void serialize( boost::mpi::packed_iarchive&, Kernel::IndividualHumanEnvironmental&, unsigned int);
    template void serialize( boost::archive::binary_iarchive&, Kernel::IndividualHumanEnvironmental&, unsigned int);
    template void serialize( boost::mpi::packed_skeleton_iarchive&, Kernel::IndividualHumanEnvironmental&, unsigned int);
}
#endif

#endif // ENABLE_POLIO
