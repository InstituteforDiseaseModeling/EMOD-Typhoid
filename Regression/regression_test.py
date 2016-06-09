#!/usr/bin/python

"""
This file is the root of regression. Almost everything here is about copying files around.
"""

import regression_utils as ru
import BaseHTTPServer
import SimpleHTTPServer
import SocketServer
import argparse
import cgi
import datetime
import glob
import httplib
import json
import os # e.g., mkdir
import re
import shutil # copyfile
import subprocess
import sys # for stdout.flush
import threading
import time # for sleep
import urllib
import urlparse
import regression_runtime_params
import regression_local_monitor
import regression_hpc_monitor
import regression_report
import regression_clg
import pdb


# global variable >:p -- actually, it's a module variable :) 
params = None
#regression_id = None
#regression_id, params
            
def setup():

    # non-main global code starts here
    parser = argparse.ArgumentParser()
    parser.add_argument("suite", help="JSON test-suite to run - e.g. full.json, sanity (converted to sanity.json), 25 (just run 25_Vector_Madagascar)")
    parser.add_argument("exe_path", metavar="exe-path", help="Path to the Eradication.exe binary to run")
    parser.add_argument("--perf", action="store_true", default=False, help="Run for performance measurement purposes")
    parser.add_argument("--hidegraphs", action="store_true", default=False, help="Suppress pop-up graphs in case of validation failure")
    parser.add_argument("--debug", action="store_true", default=False, help="Use debug path for emodules")
    parser.add_argument("--quick-start", action="store_true", default=False, help="Use QuickStart path for emodules")
    parser.add_argument("--label", help="Custom suffix for HPC job name")
    parser.add_argument("--config", default="regression_test.cfg", help="Regression test configuration [regression_test.cfg]")
    parser.add_argument("--disable-schema-test", action="store_false", default=True, help="Test schema (true by default, use to suppress schema testing)")
    parser.add_argument("--use-dlls", action="store_true", default=False, help="Use emodules/DLLs when running tests")
    parser.add_argument("--all-outputs", action="store_true", default=False, help="Use all output .json files for validation, not just InsetChart.json")
    parser.add_argument("--dll-path", help="Path to the root directory of the DLLs to use (e.g. contains reporter_plugins)")
    parser.add_argument("--skip-emodule-check", action="store_true", default=False, help="Use this to skip sometimes slow check that EMODules on cluster are up-to-date.")
    parser.add_argument("--config-constraints", default=[], action="append", help="Use this to skip sometimes slow check that EMODules on cluster are up-to-date.")
    parser.add_argument("--scons", action="store_true", default=False, help="Indicates scons build so look for custom DLLs in the build/64/Release directory.")
    parser.add_argument('--local', default=False, action='store_true', help='Run all simulations locally.')

    args = parser.parse_args()

    global params
    params = regression_runtime_params.RuntimeParameters(args)

    global emodules_map, regression_runner
    emodules_map = {}
    emodules_map[ "interventions" ] = []
    emodules_map[ "disease_plugins" ] = []
    emodules_map[ "reporter_plugins" ] = []

    regression_runner = MyRegressionRunner(params)

    if params.dll_root is not None and params.use_dlls is True:
        #print( "dll_root (remote) = " + params.dll_root )
        copyEModulesOver(params)
    else:
        print( "Not using DLLs" )

