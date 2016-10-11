/*****************************************************************************

Copyright (c) 2015 by Global Good Fund I, LLC. All rights reserved.

Except for any rights expressly granted to you in a separate license with the
Global Good Fund (GGF), GGF reserves all rights, title and interest in the
software and documentation.  GGF grants recipients of this software and
documentation no other rights either expressly, impliedly or by estoppel.

THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" AND GGF HEREBY DISCLAIMS
ALL WARRANTIES, EXPRESS OR IMPLIED, OR STATUTORY, INCLUDING IMPLIED WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.

*****************************************************************************/

#pragma once
#include "NodeEnvironmental.h"
#include "IndividualTyphoid.h"
#include "TyphoidDefs.h"
#include <iostream>
#include <list>

class ReportTyphoid;

namespace Kernel
{
    class SimulationConfig;
    class SpatialReportTyphoid;

    class INodeTyphoid : public ISupports
    {
    public:
    };

    class NodeTyphoid : public NodeEnvironmental, public INodeTyphoid
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

        // TODO Get rid of friending and provide accessors for all these floats
        friend class ::ReportTyphoid;
        friend class Kernel::SpatialReportTyphoid;

    public:
        static NodeTyphoid *CreateNode(ISimulationContext *_parent_sim, suids::suid node_suid);
        virtual ~NodeTyphoid(void);
        bool Configure( const Configuration* config );

        virtual void SetupIntranodeTransmission();
        virtual void resetNodeStateCounters(void);
        virtual void updateNodeStateCounters(IndividualHuman *ih);
        virtual void finalizeNodeStateCounters(void);
        virtual std::map< std::string, float > GetTotalContagion() const;

    protected:
        NodeTyphoid();
        NodeTyphoid(ISimulationContext *_parent_sim, suids::suid node_suid);
        void Initialize();

        const SimulationConfig* params();

        // Factory methods
        virtual Kernel::IndividualHuman *createHuman(suids::suid suid, float monte_carlo_weight, float initial_age, int gender, float above_poverty);

        // wrap base-class function between creation and deletion of polio vaccine immunity initialization distributions.
        virtual void populateNewIndividualsFromDemographics(int count_new_individuals);

    private:
#if USE_BOOST_SERIALIZATION
        friend class boost::serialization::access;
        template<class Archive>
        friend void serialize(Archive & ar, NodeTyphoid &node, const unsigned int  file_version );
#endif
    };
}
