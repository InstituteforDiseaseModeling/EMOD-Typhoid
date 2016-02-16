#!/usr/bin/python

import json
import pdb


def createWaningBlock( profile, const, init, extra ):
    second = 99999
    waning_config = {}
    if profile == "BOXDURABILITY":
        waning_config["Box_Duration"] = const
        waning_config["Initial_Effect"] = init
        waning_config["class"] = "WaningEffectBox"
    elif profile == "DECAYDURABILITY":
        waning_config["Initial_Effect"] = init
        waning_config["Decay_Time_Constant"] = const
        waning_config["class"] = "WaningEffectExponential"
    elif profile == "BOXDECAYDURABILITY":
        waning_config["Box_Duration"] = const
        waning_config["Initial_Effect"] = init
        waning_config["Decay_Time_Constant"] = extra # not quite right. Putting in function may not work. TBD
        waning_config["class"] = "WaningEffectBoxExponential"
    return waning_config 

campaign = json.loads( open( "campaign.json" ).read() )

for event in campaign["Events"]:
    if event["Event_Coordinator_Config"]["Intervention_Config"]["class"] in [ "SimpleBednet", "IRSHousingModification", "SugarTrap", "InsectKillingFence", "Larvicides", "SpaceSpraying", "SpatialRepellent", "ArtificialDiet", "OvipositionTrap", "OutdoorRestKill", "AnimalFeedKill", "SpatialRepellentHousingModification", "SimpleIndividualRepellent", "SimpleHousingModification", "ScreeningHousingModification", "InsectKillingFenceHousingModification", "ArtificialDietHousingModification", "HumanHostSeekingTrap" ]:
        iv = event["Event_Coordinator_Config"]["Intervention_Config"]
        profile = iv.pop("Durability_Time_Profile")
	kill = None
        if "Killing" in iv:
            kill = iv.pop("Killing")
        elif "Killing_Rate" in iv:
            kill = iv.pop("Killing_Rate")

        block = None
        if "Blocking_Rate" in iv:
            block = iv.pop("Blocking_Rate")
        elif "Repellency" in iv:
            block = iv.pop("Repellency")
        elif "Reduction" in iv:
            block = iv.pop("Reduction")

        kill_const = 3650
        if "Primary_Decay_Time_Constant" in iv:
            kill_const = iv.pop("Primary_Decay_Time_Constant") 

        block_const = 3650
        if "Secondary_Decay_Time_Constant" in iv:
            block_const = iv.pop("Secondary_Decay_Time_Constant") 
            
        if kill != None:
            kill_config = createWaningBlock( profile, kill_const, kill, block_const )
            iv["Killing_Config"] = kill_config

        if block != None:
            if profile == "BOXDECAYDURABILITY":
                tmp = block_const
                block_const = kill_const
                kill_const = tmp
            block_config = createWaningBlock( profile, block_const, block, kill_const )
            iv["Blocking_Config"] = block_config

        event["Event_Coordinator_Config"]["Intervention_Config"] = iv
    elif event["Event_Coordinator_Config"]["Intervention_Config"]["class"] in [ "SimpleVaccine", "SimpleImmunoglobulin" ]:
        print( "Need to port vaccine waning to config." )
        iv = event["Event_Coordinator_Config"]["Intervention_Config"]
        profile = iv.pop("Durability_Time_Profile")
        prime_const = 3650
        if "Primary_Decay_Time_Constant" in iv:
            prime_const = iv.pop("Primary_Decay_Time_Constant") 

        second_const = 3650
        if "Secondary_Decay_Time_Constant" in iv:
            second_const = iv.pop("Secondary_Decay_Time_Constant") 

        init = None
        if iv["Vaccine_Type"] == "AcquisitionBlocking":
            init = iv.pop("Reduced_Acquire")
        elif iv["Vaccine_Type"] == "TransmissionBlocking":
            init = iv.pop("Reduced_Transmit")
        elif iv["Vaccine_Type"] == "MortalityBlocking":
            init = iv.pop("Reduced_Mortality")

        waning_config = createWaningBlock( profile, prime_const, init, second_const )
        iv[ "Waning_Config" ] = waning_config
        event["Event_Coordinator_Config"]["Intervention_Config"] = iv

with open( "campaign_new.json", "w" ) as cfg:
    cfg.write( json.dumps( campaign, indent=4, sort_keys = True ) )
