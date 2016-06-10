#!/usr/bin/python

import subprocess
import datetime
import json
import time
import os
import pdb

import regression_local_monitor
import regression_utils as ru
import regression_clg as clg

class HpcMonitor(regression_local_monitor.Monitor):
    def __init__(self, sim_id, config_id, report, params, suffix, config_json=None, compare_results_to_baseline=True):
        #super(regression_local_monitor.Monitor,self).__init__( sim_id, config_id, report, params, config_json, compare_results_to_baseline )
        regression_local_monitor.Monitor.__init__( self, sim_id, config_id, report, params, config_json, compare_results_to_baseline )
        #print "Running DTK execution and monitor thread for HPC commissioning."
        self.sim_root = self.params.sim_root # override base Monitor which uses local sim directory
        self.config_json = config_json
        self.config_id = config_id
        self.suffix = suffix

    def run(self):
    
        self.__class__.sems.acquire()
        def get_num_cores( some_json ):
            num_cores = 1
            if ('parameters' in some_json) and ('Num_Cores' in some_json['parameters']):
                num_cores = some_json['parameters']['Num_Cores']
            else:
               print( "Didn't find key 'parameters/Num_Cores' in '{0}'. Using 1.".format( self.config_id ) )
               
            return int(num_cores)
    
        input_dir = self.params.input_root + self.config_json["parameters"]["Geography"] + "\\"
        sim_dir = self.sim_root + "\\" + self.sim_timestamp   # can't use os.path.join() here because on linux it'll give us the wrong dir-separator...
        if self.suffix is not None:
            job_name = self.config_json["parameters"]["Config_Name"].replace( ' ', '_' ) + "_" + self.suffix + "_(" + self.sim_timestamp + ")"
        else:
            job_name = self.config_json["parameters"]["Config_Name"].replace( ' ', '_' ) + "_(" + self.sim_timestamp + ")"
        job_name = job_name[:79]

        numcores = get_num_cores( self.config_json )

        hpc_resource_option = '/numcores:'
        hpc_resource_count  = str(numcores)
        mpi_core_option = None
        mpi_core_count  = ''

        if self.params.measure_perf:
            if numcores % self.params.cores_per_node == 0:
                hpc_resource_option = '/numnodes:'
                hpc_resource_count  = str(numcores / self.params.cores_per_node)
                mpi_core_option = '-c'
                mpi_core_count  = str(self.params.cores_per_node)
            elif numcores == self.params.cores_per_socket:
                hpc_resource_option = '/numsockets:'
                hpc_resource_count  = '1'
                mpi_core_option = '-c'
                mpi_core_count  = str(self.params.cores_per_socket)
            # "bail" here, we don't have a multiple of cores per node nor can we fit on a single socket

        #eradication.exe commandline
        eradication_bin = self.config_json['bin_path']
        eradication_options = { '--config':'config.json', '--input-path':input_dir, '--progress':' ' }

        # python-script-path is optional parameter.
        if "PSP" in self.config_json:
            eradication_options[ "--python-script-path" ] = self.config_json["PSP"]
        #if params.dll_root is not None and params.use_dlls is True:
        #    eradication_options['--dll-path'] = params.dll_root
        eradication_params = []
        eradication_command = clg.CommandlineGenerator(eradication_bin, eradication_options, eradication_params)

        #mpiexec commandline
        mpi_bin = 'mpiexec'
        mpi_options = {}
        if mpi_core_option is not None:
            mpi_options[mpi_core_option] = mpi_core_count
        mpi_params = [eradication_command.Commandline]
        mpi_command = clg.CommandlineGenerator(mpi_bin, mpi_options, mpi_params)
        
        #job submit commandline
        jobsubmit_bin = 'job submit'
        jobsubmit_options = {}
        jobsubmit_options['/workdir:'] = sim_dir
        jobsubmit_options['/scheduler:'] = self.params.hpc_head_node
        jobsubmit_options['/nodegroup:'] = self.params.hpc_node_group
        jobsubmit_options['/user:'] = self.params.hpc_user
        if self.params.hpc_password != '':
            jobsubmit_options['/password:'] = self.params.hpc_password
        jobsubmit_options['/jobname:'] = job_name
        jobsubmit_options[hpc_resource_option] = hpc_resource_count
        if self.params.measure_perf:
            jobsubmit_options['/exclusive'] = ' '
        jobsubmit_options['/stdout:'] = 'StdOut.txt'
        jobsubmit_options['/stderr:'] = 'StdErr.txt'
        jobsubmit_options['/priority:'] = 'Lowest'
        jobsubmit_params = [mpi_command.Commandline]
        jobsubmit_command = clg.CommandlineGenerator(jobsubmit_bin, jobsubmit_options, jobsubmit_params)

        #print 'simulation command line:', eradication_command.Commandline
        #print 'mpiexec command line:   ', mpi_command.Commandline
        #print 'job submit command line:', jobsubmit_command.Commandline

        hpc_command_line = jobsubmit_command.Commandline

        job_id = -1
        num_retries = -1

        while job_id == -1:
            num_retries += 1
            #print "executing hpc_command_line: " + hpc_command_line + "\n"

            p = subprocess.Popen( hpc_command_line.split(), shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
            [hpc_pipe_stdout, hpc_pipe_stderr] = p.communicate()
            #print "Trying to read hpc response..."
            #print hpc_pipe_stdout
            line = hpc_pipe_stdout

            if p.returncode == 0:
                job_id = line.split( ' ' )[-1].strip().rstrip('.')
                if str.isdigit(job_id) and job_id > 0:
                    print( self.config_id + " submitted (as job_id " + str(job_id) + ")\n" )
                else:
                    print( "ERROR: What happened here?  Please send this to Jeff:\n" )
                    print( hpc_pipe_stdout )
                    print( hpc_pipe_stderr )
                    job_id = -1
            else:
                print( "ERROR: job submit of " + self.config_id + " failed!" )
                print( hpc_pipe_stdout )
                print( hpc_pipe_stderr )

            if job_id == -1 and num_retries >= 5 and self.params.hide_graphs:
                print( "Job submission failed multiple times for " + self.config_id + ".  Aborting this test and logging error." )
                self.report.addErroringTest( self.config_id, "", sim_dir )
                return

        monitor_cmd_line = "job view /scheduler:" + self.params.hpc_head_node + " " + str(job_id)

        check_status = True
        while check_status:
            #print "executing hpc_command_line: " + monitor_cmd_line
            #print "Checking status of job " + str(job_id)
            #hpc_pipe = os.popen( monitor_cmd_line )
            hpc_pipe = subprocess.Popen( monitor_cmd_line.split(), shell=False, stdout=subprocess.PIPE )
            [hpc_pipe_stdout, hpc_pipe_stderr] = hpc_pipe.communicate()
            lines = hpc_pipe_stdout
            #for line in hpc_pipe.readlines():
            #print lines
            for line in lines.split('\n'):
                res = line.split( ':' )
                #print "DEBUG: " + str(res[0])
                if res[0].strip() == "State":
                    state = res[1].strip()
                    if state == "Failed":
                        self.__class__.completed = self.__class__.completed + 1
                        print( self.config_id + " FAILED!" )
                        check_status = False
                        self.report.addErroringTest( self.config_id, "", sim_dir )
                        #self.finish(sim_dir, False)
                    if state == "Canceled":
                        self.__class__.completed = self.__class__.completed + 1
                        print( "Canceled!" )
                        check_status = False
                        #self.finish(sim_dir, False)
                    elif state == "Completed" or state == "Finished":
                        self.__class__.completed = self.__class__.completed + 1
                        print( str(self.__class__.completed) + " out of " + str(len(ru.reg_threads)) + " completed." )
                        check_status = False

                        status_file = open(os.path.join(sim_dir, "status.txt"))
                        for status_line in status_file.readlines():
                            if status_line.startswith("Done"):
                                time_split = status_line.split('-')[1].strip().split(':')
                                self.duration = datetime.timedelta(hours=int(time_split[0]), minutes=int(time_split[1]), seconds=int(time_split[2]))
                                break

                        if self.compare_results_to_baseline:
                            if self.params.all_outputs == False:
                            # Following line is for InsetChart.json only
                                self.verify(sim_dir)
                            else:
                                # Every .json file in output (not hidden with . prefix) will be used for validation
                                for file in os.listdir( os.path.join( self.config_id, "output" ) ):
                                    if ( file.endswith( ".json" ) or file.endswith( ".csv" ) or file.endswith( ".kml" ) or file.endswith( ".bin" ) ) and file[0] != "." and file != "transitions.json" and "linux" not in file:
                                        self.verify( sim_dir, file, "Channels" )
                    break
            time.sleep(5)
        self.__class__.sems.release()


