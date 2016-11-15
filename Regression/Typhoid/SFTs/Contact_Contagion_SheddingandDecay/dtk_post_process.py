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
    isj = json.loads(open("output/InsetChart.json").read())["Channels"]
    ccp = isj["Contact Contagion Population"]["Data"]

    timestep = start_time
    lines=[]
    cum_all=[]
    cum=0
    Statpop=[]
    pop = 0
    adding_cp_log = []
    adding_cp = 0
    with open( "test.txt" ) as logfile:
        route=None
        for line in logfile:
            # collect all lines of
            if re.search("Update\(\): Time:",line):
                #calculate time step
                timestep+=1
                #lines.append(line)
                #append the accumulated shedding and reset the counter at the end of each time step.
                #data for timestep 1 is stored in cum_all[1]
                cum_all.append(cum)
                cum = 0
                adding_cp_log.append(adding_cp)
            if re.search("\[StrainAwareTransmissionGroups\] Adding ", line) and re.search("route:0", line):
                # append time step and total shedding line to lists
                #line = "TimeStep: " + str(timestep) + " " + line
                #lines.append(line)
                adding_cp = float(get_val("Adding ",line))
            if re.search("Exposing ", line) and re.search("route 'contact'", line):
                # append time step and Exposing line to lists
                line = "TimeStep: " + str(timestep) + " " + line
                lines.append(line)
                # get shedding of contact route and add it to accumulated shedding
            if re.search("depositing", line) and re.search("route contact", line):
                shedding= float(get_val("depositing ", line))
                cum += shedding
            if re.search("scaled by ", line) and re.search("route:0", line):
                # get population
                pop = float(get_val("scaled by ",line))
                Statpop.append(pop)


    success = True
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len(lines ) == 0:
            success = False
            report_file.write("Found no data matching test case.\n" )
        else:
            for line in lines:
                if re.search("Exposing", line):
                    fContact = float(get_val("fContact=", line))
                    timestep = int(get_val("TimeStep: ", line))
                    expected_cp = cum_all[timestep-start_time] / Statpop[timestep -start_time+ 1]
                    if math.fabs(fContact-expected_cp) >1e-2 :
                        success = False
                        ind_id = get_val("inividual ", line)
                        report_file.write("BAD: Individual {0} is exposed on route contact with contagion population = {1} at time {2} StatPop = {3}, expected {4}.\n".format(ind_id,fContact, timestep, Statpop[timestep+1],expected_cp))
            for x in range(1, len(cum_all)):
                #report_file.write("timestep={0}, StatPop = {1}, cum_all= {2}, adding_cp_log = {3}.\n".format(x,Statpop[x+1],cum_all[x], adding_cp_log[x]))
                if math.fabs(cum_all[x] - adding_cp_log[x])>1e-2 :
                    success = False
                    report_file.write(
                        "BAD: At time {0}, the accumulated shedding is {1} from log, expected {2}.\n".format(timestep, adding_cp_log[x], cum_all[x]))
                expected_cp = cum_all[x] / Statpop[x+ 1]
                if math.fabs(expected_cp - ccp[x-1])>1e-2 :
                    success = False
                    report_file.write(
                        "BAD: At time {0}, the accumulated shedding is {1} from InsetChart.json, expected {2}.\n".format(timestep, ccp[x-1], expected_cp))
        if success:
            report_file.write(sft.format_success_msg(success))
            os.remove( "test.txt" )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
