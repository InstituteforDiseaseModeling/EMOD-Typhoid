#!/usr/bin/python

import os

def doit( data, x_data=None, ylim=None, xlim=None, title=None, xlabel=None, ylabel=None ):
    print( "DEBUG: dtk_plot_wrapper." )
    homepath = None
    if os.environ.has_key( "HOME" ):
	homepath = os.getenv( "HOME" )
    elif os.environ.has_key( "HOMEDRIVE" ):
	homepath = os.path.join( os.getenv( "HOMEDRIVE" ), os.getenv( "HOMEPATH" ) )
    else:
        # HPC case: if none of the env vars are present, we're on the HPC and won't plot anything.
        return

    if( os.path.exists( os.path.join( homepath, ".rt_show.sft" ) ) ):
        import matplotlib.pyplot as plt
        if ylim != None:
            plt.ylim( ylim )
        if title != None:
            plt.title( title )
        if xlabel != None:
            plt.xlabel( xlabel )
        if ylabel != None:
            plt.ylabel( ylabel )
        if x_data == None:
            plt.plot( data )
        else:
            plt.plot( x_data, data )
        plt.show()
