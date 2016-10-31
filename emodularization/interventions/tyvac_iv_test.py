#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_tyvac as tyvac

def report_apply( action, prob ):
    print( "Update probability of " + action + " with value " + str( prob ) )

tyvac.my_set_callback( report_apply )

def get_schema():
    schema = tyvac.get_schema()
    print( schema )

# individual stuff
tyvac.distribute()

# Update not working yet. Crashes during callback
tyvac.update( 1 )

# batch stuff
def test_batch():
    tyvac.create_batch( 10 )
    tyvac.distribute_batch()
    for i in range( 4000 ):
        tyvac.update_batch( 1 )

test_batch()
