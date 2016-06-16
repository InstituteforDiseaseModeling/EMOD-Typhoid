import csv
import sys
import json
import os

def num(s):
    try:
        return float(s)
    except ValueError:
        return s

params_json = {}
for col in range(1,19):
    params_json[str(col)] = {}
with open( sys.argv[1], 'rb') as f:
    reader = csv.reader(f) 
    for row in reader:
        param_name = row[0]
        for col in range(1,19):
            params_json[str(col)][param_name] = num(row[col])

for scenario in params_json.keys():
    config_name_id = scenario
    config_name = "Typhoid_" + str(config_name_id)
    if os.path.exists( config_name ) == False:
        os.mkdir( config_name )

    param_overrides_path = os.path.join( config_name, "param_overrides.json" )
    po_json = {}
    po_json["parameters"] = {}
    with open( param_overrides_path, "w" ) as po:
        po_json["parameters"] = params_json[config_name_id]
        po_json["parameters"]["Default_Config_Path"] = "defaults/typhoid_default_config.json"
        po_json["parameters"]["Config_Name"] = config_name
        po.write( json.dumps( po_json, sort_keys = True, indent=4 ) )

print( "Done!" )
#print( json.dumps( params_json, sort_keys = True, indent=4 ) )
