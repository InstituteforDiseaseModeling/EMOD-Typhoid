#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_genericindividual as gi

import pdb

def get_schema():
    schema = gi.get_schema()
    print( schema )

#print( "getting schema." )
#get_schema()

print( "creating individual." )
gi.create()
for tstep in range( 0,100 ):
    print( "Updating individual." )
    gi.update( 1.0 )
