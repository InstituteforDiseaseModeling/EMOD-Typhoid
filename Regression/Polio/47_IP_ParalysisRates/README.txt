This case is showing that you can scale the paralysis rates differently.

Critical settings in config_flat.json files:

/***************************************
*** "Paralytic_Immunity_Titer": 1000 ***
***************************************/

This defaults to 3, but is dramatically raised here for the benefit of the next three:

/***************************************
*** "Paralysis_Base_Rate_WPV1": 0.8, *** #Default 0.05
*** "Paralysis_Base_Rate_WPV2": 0.3, *** #Default 0.00033
*** "Paralysis_Base_Rate_WPV3": 0.1, *** #Default 0.001
***************************************/

Again, the defaults here are much, much lower.

Lastly, the independent variable:

/************************************
*** "x_Population_Immunity": 0.0, ***
************************************/

This normally defaults to 1.0


/****************************************
*** How to run one of these multicore ***
****************************************/
In this example, my Eradication.exe build was named
Eradication-v17-Release-HINT.exe

Here is the console command to run the two core job locally

mpiexec -n 2 Eradication-v17-Release-HINT.exe --config config_flat_noimmunity.json --output-path output_2core > console_twocore.txt


/******************************************
*** Variable transmission rates in HINT ***
******************************************/

config_flat_IP_transmission_rates.json (derived from config_flat_paralysis_noimmunity_justWPV1.json)

switch demographics to
"Demographics_Filename": "Atlantis_nine_node_demographics.compiled.json;Atlantis_varaible_transmission_overlay_demographics.compiled.json", 

enable Heterogeneous_Intranode_Transmission

Results show transmission rates vary as expected.
