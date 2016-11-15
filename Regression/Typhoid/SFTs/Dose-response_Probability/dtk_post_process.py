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
    start_time = cdj["Start_Time"]
    timestep = start_time
    lines_e = []
    lines_c = []
    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search("Update\(\): Time:",line):
                #calculate time step
                timestep+=1
            if re.search("Exposing ", line) and re.search("route 'environment'", line):
                # collect dose_response probabilities and dose for route environment
                line = "TimeStep: " + str(timestep) + " " + line
                lines_e.append(line)
            if re.search("Exposing ", line) and re.search("route 'contact'", line):
                # collect dose_response probabilities and dose for route contact
                line = "TimeStep: " + str(timestep) + " " + line
                lines_c.append(line)

    success = True
    alpha = 0.175
    N50 = 1.11e6
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len(lines_c) == 0:
            success = False
            report_file.write("Found no individual exposed from route contact.\n" )
        else:
            for line in lines_c:
                dose_response = float(get_val("infects=", line))
                ind_id = get_val("inividual ", line)
                timestep = get_val("TimeStep: ", line)
                fContact = float(get_val("fContact=", line))
                dose_response_theoretic = 1.0 - math.pow(1.0 + fContact * (math.pow(2.0, 1.0 / alpha) - 1) / N50,-1.0*alpha)
                if math.fabs(dose_response_theoretic - dose_response) > 5e-2:
                    success = False
                    report_file.write(
                        "BAD: Dose-response probability for individual {0} at time {1}, route contact is {2}, expected {3}.\n".format(
                            ind_id, timestep, dose_response, dose_response_theoretic))
        if len(lines_e) == 0:
            success = False
            report_file.write("Found no individual exposed from route environment.\n")
        else:
            for line in lines_e:
                dose_response = float(get_val("infects=", line))
                ind_id = get_val("inividual ", line)
                timestep = get_val("TimeStep: ", line)
                fExposure = float(get_val("fExposure=", line))
                dose_response_theoretic = 1.0 - math.pow(1.0 + fExposure * (math.pow(2.0, 1.0 / alpha) - 1.0) / N50,-1.0 *alpha)
                if math.fabs(dose_response_theoretic - dose_response) > 5e-2:
                    success = False
                    report_file.write(
                        "BAD: Dose-response probability for individual {0} at time {1}, route environment is {2}, expected {3}.\n".format(
                            ind_id, timestep, dose_response, dose_response_theoretic))

        if success:
              report_file.write(sft.format_success_msg(success))
              os.remove( "test.txt" )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
