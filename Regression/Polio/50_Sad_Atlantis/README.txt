Copied from 44 and tweaked from there

config_flat.json
- Changed Acquire_Rate_Default_Polio
- - From 2e-07
- - To 0.001 (1e-03)
- Changed Demographics_Filename
- - From "Atlantis_nine_node_demographics.compiled.json;Atlantis_nine_node_overlay_demographics.compiled.json"
- - To   "Atlantis_nine_node_smallpop_prev_demographics.compiled.json;Atlantis_nine_node_overlay_demographics.compiled.json"
- Changed Enable_Interventions
- - From 1
- - To 0

Atlantis_nide_node_demographics.json
Defaults
- Changed InitialPopulation
- - From 10000
- - To   1000
- Changed PrevalenceDistribution
- From 
         "PrevalenceDistributionFlag":0,
         "PrevalenceDistribution1":0,
         "PrevalenceDistribution2":0,
- To 
         "PrevalenceDistributionFlag":1,
         "PrevalenceDistribution1":0.01,
         "PrevalenceDistribution2":0.03,


