/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include "Timers.h"

namespace Kernel { 

    BEGIN_QUERY_INTERFACE_BODY(CountdownTimer) 
    END_QUERY_INTERFACE_BODY(CountdownTimer)

    CountdownTimer::CountdownTimer()
    : _timer_value( -1 )
    {
    }

    CountdownTimer::CountdownTimer( float initValue )
    : _timer_value( initValue )
    {
    }
            
    void CountdownTimer::Decrement( float dt )
    {
        if( expired() )
        {
            handle( dt );
        }
        else
        {
            _timer_value -= dt;
        }
    }

    bool CountdownTimer::expired() const
    {
        return( _timer_value <= 0 );
    }

    CountdownTimer& CountdownTimer::operator=( float newValue )
    {
        _timer_value = newValue;
        return (*this);
    }

    //REGISTER_SERIALIZABLE(CountdownTimer);
    void CountdownTimer::serialize(IArchive& ar, CountdownTimer& obj)
    {
        ar.startObject();
        CountdownTimer& ct = obj;
        ar.labelElement("timer_value") & ct._timer_value;
        ar.endObject();
    }
}

