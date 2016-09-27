#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_genericindividual as gi

def get_schema():
    schema = gi.get_schema()
    print( schema )

#print( "creating individual." )
#gi.create()
print( "getting schema." )
gi.get_schema()

#gi.update()