class MyRegressionRunner():
    def __init__(self, params):
        self.params = params
        self.dtk_hash = ru.md5_hash_of_file( self.params.executable_path )
        self.sim_dir_sem = threading.Semaphore()
        # print( "md5 of executable = " + self.dtk_hash )
        
    def copy_demographics_files_to_user_input(self, sim_id, reply_json, actual_input_dir, local_input_dir, remote_input_dir):
        input_files = reply_json['parameters'].get('Demographics_Filenames',[])
        if not input_files:
            input_files = reply_json['parameters'].get('Demographics_Filename','').split(';')
        reply_json["parameters"]["Demographics_Filenames"] = []
        for filename in input_files:
            #print( filename )
            if not filename or len( filename.strip(' ') ) == 0:
                #print( "file is empty. Skip" )
                continue
            do_copy = False

            local_source = os.path.join( local_input_dir, filename )
            remote_source = os.path.join( remote_input_dir, filename )
            dest = os.path.join( actual_input_dir, os.path.basename( filename ) )

            # For any demographics overlays WITHIN regression folder:
            # Copy directly to remote simulation working directory
            if os.path.exists(local_source):
                #print('Copying %s to remote working directory'%filename)
                sim_dir = os.path.join( self.params.sim_root, sim_id )
                do_copy = False # do my own copy here, since
                remote_path = os.path.join(sim_dir,os.path.basename(filename))
                if os.path.exists(remote_path):
                    raise Exception('Overlay with same basename has already been copied to remote simulation directory.')
                shutil.copy(local_source, remote_path)

            # Cases:
            # 0 - source file exists and is found at destination
            #   NO COPY: nothing to do if the same
            # 1 - source file exists and is found at destination
            #   COPY source to destination if _not_ the same
            # 2 - source file exists and is _not_ found at destination
            #   COPY source to destination
            # 3 - source file not found but _is_ found at destination
            #   NO COPY: use destination file
            # 4 - source file not found and _not_ found at destination
            #   NO COPY: give up

            elif os.path.exists( remote_source ):
                if os.path.exists( dest ):
                    if not ru.areTheseJsonFilesTheSame( remote_source, dest ):
                        # Case #1
                        print( "Source file '{0}' not the same as the destination file '{1}', updating.".format( remote_source, dest ) )
                        do_copy = True
                    else:
                        # Case #0: they're the same, don't copy
                        pass
                else:
                    # Case #2
                    print( "File '{0}' doesn't exist in destination directory, copying.".format( dest ) )
                    do_copy = True

            else:
                if os.path.exists( dest ):
                    # Case #3: use copy cached at destination
                    pass
                else:
                    # Case #4
                    print( "Couldn't find source file '{0}' locally ({1}) or remotely ({2})! Exiting.".format( filename, local_source, remote_source ) )
                    reply_json["parameters"]["Demographics_Filename"] = "input file ({0}) not found".format( filename )
                    return

            if do_copy:
                print( "Copy input files from " + local_input_dir + " and " + remote_input_dir + " to " + actual_input_dir + "." )
                shutil.copy( remote_source, dest )

            reply_json["parameters"]["Demographics_Filenames"].append(os.path.basename( filename ) )

        if "Demographics_Filename" in reply_json["parameters"]:
            del( reply_json["parameters"]["Demographics_Filename"] )

    def copy_climate_and_migration_files_to_user_input(self, reply_json, remote_input_dir, actual_input_dir):
        # Copy climate and migration files also
        for key in reply_json["parameters"]:
            if( "_Filename" in key and "Demographics_Filename" not in key and key != "Campaign_Filename" and key != "Custom_Reports_Filename" ):
                filename = reply_json["parameters"][key]
                if( len( filename ) == 0 ):
                    continue
                source = os.path.join( remote_input_dir, filename )
                dest = os.path.join( actual_input_dir, os.path.basename( filename ) )
                if os.path.exists( source ) and not os.path.exists( dest ):
                    #print( "Copying " + file )
                    shutil.copy( source, dest )
                dest = dest + ".json"
                source = source + ".json"
                if os.path.exists( source ) and not os.path.exists( dest ):
                    #print( "Copying " + file )
                    shutil.copy( source, dest )
        
    def copy_input_files_to_user_input(self, sim_id, config_id, reply_json, is_local):
        # Copy local demographics/input file(s) and remote base input file into user-local input directory 
        # E.g., //diamonds-hn/EMOD/home/jbloedow/input/Bihar, where "//diamonds-hn/EMOD/home/jbloedow/input/"
        # is gotten from config["home"] and "Bihar" is from config_json["Geography"]
        # Then use that directory as the input.
        local_input_dir = config_id
        remote_input_dir = os.path.join( self.params.shared_input, reply_json["parameters"]["Geography"] )
        # print( "remote_input_dir = " + remote_input_dir )
        actual_input_dir = os.path.join( self.params.user_input, reply_json["parameters"]["Geography"] )

        if os.path.exists( actual_input_dir ) == False:
            print( "Creating " + actual_input_dir )
            os.makedirs( actual_input_dir )

        self.copy_demographics_files_to_user_input(sim_id, reply_json, actual_input_dir, local_input_dir, remote_input_dir)
        self.copy_climate_and_migration_files_to_user_input(reply_json, remote_input_dir, actual_input_dir)
        self.params.use_user_input_root = True
        
    def copy_sim_file( self, config_id, sim_dir, filename ):
        if( len( filename ) != 0 ):
            filename = os.path.join( config_id, filename )
            if( os.path.exists( filename ) ) :
                #print( "Copying " + filename )
                shutil.copy( filename, sim_dir )
            else:
                print( "ERROR: Failed to find file to copy: " + filename )
        return

    def commissionFromConfigJson( self, sim_id, reply_json, config_id, report, compare_results_to_baseline=True ):
        # compare_results_to_baseline means we're running regression, compare results to reference.
        # opposite/alternative is sweep, which means we don't do comparison check at end
        # now we have the config_json, find out if we're commissioning locally or on HPC

        def is_local_simulation( some_json, id ):
            if os.name == "posix":
                return True

            global params
            return params.local_execution 

        sim_dir = os.path.join( self.params.sim_root, sim_id )
        bin_dir = os.path.join( self.params.bin_root, self.dtk_hash ) # may not exist yet

        is_local = is_local_simulation(reply_json, config_id)
        if is_local:
            print( "Commissioning locally (not on cluster)!" )
            sim_dir = os.path.join( self.params.local_sim_root, sim_id )
            bin_dir = os.path.join( self.params.local_bin_root, self.dtk_hash ) # may not exist yet
        # else:
            # print( "HPC!" )

        # create unique simulation directory
        self.sim_dir_sem.acquire()
        os.makedirs(sim_dir)
        self.sim_dir_sem.release()

        # only copy binary if new to us; copy to bin/<md5>/Eradication.exe and run from there
        # Would like to create a symlink and run from the sim dir, but can't do that on cluster; no permissions!

        # JPS - can't we just check for existence of that file?  This seems overly complicated...

        # check in bin_dir to see if our binary exists there...
        foundit = False
        bin_path = os.path.join( bin_dir, "Eradication" if os.name == "posix" else "Eradication.exe" )
        if os.path.exists(bin_dir):
            if os.path.exists(bin_path):
                foundit = True
        else:
            os.makedirs(bin_dir)
        
        if not foundit:
            print( "We didn't have it, copy it up..." )
            shutil.copy( self.params.executable_path, bin_path )
            print( "Copied!" )

        reply_json["bin_path"] = bin_path
        reply_json["executable_hash"] = self.dtk_hash

        # JPS - not sure what some of the rest of this stuff does
        # campaign_json is non, and config.json contains double the stuff it needs in the sim dir... :-(
        
        # tease out campaign json, save separately
        campaign_json = json.dumps(reply_json["campaign_json"]).replace( "u'", "'" ).replace( "'", '"' ).strip( '"' )
        reply_json["campaign_json"] = None

        # tease out custom_reports json, save separately
        if reply_json["custom_reports_json"] is not None:
            reports_json = json.dumps(reply_json["custom_reports_json"]).replace( "u'", "'" ).replace( "'", '"' ).strip( '"' )
            reply_json["custom_reports_json"] = None
            # save custom_reports.json
            f = open( sim_dir + "/custom_reports.json", 'w' )
            #f.write( json.dumps( reports_json, sort_keys=True, indent=4 ) )
            f.write( str( reports_json ) )
            f.close()

        # Use a local variable here because we don't want the PSP in the config.json that gets written out to disk
        # but we need it passed through to the monitor thread execution in the reply_json/config_json.
        py_input = None
        if "Python_Script_Path" in reply_json["parameters"]:
            psp_param = reply_json["parameters"]["Python_Script_Path"]
            if psp_param == "LOCAL":
                py_input = "."
                for py_file in glob.glob( os.path.join( config_id, "dtk_*.py" ) ):
                    regression_runner.copy_sim_file( config_id, sim_dir, os.path.basename( py_file ) )
            elif psp_param == "SHARED":
                py_input = params.py_input
            elif psp_param != "NO":
                print( psp_param + " is not a valid value for Python_Script_Path. Valid values are NO, LOCAL, SHARED. Exiting." )
                sys.exit() 
            del( reply_json["parameters"]["Python_Script_Path"] )

        self.copy_input_files_to_user_input(sim_id, config_id, reply_json, is_local)

        #print "Writing out config and campaign.json."
        # save config.json
        f = open( sim_dir + "/config.json", 'w' )
        f.write( json.dumps( reply_json, sort_keys=True, indent=4 ) )
        f.close()

        # now that config.json is written out, add Py Script Path back (if non-empty)
        if py_input is not None:
            reply_json["PSP"] = py_input

        # save campaign.json
        f = open( sim_dir + "/campaign.json", 'w' )
        #f.write( json.dumps( campaign_json, sort_keys=True, indent=4 ) )
        f.write( str( campaign_json ) )
        f.close()

        f = open( sim_dir + "/emodules_map.json", 'w' )
        f.write( json.dumps( emodules_map, sort_keys=True, indent=4 ) )
        f.close()
        
        # ------------------------------------------------------------------
        # If you uncomment the following line, it will copy the program database
        # file to the directory where a simulation will run (i.e. with the config.json file).
        # This will help you get a stack trace with files and line numbers.
        # ------------------------------------------------------------------
        #print( "Copying PDB file...." )
        #shutil.copy( "../Eradication/x64/Release/Eradication.pdb", sim_dir )
        # ------------------------------------------------------------------

        if os.path.exists( os.path.join( config_id, "dtk_post_process.py" ) ):
            regression_runner.copy_sim_file( config_id, sim_dir, "dtk_post_process.py" )

        monitorThread = None # need scoped here

        #print "Creating run & monitor thread."
        if is_local_simulation(reply_json, config_id):
            monitorThread = regression_local_monitor.Monitor( sim_id, config_id, report, self.params, reply_json, compare_results_to_baseline )
        else:
            monitorThread = regression_hpc_monitor.HpcMonitor( sim_id, config_id, report, self.params, params.label, reply_json, compare_results_to_baseline )

        #monitorThread.daemon = True
        monitorThread.daemon = False
        #print "Starting run & monitor thread."
        monitorThread.start()

        #print "Monitor thread started, notify data service, and return."
        return monitorThread

    def doSchemaTest( self ):
        #print( "Testing schema generation..." )
        test_schema_path = "test-schema.json"
        subprocess.call( [ params.executable_path, "--get-schema", "--schema-path", test_schema_path ], stdout=open(os.devnull) )
        try:
            schema = json.loads( open( test_schema_path ).read() )
            print( "schema works." )
            os.remove( test_schema_path )
            return "pass"
        except Exception as ex:
            print( "schema failed!" )
            return "fail"

