#!/usr/bin/python

import json
import dtk_typhoidindividual as ti
import random
import pdb
import unittest
import ast

def expose( action, prob ):
    print( "expose: " + action + " with value " + str( prob ) ) 
    if ti.is_infected() == 0 and random.random() < 1.0:
        if ti.get_immunity() == 1:
            print( "Individual is immune. Don't infect." )
            return 0
        return 1
    else:
        print( "Let's NOT infect based on random draw." )
        return 0

ti.my_set_callback( expose )

def getSerializedIndividualAsJson( man ):
    the_man = json.loads( str( man.serialize() ) ) 
    return the_man

class TestStringMethods(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestStringMethods, self).__init__(*args, **kwargs)

    def setUp(self):
        pass
        #self.mean = 10.0

    def test_uninfected_at_start(self):
        ti.create()
        serial_man_json = json.loads( str( ti.serialize() ) )
        num_infections = len(serial_man_json[ "individual" ]["infections"] ) 
        self.assertEqual( num_infections, 0 )

    def test_infected_after_one(self):
        #self.assertTrue('FOO'.isupper())
        #self.assertFalse('Foo'.isupper())
        ti.create()
        ti.update()
        serial_man = getSerializedIndividualAsJson( ti )
        num_infections = len(serial_man[ "individual" ]["infections"] ) 
        self.assertEqual( num_infections, 1 )

if __name__ == '__main__':
    #ti.create()
    #ti.update()
    #myman = getSerializedIndividualAsJson( ti )
    #num_infections = len(myman[ "individual" ]["infections"] ) 
    #print( "Num infections = " + str( num_infections  ) )
    unittest.main()

