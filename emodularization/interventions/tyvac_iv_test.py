#!/usr/bin/python

import json
import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_tyvac as tyvac

def report_apply( action, prob ):
    print( "Update probability of " + action + " with value " + str( prob ) )

tyvac.my_set_callback( report_apply )

def get_schema():
    schema = tyvac.get_schema()
    print( schema )

def test_contact_shedding():
    tv_json = json.loads( open( "tv.json" ).read() )
    tv_json[ "Mode" ] = "Shedding"
    with open( "tv.json", "w" ) as tv_writer:
        tv_writer.write( json.dumps( tv_json, indent=4, sort_keys = True ) )

    tyvac.distribute() 
    tyvac.update( 1 ) 
    mods = tyvac.get_modifiers() 

    contact_shed = mods[0]
    contact_exp = mods[1]
    contact_num = mods[2]
    enviro_shed = mods[3]
    enviro_exp = mods[4]
    enviro_num = mods[5]

    eff = tv_json[ "Changing_Effect" ][ "Initial_Effect" ]
    if eff != 1 - contact_shed:
        print( "Failed." )
    else:
        print( "Pass." )

def test_contact_dose():
    tv_json = json.loads( open( "tv.json" ).read() )
    tv_json[ "Mode" ] = "Dose"
    with open( "tv.json", "w" ) as tv_writer:
        tv_writer.write( json.dumps( tv_json, indent=4, sort_keys = True ) )

    tyvac.distribute()
    tyvac.update( 1 )

    mods = tyvac.get_modifiers() 
    contact_shed = mods[0]
    contact_exp = mods[1]
    contact_num = mods[2]
    enviro_shed = mods[3]
    enviro_exp = mods[4]
    enviro_num = mods[5]

    eff = tv_json[ "Changing_Effect" ][ "Initial_Effect" ]
    if eff != 1 - contact_exp:
        print( "Failed." )
    else:
        print( "Pass." )


# batch stuff
def test_batch():
    tyvac.create_batch( 10 )
    tyvac.distribute_batch()
    for i in range( 4000 ):
        tyvac.update_batch( 1 )

#test_batch()
test_contact_shedding()
test_contact_dose()
