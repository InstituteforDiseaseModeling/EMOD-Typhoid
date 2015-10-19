import json
import argparse
import os

def main():
    parser = argparse.ArgumentParser(description='Some tool Hao wrote and cwiswell edited')
    parser.add_argument('f1', nargs='?', default='output', help='Polio survey folder [output]')
    args = parser.parse_args()
    files = os.listdir(args.f1)
    surveys = []
    for f in files:
        if f.startswith("PolioSurvey"):
            surveys.append(f)
    surveys.sort()
    
    datas = []
    for survey in surveys:
        datas.append(json.load(open(args.f1 + "\\" + survey)))
    #For each survey
    ##For each patient in patient_array (there should only be one)
    ###Load serum_NAb key
    ####For each virus type (PV1, PV2, PV3) in serum_NAb
    #####get the length, divide by 12, ditch remainder, assign to monthlength
    ######
    for data in datas:
      for p in data["patient_array"]:
        serums = p["serum_NAb"]
        averages = {}
        for s in serums:
            Nab = serums[s]
            averages[s] = []
            month_length = len(Nab)/12
            for m in range (0,12):
                tmp_sum = 0
                start_day = m * month_length
                for d in range(0, month_length):
                    tmp_sum = tmp_sum + Nab[start_day + d]
                averages[s].append(tmp_sum)
    for virus_type in averages:
        print "Virus type: " + virus_type
        print "Averages by month:"
        print averages[virus_type]
        print "-------------------"

if __name__=="__main__":
    main()
