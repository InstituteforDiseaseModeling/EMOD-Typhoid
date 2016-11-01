#!/usr/bin/python

import math
from scipy import stats

"""
This module centralizes some small bits of functionality for SFT tests
to make sure we are using the same strings for messages and filenames.
"""

sft_output_filename = "scientific_feature_report.txt"

def format_success_msg( success ):
    report_msg = "SUMMARY: Success={0}\n".format( success )
    return report_msg 

def test_lognorm(timers,mu,sigma,report_file, category):
    """
        kstest for lognormal distribution
    """
    #print( "Running test_lognorm for " + category )
    scale=math.exp(mu)
    result = stats.kstest(timers, 'lognorm', args=(sigma, 0, scale))
    #print( str( result ) )
    p = s = 0
    # NOTE: different versions of kstest seem to produce different output.
    if "pvalue" in result:
        p = float(get_val("pvalue=", str(result)))
        s = float(get_val("statistic=", str(result)))
    else:
        s = result[0]
        p = result[1]
    report_file.write("s is {0}, p is : {1} for {2}.\n".format(s, p, category))

    # calculate the critical values for Statistic from KS table
    ks_table=[0.975, 0.842, 0.708, 0.624, 0.565, 0.521, 0.486, 0.457, 0.432, 0.410 , 0.391, 0.375, 0.361, 0.349, 0.338, 0.328, 0.318, 0.309, 0.301, 0.294, 0.270, 0.240, 0.230]
    critical_value_s = 0
    if len(timers) <= 20:
        critical_value_s = ks_table[len(timers)-1]
    elif len(timers) <= 25:
        critical_value_s = ks_table[20]
    elif len(timers) <= 30:
        critical_value_s = ks_table[21]
    elif len(timers) <= 35:
        critical_value_s = ks_table[22]
    else:
        critical_value_s = 1.36/math.sqrt(len(timers))

    if s > critical_value_s or p < 5e-2:
        report_file.write("BAD: log normal kstest result for {0} is: statistic={1}, pvalue={2}, expected s close to {3} and p larger than 0.05.\n".format(category, s, p, critical_value_s)) 
        return False
    else:
        return True
    
