import json
import matplotlib.pyplot as plt
import numpy as np
from random import gauss
from random import random

def main():
    data = json.load(open('output/PolioSurveyJSONAnalyzer_m3_1_402.json'))
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

    #generated data
    value_avg={}
    value_stddev = {}
    value_max = {}
    challenge_dose = {}
    interference = {}
    SI = {}
    
    value_avg["PV1"] = 5.92
    value_stddev["PV1"]  = 2.3
    value_max["PV1"] = 10.7
    value_avg["PV2"] = 6.66
    value_stddev["PV2"] = 2.5
    value_max["PV2"] = 11.3
    value_avg["PV3"] = 5.51
    value_stddev["PV3"] = 2.5
    value_max["PV3"] = 15

    challenge_dose["PV1"] = 1333521.0
    challenge_dose["PV2"] = 1333521.0
    challenge_dose["PV3"] = 1333521.0

    interference["PV1"] = 0.0
    interference["PV2"] = 0.178
    interference["PV3"] = 0.354

    neutralization = 1.0

    SI["PV1"] = 0.171
    SI["PV2"] = 0.296
    SI["PV3"] = 0.137

    n = 10000

    output_value = {}
    for v in virus_type:
        output_value[v] = []
        p_inf = 1-pow(1.0 + challenge_dose[v], -SI[v] * neutralization)
        for i in xrange(n):
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
            output_value[v].append(output_temp)
    

    bins = range(0,12)
    bins2 = []
    for i in bins:
        bins2.append(bins[i]+0.5)
    plt.figure(1)
    plt.subplot(321)
    plt.hist(np.log2(mucosal_Nab["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Mucosal Log2 Nab")
    plt.subplot(323)
    plt.hist(np.log2(mucosal_Nab["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Mucosal Log2 Nab")
    plt.subplot(325)
    plt.hist(np.log2(mucosal_Nab["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Mucosal Log2 Nab")
    plt.subplot(322)
    plt.hist(np.log2(humoral_Nab["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Humoral Log2 Nab")
    plt.subplot(324)
    plt.hist(np.log2(humoral_Nab["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Humoral Log2 Nab")
    plt.subplot(326)
    plt.hist(np.log2(humoral_Nab["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_value["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Humoral Log2 Nab")
    plt.tight_layout()
    plt.show()
    

if __name__=="__main__":
    main()
