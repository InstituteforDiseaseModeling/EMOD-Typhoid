#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_bednet as bednet

def report_apply( prob ):
    print( str( prob ) )

bednet.my_set_callback( report_apply )
bednet.distribute()

# Update not working yet. Crashes during callback
#bednet.update( 1 )

# This works if you want to use it.
#schema = bednet.get_schema()
#print( schema )
