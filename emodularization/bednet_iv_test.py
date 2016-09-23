#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_bednet as bednet

def report_apply( action, prob ):
    print( "Update probability of " + action + " with value " + str( prob ) )

bednet.my_set_callback( report_apply )

def get_schema():
    schema = bednet.get_schema()
    print( schema )

# individual stuff
bednet.distribute()

# Update not working yet. Crashes during callback
bednet.update( 1 )


# batch stuff
bednet.create_batch( 100 )
bednet.distribute_batch()
bednet.update_batch( 1 )
