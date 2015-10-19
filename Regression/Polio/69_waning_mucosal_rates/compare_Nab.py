import json
import matplotlib.pyplot as plt
import numpy as np
from random import gauss
from random import random
import argparse

def main():
    parser = argparse.ArgumentParser(description='Some tool Hao wrote')
    parser.add_argument('f1', nargs='?', default='output\\PolioSurveyJSONAnalyzer_Initial_1.json', help='Polio survey filename [output\PolioSurveyJSONAnalyzer_Initial_1.json]')
    parser.add_argument('f2', nargs='?', default='output\\PolioSurveyJSONAnalyzer_Day100_1.json', help='Polio survey filename [output\PolioSurveyJSONAnalyzer_Day100_1.json]')
    args = parser.parse_args()
    data1 = json.load(open(args.f1))
    patient1 = data1["patient_array"]
    data2 = json.load(open(args.f2))
    patient2 = data2["patient_array"]
    virus_type = ["PV1", "PV2", "PV3"]
    mucosal_Nab1 = {}
    humoral_Nab1 = {}
    mucosal_Nab2 = {}
    humoral_Nab2 = {}
    for v in virus_type:
        mucosal_Nab1[v]=[]
        humoral_Nab1[v]=[]
        mucosal_Nab2[v]=[]
        humoral_Nab2[v]=[]
    for p in patient1:
        for v in virus_type:
            mucosal_Nab1[v].append(p["mucosal_Nab"][v][0])
            humoral_Nab1[v].append(p["humoral_Nab"][v][0])
    for p in patient2:
        for v in virus_type:
            mucosal_Nab2[v].append(p["mucosal_Nab"][v][0])
            humoral_Nab2[v].append(p["humoral_Nab"][v][0])

    bins = range(0,12)
    bins2 = []
    for i in bins:
        bins2.append(bins[i]+0.5)
    plt.figure(1)
    plt.subplot(321)
    plt.hist(np.log2(mucosal_Nab1["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(mucosal_Nab2["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Mucosal Log2 Nab")
    plt.subplot(323)
    plt.hist(np.log2(mucosal_Nab1["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(mucosal_Nab2["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Mucosal Log2 Nab")
    plt.subplot(325)
    plt.hist(np.log2(mucosal_Nab1["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(mucosal_Nab2["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Mucosal Log2 Nab")
    plt.subplot(322)
    plt.hist(np.log2(humoral_Nab1["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(humoral_Nab2["PV1"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 1 Humoral Log2 Nab")
    plt.subplot(324)
    plt.hist(np.log2(humoral_Nab1["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(humoral_Nab2["PV2"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 2 Humoral Log2 Nab")
    plt.subplot(326)
    plt.hist(np.log2(humoral_Nab1["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.hist(np.log2(humoral_Nab2["PV3"]), log=True, bins = bins, alpha=0.5)
    plt.xticks(bins2, ["%s" % i for i in bins])
    plt.title("Type 3 Humoral Log2 Nab")
    plt.tight_layout() #TODO: Make sure that we want a supertitle here
    plt.suptitle("{first} vs {second}".format(first=args.f1, second=args.f2))
    plt.show()
    

if __name__=="__main__":
    main()
