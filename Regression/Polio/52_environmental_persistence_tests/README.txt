the "default" test is the same as 52_Persistence_Rate0p5

Run the rest of these tests like so.

First, copy Eradication.exe into this folder.

Next, run Eradication.exe three times, like so:

Eradication.exe -C 52_Persistence_Rate0p0\config_flat.json -O 52_Persi
stence_Rate0p0\output > 52_Persistence_Rate0p0\console.txt

Eradication.exe -C 52_Persistence_Rate0p5\config_flat-noSurvey.json -O 52_Persi
stence_Rate0p5\output > 52_Persistence_Rate0p5\console.txt

Eradication.exe -C 52_Persistence_Rate1p0\config_flat-noSurvey.json -O 52_Persi
stence_Rate1p0\output > 52_Persistence_Rate1p0\console.txt