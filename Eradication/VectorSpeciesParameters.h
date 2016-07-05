/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once

#include <vector>

#include "Common.h"
#include "VectorContexts.h"
#include "VectorEnums.h"
#include "Configure.h"

namespace Kernel
{
    class LarvalHabitatParams : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject) { return e_NOINTERFACE; }

        public:
            LarvalHabitatParams() {}
            virtual void ConfigureFromJsonAndKey( const Configuration* inputJson, const std::string& key );
            virtual json::QuickBuilder GetSchema();
            std::map< VectorHabitatType::Enum, float > habitat_map; 
    };

    class VectorSpeciesParameters : public JsonConfigurable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        static VectorSpeciesParameters* CreateVectorSpeciesParameters(const std::string& vector_species_name);
        virtual ~VectorSpeciesParameters();
        bool Configure( const ::Configuration *json );
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);

        LarvalHabitatParams habitat_params;
        float aquaticarrhenius1;
        float aquaticarrhenius2;
        float infectedarrhenius1;
        float infectedarrhenius2;
        float immatureduration;
        float daysbetweenfeeds;
        float anthropophily;
        float eggbatchsize;
        float infectedeggbatchmod;
        float infectiousmortalitymod;
        float aquaticmortalityrate;
        float adultlifeexpectancy;
        float transmissionmod;
        float acquiremod;
        float infectioushfmortmod;
        float indoor_feeding;

        // derived values (e.g. 1/daysbetweenfeeds = feedingrate)
        float feedingrate;
        float adultmortality;
        float immaturerate;

        static void serialize(IArchive&, VectorSpeciesParameters*&);

    protected:
        VectorSpeciesParameters();
        void Initialize(const std::string& vector_species_name);

    private:
        std::string _species;
    };
}
