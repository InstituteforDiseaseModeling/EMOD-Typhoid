#!/usr/bin/python
# This SFT test the following statements:
# All new infections from route contact are in category high
# All new infections from outbreak are in category low
# At each time step, the # of new infections from route contact in log matches the data in "New Infections By Route (CONTACT)" channel in InsetChart.json
# At each time step, the # of new infections from route environment in log matches the data in "New Infections By Route (ENVIRONMENT)" channel in InsetChart.json

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
    isj=json.loads(open("output\InsetChart.json").read())["Channels"]
    nibrc=isj["New Infections By Route (CONTACT)"]["Data"]
    nibre=isj["New Infections By Route (ENVIRONMENT)"]["Data"]

    timestep=start_time
    count_new_infection = 0
    count_Outbreak = 0
    count_contact = 0
    count_enviro = 0
    new_infection=[]
    new_Outbreak = []
    new_contact = []
    new_enviro = []
    lines=[]

    #length = os.path.getsize("test.txt")

    with open( "test.txt" ) as logfile:
        for num, line in enumerate(logfile):
            if re.search("Update\(\): Time:",line):
                #calculate time step and collect counters
                timestep+=1
                new_infection.append(count_new_infection)
                new_Outbreak.append(count_Outbreak)
                new_contact.append(count_contact)
                new_enviro.append(count_enviro)
                # reset all counters at each time step
                count_new_infection = 0
                count_Outbreak = 0
                count_contact = 0
                count_enviro = 0
            if re.search( "Calculated prepatent duration", line ):
                count_new_infection+=1
                line = "line: "+ str(num) + " TimeStep: " + str(timestep) + " " + line
                lines.append(line)
            if re.search( "AcquireNewInfection: route 0", line ):
                #print line
                count_Outbreak += 1
                line = "line: "+ str(num) + " TimeStep: " + str(timestep) + " " + line
                lines.append(line)
            if re.search( "AcquireNewInfection: route 1", line ):
                #print line
                count_contact += 1
                line = "line: "+ str(num) + " TimeStep: " + str(timestep) + " " + line
                lines.append(line)
            if re.search( "AcquireNewInfection: route 2", line ):
                #print line
                count_enviro += 1
                line = "line: "+ str(num) + " TimeStep: " + str(timestep) + " " + line
                lines.append(line)

    success = True
    error_log = []
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len(new_infection) == 0 or len(lines) == 0:
            success = False
            report_file.write( "Found no data matching test case.\n" )
        else:
            for x in range(0, len(new_infection)-1):
                new_infection_log = new_infection[x+1]
                new_outbreak_log = new_Outbreak[x+1]
                new_contact_log = new_contact[x+1]
                new_enviro_log = new_enviro[x+1]
                total_log=new_outbreak_log+new_enviro_log+new_contact_log
                new_infection_contact_output=nibrc[x]
                new_infection_environment_output=nibre[x]
                if new_infection_log !=  total_log:
                    success = False
                    report_file.write("BAD: At time {0}: new prepatent case = {1}, expected {2}(new infection by route (contact) = {3}, new infection by route (environment) = {4}, new infection by Outbreak = {5}).\n".format(x+start_time + 1,new_infection_log, total_log, new_contact_log, new_enviro_log, new_outbreak_log))
                if new_contact_log != new_infection_contact_output:
                    success = False
                    report_file.write(
                        "BAD: At time {0}: new infection by contact route is {1} from Stdout while it's {2} from InsetChart ).\n".format(
                            x + start_time + 1, new_contact_log, new_infection_contact_output))
                if new_enviro_log != new_infection_environment_output:
                    success = False
                    report_file.write(
                        "BAD: At time {0}: new infection by environment route is {1} from Stdout while it's {2} from InsetChart ).\n".format(
                            x + start_time + 1, new_enviro_log, new_infection_environment_output))
        for i in range(0, len(lines)):
            line = lines[i]
            if re.search("AcquireNewInfection: route 0", line):
                next_line= lines[i+1]
                if not re.search("doseTracking = Low", next_line):
                    error_log.append(line)
                    error_log.append(next_line)
            if re.search("AcquireNewInfection: route 1", line):
                next_line= lines[i+1]
                if not re.search("doseTracking = High", next_line):
                    error_log.append(line)
                    error_log.append(next_line)
        if len(error_log) != 0:
            success = False
            report_file.write("BAD: Some infected individuals didn't fall in the right doseTracking categories. Expected: route 0 - doseTracking = Low and route 1 - doseTracking = High. Please see the details in \"categogy_error_log.txt\".\n")
            with open("category_error_log.txt","w") as log:
                for line in error_log:
                    log.write(line)

        if success:
            report_file.write( sft.format_success_msg( success ) )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
