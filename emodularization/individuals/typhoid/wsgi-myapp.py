import sys
import os
import syslog
import json
import random
#import ti

import dtk_typhoidindividual as ti
from cgi import parse_qs, escape

def expose( action, prob ):
    print( "expose: " + action + " with value " + str( prob ) ) 
    
    if random.random() < 0.95:
        if ti.get_immunity() < 1 and ti.is_infected() == False:
            print( "Let's infect based on random draw." )
            return 1
        else:
            return 0
    else:
        print( "Let's NOT infect based on random draw." )
        return 0

ti.my_set_callback( expose )

def application(environ,start_response):
    status='200 OK'
    output='Hello world'
    #os.makedirs( "hello" )

    #schema = ti.get_schema()
    d = parse_qs( environ["QUERY_STRING"] )
    tsteps = d.get('tsteps', [''])[0]
    get_schema = d.get('schema', [''])[0]
    tai = d.get('tai', [''])[0]
    #os.chdir( "/usr/local/wsgi/scripts/" )
    #config_json = json.loads( open( "ti.json", 'r' ).read() )

    output = '<html>'
    #output += os.getcwd()
    #output += str( get_schema )
    infection_history = []
    suscept_history = []
    infectiousness_history = []
    if get_schema == '1':
        output += "<pre>" + ti.get_schema() + "</pre>"
        #output += 'schema will be here...<br>'
    elif tsteps == '':
        schema = ti.get_schema()
        output += "<h2>Typhoid Single-Individual Intrahost Demo</h2>"
        output += "Configure parameters and enter number of timesteps to run:<br><br>"
        output += '<form action="myapp">'
        output += 'Acute Infectiousness: <input type="text" name="tai" value="4000"><br>'
        #for param in schema.keys():
            #output += param + ': <input type="text" name="' + tbd + '" value="' + schema[param]["default"] + '"><br>'
        output += 'Timesteps: <input type="text" name="tsteps" value="1"><br><br>\
                   <input type="submit" value="Submit"><hr>\
                   </form>'
        output += '<form action="myapp" Schema:<br><input type="hidden" name="schema" value="1"><input type="submit" value="schema"></form>'
    else:
        #output += str( environ["QUERY_STRING"] )
        os.chdir( "/usr/local/wsgi/scripts/" )
        syslog.syslog( syslog.LOG_INFO, "Setting config.json param(s): Typhoid_Acute_Infectiousness=" + tai ) 
        ti.set_param( ( "Typhoid_Acute_Infectiousness", int(tai) ) )
        syslog.syslog( syslog.LOG_INFO, "Creating Typhoid individual." )
        ti.create()

        #with open( "ti.json", 'w' ) as ti_json:
        #    ti_json.write( json.dumps( config_json, indent=4, sort_keys=True ) )

        #f = open( sim_dir + "/config.json", 'w' )
        #f.write( json.dumps( reply_json, sort_keys=True, indent=4 ) )
        #f.close()

        tsteps = int( tsteps )
        syslog.syslog( syslog.LOG_INFO, "tsteps = " + str(tsteps) + " of type " + str(type(tsteps)) )
        for tstep in xrange( tsteps ):
            syslog.syslog( syslog.LOG_INFO, "Updating Typhoid individual for timestep: " + str( tstep ) )
            syslog.syslog( syslog.LOG_INFO, "Updating individual, age = {0}, infected = {1}, immunity = {2}.".format( ti.get_age(), ti.is_infected(), ti.get_immunity() ) )
            ti.update( 1 )
            syslog.syslog( syslog.LOG_INFO, "Finished updating individual." )
            ind_json = ti.serialize()
            serial_man = json.loads( ind_json )
            if serial_man["individual"]["m_is_infected"]:
                pp = serial_man["individual"]["infections"][0]["prepatent_timer"]
                ac = serial_man["individual"]["infections"][0]["acute_timer"]
                sc = serial_man["individual"]["infections"][0]["subclinical_timer"]
                ch = serial_man["individual"]["infections"][0]["chronic_timer"]
                if pp > -2:
                    infection_history.append( 'P' )
                elif ac > -2:
                    infection_history.append( 'A' )
                elif sc > -2:
                    infection_history.append( 'S' )
                elif ch > -2:
                    infection_history.append( 'C' )
            else:
                infection_history.append( 'U' )
            suscept_history.append( ti.get_immunity() )
            inf_ness = serial_man["individual"]["infectiousness"]
            infectiousness_history.append( inf_ness )

        ind_json = ti.serialize()
        print( str( ind_json ) )
        output += "<br>"
        output += "After " + str( tsteps ) + " timesteps, the individual has the following attributes:<br/>"
        serial_man = json.loads( ind_json )
        output += ( "<br>age=" + str(serial_man["individual"]["m_age"]/365.0) )
        output += ( "<br>infected status=" + str(serial_man["individual"]["m_is_infected"]) )
        output += ( "<br>number of (historical) infections=" + str(serial_man["individual"]["_infection_count"]) )
        if serial_man["individual"]["m_is_infected"]:
            output += ( "<br>infectiousness=" + str(serial_man["individual"]["infectiousness"]) )
            output += ( "<br>prepatent_timer=" + str(serial_man["individual"]["infections"][0]["prepatent_timer"]) )
            output += ( "<br>acute_timer=" + str(serial_man["individual"]["infections"][0]["acute_timer"]) )
            output += ( "<br>subclinical_timer=" + str(serial_man["individual"]["infections"][0]["subclinical_timer"]) )
            output += ( "<br>chronic_timer=" + str(serial_man["individual"]["infections"][0]["chronic_timer"]) )
        output += ( "<br>infection stage history=" + str(infection_history) )
        output += ( "<br>infectiousness history=" + str(infectiousness_history) )
        output += ( "<br>immunity history=" + str(suscept_history) )
        #output += str( ind_json )
        output += "<br>"
    output += '</html>'

    response_headers=[('Content-type','text/html'),
            ('Content-Length',str(len(output)))]
    start_response(status,response_headers)
    return [output]
