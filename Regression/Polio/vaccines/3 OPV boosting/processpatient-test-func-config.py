import json
import matplotlib.pyplot as plt
import numpy as np
from random import gauss
import random
from math import log, exp
import json
import ConfigParser

class AntibodySimulator(object):
    def __init__(self):
        config_file = "antibody_test.cfg"
        test_key = "TEST_PARAMS"
        test_type_key = "type"
        disease_key = "DISEASE"
        virus_types_key = "virus_types"
        
        ranges_typical_key="OPV_RANGES_TYPICAL"
        boost_params_key="OPV_BOOST_PARAMS"
        
        self.config = ConfigParser.ConfigParser()
        self.config.read(config_file)
        self.test_type = self.config.get(test_key, test_type_key)
        virus_types = self.config.get(disease_key, virus_types_key)
        self.virus_types = []
        for type in virus_types.split(','):
            self.virus_types.append(type)

        self.value_avg={}
        self.value_stddev = {}
        self.value_max = {}
        self.boost_avg = {}
        self.boost_stddev = {}
        self.challenge_dose = {}
        self.interference = {}
        self.SI = {}
        
        for type in self.virus_types:
            self.value_avg[type] = self.config.getfloat(ranges_typical_key, "value_avg_{t}".format(t=type))
            self.value_stddev[type] = self.config.getfloat(ranges_typical_key, "value_stddev_{t}".format(t=type))
            self.value_max[type] = self.config.getfloat(ranges_typical_key, "value_max_{t}".format(t=type))
            self.boost_avg[type] = self.config.getfloat(boost_params_key, "boost_avg_{t}".format(t=type))
            self.boost_stddev[type] = self.config.getfloat(boost_params_key, "boost_stddev_{t}".format(t=type))
            self.challenge_dose[type] = self.config.getfloat(ranges_typical_key, "challenge_dose_{t}".format(t=type))
            self.interference[type] = self.config.getfloat(ranges_typical_key, "interference_{t}".format(t=type))
            self.SI[type] = self.config.getfloat(ranges_typical_key, "SI_{t}".format(t=type))
        
    def determine_antibody(self, ab, virus_type):
        neutralization = 1+0.034*(ab-1.0)*(1.0-exp(-1.0/0.034))/ab
        p_inf = 1-pow(1.0 + self.challenge_dose[virus_type], -self.SI[virus_type] * neutralization)
        interference_factor = 0.0
        p_inf_new = (1-interference_factor)*p_inf
        rand_num = gauss(0,1)
        if rand_num >= 2:
            rand_num = 2
        if rand_num <= -2:
            rand_num = -2
        if random.random() < p_inf_new:
            if ab > 1:
                log2Nab_boost = self.boost_avg[virus_type] + rand_num* self.boost_stddev[virus_type]
                Nab_saturation = log(2) - log(ab) / self.value_max[virus_type]
                boost_value = ab * exp(log2Nab_boost * Nab_saturation)
                return boost_value
            else:
                output_temp = pow(2, self.value_avg[virus_type] + rand_num * self.value_stddev[virus_type])
                return output_temp
        else:
            return ab
def main():
    random.seed(1)
    simulator = AntibodySimulator()
    output_value1 = {}
    output_value2 = {}
    output_value3 = {}
    infections = 0
    virus_type = ["PV1", "PV2", "PV3"]
    for v in virus_type:
        output_value1[v] = []
        output_value2[v] = []
        output_value3[v] = []
        n = 10000
        n = 10
        #priming
        for i in xrange(n):
            ab = 1
            ab_1 = simulator.determine_antibody(ab, v)
            output_value1[v].append(ab_1)
            ab_2 = simulator.determine_antibody(ab_1, v)
            output_value2[v].append(ab_2)
            ab_3 = simulator.determine_antibody(ab_2, v)
            output_value3[v].append(ab_3)
    outputs = []
    outputs.append(output_value1)
    outputs.append(output_value2)
    outputs.append(output_value3)
    stuff = {}
    stuff["outputs"] = outputs
    stuff["infections"] = infections
    with open('data-func-test.json', 'w') as outfile:
        json.dump(stuff, outfile)


if __name__=="__main__":
    main()
