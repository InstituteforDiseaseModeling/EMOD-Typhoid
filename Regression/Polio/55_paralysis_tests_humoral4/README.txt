55_paralysis_tests_humoral4

Tests based on Kevin McCarthy's Polio work.  Each of these tets uses WPV_humoral_4_demographics.compiled.json.

/***********
*** sims ***
***********/

Rate0p005_humoral4_OPV3
Rate1_humoral4_OPV3

These tests use an OPV3 campaign, and vary the Paralysis rate from 0.005 to 1.0 for each strain.

Rate0p005_humoral4_WPV1
Rate1_humoral4_WPV1

These tests use a WPV1 campaign, and vary the Paralysis rate from 0.005 to 1.0 for each strain.

/*************
*** default **
*************/

The config_flat.json file is identical to 

Rate1_humoral4_WPV1\config_flat_1_humoral4_WPV1.json

with the exception of using campaign.json.  

campaign.json is identical to campaign_WPV1.json

Making those simulations identical.

/**************
*** scripts ***
**************/

plotAllChartsPolio-onlyCumulativeParalytic.py 
- This is like "plotAllCharts.py," but the only paralytic channel it plots is "Cumulative Paralytic Cases."  If this chart is all zeroes, there is no need for the rest of the paralytic channels.

plotAllChartsPolio-allParalyticChannels
- This is like "plotAllCharts.py," but only plots the paralytic channels.  This chart may be useful if you saw paralytic cases in the chart from the above script.

/**************
*** running ***
**************/

Eradication.exe -C config_flat.json > console.txt

Eradication.exe -C Rate0p005_humoral4_OPV3\config_flat_0p005_humoral4_OPV3.json -O Rate0p005_humoral4_OPV3\output > Rate0p005_humoral4_OPV3\console.txt

Eradication.exe -C Rate0p005_humoral4_WPV1\config_flat_0p005_humoral4_WPV1.json -O Rate0p005_humoral4_WPV1\output > Rate0p005_humoral4_WPV1\console.txt

Eradication.exe -C Rate1_humoral4_OPV3\config_flat_1_humoral4_OPV3.json -O Rate1_humoral4_OPV3\output > Rate1_humoral4_OPV3\console.txt

Eradication.exe -C Rate1_humoral4_WPV1\config_flat_1_humoral4_WPV1.json -O Rate1_humoral4_WPV1\output > Rate1_humoral4_WPV1\console.txt

/***************
*** plotting ***
***************/

plotAllChartsPolio-allParalyticChannels.py Rate0p005_humoral4_OPV3\output\InsetChart.json Rate1_humoral4_OPV3\output\InsetChart.json Rate0p005_v_Rate1_both_humoral4_OPV3

plotAllChartsPolio-onlyCumulativeParalytic.py Rate0p005_humoral4_OPV3\output\InsetChart.json Rate1_humoral4_OPV3\output\InsetChart.json Rate0p005_v_Rate1_both_humoral4_OPV3

plotAllChartsPolio-onlyCumulativeParalytic.py Rate1_humoral4_OPV3\output\InsetChart.json ..\54_paralysis_tests_humoral0\Rate1_humoral0_OPV3\output\InsetChart.json RedHumoral4_v_BlueHumoral0_both_OPV3_Rate1

plotAllChartsPolio-allParalyticChannels.py Rate1_humoral4_OPV3\output\InsetChart.json ..\54_paralysis_tests_humoral0\Rate1_humoral0_OPV3\output\InsetChart.json RedHumoral4_v_BlueHumoral0_both_OPV3_Rate1

plotAllChartsPolio-onlyCumulativeParalytic.py Rate1_humoral4_OPV3\output\InsetChart.json Rate1_humoral4_WPV1\output\InsetChart.json OPV3_v_WPV1_both_humoral4_rate1

plotAllChartsPolio-allParalyticChannels.py Rate1_humoral4_OPV3\output\InsetChart.json Rate1_humoral4_WPV1\output\InsetChart.json OPV3_v_WPV1_both_humoral4_rate1_only_paralytic