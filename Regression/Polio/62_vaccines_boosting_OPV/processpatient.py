import json
import matplotlib.pyplot as plt
import numpy as np
from random import gauss
from random import random
from math import log, exp

def main():
    survey1 = "output/PolioSurveyJSONAnalyzer_survey01_1_33.json"
    survey2 = "output/PolioSurveyJSONAnalyzer_survey11_1_133.json"
    survey3 = "output/PolioSurveyJSONAnalyzer_survey024_1_233.json"
    data = json.load(open(survey1))
    data2 = json.load(open(survey2))    
    data3 = json.load(open(survey3))
    
    patient = data["patient_array"]
    virus_type = ["PV1", "PV2", "PV3"]
    mucosal_Nab = {}
    humoral_Nab = {}
    for v in virus_type:
        mucosal_Nab[v]=[]
        humoral_Nab[v]=[]
    for p in patient:
        for v in virus_type:
            mucosal_Nab[v].append(p["mucosal_Nab"][v][0])
            humoral_Nab[v].append(p["humoral_Nab"][v][0])


    patient = data2["patient_array"]
    mucosal_Nab2 = {}
    humoral_Nab2 = {}
    for v in virus_type:
        mucosal_Nab2[v]=[]
        humoral_Nab2[v]=[]
    for p in patient:
        for v in virus_type:
            mucosal_Nab2[v].append(p["mucosal_Nab"][v][0])
            humoral_Nab2[v].append(p["humoral_Nab"][v][0])

    patient = data3["patient_array"]
    mucosal_Nab3 = {}
    humoral_Nab3 = {}
    for v in virus_type:
        mucosal_Nab3[v]=[]
        humoral_Nab3[v]=[]
    for p in patient:
        for v in virus_type:
            mucosal_Nab3[v].append(p["mucosal_Nab"][v][0])
            humoral_Nab3[v].append(p["humoral_Nab"][v][0])

    #generated data
    value_avg={}
    value_stddev = {}
    value_max = {}
    boost_avg = {}
    boost_stddev = {}
    challenge_dose = {}
    interference = {}
    SI = {}
    
    value_avg["PV1"] = 5.92
    value_stddev["PV1"]  = 2.3
    value_max["PV1"] = 15.0
    value_avg["PV2"] = 6.66
    value_stddev["PV2"] = 2.5
    value_max["PV2"] = 15.0
    value_avg["PV3"] = 5.51
    value_stddev["PV3"] = 2.5
    value_max["PV3"] = 15

    boost_avg["PV1"] = 5.33
    boost_stddev["PV1"]  = 0.5
    boost_avg["PV2"] = 6.18
    boost_stddev["PV2"] = 2.5
    boost_avg["PV3"] = 3.14
    boost_stddev["PV3"] = 0.4

    challenge_dose["PV1"] = 1333521.0
    challenge_dose["PV2"] = 1333521.0
    challenge_dose["PV3"] = 1333521.0

    interference["PV1"] = 0.0
    interference["PV2"] = 0.178
    interference["PV3"] = 0.354

    SI["PV1"] = 0.171
    SI["PV2"] = 0.296
    SI["PV3"] = 0.137

    n = 10000

    output_value1 = {}
    output_value2 = {}
    output_value3 = {}
    for v in virus_type:
        output_value1[v] = []
        output_value2[v] = []
        output_value3[v] = []

        #priming
        for i in xrange(n):
            ab = 1
            neutralization = (1+0.034*(ab-1.0)*(1.0-exp(-1.0/0.034)))/ab
            p_inf = 1-pow(1.0 + challenge_dose[v], -SI[v] * neutralization)
            rand_num = gauss(0,1)
            if rand_num >= 2:
                rand_num = 2
            if rand_num <= -2:
                rand_num = -2
                
            interference_factor = 0.0
#            if random() < p_inf:
#                interference_factor = interference[v]
            p_inf_new = (1-interference_factor)*p_inf
            if random() < p_inf_new:
                output_temp = pow(2, value_avg[v] + rand_num * value_stddev[v])
            else:
                output_temp = 1.0
            output_value1[v].append(output_temp)

        #boost
        for ab in output_value1[v]:
            neutralization = (1+0.034*(ab-1.0)*(1.0-exp(-1.0/0.034)))/ab
            p_inf = 1-pow(1.0 + challenge_dose[v], -SI[v] * neutralization)
            interference_factor = 0.0
