Vaccines priming tests.  These rests require the PolioSurvey or they won't mean anything.

Eradication.exe -C config.json -O output --dll-path ..\..\..\x64\Release
processpatient-OPV-priming.py output\PolioSurveyJSONAnalyzer_m3_1_402.json default

Eradication.exe -C config_high.json -O output_high --dll-path ..\..\..\x64\Release
processpatient-OPV-priming.py output_high\PolioSurveyJSONAnalyzer_m3_1_402.json high

Eradication.exe -C config_low.json -O output_low --dll-path ..\..\..\x64\Release
processpatient-OPV-priming.py output_low\PolioSurveyJSONAnalyzer_m3_1_402.json low

Script generates "expected" data and plots against real data.


