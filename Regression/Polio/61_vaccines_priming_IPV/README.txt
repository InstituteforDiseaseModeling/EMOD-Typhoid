Priming tests for IPV, for ERAD-1502

You'll need to have the PolioSurveyReporter to do this

> Eradication.exe -C config.json -O output --dll-path ..\..\..\x64\Release

> processpatient-IPV.py output\PolioSurveyJSONAnalyzer_survey_1_9.json

> Eradication.exe -C config_high.json -O output_high --dll-path ..\..\..\x64\Release

> processpatient-IPV.py output_high\PolioSurveyJSONAnalyzer_survey_1_9.json high

> Eradication.exe -C config_low.json -O output_low --dll-path ..\..\..\x64\Release

> processpatient-IPV.py output_low\PolioSurveyJSONAnalyzer_survey_1_9.json low

