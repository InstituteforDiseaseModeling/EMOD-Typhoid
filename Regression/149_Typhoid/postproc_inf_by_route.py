#!/usr/bin/python

import json
import pdb
import sys
import os

if os.path.exists( 'stdout.txt') == False:
    sys.exit()

with open('stdout.txt') as report:
    lines = report.readlines()

report_out = json.loads( "{}" )
report_out['CONTACT'] = []
report_out['ENVIRONMENT'] = []

infections_this_tstep_by_type = {}

for line in lines:
    #print( line )
    #pdb.set_trace()
    parsed = line.split()
    #print( parsed[0] )
    if len(parsed) < 5:
        continue
    if len(parsed) > 5 and parsed[5] == "Time:":
        time = parsed[6]
        # new timestep; record tallies and clear
        for route in report_out:
            if route in infections_this_tstep_by_type:
                report_out[route].append( infections_this_tstep_by_type[ route ] )
            else:
                report_out[route].append( 0 )

        # clear infections tally
        for route in infections_this_tstep_by_type:
            infections_this_tstep_by_type[route] = 0
        #print time
    elif parsed[4] == "INDIVIDUAL":
        route = parsed[7].strip('.')
        if route in infections_this_tstep_by_type:
            infections_this_tstep_by_type[ route ] = infections_this_tstep_by_type[ route ] + 1
        else:
            infections_this_tstep_by_type[ route ] = 1
        #print route

icj = json.loads( open( "output/InsetChart.json" ).read() )
for route in report_out:
    label = "New Infections By Route (" + route + ")"
    icj["Channels"][ label ] = {}
    icj["Channels"][ label ]["Units"] = "Infections"
    icj["Channels"][ label ]["Data"] = report_out[ route ]

#print( json.dumps( report_out, sort_keys=True, indent=4 ) )
with open( "output/InsetChart.json", "w" ) as icj_file:
    icj_file.write( json.dumps( icj, indent=4 ) )

