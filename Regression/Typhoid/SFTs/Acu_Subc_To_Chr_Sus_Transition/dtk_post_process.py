#!/usr/bin/python
# This SFT test the following statements:
# All infections begin in prepatent.
# The proportion of individuals who move to acute infections is determined by the config parameter Config:TSF. The remainder shall move to subclinical.
# All new acute cases and subclinical cases are transited from prepatent state only.

import re
import json
import math
import pdb
import os
import dtk_sft as sft

# C version: infectiousness = exp( -1 * _infectiousness_param_1 * pow(duration - _infectiousness_param_2,2) ) / _infectiousness_param_3;
def get_val( key, line ):
    regex = key + "(\d*\.*\d*)"
    match = re.search(regex, line)
    if match != None:
        return match.group(1)
    else:
        raise LookupError


def get_char( key, line ):
    regex = key + "(\w*\d*\w*\d*)"
    match = re.search(regex, line)
    if match != None:
        return match.group(1)
    else:
        raise LookupError

def application( report_file ):
    #pdb.set_trace()
    #print( "Post-processing: " + report_file )

    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    start_time=cdj["Start_Time"]
    timestep=start_time
    lines=[]
    count_chronic=0
    count_recovered=0

    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search("Update\(\): Time:",line):
                #calculate time step
                timestep+=1
            if re.search( "just went chronic", line ):
                #append time step and all Infection stage transition to list
                line="TimeStep: "+str(timestep)+ " " + line
                lines.append( line )
                count_chronic += 1
            if re.search("just recovered", line):
                # append time step and all Infection stage transition to list
                line = "TimeStep: " + str(timestep) + " " + line
                lines.append(line)
                count_recovered += 1

    success = True
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len( lines ) == 0 :
            success = False
            report_file.write( "Found no data matching test case.\n" )
        else:
            for line in lines:
                age = float(get_val(" age ", line))
                sex = "female" if (re.search("sex 1", line) or re.search("sex Female", line)) else "male"
                previous_infection_stage = get_char("from ", line)
                if count_chronic==0:
                    success = False
                    report_file.write("Found no Chronic case in data.\n")
                if count_recovered==0:
                    success = False
                    report_file.write("Found no Recovered case in data.\n")
                if (not re.search("from subclinical", line)) and (not re.search("from acute", line)) and (not re.search("from Subclinical", line)) and (not re.search("from Acute", line)) and (not re.search("from SubClinical", line)):
                    success = False
                    if re.search("just went chronic", line):
                        report_file.write("BAD: individual age {0}, sex {1} went to Chronic state from {2} state, expected Acute state or SubClinical state.\n".format(age,sex, previous_infection_stage))
                    else:
                        ind_id=get_val("Individual ", line)
                        report_file.write("BAD: individual {0} age {1}, sex {2} went to Susceptible state from {3} state, expected Acute state or SubClinical state.\n".format(ind_id, age,sex, previous_infection_stage))
        if success:
            report_file.write( sft.format_success_msg( success ) )
            os.remove( "test.txt" )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
