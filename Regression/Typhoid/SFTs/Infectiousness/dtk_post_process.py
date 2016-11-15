#!/usr/bin/python

import re
import json
import math
import pdb
import os
import dtk_sft as sft

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

def calc_expected_infectiousness( line, treatment_list):
    """
    calculate the infectiousness based on the state and the params in config.json.
    """
    states=["state PRE.", "state SUB.", "state CHR.", "state ACU."]
    expected_infectiousness = None
    state = None
    for regex in states:
        match = re.search(regex, line)
        if match != None:
            if regex == "state PRE.":
                state = "Prepatent"
                expected_infectiousness = tai * tpri
            elif regex == "state SUB.":
                state = "Subclinical"
                expected_infectiousness = tai * tsri
            elif regex == "state CHR.":
                state = "Chronic"
                expected_infectiousness = tai * tcri
            elif regex == "state ACU.":
                state = "Acute"
                expected_infectiousness = tai
                ind_id = get_val("Individual ", line)
                if ind_id in treatment_list:
                    expected_infectiousness *= treatment_multiplier
            return {'expected_infectiousness': expected_infectiousness, 'state': state}

    
def application( report_file ):
    #pdb.set_trace()
    #print( "Post-processing: " + report_file )

    # get params from config.json
    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    global tai
    tai = cdj["Typhoid_Acute_Infectiousness"]
    global tpri
    tpri = cdj["Typhoid_Prepatent_Relative_Infectiousness"]
    # remove param tbiam
    # global tbiam
    #tbiam =0.5
    global treatment_multiplier
    treatment_multiplier = 0.5
    global tsri
    tsri = cdj["Typhoid_Subclinical_Relative_Infectiousness"]
    global tcri
    tcri = cdj["Typhoid_Chronic_Relative_Infectiousness"]
    start_time=cdj["Start_Time"]

    lines = []
    timestep=start_time

    #collect the following logs and add time step in list lines
    #00:00:01 [0] [V] [IndividualTyphoid] Individual 9927 depositing 20000.000000 to route contact: (antigen=0, substrain=2) at time 9.000000 in state SUB.
    #00:00:01 [0] [V] [InfectionTyphoid] Individual ID: 4016, State: Acute, GetTreatment: True.
    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search("Update\(\): Time:",line):
                #calculate time step
                timestep+=1
            if re.search( "depositing", line ):
                line = "TimeStep: " + str(timestep) + " " + line
                lines.append( line )
            if re.search("GetTreatment: True", line):
                line = "TimeStep: " + str(timestep) + " " + line
                lines.append(line)
            if re.search("->Acute", line):
                line = "TimeStep: " + str(timestep) + " " + line
                lines.append(line)

    success = True
    treatment_list=[]

    with open( sft.sft_output_filename, "w" ) as report_file:
        if len( lines ) == 0:
            success = False
            report_file.write( "Found no data matching test case.\n" )
        else:
            pre_count=0
            acu_count=0
            sub_count=0
            chr_count=0
            for line in lines:
                if re.search("GetTreatment",line):
                    #collect a list of individual Ids that get treatment
                    Ind_id=get_val("Individual ID: ", line)
                    treatment_list.append(Ind_id)
                elif re.search("state SUS.", line):
                    success=False
                    report_file.write("BAD: Found individual in Susceptible state has infectiousness. See details: {}.\n".format(line))
                elif re.search("->Acute", line):
                    indid=get_val("Individual=",line)
                    if indid in treatment_list:
                        # remove ind id from treatment list if an individual move to Acute state again.
                        treatment_list.remove(indid)
                else:
                    if re.search("state PRE",line):
                        pre_count += 1
                    if re.search("state ACU",line):
                        acu_count += 1
                    if re.search("state SUB",line):
                        sub_count += 1
                    if re.search("state CHR",line):
                        chr_count += 1
                    #validate infectiousness
                    if re.search("to route contact:", line): route = "contact"
                    if re.search("to route environmental:", line): route = "environment"

                    actual_infectiousness = float(get_val("depositing ", line))
                    result=calc_expected_infectiousness(line, treatment_list)
                    expected_infectiousness=result['expected_infectiousness']
                    state=result['state']
                    ind_id = get_val("Individual ", line)
                    if math.fabs(actual_infectiousness-expected_infectiousness)>1e2:
                        success = False
                        report_file.write( "BAD: Individual {0} depositing {1} to route {2} in state: {3}. Expected infectiousness: {4}\n".format(ind_id, actual_infectiousness,route, state, expected_infectiousness))
            if pre_count==0:
                success = False
                report_file.write("Found no infectiousness data in Prepatent state.\n")
            if acu_count==0:
                success = False
                report_file.write("Found no infectiousness data in Acute state.\n")
            if sub_count==0:
                success = False
                report_file.write("Found no infectiousness data in SubClinical state.\n")
            if chr_count==0:
                success = False
                report_file.write("Found no infectiousness data in Chronic state.\n")


            if success:
                report_file.write( sft.format_success_msg( success ) )
                os.remove( "test.txt" )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
