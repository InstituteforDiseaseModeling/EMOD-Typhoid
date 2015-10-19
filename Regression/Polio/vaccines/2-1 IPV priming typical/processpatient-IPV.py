import json
import matplotlib.pyplot as plt
import numpy as np
from random import gauss
from random import random
from math import log
import argparse

def main():
    parser = argparse.ArgumentParser(description='Some tool Hao wrote')
    parser.add_argument('filename', nargs='?', default='output\\PolioSurveyJSONAnalyzer_m1_1_1.json', help='Polio survey filename [output\PolioSurveyJSONAnalyzer_m1_-1_0]')
    args = parser.parse_args()
    data = json.load(open(args.filename))
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
    
    value_avg["PV1"] = 3.22
    value_stddev["PV1"]  = 0.59
    value_max["PV1"] = 15.1
    value_avg["PV2"] = 3.7
    value_stddev["PV2"] = 1.0
    value_max["PV2"] = 15.5
    value_avg["PV3"] = 3.62
    value_stddev["PV3"] = 0.7
    value_max["PV3"] = 14.7

    mucosal_immunogenicity = 0.107

    n = 10000

    output_value = {}
    output_mucosal = {}
    for v in virus_type:
        output_value[v] = []
        output_mucosal[v] = []
        p_inf = 1
        for i in xrange(n):
            rand_num = gauss(0,1)
            if rand_num >= 2:
                rand_num = 2
            if rand_num <= -2:
                rand_num = -2           
            output_temp = pow(2, value_avg[v] + rand_num * value_stddev[v])
            output_temp2 = pow(2, mucosal_immunogenicity*(value_avg[v] + rand_num * value_stddev[v]))
            output_value[v].append(output_temp)
            output_mucosal[v].append(output_temp2)

    bins = range(0,12)
    bins2 = []
    for i in bins:
        bins2.append(bins[i]+0.5)
    plt.figure(1)
    plt.subplot(321)
    plt.hist(np.log2(mucosal_Nab["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Mucosal Log2 Nab")
    plt.subplot(323)
    plt.hist(np.log2(mucosal_Nab["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Mucosal Log2 Nab")
    plt.subplot(325)
    plt.hist(np.log2(mucosal_Nab["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(output_mucosal["PV3"]), log=True, bins = bins, alpha=0.5)
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
    plt.tight_layout() #TODO: Make sure that we want a supertitle here
    plt.suptitle(args.filename)
    plt.show()
    

if __name__=="__main__":
    main()