#            if random() < p_inf:
#                interference_factor = interference[v]
            p_inf_new = (1-interference_factor)*p_inf
            if random() < p_inf_new:
                if ab > 1:
                    rand_num = gauss(0,1)
                    if rand_num >= 2:
                        rand_num = 2
                    if rand_num <= -2:
                        rand_num = -2
                    log2Nab_boost = boost_avg[v] + rand_num* boost_stddev[v]
                    Nab_saturation = log(2) - log(ab) / value_max[v]
                    if Nab_saturation < 0:
                         Nab_saturation = 0
                    boost_value = ab * exp(log2Nab_boost * Nab_saturation)
                    output_value2[v].append(boost_value)
                else:
                    rand_num = gauss(0,1)
                    if rand_num >= 2:
                        rand_num = 2
                    if rand_num <= -2:
                        rand_num = -2                   
                    output_temp = pow(2, value_avg[v] + rand_num * value_stddev[v])
                    output_value2[v].append(output_temp)
            else:
                output_value2[v].append(ab)

        #boost2
        for ab in output_value2[v]:
            neutralization = (1+0.034*(ab-1.0)*(1.0-exp(-1.0/0.034)))/ab
            p_inf = 1-pow(1.0 + challenge_dose[v], -SI[v] * neutralization)
            interference_factor = 0.0
#            if random() < p_inf:
#                interference_factor = interference[v]
            p_inf_new = (1-interference_factor)*p_inf
            if random() < p_inf_new:
                if ab > 1:
                    rand_num = gauss(0,1)
                    if rand_num >= 2:
                        rand_num = 2
                    if rand_num <= -2:
                        rand_num = -2
                    log2Nab_boost = boost_avg[v] + rand_num* boost_stddev[v]
                    Nab_saturation = log(2) - log(ab) / value_max[v]
                    boost_value = ab * exp(log2Nab_boost * Nab_saturation)
                    output_value3[v].append(boost_value)
                else:
                    rand_num = gauss(0,1)
                    if rand_num >= 2:
                        rand_num = 2
                    if rand_num <= -2:
                        rand_num = -2
                    output_temp = pow(2, value_avg[v] + rand_num * value_stddev[v])
                    output_value3[v].append(output_temp)
            else:
                output_value3[v].append(ab)
    

    bins = range(0,12)
    bins2 = []
    for i in bins:
        bins2.append(bins[i]+0.5)
    plt.figure(1)
    plt.subplot(321)
    plt.hist(np.log2(mucosal_Nab["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value1["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Mucosal Log2 Nab")
    plt.subplot(323)
    plt.hist(np.log2(mucosal_Nab["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value1["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Mucosal Log2 Nab")
    plt.subplot(325)
    plt.hist(np.log2(mucosal_Nab["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value1["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Mucosal Log2 Nab")
    plt.subplot(322)
    plt.hist(np.log2(humoral_Nab["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value1["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Humoral Log2 Nab")
    plt.subplot(324)
    plt.hist(np.log2(humoral_Nab["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value1["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Humoral Log2 Nab")
    plt.subplot(326)
    plt.hist(np.log2(humoral_Nab["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value1["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Humoral Log2 Nab")
    plt.suptitle("chart 1: " + survey1)
    plt.tight_layout()
    plt.figure(2)
    plt.subplot(321)
    plt.hist(np.log2(mucosal_Nab2["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value2["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Mucosal Log2 Nab")
    plt.subplot(323)
    plt.hist(np.log2(mucosal_Nab2["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value2["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Mucosal Log2 Nab")
    plt.subplot(325)
    plt.hist(np.log2(mucosal_Nab2["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value2["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Mucosal Log2 Nab")
    plt.subplot(322)
    plt.hist(np.log2(humoral_Nab2["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value2["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Humoral Log2 Nab")
    plt.subplot(324)
    plt.hist(np.log2(humoral_Nab2["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value2["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Humoral Log2 Nab")
    plt.subplot(326)
    plt.hist(np.log2(humoral_Nab2["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value2["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Humoral Log2 Nab")
    plt.suptitle("chart 2: " + survey2)
    plt.tight_layout()
    plt.figure(3)
    plt.subplot(321)
    plt.hist(np.log2(mucosal_Nab3["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value3["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Mucosal Log2 Nab")
    plt.subplot(323)
    plt.hist(np.log2(mucosal_Nab3["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value3["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Mucosal Log2 Nab")
    plt.subplot(325)
    plt.hist(np.log2(mucosal_Nab3["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value3["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Mucosal Log2 Nab")
    plt.subplot(322)
    plt.hist(np.log2(humoral_Nab3["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value3["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Humoral Log2 Nab")
    plt.subplot(324)
    plt.hist(np.log2(humoral_Nab3["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value3["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Humoral Log2 Nab")
    plt.subplot(326)
    plt.hist(np.log2(humoral_Nab3["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value3["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Humoral Log2 Nab")
    plt.suptitle("chart 3: " + survey3)
    plt.tight_layout()
    plt.show()
    

if __name__=="__main__":
    main()
