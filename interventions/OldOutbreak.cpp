/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "stdafx.h"
#include "Outbreak.h"

#include "Exceptions.h"
#include "InterventionFactory.h"
#include "NodeEventContext.h"  // for IOutbreakConsumer methods

static const char * _module = "Outbreak";

// Important: Use the instance method to obtain the intervention factory obj instead of static method to cross the DLL boundary
// NO USAGE like this:  GET_CONFIGURABLE(SimulationConfig)->number_substrains in DLL

namespace Kernel
{
    ENUM_DEFINE(OutbreakType, 
        ENUM_VALUE_SPEC(PrevalenceIncrease , 1)
        ENUM_VALUE_SPEC(ImportCases        , 2))

    BEGIN_QUERY_INTERFACE_BODY(Outbreak)
        HANDLE_INTERFACE(IConfigurable)
        //HANDLE_INTERFACE(IDistributableIntervention)
        HANDLE_INTERFACE(INodeDistributableIntervention)
        HANDLE_INTERFACE(IOutbreak)
        HANDLE_ISUPPORTS_VIA(INodeDistributableIntervention)
    END_QUERY_INTERFACE_BODY(Outbreak)


    IMPLEMENT_FACTORY_REGISTERED(Outbreak)

    Outbreak::Outbreak() : import_age(DAYSPERYEAR)
    {
        initConfigTypeMap( "Antigen", &antigen, Antigen_DESC_TEXT, 0, 10, 0 );
        initConfigTypeMap( "Genome",  &genome,  Genome_DESC_TEXT, -1, 16777216, 0 );
    }

    bool
    Outbreak::Configure(
        const Configuration * inputJson
    )
    {
        initConfig( "Outbreak_Source", outbreak_source, inputJson, MetadataDescriptor::Enum("Outbreak_Source", Outbreak_Source_DESC_TEXT, MDD_ENUM_ARGS(OutbreakType)) );
        if ( outbreak_source == OutbreakType::ImportCases || JsonConfigurable::_dryrun )
        {
            initConfigTypeMap( "Import_Age", &import_age, Import_Age_DESC_TEXT, 0, 100000, DAYSPERYEAR );
        }
        JsonConfigurable::Configure( inputJson );
        return true;
    }

    bool Outbreak::Distribute(INodeEventContext *context, IEventCoordinator2* pEC)
    {
        bool wasDistributed = false;

        IOutbreakConsumer *ioc;
        if (s_OK == context->QueryInterface(GET_IID(IOutbreakConsumer), (void**)&ioc))
        {
            StrainIdentity* strain_identity = GetNewStrainIdentity(context);   // JPS: no need to create strain before we call this if we're calling into node...?

            if(this->outbreak_source == OutbreakType::PrevalenceIncrease)
            {
                ioc->IncreasePrevalence(strain_identity, pEC);
                wasDistributed = true;
            }
            else if(this->outbreak_source == OutbreakType::ImportCases)
            {
                ioc->AddImportCases(strain_identity, import_age, pEC);
                wasDistributed = true;
            }
            delete strain_identity;
        }
        else
        {
            throw QueryInterfaceException( __FILE__, __LINE__, __FUNCTION__, "context", "IOutbreakConsumer", "INodeEventContext" );
        }

        return wasDistributed;
    }

    void Outbreak::Update( float dt )
    {
        LOG_WARN("updating outbreak (?!?)\n");
        // Distribute() doesn't call GiveIntervention() for this intervention, so it isn't added to the NodeEventContext's list of NDI
    }

    Kernel::StrainIdentity* Outbreak::GetNewStrainIdentity(INodeEventContext *context)
    {
        StrainIdentity *outbreak_strainID = NULL;
        
        
        // Important: Use the instance method to obtain the intervention factory obj instead of static method to cross the DLL boundary
        // NO usage of GET_CONFIGURABLE(SimulationConfig)->number_substrains in DLL
        IGlobalContext *pGC = NULL;
        const SimulationConfig* simConfigObj = NULL;
        if (s_OK == context->QueryInterface(GET_IID(IGlobalContext), (void**)&pGC))
        {
            simConfigObj = pGC->GetSimulationConfigObj();
        }
        if (!simConfigObj)
        {
            throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "The pointer to IInterventionFactory object is not valid (could be DLL specific)" );
        }

        if ( genome < 0 )
        {
            int ss = simConfigObj->number_substrains;
            if (ss & (ss-1))
            {
                throw GeneralConfigurationException( __FILE__, __LINE__, __FUNCTION__, "Only supporting random genome generation for Number_Substrains as factor of two." );
            }
            unsigned int BARCODE_BITS = 0;
            while(ss >>= 1) ++BARCODE_BITS;
            uint32_t genome = context->GetRng()->ul() & ((1 << BARCODE_BITS)-1);
            //genome = context->GetRng()->i(simConfigObj->number_substrains);
            outbreak_strainID = _new_ StrainIdentity(antigen, genome);
            LOG_DEBUG_F("random genome generation... antigen: %d\t genome: %d\n", antigen, genome);
        }
        else if (genome >= 0 && genome < simConfigObj->number_substrains )
        {
            outbreak_strainID = _new_ StrainIdentity(antigen, genome);
        }
        else
        {
            // std::cerr << "Outbreak Genome " << genome << " must be less than number_substrains " << simConfigObj->number_substrains << std::endl;
            throw IncoherentConfigurationException( __FILE__, __LINE__, __FUNCTION__, "genome", genome, "number_substrains", simConfigObj->number_substrains );
        }

        return outbreak_strainID;
    }

}

#if 0
namespace Kernel {
    template<class Archive>
    void serialize(Archive &ar, Outbreak &ob, const unsigned int v)
    {
        ar & ob.outbreak_source;
        ar & ob.antigen;
        ar & ob.genome;
        ar & ob.import_age;
    }
}
#endif
