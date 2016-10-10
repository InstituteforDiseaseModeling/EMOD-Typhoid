#!/usr/bin/python

import sys
sys.path.append( "./build/lib.linux-x86_64-2.7" )
import dtk_mathfunc

print( "FIXED" )
print( str( dtk_mathfunc.get_fixed_draw() ) )

print( "UNIFORM" )
for i in range( 0,100 ):
    print( str( dtk_mathfunc.get_uniform_draw( 0, 100 ) ) )

print( "GAUSSIAN" )
for i in range( 0,100 ):
    print( str( dtk_mathfunc.get_gaussian_draw( 50, 10 ) ) )

print( "EXPONENTIAL" )
for i in range( 0,100 ):
    print( str( dtk_mathfunc.get_exponential_draw( 100 ) ) )

print( "POISSON" )
for i in range( 0,100 ):
    print( str( dtk_mathfunc.get_poisson_draw( 100 ) ) )

print( "LOG_NORMAL" )
for i in range( 0,100 ):
    print( str( dtk_mathfunc.get_lognormal_draw( 50, 10 ) ) )

print( "BIMODAL" )
for i in range( 0,100 ):
    print( str( dtk_mathfunc.get_bimodal_draw( 0.5, 10 ) ) )

print( "WEIBULL" )
for i in range( 0,100 ):
    print( str( dtk_mathfunc.get_weibull_draw( 50, 10 ) ) )
