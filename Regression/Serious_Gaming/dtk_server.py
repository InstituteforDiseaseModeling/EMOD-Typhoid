#!/usr/bin/python

import SocketServer
import socket
import os
import sys
import time
import json
import shutil
import pdb

os.putenv( "GAME_MODE", "1" )

class MyTCPHandler(SocketServer.BaseRequestHandler):

    running = False
    state = "STOPPED"
    timestep = 0
    game_port = -1

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
        self.request.sendall( "hello!\nType \"RUN\", \"STEP\", \"KILL\", \"NEW_EVENT\", or \"NEW_EVENT_COST\".\n" )
        self.ref_json = json.loads( open( "output/BinnedReport.json" ).read() )["Channels"]
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
                    msg = self.insert_new_camp_event_and_step_reload( self.data.strip( "NEW_EVENT:" ) )
                    print( msg )
                elif self.data.startswith( "NEW_EVENT_COST:" ):
                    print( "Received NEW_EVENT_COST. Everything after colon better be valid json." )
                    msg = self.cost_proposed_event( self.data.strip( "NEW_EVENT_COST:" ) ) + "\n"
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
        os.putenv( "GAME_PORT", str(self.game_port) )
        #exec_string = "../../build/Eradication_game --config config.json -I . -O testing &"
        #exec_string = "/home2/jbloedow/trunk/build/Eradication_game --config config.json -I . -O testing &"
        #exec_string = "/home2/jbloedow/trunk/build/x64/Release/Eradication/Eradication --config config.json -I . -O testing &"
        exec_string = "/home/jbloedow/EMOD/DtkTrunk-IDM/build/x64/Release/Eradication/Eradication --config config.json -I . -O testing &"
        dtk_run = os.system( exec_string )
        time.sleep(2)
        self.dtk_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.dtk_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.dtk_socket.connect(("localhost", self.game_port))
        status_report = json.loads( "{}" )
        status_report["status"] = "running"
        status_report["Age_Bins_Max_Ages"] = [ 5*365.0, 12*365.0, 20*365.0, 55*365.0, 999999.0 ]
        self.state = "RUNNING"
        self.timestep = 0
        return json.dumps( status_report )

    def kill_dtk(self):
        print( "Terminating all DTK instances." )
        #os.system( "killall Eradication_game" )
        os.system( "killall Eradication" )
        self.dtk_socket.close()
        status_report = json.loads( "{}" )
        status_report["status"] = "stopped"
        self.running = False
        self.state = "STOPPED"
        return json.dumps( status_report )

    def pass_through(self, msg):
        self.dtk_socket.sendall( msg )
        data = self.dtk_socket.recv( 2000 )
        ic_json = json.loads(data.split('\n')[0])
        binned_json = json.loads(data.split('\n')[1])

        #print( "Not including averted cases yet since move to Binned Report. TBD. Pls be patient." )
        #pdb.set_trace()
        cur = binned_json["New Clinical Cases"][0]
        ref = self.ref_json["New Clinical Cases"]["Data"]
        binned_json["New Clinical Cases Averted"] = []
        for idx in range(0,5):
            binned_json["New Clinical Cases Averted"].append( ref[idx][self.timestep] )

        return_packet = {}
        return_packet["InsetChart"] = ic_json
        return_packet["Binned"] = json.dumps(binned_json).replace( "[[", "[" ).replace( "]]", "]")
        return_packet["Timestep"] = self.timestep

        return_string = (json.dumps(return_packet) + "\n").replace( "u'", "'" )
        self.timestep = self.timestep + 1

        return return_string

    def insert_new_camp_event_and_step_reload( self, new_event_params ):
        """
        NEW_EVENT: {
                     "Rollout_Distribution": "BOX",
                     "Percentage_Of_Target_Population_Reached": 0.9,
                     "Target_Population_Min_Age_In_Years": 5,
                     "Target_Population_Max_Age_In_Years": 50,
                     "Timesteps_From_Now": 1,
                     "Intervention_Quality": "Medium",
                     "Intervention": "BEDNET",
                     "Length_Of_Rollout_In_Days": 21
                   }
        NEW_EVENT: { "Rollout_Distribution": "BOX", "Percentage_Of_Target_Population_Reached": 1.0, "Target_Population_Min_Age_In_Years": 0, "Target_Population_Max_Age_In_Years": 125, "Timesteps_From_Now": 1, "Intervention_Quality": "High", "Intervention": "BEDNET", "Length_Of_Rollout_In_Days": 1 }
        """
        print( "json?: " + new_event_params )
        event_params_json = json.loads( new_event_params )
        print( "Parsed json. Now create corresponding event, append to campaign.json, and save." )
        #pdb.set_trace()
        # Create new event, load campaign, append to campaign, save
        camp_event = self.create_event_from_json( event_params_json )
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
        print( "Returning: " + json.dumps(data_json) )
        return (json.dumps(data_json) + "\n").replace( "u'", "'" )
        
    def create_event_from_json( self, epj ):
        reference_campaign_string = """
        { "Event_Coordinator_Config": { "Demographic_Coverage": 1.0, "Intervention_Config": { "class": "DelayedIntervention" }, "Target_Demographic": "ExplicitAgeRanges", "class": "ReferenceTrackingEventCoordinator", "Update_Period": 1, "Duration": 1, "Time_Value_Map": { "Times": [ 0 ], "Values": [ 0 ] } }, "Nodeset_Config": { "class": "NodeSetAll" }, "Start_Day": 9999, "class": "CampaignEvent" }
        """
        #{ "Event_Coordinator_Config": { "Demographic_Coverage": 1.0, "Intervention_Config": { "class": "DelayedIntervention" }, "Target_Demographic": "ExplicitAgeRanges", "class": "StandardInterventionDistributionEventCoordinator" }, "Nodeset_Config": { "class": "NodeSetAll" }, "Start_Day": 9999, "class": "CampaignEvent" }
        camp_event = json.loads( reference_campaign_string.strip()  )
        camp_event["Start_Day"] = self.timestep + epj["Timesteps_From_Now"]

        if not (epj["Intervention"] == "DRUG" and epj["Intervention_Quality"] == "Low"):
            #camp_event["Event_Coordinator_Config"]["Demographic_Coverage"] = epj["Percentage_Of_Target_Population_Reached"]
            camp_event["Event_Coordinator_Config"]["Target_Age_Min"] = epj["Target_Population_Min_Age_In_Years"]
            camp_event["Event_Coordinator_Config"]["Target_Age_Max"] = epj["Target_Population_Max_Age_In_Years"]
            camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Actual_IndividualIntervention_Configs"] = []

            if epj["Rollout_Distribution"] == "BOX":
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Delay_Distribution"] = "UNIFORM_DURATION"
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Delay_Period_Min"] = 0
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Delay_Period_Max"] = epj["Length_Of_Rollout_In_Days"]
            elif epj["Rollout_Distribution"] == "GAUSSIAN":
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Delay_Distribution"] = "GAUSSIAN_DURATION"
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Delay_Period_Mean"] = epj["Length_Of_Rollout_In_Days"]/2
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Delay_Period_Std_Dev"] = epj["Length_Of_Rollout_In_Days"]/4
            else:
                print( "ERROR: Rollout_Distribution type not supported: " + epj["Rollout_Distribution"] )
        else:
            print( "NOT USING DELAYED INTERVENTION TO DISTRIBUTE CAMPAIGN OVER TIME." )

        if epj["Intervention"] == "BEDNET":
            actual_intervention_config = {}
            actual_intervention_config[ "class" ] = "SimpleBednet"
            actual_intervention_config[ "Cost_To_Consumer" ] = 1.0
            if epj["Intervention_Quality"] == "High":
                actual_intervention_config[ "Killing_Rate" ] = 0.9
                actual_intervention_config[ "Blocking_Rate" ] = 0.9
            elif epj["Intervention_Quality"] == "Medium":
                actual_intervention_config[ "Killing_Rate" ] = 0.5
                actual_intervention_config[ "Blocking_Rate" ] = 0.9
            elif epj["Intervention_Quality"] == "Low":
                actual_intervention_config[ "Killing_Rate" ] = 0.0
                actual_intervention_config[ "Blocking_Rate" ] = 0.9
            camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Actual_IndividualIntervention_Configs"].append( actual_intervention_config )
            camp_event["Event_Coordinator_Config"]["Time_Value_Map"]["Values"][0] = epj["Percentage_Of_Target_Population_Reached"]

        elif epj["Intervention"] == "IRS":
            actual_intervention_config = {}
            actual_intervention_config[ "class" ] = "IRSHousingModification"
            if epj["Intervention_Quality"] == "High":
                actual_intervention_config[ "Killing_Rate" ] = 0.95
                actual_intervention_config[ "Blocking_Rate" ] = 0.0
            elif epj["Intervention_Quality"] == "Medium":
                actual_intervention_config[ "Killing_Rate" ] = 0.66
                actual_intervention_config[ "Blocking_Rate" ] = 0.0
            elif epj["Intervention_Quality"] == "Low":
                actual_intervention_config[ "Killing_Rate" ] = 0.3
                actual_intervention_config[ "Blocking_Rate" ] = 0.0
            camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Actual_IndividualIntervention_Configs"].append( actual_intervention_config )

        elif epj["Intervention"] == "DRUG":
            actual_intervention_config = {}

            if epj["Intervention_Quality"] == "Low":
            # For Low, Case Management: give drugs to currently symptomatic and on-symptomatic
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["class"] = "NodeLevelHealthTriggeredIV"
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Trigger_Condition"] = "NewClinicalCase"
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Demographic_Coverage"] = epj["Percentage_Of_Target_Population_Reached"]
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Target_Age_Min"] = epj["Target_Population_Min_Age_In_Years"]
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Target_Age_Max"] = epj["Target_Population_Max_Age_In_Years"]

                actual_intervention_config ["class"] = "AntimalarialDrug"
                actual_intervention_config ["Dosing_Type"] = "FullTreatmentNewDetectionTech" 
                actual_intervention_config ["Drug_Type"] = "Artemether_Lumefantrine"
            elif epj["Intervention_Quality"] == "Medium":
            # For Mid, CM + MSAT: also systematically test "everyone" over some period of time, and treat-if-infected
            # for now just do MSAT
                actual_intervention_config[ "class" ] = "SimpleDiagnostic"
                actual_intervention_config[ "Event_Or_Config" ] = "Config"
                actual_intervention_config[ "Positive_Diagnosis_Config" ] = {}
                actual_intervention_config[ "Positive_Diagnosis_Config" ]["class"] = "AntimalarialDrug"
                actual_intervention_config[ "Positive_Diagnosis_Config" ]["Dosing_Type"] = "FullTreatmentNewDetectionTech" 
                actual_intervention_config[ "Positive_Diagnosis_Config" ]["Drug_Type"] = "Artemether_Lumefantrine"
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Actual_IndividualIntervention_Configs"].append( actual_intervention_config )
                camp_event["Event_Coordinator_Config"]["Number_Repetitions"] = 9
                camp_event["Event_Coordinator_Config"]["Timesteps_Between_Repetitions"] = 9
            elif epj["Intervention_Quality"] == "High":
            # For Hi, CM + MDA: treat everyone
            # for now just do the additional MSAT, rely on calling code to call for Low and Medium (TBD)
            #camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Actual_IndividualIntervention_Config"] = actual_intervention_config
                actual_intervention_config["class"] = "AntimalarialDrug"
                actual_intervention_config["Dosing_Type"] = "FullTreatmentNewDetectionTech" 
                actual_intervention_config["Drug_Type"] = "Artemether_Lumefantrine"
                camp_event["Event_Coordinator_Config"]["Intervention_Config"]["Actual_IndividualIntervention_Configs"].append( actual_intervention_config )
                camp_event["Event_Coordinator_Config"]["Number_Repetitions"] = 10
                camp_event["Event_Coordinator_Config"]["Timesteps_Between_Repetitions"] = 10
            else:
                print( "Unsupported Intervention_Quality value: " + epj["Intervention_Quality"] )


        else:
            print( "ERROR: Intervention type not supported: " + epj["Intervention"] )

        return camp_event

    def cost_proposed_event( self, proposed_campaign ):
        #pdb.set_trace()
        try:
            return_msg = json.loads( "{}" )
            return_value = -1

            event_params_json = None
            try:
                event_params_json = json.loads( proposed_campaign )
            except Exception as ex:
                return_msg["error"] = str(ex)

            iq       = event_params_json["Intervention_Quality"]
            iv       = event_params_json["Intervention"]
            coverage = event_params_json["Percentage_Of_Target_Population_Reached"]
            age_min  = event_params_json["Target_Population_Min_Age_In_Years"]
            age_max  = event_params_json["Target_Population_Max_Age_In_Years"]
            rollout  = event_params_json["Rollout_Distribution"]
            rollout_length = event_params_json["Length_Of_Rollout_In_Days"]

            cost = 0

            if iv == "BEDNET":
                cost = 5
            elif iv == "IRS":
                cost = 15
            elif iv == "DRUG":
                cost = 10

            if iq == "Low":
                cost *= 1
            elif iq == "Medium":
                cost *= 2
            elif iq == "High":
                cost *= 3

            num_individuals = 1000
            num_individuals *= coverage
            num_individuals *= (age_max-age_min)/100.0

            cost *= num_individuals

            cost *= 2.0/rollout_length

            return_value = cost

            print( "Calculated projected cost of campaign as: " + str(return_value) )
            return_msg["Projected_Cost"] = return_value

        except Exception as ex:
            return_msg["error"] = "Missing Key: " + str(ex)

        return json.dumps( return_msg )

if __name__ == "__main__":
    HOST, PORT = "10.129.110.137", 7777

    if len( sys.argv ) > 1:
        PORT = int(sys.argv[1])
        print( "Using " + str(PORT) + " as python server port." )
    if len( sys.argv ) > 2:
        HOST = sys.argv[2]
        print( "Using " + HOST + " as python server host IP." )
    MyTCPHandler.game_port = PORT + 100

    # Create the server, binding to localhost on port 9999
    SocketServer.TCPServer.allow_reuse_address = True
    server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
