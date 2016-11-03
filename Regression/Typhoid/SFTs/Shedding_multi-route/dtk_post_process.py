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

def get_char( key, line ):
    """
    We might want to move this into the dtk_sft module.
    """
    regex = key + "(\w*)"
    match = re.search(regex, line)
    if match != None:
        return match.group(1)
    else:
        raise LookupError
    
def application( report_file ):
    #pdb.set_trace()
    #print( "Post-processing: " + report_file )
    cdj = json.loads(open("config.json").read())["parameters"]
    start_time = cdj["Start_Time"]
    timestep = start_time
    lines_contact = []
    lines_environment = []
    with open( "test.txt" ) as logfile:
        route=None
        for line in logfile:
            # collect all lines of
            if re.search("Update\(\): Time:",line):
                #calculate time step
                timestep+=1
            if re.search( "depositing", line ):
                #append time step and depositing line to lists
                ind_id = get_val("Individual ", line)
                infectiousness= get_val("depositing ", line)
                line="TimeStep: "+str(timestep)+ " ind_id: " + str(ind_id) + " " + line
                if re.search("contact", line):
                    lines_contact.append(line)
                elif re.search("environment", line):
                    lines_environment.append(line)
    success = True
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len( lines_contact ) == 0 or len( lines_environment ) == 0:
            success = False
            report_file.write("Found no data matching test case.\n" )
        else:
            for line in lines_contact:
                infectiousness1 = None
                infectiousness2 = None
                ind_id = get_val("ind_id: ", line)
                timestep = get_val("TimeStep: ", line)
                infectiousness1 = get_val("depositing ", line)
                for l in lines_environment:
                    if re.search("ind_id: "+str(ind_id)+ " ",l) and re.search("TimeStep: "+ str(timestep) + " ", l):
                        infectiousness2 = get_val("depositing ", l)
                        if infectiousness1 != infectiousness2:
                            success = False
                            report_file.write("BAD: Individual {0} is depositing {1} to route {2} and {3} to route {4} at time {5}.\n".format(ind_id, infectiousness1, "contact", infectiousness2, "environment", timestep))
                        lines_environment.remove(l)
                        break
                if infectiousness2 == None:
                    success = False
                    report_file.write("BAD: Individual {0} is not depositing to route environment at time {1}.\n".format(ind_id, timestep))
            if len(lines_environment) != 0:
                success = False
                report_file.write("BAD: {0} Individuals are not depositing to route contact while they are deposinting to route contact.\n".format(len(lines_environment),timestep))
                for l in lines_environment:
                    report_file.write(l)

        if success:
            report_file.write(sft.format_success_msg(success))

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
