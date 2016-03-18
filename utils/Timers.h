/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#pragma once
#include "IArchive.h"
#include "ISerializable.h"
#include <functional>

namespace Kernel { 
    /*class ICountdownTimerCallback
    {
        public:
            virtual void Callback( float dt ) = 0;
    };*/

    class CountdownTimer : public ISerializable
    {
        IMPLEMENT_DEFAULT_REFERENCE_COUNTING()
        virtual QueryResult QueryInterface(iid_t iid, void **ppvObject);
        public:
            CountdownTimer();
            CountdownTimer( float initValue );
            void Decrement( float dt );
            operator float() const { return _timer_value; }
            CountdownTimer& operator=( float newValue );
            std::function< void(float) > handle;
            static void serialize(IArchive& ar, CountdownTimer & ct);
            //bool Expired() const;
            float _timer_value; // should be protected, but public so it can be used in initConfigTM
        protected:
            bool expired() const;
        private:
        
    };
}

