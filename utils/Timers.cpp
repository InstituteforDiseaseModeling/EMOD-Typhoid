/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <stdafx.h>
#include "Timers.h"

namespace Kernel { 

    BEGIN_QUERY_INTERFACE_BODY(CountdownTimer) 
    END_QUERY_INTERFACE_BODY(CountdownTimer)

    CountdownTimer::CountdownTimer()
    : NonNegativeFloat( 0 ) // need diff base class (RangedFloat?) if we want to init with -1
    {
    }

    CountdownTimer::CountdownTimer( float initValue )
    : NonNegativeFloat( initValue )
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
            _value -= dt;
        }
    }

    bool CountdownTimer::expired() const
    {
        return( _value <= 0 );
    }

    //REGISTER_SERIALIZABLE(CountdownTimer);
    void CountdownTimer::serialize(IArchive& ar, CountdownTimer& obj)
    {
        ar.startObject();
        CountdownTimer& ct = obj;
        //ar.labelElement("timer_value") & ct._timer_value;
        ar.endObject();
    }
}

