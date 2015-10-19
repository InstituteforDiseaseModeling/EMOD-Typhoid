4 sims from Kevin McCarthy's polio work.

/***********
*** Sims ***
***********/
VaccineTiter1_mucosaltiter0 is minimum vaccine titer, minimum mucosal.

VaccineTiter1e7_mucosaltiter11 is max/max

VaccineTiter1e3p9_mucosaltiter6 is "right in the middle"

VaccineTiter1e6p1_mucosaltiter0 is "what we think are realistic values," and is the default case (config_flat.json in this folder)

/************
*** plots ***
************/

Realish_v_Middling plots
Red ) VaccineTiter1e6p1_mucosaltiter0 
Blue) VaccineTiter1e3p9_mucosaltiter6

MinMinRed_v_MaxMaxBlue plots
Red ) VaccineTiter1_mucosaltiter0 
Blue) VaccineTiter1e7_mucosaltiter11 

/**************
*** scripts ***
**************/

plotAllChartsPolio-onlyCumulativeParalytic.py

This is a custom version of plotAllCharts.py that leaves out all of the paralysis charts other that Cumulative Paralytic Cases.  If you see that chart go above zero, you  should run another plot with the traditional script.

/*************
*** to run ***
*************/

Eradication.exe -C VaccineTiter1e7_mucosaltiter11\config_flat_vaxTiter1e7_mucosalTiter11.json -O VaccineTiter1e7_mucosaltiter11\output > VaccineTiter1e7_mucosaltiter11\console.txt

Eradication.exe -C VaccineTiter1_mucosaltiter0\config_flat_vaxTiter1_mucosalTiter0.json -O VaccineTiter1_mucosaltiter0\output > VaccineTiter1_mucosaltiter0\console.txt

Eradication.exe -C VaccineTiter1e3p9_mucosaltiter6\config_flat_vaxTiter1e3p9_mucosalTiter6.json -O VaccineTiter1e3p9_mucosaltiter6\output > VaccineTiter1e3p9_mucosaltiter6\console.txt

Eradication.exe -C VaccineTiter1e6p1_mucosaltiter0\config_flat_vaxTiter1e6p1_mucosalTiter0.json -O VaccineTiter1e6p1_mucosaltiter0\output > VaccineTiter1e6p1_mucosaltiter0\console.txt

/**************
*** to plot ***
**************/

plotAllChartsPolio-onlyCumulativeParalytic.py VaccineTiter1_mucosaltiter0\output\InsetChart.json VaccineTiter1e7_mucosaltiter11\output\InsetChart.json MinMinRed_v_MaxMaxBlue

plotAllChartsPolio-onlyCumulativeParalytic.py VaccineTiter1e6p1_mucosaltiter0\output\InsetChart.json VaccineTiter1e3p9_mucosaltiter6\output\InsetChart.json RealishRed_v_MiddlingBlue