#!/usr/bin/python

import re
import json
import pdb
import os
import dtk_sft as sft
import numpy as np
#import test_lognorm

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
    lines = []
    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search( "Calculated prepatent duration", line ):
                # append New Infection to list
                lines.append(line)

    #cdj = json.loads( open( "config.json" ).read() )["parameters"]

    lognormal_mu_h = 1.5487
    lognormal_sigma_h=0.3442
    lognormal_mu_m = 2.002
    lognormal_sigma_m = 0.7604
    lognormal_mu_l = 2.235
    lognormal_sigma_l = 0.4964
    success = True
    with open( "ye.txt", "w" ) as report_file:
        for line in lines:
            report_file.write(line)


    with open( sft.sft_output_filename, "w" ) as report_file:
        timers_h=[]
        timers_m=[]
        timers_l=[]
        if len( lines ) == 0:
            success = False
            report_file.write( "Found no data matching test case.\n")
        else:
            for line in lines:
                duration=float(get_val("Calculated prepatent duration = ", line))
                if re.search("doseTracking = Low", line):
                    timers_l.append(duration)
                elif re.search("doseTracking = High", line):
                    timers_h.append(duration)
                elif re.search("doseTracking = Medium", line):
                    timers_m.append(duration)
                else:
                    category=line.split()[-1]
                    report_file.write("Found dose category: {} that does not fall in High, Medium and Low category.\n".format(category))

            if timers_l==[] and timers_h==[] and timers_m==[]:
                report_file.write( "Found no data matching test case.\n")
            else:
                if timers_l==[]:
                    report_file.write("Found no Prepatent case in Category doseTracking = Low.\n")
                elif not sft.test_lognorm(timers_l,lognormal_mu_l,lognormal_sigma_l,report_file, "Low"):
                    success=False
                if timers_m == []:
                    report_file.write("Found no Prepatent case in Category doseTracking = Medium.\n")
                elif not sft.test_lognorm(timers_m, lognormal_mu_m, lognormal_sigma_m, report_file, "Medium"):
                    success = False
                if timers_h == []:
                    report_file.write("Found no Prepatent case in Category doseTracking = High.\n")
                elif not sft.test_lognorm(timers_h, lognormal_mu_h, lognormal_sigma_h, report_file, "high"):
                    success = False

            if not success:
                report_file.write("Timers and logs in sorted_duration.json\n")
                myjson = {}
                duration_timers_high = {"Data": sorted(timers_h)}
                duration_timers_medium = {"Data": sorted(timers_m)}
                duration_timers_low = {"Data": sorted(timers_l)}
                myjson["duration_timers_high"] = duration_timers_high
                myjson["duration_timers_medium"] = duration_timers_medium
                myjson["duration_timers_low"] = duration_timers_low

                with open("sorted_duration.json", "w") as outfile:
                    outfile.write(json.dumps(myjson, indent=4))
            else:
                report_file.write( sft.format_success_msg( success ) )



    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
