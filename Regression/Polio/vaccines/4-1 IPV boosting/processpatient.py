import json
import matplotlib.pyplot as plt
import numpy as np
from random import gauss
from random import random
from math import log, exp

def main():
    data = json.load(open('output/PolioSurveyJSONAnalyzer_survey_025_1_1.json'))
    data2 = json.load(open('output/PolioSurveyJSONAnalyzer_survey_125_1_1.json'))    
    data3 = json.load(open('output/PolioSurveyJSONAnalyzer_survey_225_1_1.json'))
    
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
    
    value_avg["PV1"] = 3.22
    value_stddev["PV1"]  = 0.59
    value_max["PV1"] = 15.1
    value_avg["PV2"] = 3.7
    value_stddev["PV2"] = 1
    value_max["PV2"] = 15.5
    value_avg["PV3"] = 3.62
    value_stddev["PV3"] = 0.7
    value_max["PV3"] = 14.7

    boost_avg["PV1"] = 5.08
    boost_stddev["PV1"]  = 0.9
    boost_avg["PV2"] = 6.16
    boost_stddev["PV2"] = 0.8
    boost_avg["PV3"] = 6.37
    boost_stddev["PV3"] = 0.6

    mucosal_immunogenicity = 0.107

    n = 10000

    output_value1 = {}
    output_value2 = {}
    output_value3 = {}
    output_mucosal1 = {}
    output_mucosal2 = {}
    output_mucosal3 = {}
    for v in virus_type:
        output_value1[v] = []
        output_value2[v] = []
        output_value3[v] = []
        output_mucosal1[v] = []
        output_mucosal2[v] = []
        output_mucosal3[v] = []
        #priming
        for i in xrange(n):
            rand_num = gauss(0,1)
            if rand_num >= 2:
                rand_num = 2
            if rand_num <= -2:
                rand_num = -2

            output_temp = pow(2, value_avg[v] + rand_num * value_stddev[v])
            output_temp2 = pow(2, mucosal_immunogenicity*(value_avg[v] + rand_num * value_stddev[v]))
            output_value1[v].append(output_temp)
            output_mucosal1[v].append(output_temp2)
            
        #boost
        for i in xrange(len(output_value1[v])):
            ab = output_value1[v][i]
            abm = output_mucosal1[v][i]
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
                Nab_saturation2 = log(2) - log(abm) / value_max[v]
                if Nab_saturation2 < 0:
                     Nab_saturation2 = 0
                boost_value = ab * exp(log2Nab_boost * Nab_saturation)
                boost_value2 = abm * exp(log2Nab_boost * Nab_saturation2 * mucosal_immunogenicity)
                output_value2[v].append(boost_value)
                output_mucosal2[v].append(boost_value2)
            else:
                rand_num = gauss(0,1)
                if rand_num >= 2:
                    rand_num = 2
                if rand_num <= -2:
                    rand_num = -2                   
                output_temp = pow(2, value_avg[v] + rand_num * value_stddev[v])
                output_temp2 = pow(2, mucosal_immunogenicity*(value_avg[v] + rand_num * value_stddev[v]))
                output_value2[v].append(output_temp)
                output_mucosal2[v].append(output_temp2)

        #boost2
        for i in xrange(len(output_value2[v])):
            ab = output_value2[v][i]
            abm = output_mucosal2[v][i]
            if ab > 1:
                rand_num = gauss(0,1)
                if rand_num >= 2:
                    rand_num = 2
                if rand_num <= -2:
                    rand_num = -2
                log2Nab_boost = boost_avg[v] + rand_num* boost_stddev[v]
                Nab_saturation = log(2) - log(ab) / value_max[v]
                Nab_saturation2 = log(2) - log(abm) / value_max[v]
                if Nab_saturation2 < 0:
                     Nab_saturation2 = 0
                boost_value = ab * exp(log2Nab_boost * Nab_saturation )
                boost_value2 = abm * exp(log2Nab_boost * Nab_saturation2 * mucosal_immunogenicity)
                output_value3[v].append(boost_value)
                output_mucosal3[v].append(boost_value2)
            else:
                rand_num = gauss(0,1)
                if rand_num >= 2:
                    rand_num = 2
                if rand_num <= -2:
                    rand_num = -2
                output_temp = pow(2, value_avg[v] + rand_num * value_stddev[v])
                output_temp2 = pow(2, mucosal_immunogenicity*(value_avg[v] + rand_num * value_stddev[v]))
                output_value3[v].append(output_temp)
                output_mucosal3[v].append(output_temp2)

    bins = range(0,12)
    bins2 = []
    for i in bins:
        bins2.append(bins[i]+0.5)
    plt.figure(1)
    plt.subplot(321)
    plt.hist(np.log2(mucosal_Nab["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal1["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Mucosal Log2 Nab")
    plt.subplot(323)
    plt.hist(np.log2(mucosal_Nab["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal1["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Mucosal Log2 Nab")
    plt.subplot(325)
    plt.hist(np.log2(mucosal_Nab["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal1["PV3"]), log=True, bins = bins, alpha=0.5)
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
    plt.suptitle("PolioSurveyJSONAnalyzer_survey_025_1_1.json")
    plt.tight_layout()
    plt.figure(2)
    plt.subplot(321)
    plt.hist(np.log2(mucosal_Nab2["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal2["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Mucosal Log2 Nab")
    plt.subplot(323)
    plt.hist(np.log2(mucosal_Nab2["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal2["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Mucosal Log2 Nab")
    plt.subplot(325)
    plt.hist(np.log2(mucosal_Nab2["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal2["PV3"]), log=True, bins = bins, alpha=0.5)
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
    plt.suptitle("PolioSurveyJSONAnalyzer_survey_125_1_1.json")
    plt.tight_layout()
    plt.figure(3)
    plt.subplot(321)
    plt.hist(np.log2(mucosal_Nab3["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal3["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Mucosal Log2 Nab")
    plt.subplot(323)
    plt.hist(np.log2(mucosal_Nab3["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal3["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Mucosal Log2 Nab")
    plt.subplot(325)
    plt.hist(np.log2(mucosal_Nab3["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal3["PV3"]), log=True, bins = bins, alpha=0.5)
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
    plt.suptitle("PolioSurveyJSONAnalyzer_survey_225_1_1.json")
    plt.tight_layout()
    plt.show()
    

if __name__=="__main__":
    main()
