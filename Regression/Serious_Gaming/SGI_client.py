#!/usr/bin/python

import curses
import time
import socket
import json
import operator # for array math

stdscr = curses.initscr()
curses.noecho()
curses.cbreak()
stdscr.keypad(1)

stdscr.addstr(2, 10, "EMOD Serious Gaming Prototype",
                      curses.A_REVERSE)
stdscr.refresh()


HOST = 'ivlabsdvapp61'    # The remote host
PORT = 7778              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
data = s.recv(1024)

state = "GO"

#print 'Received', repr(data)
#stdscr.addstr(20, 10, repr(data), curses.A_REVERSE)
#stdscr.refresh()

time.sleep(1)
s.sendall('RUN')
data = s.recv(1024)
#stdscr.addstr(20, 10, repr(data), curses.A_REVERSE)
#stdscr.refresh()

total_averted = [ 0, 0, 0, 0, 0]

stdscr.nodelay(1)

for i in range(0,100):
    time.sleep(0.25)
    if state == "GO":
        s.sendall('STEP')
        data = s.recv(2000)
        #print( data )
        try:
            data_json = json.loads( data )
            binned_json = json.loads(data_json["Binned"])
            keys = str( binned_json.keys() )
            averted = binned_json["New Clinical Cases Averted"]
            total_averted = map( operator.add, total_averted, averted )
            ts = "Timestep: " + str(data_json["Timestep"])
            stdscr.addstr(18, 10, ts, curses.A_REVERSE)
            stdscr.addstr(20, 10, str(averted), curses.A_REVERSE)
            offset = 0
            for bin_idx in total_averted:
                aver_graph = ""
                offset += 1
                for case in range( 0,bin_idx ):
                    aver_graph += "*"
                stdscr.addstr(4+offset, 20, (aver_graph), curses.A_REVERSE)
            stdscr.refresh()
        except Exception as ex:
            print( "Exception: " + str(ex) + "\n" + data )

    c = stdscr.getch()
    if c== ord('P'):
        state = "PAUSED"
    elif c== ord('G'):
        state = "GO"
    elif c== ord('C'):
        state = "PAUSED"
        # display campaign event input fields
        curses.echo()
        stdscr.nodelay(0)

        new_event_json = json.loads( "{}" )
        new_event_json["Rollout_Distribution"] = "BOX"
        new_event_json["Percentage_Of_Target_Population_Reached"] = 1.0
        new_event_json["Target_Population_Min_Age_In_Years"] = 0
        new_event_json["Target_Population_Max_Age_In_Years"] = 125
        new_event_json["Timesteps_From_Now"] = 1
        new_event_json["Intervention_Quality"] = "High"
        new_event_json["Intervention"] = ""
        new_event_json["Length_Of_Rollout_In_Days"] = 1

        stdscr.addstr(22, 10, "Intervention Type (B=Bednet, S=IRS, D=Drug): ", curses.A_REVERSE)
        iv = stdscr.getch()
        if iv == ord('B') or iv == ord('b'):
            new_event_json["Intervention"] = "BEDNET"
        elif iv == ord('S') or iv == ord('s'):
            new_event_json["Intervention"] = "IRS"
        elif iv == ord('D') or iv == ord('d'):
            new_event_json["Intervention"] = "DRUG"
        new_event_msg = "NEW_EVENT:" + json.dumps( new_event_json )
        s.sendall( new_event_msg )

        stdscr.nodelay(1)
        curses.noecho()
        state = "GO"

s.close()
curses.nocbreak()
stdscr.keypad(0)
curses.echo()

curses.endwin()
