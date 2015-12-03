/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "NodeEnvironmental.h"
#include <list>

class ReportPolio;

namespace Kernel
{
    class SimulationConfig;
    class SpatialReportPolio;

    class INodePolio : public ISupports
    {
    public:
        virtual float GetMeanAgeInfection() const = 0;
        virtual float GetNewDiseaseSusceptibleInfections() const = 0;
    };

    class NodePolio : public NodeEnvironmental, public INodePolio
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        // TODO Get rid of friending and provide accessors for all these floats
        friend class ::ReportPolio;
        friend class Kernel::SpatialReportPolio;

    public:
        static NodePolio *CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid);
        virtual ~NodePolio(void);
        /* clorton virtual */ bool Configure( const Configuration* config ) /* clorton override */;

        virtual void resetNodeStateCounters(void) override;
        virtual void updateNodeStateCounters(IIndividualHuman *ih);
        virtual void finalizeNodeStateCounters(void) override;

        float GetNewDiseaseSusceptibleInfections(void) const override {return newDiseaseSusceptibleInfections;}
        float GetNewDiseaseSusceptibleInfectionsUnder5(void) const {return newDiseaseSusceptibleInfectionsUnder5;}
        float GetNewDiseaseSusceptibleInfectionsOver5(void) const {return newDiseaseSusceptibleInfectionsOver5;}

        virtual float GetMeanAgeInfection() const override;

    protected:
        static const int infection_averaging_window = 30;   // = 30 time steps
        static const float virus_reporting_interval;        // (days)
        float virus_lastReportTime;

        float newDiseaseSusceptibleInfections; // used to report infected individuals who are susceptible to paralysis
        float newDiseaseSusceptibleInfectionsUnder5; // used to report infected individuals who are susceptible to paralysis
        float newDiseaseSusceptibleInfectionsOver5; // used to report infected individuals who are susceptible to paralysis

        float infectionsTotal;

        float mean_age_infection;      // (years)
        float n_people_age_infection;  //(people-days)
        float newInfectedPeople;
        float newInfectedPeopleAgeProduct;

        int window_index;
        std::list<float> infected_people_prior; // [infection_averaging_window];
        std::list<float> infected_age_people_prior; // [infection_averaging_window];

        NodePolio();
        NodePolio(ISimulationContext *_parent_sim, suids::suid node_suid);
        /* clorton virtual */ void Initialize() /* clorton override */;

        const SimulationConfig* params();

        // Factory methods
        virtual Kernel::IIndividualHuman* createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender, float above_poverty);

        virtual void LoadImmunityDemographicsDistribution() override;
        virtual float drawInitialImmunity(float ind_init_age) override;

        DECLARE_SERIALIZABLE(NodePolio);
    };

    class NodePolioTest : public NodePolio
    {
    public:
        static NodePolioTest *CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid);

    protected:
        NodePolioTest(ISimulationContext *_parent_sim, suids::suid node_suid);
    };
}