# Copy just build dlls to deployed places based on commandline argument 
# - The default is to use all of the DLLs found in the location the DLL projects
#   place the DLLs (<trunk>\x64\Release).
# - --dll-path allows the user to override this default path
def copyEModulesOver( params ):

    print "src_root = " + params.src_root

    if params.dll_path is not None:
        emodule_dir = params.dll_path
    else:
        if params.scons:
            emodule_dir = os.path.join( params.src_root, "build" )
            emodule_dir = os.path.join( emodule_dir, "x64" )
        else:
            emodule_dir = os.path.join( params.src_root, "x64" )
        if params.debug == True:
            emodule_dir = os.path.join( emodule_dir, "Debug" )
        elif params.quick_start == True:
            emodule_dir = os.path.join( emodule_dir, "QuickStart" )
        else:
            emodule_dir = os.path.join( emodule_dir, "Release" )

    print( 'Assuming emodules (dlls) are in local directory: ' + emodule_dir )

    if os.path.exists( emodule_dir ) == False:
        print( "Except that directory does not exist!  Not copying emodules." )
        return

    #print "dll_root = " + params.dll_root

    dll_dirs = [ "disease_plugins",  "reporter_plugins", "interventions"]

    for dll_subdir in dll_dirs:
        dlls = glob.glob( os.path.join( os.path.join( emodule_dir, dll_subdir ), "*.dll" ) )
        for dll in dlls:
            dll_hash = md5_hash_of_file( dll )
            #print( dll_hash )
            # 1) calc md5 of dll
            # 2) check for existence of rivendell (or whatever) for <root>/emodules/<subdir>/<md5>
            # 3) if no exist, create and copy
            # 4) put full path in emodules_json
            # 5) write out emodules_json when done to target sim dir
            try:
                target_dir = os.path.join( params.dll_root, dll_subdir )
                target_dir = os.path.join( target_dir, dll_hash )

                if params.sec:
                    print( dll + " will be used without checking 'new-ness'." )
                elif not (os.path.isdir( target_dir ) ):
                    print( dll + ": copying to cluster" )
                else:
                    print( dll + ": Already on cluster" )

                if not (os.path.isdir( target_dir ) ) and params.sec == False: # sec = command-line option to skip this
                    os.makedirs( target_dir )
                    shutil.copy( dll, os.path.join( target_dir, os.path.basename( dll ) ) )

                emodules_map[ dll_subdir ].append( os.path.join( target_dir, os.path.basename( dll ) ) )
    
            except IOError:
                print "Failed to copy dll " + dll + " to " + os.path.join( os.path.join( params.dll_root, dll_dirs[1] ), os.path.basename( dll ) ) 
                ru.final_warnings += "Failed to copy dll " + dll + " to " + os.path.join( os.path.join( params.dll_root, dll_dirs[1] ), os.path.basename( dll )) + "\n"

