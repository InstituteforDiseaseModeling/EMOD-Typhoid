#!/usr/bin/python

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
    
def application( report_file ):
    #pdb.set_trace()
    #print( "Post-processing: " + report_file ) 
    lines = []

    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    simulaiton_duration=cdj["Simulation_Duration"]
    start_time=cdj["Start_Time"]

    timestep=start_time
    #acute_count and treatment_count are tracking the accumulated # of Acute cases and treatments
    acute_count=0
    treatment_count=0
    exception_count=0



    #create a list lines to store logs
    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search("Update\(\): Time:",line):
                #calculate time step
                timestep+=1
            if re.search( "Treatment", line ):
                #append time step and treatment information to list
                line="TimeStep: "+str(timestep)+ " " + line
                treatment_count+=1
                lines.append( line )
            if re.search("Prepatent->Acute", line):
                # ignore the acute case in the last 4 days
                if timestep <= simulaiton_duration -4:
                    # append time step and information related to individual movin gto acute to list
                    line="TimeStep: "+str(timestep) + " " +line
                    lines.append(line)
                    acute_count += 1
                    # add those move to other state before treatment from exception count
                    acute_duration= get_val("acute dur=",line)
                    if acute_duration < 5 :
                        exception_count += 1

    with open( "lines.txt", "w" ) as report_file:
        for line in lines:
            report_file.write(line)


    success = True

    #write result to scientific_feature_report.txt
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len( lines ) == 0:
            success = False
            report_file.write( "Found no data matching test case.\n" )
        elif acute_count==0 or treatment_count==0:
            success = False
            report_file.write("Found no acute or treatment cases. acute_count={0},treatment_count={1}.\n".format(acute_count,treatment_count))
        else:
            treatment_timestep=None
            acute_timestep=None
            ind_id=-1
            expeted_treatment_delay=5 #hardcoded, an individual move to acute state at day 1 will get treatment at day 5

            for i in range(len(lines)-1, -1, -1):
                if re.search("Treatment",lines[i]):
                    #get Id and treatmetn time step from Treatmnet line
                    ind_id=get_val("Individual ID: ",lines[i])
                    treatment_timestep=float(get_val("TimeStep: ", lines[i]))
                    for j in range(i-1, -1, -1):
                        if re.search("Prepatent->Acute",lines[j]) and re.search(ind_id, lines[j]):
                            #get acute time step and skip the interloop
                            acute_timestep=float(get_val("TimeStep: ", lines[j]))
                            break
                    if acute_timestep==None:
                        #make sure individual entering Acute stage before get treatment
                        success = False
                        report_file.write("BAD: Treatment was given to Individual {} before entering Acute stage\n".format(ind_id))
                    else:
                        actual_treatment_delay=treatment_timestep - acute_timestep +1
                        if math.fabs(actual_treatment_delay - expeted_treatment_delay)>1e-2:
                            success = False
                            report_file.write("BAD: Treatment happened for individual {0} on {1}th day of acute infection, expected {2}th\n".format(ind_id, actual_treatment_delay,expeted_treatment_delay))
                            report_file.write("treatment_timestep: {0}, acute_timestep: {1}.\n".format(treatment_timestep, acute_timestep))

        rate= treatment_count/float(acute_count-exception_count) if (acute_count - exception_count) != 0 else 0
        Treatment_Probability=1
        if math.fabs(rate-Treatment_Probability)>1e-2:
            success = False
            report_file.write("BAD: The proportion of people get treatment in Acute stage is {0},acute_count={1},treatment_count={2}, expected Treatment_Probability = {3}\n".format(rate,acute_count,treatment_count, Treatment_Probability))
        if success:
            report_file.write(sft.format_success_msg( success ) +"All Treatment happened on {0}th day of acute infection.\nThe proportion of people get treatment in Acute stage is {1}\n".format(expeted_treatment_delay,rate))

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
