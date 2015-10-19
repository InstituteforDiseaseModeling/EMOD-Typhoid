Maternal decay

Run the sim with the survey on, and then run the python script thusly

plot_single_decay.py output
Virus type: PV1
Averages by month:
[136.35883999999996, 51.017779999999995, 19.087962, 7.141631, 2.6719953, 0.9997094999999998, 0.3740346200000001, 0.13994252, 0.05235855, 0.019589614999999998, 0.007329321000000001, 0.0027422177]
-------------------
Virus type: PV3
Averages by month:
[136.35883999999996, 51.017779999999995, 19.087962, 7.141631, 2.6719953, 0.9997094999999998, 0.3740346200000001, 0.13994252, 0.05235855, 0.019589614999999998, 0.007329321000000001, 0.0027422177]
-------------------
Virus type: PV2
Averages by month:
[136.35883999999996, 51.017779999999995, 19.087962, 7.141631, 2.6719953, 0.9997094999999998, 0.3740346200000001, 0.13994252, 0.05235855, 0.019589614999999998, 0.007329321000000001, 0.0027422177]
-------------------


What you will see are the serum_NAb averages for each 30 day month (we leave off the last 5 days of the year).

You should see them decay.  If you want the tail to be longer, you can change the 
        "Maternal_log2NAb_mean": 3,

to 
        "Maternal_log2NAb_mean": 6,

or whatever.  This is a single-individual simulation.