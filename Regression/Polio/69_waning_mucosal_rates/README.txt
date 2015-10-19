The point of this test is to verify mucosal immunity waning, and waning rates.  This is done by running a simulation that gives out a tOPV dose on Day 1, and comparing the cumulative Nab on day 1 after the tOPV to day 100.  The expected result is that this wanes, and it wanes more if the fast fraction is larger / faster.  See 68_waning_humoral_rates for more.


/********************
*** running these ***
********************/
Eradication.exe -C config-survey.json -O output --dll-path ..\..\..\x64\Release > console_output.txt

Eradication.exe -C config_mostly_fast.json -O output_mostly_fast --dll-path ..\..\..\x64\Release > console_mostly_fast.txt

Eradication.exe -C config_mostly_slow.json -O output_mostly_slow --dll-path ..\..\..\x64\Release > console_mostly_slow.txt

/*********************
*** plotting these ***
*********************/

First, the day 1 vs day 100 with defaults (expect to see the green bars on the left):

compare_Nab.py output\PolioSurveyJSONAnalyzer_Initial_1.json output\PolioSurveyJSONAnalyzer_Day100_1.json

Second, the day 100 default vs day 100 mostly fast (expect to see the green bars on the left):

compare_Nab.py output\PolioSurveyJSONAnalyzer_Day100_1.json output_mostly_fast\PolioSurveyJSONAnalyzer_Day100_1.json

Lastly, the day 100 default vs day 100 mostly slow (expect to see the green bars on the RIGHT this time):

compare_Nab.py output\PolioSurveyJSONAnalyzer_Day100_1.json output_mostly_slow\PolioSurveyJSONAnalyzer_Day100_1.json


/******************************
*** stuff to note in charts ***
******************************/

You'll note that the mucosal histogram changes, but the humoral does not!  ALAS!  But all is not lost, as these charts are plotting one moment in time.  Indeed, if we make mucosal immunity wane faster, but don't change the humoral rates, we would _expect_ the humoral immunity to be identical.
