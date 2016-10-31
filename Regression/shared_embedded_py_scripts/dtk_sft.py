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
    #if s > 1e-1 or p < 5e-2:
    # changing to use p-value only.
    if p < 5e-2:
        report_file.write("BAD: log normal kstest result for {0} is: statistic={1}, pvalue={2}, expected s close to 0 and p larger than 0.05.\n".format(category, s, p))
        return False
    else:
        return True
    
