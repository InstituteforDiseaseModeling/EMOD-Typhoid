#!/usr/bin/python
import json
#import yaml
import pdb
import xlrd

def application( config_file_name ):
    if config_file_name.endswith( ".json" ):
        tlc = json.loads( open( config_file_name ).read() )
        params = {}
        param_key = "parameters"
        path_key = "paths"
        if tlc.has_key( param_key ):
            print( "'config.json' already has 'parameters' key so returning as-is" )
            return config_file_name 
        elif tlc.has_key( path_key ):
            print( "'config.json' has 'paths' key -> stitching." )
            params[param_key] = {}
            if path_key in tlc.keys():
                for path in tlc[path_key]:
                    #print( "Pre-processing " + path )
                    param_set = json.loads( open( path ).read() )
                    params[param_key].update( param_set[param_key] )
            stitched_output_config_file_name = ".config_stitched.json"
            with open( stitched_output_config_file_name , "w" ) as handle:
                handle.write( json.dumps( params, indent=4, sort_keys=True ) )

            return stitched_output_config_file_name 

    elif config_file_name.endswith( ".yaml" ):
        config_yaml_f = open( config_file_name )
        config = yaml.safe_load( config_yaml_f )
        config_json_str = json.dumps( config )
        config_json = json.loads( config_json_str  )
        output_config_file_name = ".config_json.json"
        with open( output_config_file_name, "w" ) as handle:
            handle.write( json.dumps( config_json, indent=4, sort_keys=True ) )
        return output_config_file_name 

    elif config_file_name.endswith( ".xlsx" ) or config_file_name.endswith( ".xlsm" ):
        wb = xlrd.open_workbook( config_file_name ) 
        # just support single worksheet for now
        config_json = json.loads( "{}" )
        param_key = "parameters"
        config_json[ param_key ] = {}

        for sheet in wb.sheets():
            if sheet.name.startswith( "STI_Network_Params" ):
                #pdb.set_trace()
                config_json[param_key]["STI_Network_Params_By_Property"] = {}
                name_parsed = sheet.name.split( "-" )
                ipn = name_parsed[1]
                snpbp_key = name_parsed[2]
                config_json[param_key]["STI_Network_Params_By_Property"]["Individual_Property_Name"] = ipn 
                config_json[param_key]["STI_Network_Params_By_Property"][ snpbp_key ] = {}
                # print( "handle differently" )
                for row_id in range(0,sheet.nrows):
                    row = sheet.row(row_id)
                    param_name = row[0].value
                    param_value = row[1].value
                    if isinstance( param_value,basestring) and param_value.startswith( "[" ):
                        param_list = param_value.strip( "[" ).strip( "]" ).split()
                        param_value = param_list
                    print( param_name, param_value )
                    config_json[param_key]["STI_Network_Params_By_Property"][ snpbp_key ][param_name] = param_value
            elif sheet.name.startswith( "Campaign" ):
                campaign_json = {}
                campaign_json["Events"] = []
                campaign_json["Use_Defaults"] = 1
                for event in range(1,sheet.ncols):
                    campaign_event = {}
                    campaign_event["class"] = "CampaignEvent"
                    campaign_event["Nodeset_Config"] = {}
                    campaign_event["Nodeset_Config"]["class"] = "NodeSetAll" 
                    campaign_event["Event_Coordinator_Config"] = {}
                    campaign_event["Event_Coordinator_Config"]["Intervention_Config"] = {}
                    campaign_event["Event_Coordinator_Config"]["class"] = "StandardInterventionDistributionEventCoordinator"
                    print( "Creating campaign event..." )
                    for row_id in range(0,sheet.nrows):
                        row = sheet.row(row_id)
                        param_name = row[0].value
                        param_value = row[ event ].value
                        if param_value is None:
                            continue
                        if param_name in [ "Target_Demographic", "Demographic_Coverage" ]:
                            campaign_event["Event_Coordinator_Config"][ param_name ] = param_value
                        elif param_name in [ "Start_Day" ]:
                            campaign_event[ param_name ] = param_value
                        else:
                            campaign_event["Event_Coordinator_Config"]["Intervention_Config"][ param_name ] = param_value
                    campaign_json["Events"].append( campaign_event )
                campaign_file = open( "campaign.json", "w+" )
                campaign_file.write( json.dumps( campaign_json, indent=4, sort_keys=True ) )
                campaign_file.close()
            else:
                for row_id in range(0,sheet.nrows):
                    row = sheet.row(row_id)
                    param_name = row[0].value
                    param_value = row[1].value
                    if isinstance( param_value,basestring) and param_value.startswith( "[" ):
                        param_list = param_value.strip( "[" ).strip( "]" )
			if ',' in param_list:
			    param_list = param_list.split( ',' )
			else:
			    param_list = param_list.split()
                        #pdb.set_trace()
                        param_list_new = []
                        for param in param_list:
			    param = param.strip( " " ).strip( "'" ).strip( "u'" )
                            if param.isalpha() == False:
			    	print( param )
			    	#pdb.set_trace()
				try:
                                    param = float(param)
			        except Exception as ex:
				    print( ex )
			    param_list_new.append( param )
                        param_value = param_list_new
                    #print( param_name, param_value )
                    config_json[param_key][param_name] = param_value

        output_config_file_name = ".config_from_excel.json"
        with open( output_config_file_name, "w" ) as handle:
            handle.write( json.dumps( config_json, indent=4, sort_keys=True ) )
        return output_config_file_name 

    else:
        print( "Non-json files not supported (yet)." )
        return config_file_name
