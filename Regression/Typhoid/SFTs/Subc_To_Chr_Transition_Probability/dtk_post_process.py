#!/usr/bin/python
# This SFT test the following statements:
# All subclinical infections go to Chronic or Susceptible state.
# The proportion of individuals who move to chronic infections is determined by the config parameter Config:CPG multiply by hardcoded Gallstones table. The remainder shall move to Susceptible.
# This test passes when the number of went to Chronic cases is within Binormal 95% confidence interval
# for each test, there is 5% of chance that we will reject the hypothesis while it's true

import re
import json
import math
import pdb
import os
import dtk_sft as sft

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

    cdj = json.loads( open( "config.json" ).read() )["parameters"]
    start_time=cdj["Start_Time"]
    timestep=start_time
    lines=[]

    with open( "test.txt" ) as logfile:
        for line in logfile:
            if re.search("Update\(\): Time:",line):
                #calculate time step
                timestep+=1
            if re.search( "just went chronic", line) and (re.search("from subclinical", line) or re.search("from Subclinical", line)):
                #append time step and all Infection stage transition to list
                line="TimeStep: "+str(timestep)+ " " + line
                lines.append( line )
            if re.search("just recovered", line) and (re.search("from subclinical", line) or re.search("from Subclinical", line)):
                # append time step and all Infection stage transition to list
                line = "TimeStep: " + str(timestep) + " " + line
                lines.append(line)

    # 4*10 list to store the count for cases [0][]: Chr_male, [1][]: Chr_female, [2][]: Sus_male. [3][]: Sus_female
    count=[[0]*9 for _ in range(4)]

    tcpm=cdj["Typhoid_Carrier_Probability_Male"]
    tcpf=cdj["Typhoid_Carrier_Probability_Female"]

    gpag_male=[0.0, 0.0, 0.045, 0.134, 0.167, 0.198, 0.247, 0.435, 0.4]
    gpag_female=[0.0, 0.097, 0.234, 0.431, 0.517, 0.60, 0.692, 0.692, 0.555]



    success = True
    with open( sft.sft_output_filename, "w" ) as report_file:
        if len( lines ) == 0 :
            success = False
            report_file.write( "Found no data matching test case.\n" )
        else:
            for line in lines:
                age = float(get_val(" age ", line))
                sex = "female" if (re.search("sex 1", line) or re.search("sex Female", line)) else "male"
                if re.search("just went chronic", line):
                    # to Chronic
                    #  python 2.7 the (int / int) operator is integer division
                    i = int(age) / 10
                    # for age > 80, put them into the last age group
                    if i > 8: i = 8
                    if sex=="male":
                        count[0][i] += 1
                    else:
                        count[1][i] += 1
                else:
                    # to Susceptible
                    # python 2.7 the (int / int) operator is integer division
                    i = int(age) / 10
                    # for age > 80, put them into the last age group
                    if i > 8: i = 8
                    if sex == "male":
                        count[2][i] += 1
                    else:
                        count[3][i] += 1
            # calculate theoretic probability of becoming a Chronic carrier in two 1*9 lists
            theoretic_p_male=[x*tcpm for x in gpag_male]
            theoretic_p_female=[x*tcpf for x in gpag_female]
            # calculate actual probability of becoming a Chronic carrier in four 1*9 lists
            actual_p_male=[x/float(x+y) if (x+y)!=0 else -1 for x, y in zip(count[0],count[2])]
            actual_p_female=[x/float(x+y) if (x+y)!=0 else -1 for x, y in zip(count[1],count[3])]

            for x in range(0,9):
                age = ["0-9", "10-19", "20-29", "30-39", "40-49", "50-59", "60-69", "70-79", "80+"]
                # calculate these counts for error logging
                actual_chr_count_male = count[0][x]
                actual_count_male = count[0][x] + count[2][x]
                actual_chr_count_female = count[1][x]
                actual_count_female = count[1][x] + count[3][x]
                # Male
                category = 'sex: Male, age: ' + age[x]
                if actual_count_male == 0:
                    success = False
                    report_file.write("Found no male in age group {0} went to Chronic state or was recovered from SubClinical state.\n".format(age[x]))
                elif theoretic_p_male[x] < 5e-2 or theoretic_p_male[x] > 0.95:
                    # for cases that binormal confidence interval will not work: prob close to 0 or 1
                    if math.fabs(actual_p_male[x] - theoretic_p_male[x]) > 5e-2:
                        success = False
                        report_file.write("BAD: Proportion of {0} SubClinical cases that become Chronic is {1}, expected {2}.\n".format(category,actual_p_male[x], theoretic_p_male[x]))
                elif not sft.test_binormal_95ci(actual_chr_count_male, actual_count_male, theoretic_p_male[x],report_file, category):
                    success = False

                # Female
                category = 'sex: Female, age: ' + age[x]
                if actual_count_female == 0:
                    success = False
                    report_file.write("Found no female in age group {0} went to Chronic state or was recovered from SubClinical state.\n".format(age[x]))
                elif theoretic_p_female[x] < 1e-2 or theoretic_p_female[x] > 0.99:
                    # for cases that binormal confidence interval will not work: prob close to 0 or 1
                    if math.fabs(actual_p_female[x] - theoretic_p_female[x]) > 5e-2:
                        success = False
                        report_file.write("BAD: Proportion of {0} SubClinical cases that become Chronic is {1}, expected {2}.\n".format(category,actual_p_female[x], theoretic_p_female[x]))
                elif not sft.test_binormal_95ci(actual_chr_count_female, actual_count_female, theoretic_p_female[x],report_file, category):
                    success = False
        if success:
            report_file.write( sft.format_success_msg( success ) )
            os.remove( "test.txt" )

    #dtk_plot_wrapper.doit( actual_infectiousness, title="Asymptomatic Naive Infectiousness over time" );

if __name__ == "__main__":
    # execute only if run as a script
    application( "" )
