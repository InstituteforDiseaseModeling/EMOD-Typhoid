54_paralysis_tests_humoral0

Tests based on Kevin McCarthy's Polio work.  Each of these tests uses WPV_humoral_0_demographics.compiled.json.

/***********
*** sims ***
***********/

Rate0p005_humoral0_OPV3
Rate1_humoral0_OPV3

These tests use an OPV3 campaign, and vary the Paralysis rate from 0.005 to 1.0 for each strain.

Rate0p005_humoral0_WPV1
Rate1_humoral0_WPV1

These tests use a WPV1 campaign, and vary the Paralysis rate from 0.005 to 1.0 for each strain.

/*************
*** default **
*************/

The config_flat.json file is identical to 

Rate0p005_humoral0_OPV3\config_flat_0p005_humoral0_OPV3.json

with the exception of using campaign.json.

campaign.json is identical to campaign_OPV3.json

Making those simulations identical.

/**************
*** running ***
**************/

for other than the standard sim...

Eradication.exe -C Rate0p005_humoral0_OPV3\config_flat_0p005_humoral0_OPV3.json -O Rate0p005_humoral0_OPV3\output > Rate0p005_humoral0_OPV3\console.txt

Eradication.exe -C Rate0p005_humoral0_WPV1\config_flat_0p005_humoral0_WPV1.json -O Rate0p005_humoral0_WPV1\output > Rate0p005_humoral0_WPV1\console.txt

Eradication.exe -C Rate1_humoral0_OPV3\config_flat_1_humoral0_OPV3.json -O Rate1_humoral0_OPV3\output > Rate1_humoral0_OPV3\console.txt

Eradication.exe -C Rate1_humoral0_WPV1\config_flat_1_humoral0_WPV1.json -O Rate1_humoral0_WPV1\output > Rate1_humoral0_WPV1\console.txt

/************
*** plots ***
************/

The following plots compare varying paralysis rates, keeping all else the same

plotAllChartsPolio-onlyCumulativeParalytic.py Rate0p005_humoral0_OPV3\output\InsetChart.json Rate1_humoral0_OPV3\output\InsetChart.json Rate0p005_v_Rate1

plotAllChartsPolio-allParalyticChannels.py Rate0p005_humoral0_OPV3\output\InsetChart.json Rate1_humoral0_OPV3\output\InsetChart.json Rate0p005_v_Rate1

The following plots compare the different vaccines with a paralysis rate of 1

plotAllChartsPolio-onlyCumulativeParalytic.py Rate1_humoral0_WPV1\output\InsetChart.json Rate1_humoral0_OPV3\output\InsetChart.json WPV1_v_OPV3

plotAllChartsPolio-allParalyticChannels.py Rate1_humoral0_WPV1\output\InsetChart.json Rate1_humoral0_OPV3\output\InsetChart.json WPV1_v_OPV3