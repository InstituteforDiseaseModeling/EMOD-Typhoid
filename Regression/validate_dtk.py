#!/usr/bin/python

"""
=======
Purpose
=======
This purpose of this script is to help you figure out where the runtime output of 
the DTK first varies from the expected output.  When you run this script, you 
pass it a stdout (logfile) that you have generated from the "good" version of the DTK.
Then you configure in code the test executable that is producing different output. 
In theory, for the same config.json, the output should be identical. In that case,
you'll get the output "Matches!". But if not, the script will tell you the first
line of output that varies, with both the reference line and the test line.
You can change the log level and re-run (both).
"""

import subprocess
import sys
import re


if len( sys.argv ) == 1:
    print( "Usage: " + sys.executable + " <reference_stdout.txt>" )

ref = open( sys.argv[1] )

num_failures_allowed = 1
print( "Allowing " + str(num_failures_allowed) + " failures before exit. To change, configure in code." )

show_progress = True
print( "Showing progress (i.e., Timestep lines). To change, configure in code." )

exec_string = "../../../Eradication/x64/Release/Eradication.exe --config config.json -I ../input/"
print( "Using '" + exec_string + "' as test executable string. To change, configure in code." )
p = subprocess.Popen( exec_string.split(), shell=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE )
for line in iter(p.stdout.readline, b''):
    if line.startswith( "Intellectual Ventures(R)" ) or re.search( "Using .* path", line ):
        ref.readline()
        continue

    # Might want to ignore specific things here
    if "SetContextTo" in line or "AddRelationship" in line:
        ref.readline()
        continue

    if "Time:" in line and show_progress:
        print( line )

    next_ref_line_raw = ref.readline()
    next_ref_line = re.sub( "..:..:..", "NA", next_ref_line_raw.rstrip() )

    proc_line = re.sub( "..:..:..", "NA", line.rstrip() )
    if proc_line != next_ref_line:
        print line + "DIFFERS FROM\n" + next_ref_line_raw
        num_failures_allowed -= 1
        if num_failures_allowed == 0:
            sys.exit(0)

print( "Matches!" )
