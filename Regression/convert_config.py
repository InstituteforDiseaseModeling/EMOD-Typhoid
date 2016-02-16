#!/usr/bin/python

import json
import sys

config = json.loads( open( sys.argv[1] ).read() )

for species in config["parameters"]["Vector_Species_Params"]:
    print( species )
    habitat_array = config["parameters"]["Vector_Species_Params"][species]["Habitat_Type"]
    rhf_array = config["parameters"]["Vector_Species_Params"][species]["Required_Habitat_Factor"]
    config["parameters"]["Vector_Species_Params"][species].pop( "Habitat_Type" )
    config["parameters"]["Vector_Species_Params"][species].pop( "Required_Habitat_Factor" )
    config["parameters"]["Vector_Species_Params"][species]["Larval_Habitat_Types"] = {}
    counter = 0
    for hab in habitat_array :
        config["parameters"]["Vector_Species_Params"][species]["Larval_Habitat_Types"][hab] = rhf_array[counter]
        counter = counter + 1

with open( "config_new.json", "w" ) as cfg:
    cfg.write( json.dumps( config, indent=4, sort_keys = True ) )
