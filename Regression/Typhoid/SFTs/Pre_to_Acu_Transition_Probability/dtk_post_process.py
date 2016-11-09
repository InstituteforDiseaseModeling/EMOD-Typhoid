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

def get_char_before( key, line ):
    regex ="(\w*\d*\w*\d*)" + key
    match = re.search(regex, line)
    if match != None:
        return match.group(1)
    else:
        raise LookupError

def get_char_after( key, line ):
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
    tsf = cdj["Typhoid_Symptomatic_Fraction"]
    start_time=cdj["Start_Time"]
    lines = []
    timestep=start_time
    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search("Update\(\): Time:",line):
                #calculate time step
                timestep+=1
            if re.search( "Infection stage transition", line ):
                #append time step and all Infection stage transition to list
                line="TimeStep: "+str(timestep)+ " " + line
                lines.append( line )

    success = True
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len( lines ) == 0:
            success = False
            report_file.write( "Found no data matching test case.\n" )
        else:
            subcount = 0
            acutecount = 0
            for line in lines:
                if (((not re.search("->SubClinical:", line))  and (not re.search("->Acute:", line))) and re.search(", Prepatent->", line)) or (re.search("->SubClinical:", line) or re.search("->Acute:", line)) and not re.search(", Prepatent->", line):
                    success = False
                    ind_id=get_val("Individual=", line)
                    current_infection_stage=get_char_after("->", line)
                    previous_infection_stage=get_char_before("->", line)
                    report_file.write("BAD: individuals {0} went to {1} state from {2} state, expected Prepatent->Acute or Prepatent->SubClinical.\n".format(ind_id,current_infection_stage, previous_infection_stage))
                elif re.search(", Prepatent->Acute:", line):
                    #count # of cases: from Prepatent to Acute
                    acutecount+=1
                elif re.search(", Prepatent->SubClinical:", line):
                    #count # of cases: from Prepatent to Subclinical
                    subcount += 1
            if subcount+acutecount == 0:
                success = False
                report_file.write("Found no individual exits Prepatent state in log.\n")
            else:
                actual_tsf=acutecount/float(subcount+acutecount)
                if math.fabs( actual_tsf - tsf)>5e-2 :
                    success = False
                    report_file.write("BAD: Proportion of prepatent cases that become acute vs. subclinical is {0} instead of {1}. Actual Acute case = {2} vs. Actual SubClinical case = {3}.\n".format(actual_tsf,tsf, acutecount, subcount))

        if success:
            report_file.write( sft.format_success_msg( success ) )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
