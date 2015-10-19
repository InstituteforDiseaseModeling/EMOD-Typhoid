#!/usr/bin/python

#import pymongo
import matplotlib.pyplot as plt
import json
import sys
import pylab

def plotCompareFromDisk( first_filename, second_filename, label = "" ):
    ref_sim = open( first_filename )
    ref_data = json.loads( ref_sim.read() )

    test_sim = open( second_filename )
    test_data = json.loads( test_sim.read() )

    num_chans = ref_data["Header"]["Channels"]
    diff_chans = []
    for chan_title in sorted(ref_data["Channels"]):
        if ref_data["Channels"][chan_title]["Data"] != test_data["Channels"][chan_title]["Data"]:
            diff_chans.append(chan_title)

    square_root = 4
    if len(diff_chans) > 30:
        square_root = 6
    elif len(diff_chans) > 16:
        square_root = 5

    idx = 0
    for chan_title in diff_chans:
        idx_x = idx%square_root
        idx_y = int(idx/square_root)

        if chan_title not in test_data["Channels"]:
            continue

        try:
            subplot = plt.subplot2grid( (square_root,square_root), (idx_y,idx_x)  ) 
            subplot.plot( ref_data["Channels"][chan_title]["Data"], 'r-')
            subplot.plot(test_data["Channels"][chan_title]["Data"], 'b-')
            plt.setp( subplot.get_xticklabels(), fontsize='4' )
            plt.title( chan_title, fontsize='6' )
        except Exception as ex:
            print str(ex)
        if idx == (square_root*square_root)-1:
            break

        idx += 1
        
    plt.suptitle( label + " (red=reference, blue=test)")
    plt.subplots_adjust( bottom=0.05 )
    plt.show()

def main():
    #plotOneFromDisk()
    if len( sys.argv ) < 3:
        print( "Usage: plotAllCharts.py <ref_inset_chart_json_path> <test_inset_chart_json_path> <label>" )
        sys.exit(0)
    else:
        plotCompareFromDisk (sys.argv[1], sys.argv[2],  sys.argv[3] )

if __name__ == "__main__":
    main()
