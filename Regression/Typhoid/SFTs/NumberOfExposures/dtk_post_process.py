#!/usr/bin/python

import re
import json
import math
import pdb
import os
import dtk_sft as sft
import numpy as np
from scipy import stats

# C version: infectiousness = exp( -1 * _infectiousness_param_1 * pow(duration - _infectiousness_param_2,2) ) / _infectiousness_param_3;



def get_val( key, line ):    
    """
    We might want to move this into the dtk_sft module.
    """
    regex = key + "(\d*\.*\d*)"
    match = re.search(regex, line)
    if match != None:   
        return match.group(1)
    else:
        raise LookupError


def application( report_file ):
    #pdb.set_trace()
    #print( "Post-processing: " + report_file )
    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    tcer = float(cdj["Typhoid_Contact_Exposure_Rate"])
    teer = float(cdj["Typhoid_Environmental_Exposure_Rate"])
    start_time = cdj["Start_Time"]

    timestep = start_time
    num_exposures_contact = []
    num_exposures_enviro = []
    with open( "test.txt" ) as logfile:
        route=None
        for line in logfile:
            if re.search("Exposing ", line) and re.search("route 'environment'", line):
                # collect # of exposures for route contact
                num_exp_e=get_val("num_exposures=", line)
                num_exposures_enviro.append(num_exp_e)
            if re.search("Exposing ", line) and re.search("route 'contact'", line):
                # collect # of exposures for route environment
                num_exp_c = get_val("num_exposures=", line)
                num_exposures_contact.append(num_exp_c)

    success = True
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len(num_exposures_enviro) == 0:
            success = False
            report_file.write("Found no individual exposed from route environment.\n" )
        elif not sft.test_poisson(num_exposures_enviro, teer, report_file,'environment'):
            success =False
        if len(num_exposures_contact) == 0:
            success = False
            report_file.write("Found no individual exposed from route contact.\n" )
        elif not sft.test_poisson(num_exposures_contact, tcer, report_file,'contact'):
            success = False

        if success:
            report_file.write(sft.format_success_msg(success))
            os.remove( "test.txt" )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
