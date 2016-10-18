import sys
import os
import syslog
import json
import random
import string

import dtk_typhoidindividual as ti
from cgi import parse_qs, escape

var_to_param_map = {}

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

def convert_param_name_to_var_name( param_name ):
    param_name = param_name.translate(None,string.ascii_lowercase)
    param_name = param_name.replace( '_', '' )
    param_name = param_name.lower()
    return param_name

def application(environ,start_response):
    status='200 OK'
    #output='Hello world'
    #os.makedirs( "hello" )

    #schema = ti.get_schema()
    d = parse_qs( environ["QUERY_STRING"] )
    tsteps = d.get('tsteps', [''])[0]
    get_schema = d.get('schema', [''])[0]
    tai = d.get('tai', [''])[0]
    #os.chdir( "/usr/local/wsgi/scripts/" )
    #config_json = json.loads( open( "ti.json", 'r' ).read() )

    output = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <style>
        html, body { width: 100%; height: 100%; margin: 0; padding: 0; font-family: "Georgia",serif; font-size: 1em; line-height: 2; }
        section { display: inline-block; padding: 2em 3em; box-sizing: content-box; }
        dl { display: table-row; }
        dt, dd { display: table-cell; padding: 2px 4px; font-weight: normal; border-top: 1px dotted silver; }
        dt { padding-right: 2em; }
        dl:hover dt, dl:hover dd { background: #fefe88; }
        dd, input[type=text] { font-family: "Consolas", "Menlo", serif; font-size: 1em; color: crimson; padding: 4px; }
        input[type=submit] { width: 99%; box-sizing: border-box; padding: 6px; }
        em { color: crimson; }
    </style>
</head>
<body>
<main>
    <article>
        <section>
"""

    #output += os.getcwd()
    #output += str( get_schema )
    infection_history = []
    suscept_history = []
    infectiousness_history = []

    if get_schema == '1':
        output += "<pre>" + ti.get_schema() + "</pre>"

    else:
        #output += str( environ["QUERY_STRING"] )
        os.chdir( "/usr/local/wsgi/scripts/" )
        schema = json.loads( ti.get_schema() )
        syslog.syslog( syslog.LOG_INFO, str( schema.keys() ) )

        output += """
            <h2>Typhoid Single-Individual Intrahost Demo</h2>
            <form action="myapp">
"""

        for param in sorted( schema.keys() ):
            if "Infectiousness" in param:
                output += """
                <dl>
                    <dt><label>{0}</label></dt><dd><input type="text" name="{1}" size="8" value="{2}"></dd>
                </dl>
""".format(str(param), convert_param_name_to_var_name( str( param ) ), 
                #(str(schema[param]["default"]), d.get(param, [""])[0])[d.get(param, [""])[0] != ""]
                str(schema[param]["default"])
        )

                #the following *should* work to populate {2} value above (default if not in query string).
                #(str(schema[param]["default"]), d.get(param, [""])[0])[d.get(param, [""])[0] != ""]

        output += """
                <dl>
                    <dt><label>Timesteps</label></dt><dd><input type="text" name="tsteps" size="8" value="{0}"></dd>
                </dl>
                <dl>
                    <dt><em>Set parameters and number of timesteps, then...</em></dt>
                    <dd><input type="submit" value="Run"/></dd>
                </dl>
            </form>
        """.format( 1 if tsteps == '' else tsteps )
        if tsteps != '':

            for param in schema.keys():
                syslog.syslog( syslog.LOG_INFO, param )
                if "Infectiousness" in param:
                    var_name = convert_param_name_to_var_name( str( param ) )
                    var_to_param_map[ var_name ] = param
                    syslog.syslog( syslog.LOG_INFO, var_name + ", " + str( param ) )
                    qs_val = d.get(var_name, [''])[0]
                    syslog.syslog( syslog.LOG_INFO, str( qs_val ) )
                    if qs_val != None:
                        syslog.syslog( syslog.LOG_INFO, "Setting config.json param(s): " + str( param ) + "=" + (qs_val) )
                        ti.set_param( ( param, int(qs_val) ) )
            syslog.syslog( syslog.LOG_INFO, "Creating Typhoid individual." )
            ti.create( 30, 'M' )

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
                ti.update()
                syslog.syslog( syslog.LOG_INFO, "Finished updating individual." )
                ind_json = ti.serialize()
                serial_man = json.loads( ind_json )
                state = serial_man["individual"]["state_to_report"]
                infection_history.append( state )
                suscept_history.append( ti.get_immunity() )
                inf_ness = serial_man["individual"]["infectiousness"]
                infectiousness_history.append( inf_ness )

            ind_json = ti.serialize()
            print( str( ind_json ) )

            output += """
        </section>
        <section>
            <h3>After {0}, the individual has the following attributes:</h3>
""".format(str( tsteps ))

            serial_man = json.loads( ind_json )
            output += """
            <dl>
                <dt>age</dt>
                <dd>{0}</dd>
            </dl>
            <dl>
                <dt>infected status</dt>
                <dd>{1}</dd>
            </dl>
            <dl>
                <dt>number of (historical) infections</dt>
                <dd>{2}</dd>
            </dl>
""".format(str(serial_man["individual"]["m_age"]/365.0), str(serial_man["individual"]["m_is_infected"]), str(serial_man["individual"]["_infection_count"]))

            if serial_man["individual"]["m_is_infected"]:
                output += ( "<dl><dt>infectiousness</dt><dd>" + str(serial_man["individual"]["infectiousness"]) + "</dd></dl>")
                output += ( "<dl><dt>prepatent_timer</dt><dd>" + str(serial_man["individual"]["infections"][0]["prepatent_timer"]) + "</dd></dl>")
                output += ( "<dl><dt>acute_timer</dt><dd>" + str(serial_man["individual"]["infections"][0]["acute_timer"]) + "</dd></dl>")
                output += ( "<dl><dt>subclinical_timer</dt><dd>" + str(serial_man["individual"]["infections"][0]["subclinical_timer"]) + "</dd></dl>")
                output += ( "<dl><dt>chronic_timer</dt><dd>" + str(serial_man["individual"]["infections"][0]["chronic_timer"]) + "</dd></dl>")

            output += """
        </section>
        <section>
            <dl>
                <dt>infection stage history</dt>
                <dd>{0}</dd>
            </dl>
            <dl>
                <dt>infectiousness history</dt>
                <dd>{1}</dd>
            </dl>
            <dl>
                <dt>immunity history</dt>
                <dd>{2}</dd>
            </dl>
""".format(str(infection_history), str(infectiousness_history), str(suscept_history))


    output += """
        </section>
    </article>
</main>
</body>
</html>
"""

    response_headers=[("Content-type","text/html"),
            ("Content-Length",str(len(output)))]
    start_response(status,response_headers)
    return [output]
