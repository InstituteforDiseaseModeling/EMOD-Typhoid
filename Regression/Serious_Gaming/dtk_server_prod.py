#!/usr/bin/python

import SocketServer
import socket
import os
import time
import json
import shutil
import pdb

os.putenv( "GAME_MODE", "1" )

class MyTCPHandler(SocketServer.BaseRequestHandler):

    running = False
    state = "STOPPED"
    timestep = 0

    """
    The RequestHandler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """

    def handle(self):
        if self.running == True:
            msg = "Another instance is already running the DTK. It needs to shut down and disconnect first.\n"
            print( msg )
            self.request.sendall( msg )
            return

        self.running = True
        self.request.sendall( "hello!\nType \"RUN\", \"STEP\", or \"KILL\"\n" )
        #self.dtk_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while True:
            # self.request is the TCP socket connected to the client
            try:
                self.data = self.request.recv(1024).strip()
                print "{} wrote:".format(self.client_address[0])
                msg = None
                print self.data
                if self.data == "RUN":
                    if self.state == "RUNNING":
                        msg = "Already running. Ignoring RUN command."
                    else:
                        msg = self.run_dtk() + "\n"
                elif self.data == "KILL":
                    msg = self.kill_dtk() + "\n"
                elif self.data == "STEP":
                    msg = self.pass_through( self.data )
                elif self.data.startswith( "NEW_EVENT:" ):
                    print( "Received NEW_EVENT. Everything after colon better be valid json." )
                    #pdb.set_trace()
                    msg = self.insert_new_camp_event_and_step_reload( self.data.strip( "NEW_EVENT:" ) )
                    print( msg )
                else:
                    msg = self.data + " is not a recognized command.\n"
                    print( msg )
                # just send back the same data, but upper-cased
                self.request.sendall( msg )
            except Exception as ex:
                print( "Exception: assuming client disconnected. Force kill." )
                self.kill_dtk()
                return

    def run_dtk(self):
        shutil.copy( "campaign_master.json", "campaign.json" )
        game_port = 7887
        os.putenv( "GAME_PORT", str(game_port) )
        #exec_string = "../../build/Eradication_game --config config.json -I . -O testing &"
        exec_string = "/home2/jbloedow/trunk/build/Eradication_game --config config.json -I . -O testing &"
        #exec_string = "/home2/jbloedow/trunk/build/x64/Release/Eradication/Eradication --config config.json -I . -O testing &"
        dtk_run = os.system( exec_string )
        time.sleep(2)
        self.dtk_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.dtk_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.dtk_socket.connect(("localhost", game_port))
        status_report = json.loads( "{}" )
        status_report["status"] = "running"
        self.state = "RUNNING"
        self.timestep = 0
        return str( status_report )

    def kill_dtk(self):
        print( "Terminating all DTK instances." )
        os.system( "killall Eradication_game" )
        #os.system( "killall Eradication" )
        self.dtk_socket.close()
        status_report = json.loads( "{}" )
        status_report["status"] = "stopped"
        self.running = False
        self.state = "STOPPED"
        return str( status_report )

    def pass_through(self, msg):
        self.dtk_socket.sendall( msg )
        data = self.dtk_socket.recv( 2000 )
        data_json = json.loads(data)
        data_json["Timestep"] = self.timestep
        self.timestep = self.timestep + 1
        return (str(data_json) + "\n").replace( "u'", "'" )

    def insert_new_camp_event_and_step_reload( self, new_event_params ):
        print( "json?: " + new_event_params )
        event_params_json = json.loads( new_event_params )
        print( "Parsed json. Now create corresponding event, append to campaign.json, and save." )
        # Create new event, load campaign, append to campaign, save
        camp_event = json.loads( '{"Event_Coordinator_Config": { "Demographic_Coverage": 1.0, "Intervention_Config": { "class": "DelayedIntervention" }, "Target_Demographic": "ExplicitAgeRanges", "class": "StandardInterventionDistributionEventCoordinator" }, "Nodeset_Config": { "class": "NodeSetAll" }, "Start_Day": 9999, "class": "CampaignEvent" }' )
        camp_event["Event_Coordinator_Config"]["Demographic_Coverage"] = event_params_json["Percentage_Of_Target_Population_Reached"]
        camp_event["Event_Coordinator_Config"]["Target_Age_Min"] = event_params_json["Target_Population_Min_Age_In_Years"]
        camp_event["Event_Coordinator_Config"]["Target_Age_Max"] = event_params_json["Target_Population_Max_Age_In_Years"]
        camp_event["Start_Day"] = self.timestep + event_params_json["Timesteps_From_Now"]
        camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Actual_IndividualIntervention_Configs"] = []

        if event_params_json["Rollout_Distribution"] == "BOX":
            camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Delay_Distribution"] = "UNIFORM_DURATION"
            camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Delay_Period_Min"] = 0
            camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Delay_Period_Max"] = event_params_json["Length_Of_Rollout_In_Days"]
        else:
            print( "ERROR: Rollout_Distribution type not supported: " + event_params_json["Rollout_Distribution"] )

        if event_params_json["Intervention"] == "BEDNET":
            actual_intervention_config = {}
            actual_intervention_config[ "class" ] = "SimpleBednet"
            actual_intervention_config[ "Killing_Rate" ] = 0.9
            actual_intervention_config[ "Blocking_Rate" ] = 0.9
            camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Actual_IndividualIntervention_Configs"].append( actual_intervention_config )
        else:
            print( "ERROR: Intervention type not supported: " + event_params_json["Intervention"] )

        campaign_json = json.loads( open( "campaign.json" ).read() )
        campaign_json["Events"].append( camp_event )
        with open( "campaign.json", "w" ) as handle:
            handle.write( json.dumps( campaign_json, indent=4, sort_keys=True ) )

        print( "Tell DTK to STEP_RELOAD" )
        self.dtk_socket.sendall( "STEP_RELOAD"  )
        data = self.dtk_socket.recv( 2000 )
        print( "DTK returned: " + data )
        data_json = json.loads(data)
        data_json["Timestep"] = self.timestep
        self.timestep = self.timestep + 1
        print( "Returning: " + str(data_json) )
        return (str(data_json) + "\n").replace( "u'", "'" )
        
if __name__ == "__main__":
    HOST, PORT = "10.129.110.137", 8888

    # Create the server, binding to localhost on port 9999
    SocketServer.TCPServer.allow_reuse_address = True
    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
