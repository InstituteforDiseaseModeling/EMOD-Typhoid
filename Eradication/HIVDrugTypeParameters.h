/*****************************************************************************

Copyright (c) 2014 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#pragma once

#include "Configure.h"
#include "HIVEnums.h"

namespace Kernel {
    class HIVDrugTypeParameters : public JsonConfigurable
    {
        friend class SimulationConfig;
        friend class AntiHIVMonotherapyDrug;
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()

    public:
        HIVDrugTypeParameters( const std::string& drugName );
        static HIVDrugTypeParameters* CreateHIVDrugTypeParameters( const std::string& drugName );
        virtual ~HIVDrugTypeParameters();
        virtual bool Configure( const ::Configuration *json );
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);

        typedef map< std::string, HIVDrugTypeParameters* > tHIVDTPMap;
        static tHIVDTPMap _hivdtMap;

    protected:
        void Initialize(const std::string& drugName);

        HIVDrugClass::Enum hiv_drug_class;  // Class of HIV drug
        ReverseTranscriptaseNucleosideAnalog::Enum nucleoside_analog;   // Used only for NRTI drugs to identify target

        // Viral dynamics properties:
        std::string mutation;               // Name of the mutation, e.g. M184V
        float mutation_fitness_cost;        // Fitness cost of mutation (0<s<1): R0_mutant = R0_wild * (1-s)
        float mutation_change_IC50;         // Fold change in mutant IC50 (rho>1): IC50_mutant = IC50_wild * rho
        float mutation_increase_slope;      // Slope adjustment of mutant dose response (sigma<0): m_mutant = m_wild * (1-sigma)

        float equilibrium_frequency;        // Pre-existing (steady state) frequency of mutation
        float reservoir_exit__days;         // Exit rate from the latent reservoir

        // PKPD properties
        float pkpd_Cmax__uMol;              // Steady-state peak concentration
        float pkpd_Cmin__uMol;              // Trough concentration
        float pkpd_halflife__hours;         // Half-life in hours
        float dose_interval__days;          // Interval between doses in days

    private:
        std::string _drugName;  // Drug name, should match drug listed in config.json
    };
}
