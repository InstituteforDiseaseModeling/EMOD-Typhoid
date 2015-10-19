Polio tests based on Kevin McCarthy's work.

Tests permutations on 
- config
- - "Acquire_Rate_Default_Polio" 
- - - in range [1, 1.0e-4, 9.9999999999999995e-08]
- demographics
- - "mucosal_memory_distribution3"["ResultValues"] (and 1 and 2)
- - - in range [0, 6, 11]

Checking in the following as the default
defaultacq1em4_mucosaltiter6/config_flat_defaultacq_1em4_mucosaltiter_6

/**************
*** running ***
**************/

to run this with the polio reporter, the following should work from this folder in svn (note the relative path for the dll-path parameter)

Eradication-v17-Release.exe -C config_flat_survey.json -O output --dll-path ..\..\..\reporters\x64\Release > console.txt

the following examples do not use the reporter

Eradication-v17-Release.exe -C defaultacq1em7_mucosaltiter0\config_flat_defaultacq_1em7_mucosaltiter_0.json -O defaultacq1em7_mucosaltiter0\output > defaultacq1em7_mucosaltiter0\console.txt

Eradication-v17-Release.exe -C defaultacq1em4_mucosaltiter6\config_flat_defaultacq_1em4_mucosaltiter_6.json -O defaultacq1em4_mucosaltiter6\output > defaultacq1em4_mucosaltiter6\console.txt

Eradication-v17-Release.exe -C defaultacq1_mucosaltiter11\config_flat_defaultacq_1_mucosaltiter_11.json -O defaultacq1_mucosaltiter11\output > defaultacq1_mucosaltiter11\console.txt

the following examples do use the reporter

Eradication-v17-Release.exe -C defaultacq1em7_mucosaltiter0\config_flat_defaultacq_1em7_mucosaltiter_0_survey.json -O defaultacq1em7_mucosaltiter0\output_survey --dll-path ..\..\..\reporters\x64\Release > defaultacq1em7_mucosaltiter0\console_survey.txt

Eradication-v17-Release.exe -C defaultacq1em4_mucosaltiter6\config_flat_defaultacq_1em4_mucosaltiter_6_survey.json -O defaultacq1em4_mucosaltiter6\output_survey --dll-path ..\..\..\reporters\x64\Release > defaultacq1em4_mucosaltiter6\console_survey.txt

Eradication-v17-Release.exe -C defaultacq1_mucosaltiter11\config_flat_defaultacq_1_mucosaltiter_11_survey.json -O defaultacq1_mucosaltiter11\output_survey --dll-path ..\..\..\reporters\x64\Release > defaultacq1_mucosaltiter11\console_survey.txt