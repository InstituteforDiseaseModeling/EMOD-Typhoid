#!/usr/bin/python
import json

def doit():
    icj = json.loads( open( "output/InsetChart.json" ).read() )

    csv = open( "output/InsetChart.csv", "w" )

# Channel_1_Name, Channel_2_Name, etc.
# channel_1_datum[0], channel_2_datum[0], etc
# channel_1_datum[1], channel_2_datum[1], etc
# channel_2_datum[2], channel_2_datum[2], etc

# foreach channel name...
    for chan in icj["Channels"].keys():
        csv.write( chan )
        csv.write( "," )
    csv.write("\n")

# for each timestep...
    for idx in range( len( icj["Channels"]["Births"]["Data"] ) ):
        # for each channel
        for chan in icj["Channels"].keys():
            datum = icj["Channels"][chan]["Data"][idx]
            csv.write( str(datum) )
            csv.write( "," )
        csv.write("\n")

    csv.close()
