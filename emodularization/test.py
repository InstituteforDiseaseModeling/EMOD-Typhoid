#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_datetime

dtk_datetime.update( "1" )
dtk_datetime.update( "1" )
dtk_datetime.update( "1" )
dtk_datetime.update( "1" )
dtk_datetime.update( "1" )
dtk_datetime.update( "1" )
dtk_datetime.update( "1" )
dtk_datetime.update( "1" )
dtk_datetime.create( "1" )
print( str( dtk_datetime.get_base_year() ) )

