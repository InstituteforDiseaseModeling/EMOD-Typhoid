#!/usr/bin/python

import re
import json
import math
import pdb
import os
import dtk_plot_wrapper

# C version: infectiousness = exp( -1 * _infectiousness_param_1 * pow(duration - _infectiousness_param_2,2) ) / _infectiousness_param_3;

def inf_calc( dur, c1, c2, c3 ):
    x = c1 * math.pow( (dur - c2), 2 )
    return math.exp(-1*x) /c3

def get_val( key, line ):    
    regex = key + "=(\d*\.*\d*)"
    match = re.search(regex, line)
    if match != None:   
        return match.group(1)
    else:
        raise LookupError
    
def application( report_file ):
    #pdb.set_trace()
    #print( "Post-processing: " + report_file ) 
    lines = []
    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search( "Initializing Typhoid immunity object", line ) and re.search( "age=0.000000", line ):
                lines.append( line )

    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    #c1 = cdj["Infectiousness_Asymptomatic_Naive_1"]
    success = True
    with open( "scientific_feature_report.txt", "w" ) as report_file:
        if len( lines ) == 0:
            success = False
            report_file.write( "Found no data matching test case.\n" )
        else:
            for line in lines:
                immunity = float( get_val( " immunity modifier", line ) )
                if immunity != 1.000:
                    success = False
                    report_file.write( "BAD: immunity for newborn={0} instead of 1.0\n".format( immunity ) )
                    
        if success:
            report_file.write( "SUMMARY: Success={0}\n".format( success ) )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
