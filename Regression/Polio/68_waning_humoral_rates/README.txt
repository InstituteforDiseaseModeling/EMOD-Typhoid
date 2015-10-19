The point of this test is to verify that humoral immunity wanes as per the parameters:

Default:
        "Waning_Humoral_Fast_Fraction": 0.3,
        "Waning_Humoral_Rate_Fast": 0.005,
        "Waning_Humoral_Rate_Slow": 0,

Mostly slow:
        "Waning_Humoral_Fast_Fraction": 0.1,
        "Waning_Humoral_Rate_Fast": 0.005,
        "Waning_Humoral_Rate_Slow": 0,

Mostly fast:
        "Waning_Humoral_Fast_Fraction": 0.8,
        "Waning_Humoral_Rate_Fast": 0.01,
        "Waning_Humoral_Rate_Slow": 0,


/********************
*** running these ***
********************/
With Eradication.exe in this folder

Eradication.exe -C config_mostly_fast.json -O output_mostly_fast --dll-path ..\..\..\x64\Release > console_mostly_fast.txt

Eradication.exe -C config_mostly_slow.json -O output_mostly_slow --dll-path ..\..\..\x64\Release > console_mostly_slow.txt

Eradication.exe -C config-survey.json -O output --dll-path ..\..\..\x64\Release > console_output.txt

The test is to run the sims with the survey intervention and compare survey output with compare_Nab.py.

/************************************************
*** Comparing default waning to other speeds: ***
************************************************/

compare_Nab.py output\PolioSurveyJSONAnalyzer_Day100_1.json output_mostly_slow\PolioSurveyJSONAnalyzer_Day100_1.json

This should look like default_vs_mostly_fast.png

compare_Nab.py output\PolioSurveyJSONAnalyzer_Day100_1.json output_mostly_slow\PolioSurveyJSONAnalyzer_Day100_1.json

This should look like default_vs_mostly_slow

/****************************************
*** comparing before and after waning ***
****************************************/

compare_Nab.py output\PolioSurveyJSONAnalyzer_Initial_1.json output\PolioSurveyJSONAnalyzer_Day100_1.json

This should look like default_initial_vs_default_day100