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

def get_char( key, line ):
    """
    We might want to move this into the dtk_sft module.
    """
    regex = key + "(\w*\d*\w*\d*)"
    match = re.search(regex, line)
    if match != None:
        return match.group(1)
    else:
        raise LookupError

def application( report_file ):
    #pdb.set_trace()
    #print( "Post-processing: " + report_file )

    # get params from config.json
    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    start_time=cdj["Start_Time"]
    simulation_duration = cdj["Simulation_Duration"]
    lines = []
    timestep=start_time

    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search("Update\(\): Time:",line):
                #calculate time step
                timestep+=1
            if re.search( "depositing", line ):
                line = "TimeStep: " + str(timestep) + " " + line
                lines.append( line )
            if re.search("AcquireNewInfection:", line):
                line = "TimeStep: " + str(timestep) + " " + line
                lines.append(line)
            if re.search("state_to_report", line):
                line = "TimeStep: " + str(timestep) + " " + line
                lines.append(line)
    with open ("Ye.txt", "w") as report_file:
        for line in lines:
            if re.search("ndividual 5 ",line) or re.search("individual=5 ", line):
                report_file.write(line)

    with open ("Ye_all.txt", "w") as report_file:
        for line in lines:
            report_file.write(line)

    success = True
    dict_depositing = {}
    dict_infection ={}

    with open( sft.sft_output_filename, "w" ) as report_file:
        if len( lines ) == 0:
            success = False
            report_file.write( "Found no data matching test case.\n" )
        else:
            for i in range(0, len(lines)):
                timestep = get_val("TimeStep: ", lines[i])
                Ind_id = None
                state = None
                if re.search("depositing",lines[i]):
                    Ind_id = get_val("Individual ", lines[i])
                    state = get_char("in state ", lines[i])
                    route = get_char("to route ", lines[i])
                    key = "Time  "+ str(timestep) + " individual " + str(Ind_id) + " route " + str(route)
                    if dict_depositing.has_key(key):
                        success = False
                        report_file.write("BAD: At time {0} individual {1} is depositing to route {2} more than once.\n".format(timestep, Ind_id, route))
                    else:
                        dict_depositing[key]=state
                elif re.search("AcquireNewInfection:",lines[i]):
                    Ind_id=get_val("individual=", lines[i])
                    state = "PRE"
                    if not re.search("route=0", lines[i]):
                        # if the infection is caused by contact or environment route which happen after the shedding
                        # the state is use for next step
                        timestep = int(timestep) + 1
                    key = "Time  " + str(timestep) + " individual " + str(Ind_id)
                    if timestep !=start_time + simulation_duration:
                        #the last time step has no shedding information
                        if dict_infection.has_key(key):
                            if state != dict_infection.get(key):
                                success = False
                                report_file.write("BAD: At time {0} individual {1} is reported to be in state {2} and {3}\n".format(timestep,Ind_id, state, dict_infection.get(key)))
                        else:
                            dict_infection[key]=state
                elif re.search("state_to_report", lines[i]):
                    Ind_id=get_val("individual ",lines[i])
                    state=get_char("= ",lines[i])
                    if state =='SUS':
                        # skip for susceptiable state
                        continue
                    # this state is use for next time step
                    timestep = int(timestep) + 1
                    key = "Time  "+ str(timestep) + " individual " + str(Ind_id)
                    if timestep !=start_time + simulation_duration:
                        #the last time step has no shedding information
                        if dict_infection.has_key(key):
                            if state != dict_infection.get(key):
                                success = False
                                report_file.write("BAD: At time {0} individual {1} is reported to be in state {2} and {3}\n".format(timestep,Ind_id, state, dict_infection.get(key)))
                        else:
                            dict_infection[key] = state
    # with open("dict_d.txt", "w") as report_file:
    #     for key, value in dict_depositing.iteritems():
    #         report_file.write("{0}: {1}\n".format(key,value))
    # with open("dict_i.txt", "w") as report_file:
    #     for key, value in dict_infection.iteritems():
    #         report_file.write("{0}: {1}\n".format(key, value))


            for key in dict_infection:
#                print key + "\n"
                key_1 = key + " route contact"
                key_2 = key + " route environmental"
                state = dict_infection.get(key)
                if dict_depositing.has_key(key_1):
                    depositing_state = dict_depositing.get(key_1)
                    if depositing_state != state:
                        success = False
                        report_file.write("BAD: {0} is depositing in state {1}, expected {2}.\n".format(key_1, depositing_state,state))
                    dict_depositing.pop(key_1, None)
                else:
                    success = False
                    report_file.write("BAD: {0} is in infection state {1} but it's not depositing in route contact.\n".format(key,state))
                if dict_depositing.has_key(key_2):
                    depositing_state = dict_depositing.get(key_2)
                    if depositing_state != state:
                        success = False
                        report_file.write("BAD: {0} is depositing in state {1}, expected {2}.\n".format(key_2, depositing_state, state))
                    dict_depositing.pop(key_2, None)
                else:
                    success = False
                    report_file.write("BAD: {0} is in infection state {1} but it's not depositing in route environment.\n".format(key,state))
            if len(dict_depositing) != 0:
                success = False
                report_file.write("BAD: Some individuals are depositing while they are not reported to be in Infection state. Please see \"depositing.txt\" for details\n")
                with open("depositing.txt", "w") as file:
                    for key, value in dict_depositing.iteritems():
                        file.write("{0} is depositing as state {1}.\n".format(key, value))

            if success:
                report_file.write( sft.format_success_msg( success ) )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