def main():
    report = None
    reglistjson = None
    regression_id = None

    if(str.isdigit(params.suite)):
        dirs = glob.glob(params.suite + "_*")
        for dir in dirs:
            if os.path.isdir(dir):
                print("Executing single test: " + dir)
                reglistjson = { "tests" : [ { "path" : dir } ] }
    else:
        if params.suite.endswith(".json"):
            params.suite = params.suite.replace(".json", "")
        reglistjson = json.loads( open( params.suite.split(',')[0] + ".json" ).read() )
        if "tests" in reglistjson and len( params.suite.split(',') ) > 1:
            for suite in params.suite.split(',')[1:]:
                data = json.loads( open( suite + ".json" ).read() )
                if "tests" in data:
                    reglistjson[ "tests" ].extend( data["tests"] )
                else:
                    print( suite + " does not appear to be a suite, missing key 'tests'" )

    if "tests" in reglistjson:
        p = subprocess.Popen( (params.executable_path + " -v").split(), shell=False, stdout=subprocess.PIPE )
        [pipe_stdout, pipe_stderr] = p.communicate()
        version_string = re.search('[0-9]+.[0-9]+.[0-9]+.[0-9]+', pipe_stdout).group(0)

        starttime = datetime.datetime.now()
        report = regression_report.Report(params, version_string)

        print( "Running regression...\n" )
        for simcfg in reglistjson["tests"]:
            os.chdir(ru.cache_cwd)
            sim_timestamp = str(datetime.datetime.now()).replace('-', '_' ).replace( ' ', '_' ).replace( ':', '_' ).replace( '.', '_' )
            if regression_id == None:
                regression_id = sim_timestamp

            try:
                #print "flatten config: " + os.path.join( simcfg["path"],"param_overrides.json" )
                configjson = ru.flattenConfig( os.path.join( simcfg["path"],"param_overrides.json" ) )
            except:
                report.addErroringTest(simcfg["path"], "Error flattening config.", "(no simulation directory created).")
                configjson = None

            campaign_override_fn = os.path.join( simcfg["path"],"campaign_overrides.json" )

            try:
                #print "flatten campaign: " + campaign_override_fn
                campjson = ru.flattenCampaign( campaign_override_fn, False )
            except:
                print "failed to flatten campaign: " + campaign_override_fn
                report.addErroringTest(simcfg["path"], "Failed flattening campaign.", "(no simulation directory created).")
                campjson = None

            if configjson is None:
                print("Error flattening config.  Skipping " + simcfg["path"])
                ru.final_warnings += "Error flattening config.  Skipped " + simcfg["path"] + "\n"
                continue

            constraints_satisfied = True
            if len(params.constraints_dict) != 0:
                real_params = configjson["parameters"]
                cons = params.constraints_dict
                for key in cons:
                    val = cons[key]
                    if key not in real_params.keys() or str(real_params[ key ]) != val:
                        print( "Scenario configuration did not satisfy constraint: {0} == {1} but must == {2}.".format( key, str(real_params[ key ]), val ) )
                        constraints_satisfied = False
                        continue

            if constraints_satisfied == False:
                continue

            if campjson is None:
                # Try loading directly from file
                campjson_file = open( os.path.join( simcfg["path"],"campaign.json" ) )
                campjson = json.loads( campjson_file.read().replace( "u'", "'" ).replace( "'", '"' ).strip( '"' ) )
                campjson_file.close()

            # add campaign to config
            configjson["campaign_json"] = str(campjson)

            # add custom_reports to config
            report_fn = os.path.join( simcfg["path"],"custom_reports.json" )
            if os.path.exists( report_fn ) == True:
                reportjson_file = open( report_fn )
                reportjson = json.loads( reportjson_file.read().replace( "u'", "'" ).replace( "'", '"' ).strip( '"' ) )
                reportjson_file.close()
                configjson["custom_reports_json"] = str(reportjson)
            else:
                configjson["custom_reports_json"] = None

            thread = regression_runner.commissionFromConfigJson( sim_timestamp, configjson, simcfg["path"], report )
            ru.reg_threads.append( thread )
        # do a schema test also
        if params.dts == True:
            report.schema = regression_runner.doSchemaTest()

    elif "sweep" in reglistjson:
        print( "Running sweep...\n" )
        param_name = reglistjson["sweep"]["param_name"]
        # NOTE: most code below was copy-pasted from 'tests' (regression) case above.
        # I could factor this and generalize now but future extensions of sweep capability may involve
        # a greater departure from this code path so that might be a premature optimization.
        for param_value in reglistjson["sweep"]["param_values"]:
            os.chdir(ru.cache_cwd)
            sim_timestamp = str(datetime.datetime.now()).replace('-', '_' ).replace( ' ', '_' ).replace( ':', '_' ).replace( '.', '_' )
            if regression_id == None:
                regression_id = sim_timestamp
            # atrophied? configjson_filename = reglistjson["sweep"]["path"]
            # atrophied? configjson_path = str( os.path.join( reglistjson["sweep"]["path"], "config.json" ) )

            configjson = ru.flattenConfig( os.path.join( reglistjson["sweep"]["path"], "param_overrides.json" ) )
            if configjson is None:
                print("Error flattening config.  Skipping " + simcfg["path"])
                ru.final_warnings += "Error flattening config.  Skipped " + simcfg["path"] + "\n"
                continue

            # override sweep parameter
            configjson["parameters"][param_name] = param_value

            campjson_file = open( os.path.join( reglistjson["sweep"]["path"],"campaign.json" ) )
            campjson = json.loads( campjson_file.read().replace( "u'", "'" ).replace( "'", '"' ).strip( '"' ) )
            campjson_file.close()
            configjson["campaign_json"] = str(campjson)

            report_fn = os.path.join( reglistjson["sweep"]["path"],"custom_reports.json" )
            if os.path.exists( report_fn ) == True:
                reportjson_file = open( report_fn )
                reportjson = json.loads( reportjson_file.read().replace( "u'", "'" ).replace( "'", '"' ).strip( '"' ) )
                reportjson_file.close()
                configjson["custom_reports_json"] = str(reportjson)
            else:
                configjson["custom_reports_json"] = None

            thread = regression_runner.commissionFromConfigJson( sim_timestamp, configjson, reglistjson["sweep"]["path"], None, False )
            ru.reg_threads.append( thread )
    else:
        print "Unknown state"
        sys.exit(0)

    # stay alive until done
    for thr in ru.reg_threads:
        thr.join()
        #print str(thr.sim_timestamp) + " is done."

    if report is not None:
        endtime = datetime.datetime.now()
        report.write(os.path.join("reports", "report_" + regression_id + ".xml"), endtime - starttime)
        print '========================================'
        print 'Elapsed time: ', endtime - starttime
        print '%(tests)3d tests total, %(passed)3d passed, %(failed)3d failed, %(errors)3d errors, schema: %(schema)s.' % report.Summary

    if ru.final_warnings is not "":
        print("----------------\n" + ru.final_warnings)
        #raw_input("Press Enter to continue...")

    # if doing sweep, call plotAllCharts.py with all sim_timestamps on command line.
    if "sweep" in reglistjson:
        print( "Plot sweep results...\n" )
        all_data = []
        all_data_prop = []
        
        ref_path_prop = os.path.join( str(reglistjson["sweep"]["path"]), os.path.join( "output", "PropertyReport.json" ) )
        ref_json_prop = {}
        if os.path.exists( ref_path_prop ) == True:
            ref_json_prop = json.loads( open( os.path.join( ru.cache_cwd, ref_path_prop ) ).read() )

        ref_path = os.path.join( str(reglistjson["sweep"]["path"]), os.path.join( "output", "InsetChart.json" ) )
        ref_json = json.loads( open( os.path.join( ru.cache_cwd, ref_path ) ).read() )

        for thr in ru.reg_threads:
            sim_dir = os.path.join( thr.sim_root, thr.sim_timestamp )
            icj_filename = os.path.join( sim_dir, os.path.join( "output", "InsetChart.json" ) )
            icj_json = json.loads( open( icj_filename ).read() )
            all_data.append( icj_json )
            if os.path.exists( ref_path_prop ) == True:
                prj_filename = os.path.join( sim_dir, os.path.join( "output", "PropertyReport.json" ) )
                prj_json = json.loads( open( prj_filename ).read() )
                all_data_prop.append( prj_json )
        plot_title = "Sweep over " + reglistjson["sweep"]["param_name"] + " (" + str(len(reglistjson["sweep"]["param_values"])) + " values)"
        os.chdir( ru.cache_cwd )
        import plotAllCharts
        plotAllCharts.plotBunch( all_data, plot_title, ref_json )
        if os.path.exists( ref_path_prop ) == True:
            plotAllCharts.plotBunch( all_data_prop, plot_title, ref_json_prop )
        time.sleep(1)
        
    return


if __name__ == "__main__":
    # 'twould be nice to ditch this (keeping for legacy reasons) anyone actually use this?
    if len(sys.argv) > 1 and sys.argv[1] == "--flatten":
        ru.flattenConfig( sys.argv[2] )
        sys.exit(0)

    setup()
    main()
