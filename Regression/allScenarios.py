#!/usr/bin/python

import argparse
import json
import os
import sys


def locateAllSubdirectories(directory=os.getcwd()):
    subdirectories = []
    #print >> sys.stderr, "Processing '%s'" % (directory)
    files = os.listdir(directory)
    for item in files:
        full_path = os.path.join(directory, item)
        if os.path.isdir(full_path):
            subdirectories.append(full_path)
            subdirectories.extend(locateAllSubdirectories(full_path))
    
    return subdirectories


def locateAllConfigFiles(directories):
    config_files = []
    for directory in directories:
        full_path = os.path.join(directory, 'config.json')
        if os.path.exists(full_path):
            config_files.append(full_path)

    return config_files


def loadJsonFile(filename):
    handle = open(filename)
    json_data = json.load(handle)
    handle.close()
    
    return json_data


def filterConfigFiles(config_files, sim_types):
    filtered_list = []
    for config in config_files:
        try:
            json_data = loadJsonFile(config)
            sim_type_parameter = json_data['parameters']['Simulation_Type']
            for sim_type in sim_types:
                if sim_type.lower() in sim_type_parameter.lower():
                    filtered_list.append(config)
        except Exception as e:
            print >> sys.stderr, "Couldn't process '%s' - '%s'" % (config, e)

    return filtered_list


def generateTestSuite(root, scenarios):
    tests = []
    for scenario in scenarios:
        path = os.path.dirname(os.path.relpath(scenario, root))
        tests.append({'path':path})
    
    suite = { 'tests': tests }
    return suite


def printTestSuite(suite):
    json_text = json.dumps(suite, indent=4, separators=(',', ':'), sort_keys=True)
    print '%s' % (json_text)
    
    return json_text

def main(sim_types):
    print >> sys.stderr , "Locating sims of type %s" % (sim_types)
    subdirectories = locateAllSubdirectories()
    config_files = locateAllConfigFiles(subdirectories)
    scenarios = filterConfigFiles(config_files, sim_types)
    suite = generateTestSuite(os.getcwd(), scenarios)
    printTestSuite(suite)
    

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('sims', help='sim type[s] to locate', nargs='*', type=str, default=['GENERIC'])
    args = parser.parse_args()
    
    main(args.sims)