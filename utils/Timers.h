/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "IArchive.h"
#include "ISerializable.h"

namespace Kernel { 
    class CountdownTimer : public ISerializable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        public:
            CountdownTimer();
            CountdownTimer( float initValue );
            void Decrement( float dt );
            bool Expired() const;
            operator float() const { return _timer_value; }
            CountdownTimer& operator=( float newValue );
            static void serialize(IArchive& ar, CountdownTimer & ct);
        //protected:
            float _timer_value;
        private:
        
    };
}

