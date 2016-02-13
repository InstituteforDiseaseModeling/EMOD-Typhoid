#!/usr/bin/python

import json

campaign = json.loads( open( "campaign.json" ).read() )

for event in campaign["Events"]:
    if event["Event_Coordinator_Config"]["Intervention_Config"]["class"] in [ "SimpleBednet", "IRSHousingModification" ]:
        iv = event["Event_Coordinator_Config"]["Intervention_Config"]
        profile = iv.pop("Durability_Time_Profile")
        kill = iv.pop("Killing_Rate")
        block = iv.pop("Blocking_Rate")

        kill_const = 3650
        if "Primary_Decay_Time_Constant" in iv:
            kill_const = iv.pop("Primary_Decay_Time_Constant") 

        block_const = 3650
        if "Secondary_Decay_Time_Constant" in iv:
            block_const = iv.pop("Secondary_Decay_Time_Constant") 
            
        if kill != None:
            kill_config = {}
            if profile == "BOXDURABILITY":
                kill_config["Box_Duration"] = kill_const
                kill_config["Initial_Effect"] = kill
                kill_config["class"] = "WaningEffectBox"
                iv["Killing_Config"] = kill_config
        if block != None:
            block_config = {}
            if profile == "BOXDURABILITY":
                block_config["Box_Duration"] = block_const
                block_config["Initial_Effect"] = block
                block_config["class"] = "WaningEffectBox"
                iv["Blocking_Config"] = block_config
        event["Event_Coordinator_Config"]["Intervention_Config"] = iv

with open( "campaign_new.json", "w" ) as cfg:
    cfg.write( json.dumps( campaign, indent=4, sort_keys = True ) )
