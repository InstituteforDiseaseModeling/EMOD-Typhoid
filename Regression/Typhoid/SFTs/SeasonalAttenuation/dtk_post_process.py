#!/usr/bin/python

import re
import json
import math
import pdb
import os
import dtk_plot_wrapper
import dtk_sft as sft

# C version: infectiousness = exp( -1 * _infectiousness_param_1 * pow(duration - _infectiousness_param_2,2) ) / _infectiousness_param_3;

def inf_calc( dur, c1, c2, c3 ):
    """
    Your SFT will probably have some mathematical calculation based on the requirements document.
    """
    x = c1 * math.pow( (dur - c2), 2 )
    return math.exp(-1*x) /c3

def get_val( key, line ):    
    """
    We might want to move this into the dtk_sft module.
    """
    regex = key + "=(\d*\.*\d*)"
    match = re.search(regex, line)
    if match != None:   
        return match.group(1)
    else:
        raise LookupError
    
def application( report_file ):
    """
    Parse this line from test.txt:
    00:00:00 [0] [V] [IndividualTyphoid] amplification calculated as 0.997059: day of year=1, start=360.000000, end=365.000000, ramp_up=30.000000, ramp_down=170.000000, cutoff=160.000000. 
    """
    #print( "Post-processing: " + report_file ) 
    lines = []
    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search( "Initializing Typhoid immunity object", line ) and re.search( "age=0.000000", line ):
                lines.append( line )

    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    rud = cdj[ "Typhoid_Environmental_Ramp_Up_Duration" ] 
    rdd = cdj[ "Typhoid_Environmental_Ramp_Down_Duration" ] 
    erd = cdj[ "Typhoid_Environmental_Ramp_Duration" ] # e.g., 80
    ecd = cdj[ "Typhoid_Environmental_Cutoff_Days" ] # e.g., 160
    eps = cdj[ "Typhoid_Environmental_Peak_Start" ] # e.g., 360
		
    success = True
    with open( sft.sft_output_filename, "w" ) as report_file:
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
            report_file.write( sft.format_success_msg( success ) )
            os.remove( "test.txt" )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
