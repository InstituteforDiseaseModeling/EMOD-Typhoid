import json
import matplotlib.pyplot as plt
import numpy as np
from random import gauss
from random import random
import argparse

def main():
    parser = argparse.ArgumentParser(description='Some tool Hao wrote')
    parser.add_argument('filename', nargs='?', default='output\PolioSurveyJSONAnalyzer_survey_1_13.json', help='Polio survey filename [output\PolioSurveyJSONAnalyzer_sruvey_1_13.json]')
    parser.add_argument('scenario', nargs='?', default='default', help='Interference scenario [default,bOPV_default,bOPV_no_interference,O1_O3_diff_day,ssdd]')
    #ssdd = same serum, different day
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
    
    value_avg["PV1"] = {}
    value_stddev["PV1"]  = {}
    value_max["PV1"] = {}
    value_avg["PV2"] = {}
    value_stddev["PV2"] = {}
    value_max["PV2"] = {}
    value_avg["PV3"] = {}
    value_stddev["PV3"] = {}
    value_max["PV3"] = {}
    
    value_avg["PV1"]["default"] = 5.92
    value_stddev["PV1"]["default"]  = 2.3
    value_max["PV1"]["default"] = 10.7
    value_avg["PV2"]["default"] = 6.66
    value_stddev["PV2"]["default"] = 2.5
    value_max["PV2"]["default"] = 11.3
    value_avg["PV3"]["default"] = 5.51
    value_stddev["PV3"]["default"] = 2.5
    value_max["PV3"]["default"] = 15
    
    value_avg["PV1"]["bOPV_default"] = 5.92
    value_stddev["PV1"]["bOPV_default"]  = 2.3
    value_max["PV1"]["bOPV_default"] = 10.7
    value_avg["PV2"]["bOPV_default"] = 6.66
    value_stddev["PV2"]["bOPV_default"] = 2.5
    value_max["PV2"]["bOPV_default"] = 11.3
    value_avg["PV3"]["bOPV_default"] = 5.51
    value_stddev["PV3"]["bOPV_default"] = 2.5
    value_max["PV3"]["bOPV_default"] = 15
    
    value_avg["PV1"]["bOPV_no_interference"] = 5.92
    value_stddev["PV1"]["bOPV_no_interference"]  = 2.3
    value_max["PV1"]["bOPV_no_interference"] = 10.7
    value_avg["PV2"]["bOPV_no_interference"] = 6.66
    value_stddev["PV2"]["bOPV_no_interference"] = 2.5
    value_max["PV2"]["bOPV_no_interference"] = 11.3
    value_avg["PV3"]["bOPV_no_interference"] = 5.51
    value_stddev["PV3"]["bOPV_no_interference"] = 2.5
    value_max["PV3"]["bOPV_no_interference"] = 15
    
    challenge_dose["PV1"] = {}
    challenge_dose["PV2"] = {}
    challenge_dose["PV3"] = {}

    challenge_dose["PV1"]["default"] = 1000000.0
    challenge_dose["PV2"]["default"] = 100000.0
    challenge_dose["PV3"]["default"] = 630000.0
    
    challenge_dose["PV1"]["bOPV_default"] = 1000000.0
    challenge_dose["PV2"]["bOPV_default"] = 0.0
    challenge_dose["PV3"]["bOPV_default"] = 630000.0
    
    challenge_dose["PV1"]["bOPV_no_interference"] = 1000000.0
    challenge_dose["PV2"]["bOPV_no_interference"] = 0.0
    challenge_dose["PV3"]["bOPV_no_interference"] = 630000.0

    interference["PV1"] = {}
    interference["PV2"] = {}
    interference["PV3"] = {}
    
    interference["PV1"]["default"] = 0.0
    interference["PV2"]["default"] = 0.178
    interference["PV3"]["default"] = 0.354
    
    interference["PV1"]["bOPV_default"] = 0.0
    interference["PV2"]["bOPV_default"] = 0.178
    interference["PV3"]["bOPV_default"] = 0.354
    
    interference["PV1"]["bOPV_no_interference"] = 0.0
    interference["PV2"]["bOPV_no_interference"] = 0.0
    interference["PV3"]["bOPV_no_interference"] = 0.0

    neutralization = 1.0

    SI["PV1"] = 0.171
    SI["PV2"] = 0.296
    SI["PV3"] = 0.137

    n = 10000

    output_value = {}
    for v in virus_type:
        output_value[v] = []

        for i in xrange(n):
            p_inf = 1-pow(1.0 + challenge_dose[v][args.scenario], -SI[v] * neutralization)
            rand_num = gauss(0,1)
            if rand_num >= 2:
                rand_num = 2
            if rand_num <= -2:
                rand_num = -2
            for v2 in virus_type:
                if v2 != v:
                    p_inf = (1- interference[v2][args.scenario])*p_inf
            p_inf_new = p_inf
            if random() < p_inf_new:
                output_temp = pow(2, value_avg[v][args.scenario] + rand_num * value_stddev[v][args.scenario])
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
    plt.suptitle(args.filename + " " + args.scenario)
    plt.tight_layout()
    plt.show()
    

if __name__=="__main__":
    main()
