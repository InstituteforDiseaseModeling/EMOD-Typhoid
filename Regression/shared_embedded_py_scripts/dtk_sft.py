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

def test_binormal_95ci(num_success, num_trails, prob,report_file, category):
    """
    -------------------------------------------------------
        This function test if a binormal distribution falls within the 95% confidence interval
            :param num_success: the number of successes
            :param num_trails: the number of trail
            :param prob: the probability of success for one trail
            :param report_file: for error reporting
            :param category: for error reporting
            :return: True, False
    -------------------------------------------------------
    """
    # calculate the mean and  standard deviation for binormal distribution
    mean = num_trails * prob
    standard_deviation = math.sqrt(prob * (1 - prob) * num_trails)
    # 95% confidence interval
    lower_bound = mean - 2 * standard_deviation
    upper_bound = mean + 2 * standard_deviation
    success = True
    if mean < 5 or num_trails*(1-prob) < 5:
        #The general rule of thumb for normal approximation method is that
        # the sample size n is "sufficiently large" if np >= 5 and n(1-p) >= 5
        # for cases that binormal confidence interval will not work
        success = False
        report_file.write("There is not enough sample size in group {0}: mean = {1}, sample size - mean = {2}.\n".format(category, mean,num_trails * (1 - prob)))
    elif num_success < lower_bound or num_success > upper_bound:
        success = False
        report_file.write("BAD: For category {0}, the success cases is {1}, expected 95% confidence interval ( {2}, {3}).\n".format(category, num_success,lower_bound, upper_bound))
    return success

def test_binormal_99ci(num_success, num_trails, prob,report_file, category):
    """
    -------------------------------------------------------
        This function test if a binormal distribution falls within the 99.73% confidence interval
            :param num_success: the number of successes
            :param num_trails: the number of trail
            :param prob: the probability of success for one trail
            :param report_file: for error reporting
            :param category: for error reporting
            :return: True, False
    -------------------------------------------------------
    """

    # calculate the mean and  standard deviation for binormal distribution
    mean = num_trails * prob
    standard_deviation = math.sqrt(prob * (1 - prob) * num_trails)
    # 99.73% confidence interval
    lower_bound = mean - 3 * standard_deviation
    upper_bound = mean + 3 * standard_deviation
    success = True
    if mean < 5 or num_trails*(1-prob) < 5:
        #The general rule of thumb for normal approximation method is that
        # the sample size n is "sufficiently large" if np >= 5 and n(1-p) >= 5
        success = False
        report_file.write("There is not enough sample size in group {0}: mean = {1}, sample size - mean = {2}.\n".format(category, mean, num_trails*(1-prob)))
    elif num_success < lower_bound or num_success > upper_bound:
        success = False
        report_file.write("BAD: For category {0}, the success cases is {1}, expected 99.75% confidence interval ( {2}, {3}).\n".format(category, num_success,lower_bound, upper_bound))
    return success

def calc_poisson_binormal(prob):
    """
    ------------------------------------------------------
        By definition, a Poisson binormal distribution is a sum of n independent Bernoulli distribution
        this function calculate the mean, standard deviation and variance based on probabilities from n independent Bernoulli distribution
            :param prob: list of probabilities from n independent Bernoulli distribution
            :return: mean, standard deviation and variance
    ------------------------------------------------------
    """
    mean = 0
    variance = 0
    standard_deviation = 0
    for p in prob:
        mean += p
        variance += (1-p)*p
        standard_deviation = math.sqrt(variance)
    return {'mean': mean, 'standard_deviation': standard_deviation, 'variance': variance}

def calc_poisson_prob(mean,num_event):
    """
    ----------------------------------------------------
        This function calculates approximate probability of obeserving num_event events in an interval for a Poisson distribution
            :param mean: the average number of events per interval
            :param num_event: the number of event happened in an interval
            :return: probability of obeserving num_event events in an interval
    ----------------------------------------------------
    """
    a = math.exp(-1.0*mean)
    b = math.pow(mean,float(num_event))
    c = math.factorial(int(num_event))
    prob=a * b / float(c)
    return prob

def test_poisson (trails, rate,report_file, route):
    """
    -----------------------------------------------------
        This function test if a distribution is a Poisson distribution with given rate
        I am testing it based on probability of events for a Possion distribution since kstest for poisson distribution has a bug
            :param trails: distribution to test, it contains values 0, 1, 2, ...
            :param rate: the average number of events per interval
            :param report_file:
            :param route:
            :return:
    -----------------------------------------------------
    """
    #calculate probability of k events for a Possion distribution
    success= True
    for k in trails:
        # calculate the probability of k envents in an interval
        prob=calc_poisson_prob(rate,k)
        #report_file.write("Poisson probability for {0} at rate {1} , # of event {2} is {3}, expected larger than 0.05.\n".format(route, rate, k, p))
        if prob*len(trails) < 1:
            success = False
            report_file.write("BAD: Poisson probability for {0} at rate {1} , # of event {2} is {3}, expected larger than {4}.\n".format(route, rate, k, prob, 1/float(len(trails))))
    return success

def test_lognorm(timers,mu,sigma,report_file, category):
    """
    -----------------------------------------------------
        kstest for lognormal distribution
            :param timers: the distribution to test
            :param mu: mean, which is equal to sqrt(scale)
            :param sigma: the standard deviation
            :param report_file: for error reporting
            :param category: for error reporting
            :return: True, False
    -----------------------------------------------------
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
    #report_file.write("s is {0}, p is : {1} for {2}.\n".format(s, p, category))

    # calculate the critical values for Statistic from KS table
    ks_table = [0.975, 0.842, 0.708, 0.624, 0.565, 0.521, 0.486, 0.457, 0.432, 0.410, 0.391, 0.375, 0.361, 0.349, 0.338,
                0.328, 0.318, 0.309, 0.301, 0.294, 0.270, 0.240, 0.230]
    critical_value_s = 0
    if len(timers) <= 20:
        critical_value_s = ks_table[len(timers) - 1]
    elif len(timers) <= 25:
        critical_value_s = ks_table[20]
    elif len(timers) <= 30:
        critical_value_s = ks_table[21]
    elif len(timers) <= 35:
        critical_value_s = ks_table[22]
    else:
        critical_value_s = 1.36 / math.sqrt(len(timers))

    if s > critical_value_s or p < 5e-2:
        report_file.write("BAD: log normal kstest result for {0} is: statistic={1}, pvalue={2}, expected s less than {3} and p larger than 0.05.\n".format(category, s, p, critical_value_s))
        return False
    else:
        return True


