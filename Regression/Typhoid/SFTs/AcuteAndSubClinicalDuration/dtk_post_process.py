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

def test_lognorm(timers,mu,sigma,report_file, category):
    """
        kstest for lognormal distribution
    """
    scale=math.exp(mu)
    result = stats.kstest(timers, 'lognorm', args=(sigma, 0, scale))
    p = float(get_val("pvalue=", str(result)))
    s = float(get_val("statistic=", str(result)))
    report_file.write("s is {0}, p is : {1} for {2}.\n".format(s, p, category))
    if s > 1e-1 or p < 5e-2:
        report_file.write("BAD: log normal kstest result for {0} is: statistic={1}, pvalue={2}, expected s close to 0 and p larger than 0.05.\n".format(category, s, p))
        return False
    else:
        return True
    
def application( report_file ):
    #pdb.set_trace()
    #print( "Post-processing: " + report_file ) 
    lines = []
    with open( "test.txt" ) as logfile:
        for line in logfile:
            # collect all lines of
            if re.search( "Infection stage transition", line ):
                lines.append( line )

    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    #tcri = cdj["TCRI"]

    acute_mu_over_30=1.258
    acute_sigma_over_30=0.788
    acute_mu_under_30=1.172
    acute_sigma_under_30=0.483
    subc_mu_over_30=1.258
    subc_sigma_over_30=0.788
    subc_mu_under_30=1.172
    subc_sigma_under_30=0.483


    success = True
    Timers_acute_over_30=[]
    Timers_acute_under_30=[]
    Timers_subc_over_30=[]
    Timers_subc_under_30=[]


    with open( sft.sft_output_filename, "w" ) as report_file:
        if len( lines ) == 0:
            success = False
            report_file.write( "Found no data matching test case.\n" )
        else:
            for line in lines:
                duration=None
                age=None
                if re.search("->Acute", line):
                    # get duration and convert to week
                    duration = float( get_val( "dur=", line ) )
                    duration /= 7
                    age=float(get_val("Age=", line))
                    if age > 30:
                        Timers_acute_over_30.append(duration)
                    else:
                        Timers_acute_under_30.append(duration)
                elif re.search("->SubC", line):
                    # get duration and convert to week
                    duration = float( get_val( "dur=", line ) )
                    duration /= 7
                    age=float(get_val("Age=", line))
                    if age > 30:
                        Timers_subc_over_30.append(duration)
                    else:
                        Timers_subc_under_30.append(duration)


            if Timers_acute_over_30 == [] and Timers_acute_under_30 == [] and Timers_subc_over_30 == [] and Timers_subc_under_30 == [] and Timers_chronic == []:
                report_file.write("Found no duration in the test case.\n")
            else:
                if Timers_acute_over_30 != []:
                    if not test_lognorm(Timers_acute_over_30,acute_mu_over_30,acute_sigma_over_30,report_file, "Acute_over_30"):
                        success=False
                if Timers_acute_under_30 != []:
                    if not test_lognorm(Timers_acute_under_30,acute_mu_under_30,acute_sigma_under_30,report_file, "Acute_under_30"):
                        success=False
                if Timers_subc_over_30 != []:
                    if not test_lognorm(Timers_subc_over_30,subc_mu_over_30,subc_sigma_over_30,report_file, "SubClinical_over_30"):
                        success=False
                if Timers_subc_under_30 != []:
                    if not test_lognorm(Timers_subc_under_30,subc_mu_under_30,subc_sigma_under_30,report_file, "SubClinical_under_30"):
                        success=False

        if not success:
            report_file.write("Timers and logs in sorted_duration.json\n")
            myjson = {}
            Duration_Acute_Over_30 = {"Data": sorted(Timers_acute_over_30)}
            Duration_Acute_Under_30 = {"Data": sorted(Timers_acute_under_30)}
            Duration_Subc_Over_30 = {"Data": sorted(Timers_subc_over_30)}
            Duration_Subc_Under_30 = {"Data": sorted(Timers_subc_under_30)}
            myjson["Duration_Acute_Over_30"] = Duration_Acute_Over_30
            myjson["Duration_Acute_Under_30"] = Duration_Acute_Under_30
            myjson["Duration_Subc_Over_30"] = Duration_Subc_Over_30
            myjson["Duration_Subc_Under_30"] = Duration_Subc_Under_30
            with open("sorted_duration.json", "w") as outfile:
                outfile.write(json.dumps(myjson, indent=4))
        else:
            report_file.write(sft.format_success_msg(success))

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
