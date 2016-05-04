#!/usr/bin/python

import pdb
import math
import numpy.random
import csv
import os
import sys
import settings
import json
import random

print( "\nDTK_TYPHOID_INDIVIDUAL\n" )
population = {}
# reporting variables
global SimDay
SimDay = 1

tracking_sample = [] # holds list of ids of people we decide to track and report on
tracking_sample_rate = 0 # sampling rate for tracking/reporting: 0.1 means track 10% of population
tracking_report = {} # the report itself: a map of ids to lists of states, 1 per timestep
"""
Our IndividualTyphoid class goes here. It should match up to the C++ functions that it mirrors
"""

#if os.name == "nt":
csv_path = os.path.join( settings.input_path, "OutBase.csv" ) # works on linux
#else:
#csv_path = "\\"+settings.input_path.replace("\\", "/") + "/OutBase.csv"

print(settings.input_path)
print(csv_path)
if os.path.exists( csv_path ) == False:
    print( "Could not find " + csv_path )
    sys.exit()

typhoid = csv.DictReader(open( csv_path ))
numpy.random.seed(20)
random.seed(20)
agechronicfemale = [0]*101
agechronicmale = [0]*101




#TESTING SWITCHES FOR CHRONIC CARRIERS
chronic= 'Ames' #or 'Ames'
#if chronic == 'Saul':
#    mult = 1
#elif chronic == 'Ames':
#    mult = 0


#with open('OutBase.csv', 'r') as typhoid:
for line in typhoid:
    agef = int(line['Age'])
    probf = float(line['Prob lifelong female chronic infection CTF'])
    agechronicfemale[agef] = probf
    agem = int(line['Age'])
    probm = float(line['Prob lifelong male chronic infection CTF'])
    agechronicmale[agem] = probm 

def rand2choice(prob):
    if random.random() < prob:
        return 1
    else:
        return 0

class PyIndividual:
    create_call_count = 0
    del_call_count = 0
    update_call_count = 0
    expose_call_count = 0
    ai_call_count = 0
    gibr_call_count = 0
    def __init__(self, new_id_in, new_mcw_in, new_age_in, new_sex_in ):
        self._id = new_id_in
        self._mcw = new_mcw_in
        self._age = new_age_in
        self._sex = new_sex_in
        self.infectious_timer = -100
        self.last_state_reported = ''

    def __del__(self):
        pass

    def age( self, dt ):
        self._age += dt

    def ageInYearsAsInt(self):
        return int((round(self._age/365)))

    def AcquireInfection(self):
        self.infectious_timer = 30

    def Update( self, dt ):
        #self._age += dt
        state_to_report="S"
        if self.infectious_timer > -100:
            self.infectious_timer = self.infectious_timer - dt
            state_to_report="A"
        state_to_report_str = state_to_report
        if self.last_state_reported == state_to_report:
            state_to_report_str += ":EXISTING"
        else:
            state_to_report_str += ":NEW"
        return state_to_report_str 

    def Expose( self, contagion_population, dt, route ):
        infected = 0
        draw = random.random()
        if draw < 0.0015:
            #print( "New infection!" )
            infected = 1
        return infected

    def GetInfectiousnessByRoute( self, route ):
        # NOTE: route is contact or environmental
        deposit = 0
        if self.infectious_timer > 0:
            deposit = 1
        return deposit

class ToyphoidIndividual(PyIndividual):
    def __init__(self, new_id_in, new_mcw_in, new_age_in, new_sex_in ):
        PyIndividual.__init__( self, new_id_in, new_mcw_in, new_age_in, new_sex_in )

class TyphoidIndividual(PyIndividual):
    # static config values ala config.json
    # note that this doesn't line up with anything in the C++ code
    #_acute_infectivity = 4000
    #_prepatent_infectivity = 3000
    #_subclinical_infectivity = 2000
    #_chronic_infectivity = 1000

    #What percent of individuals drink contaminated water?
    #exposure = 0.25
    N50 = 1110000
    alpha = 0.175



    # shifting probabilities
    P1 = 0.1111 #probability that an infection becomes clinical
    # P2 is age dependent so is determined below. Probability of becoming a chronic carrier from a SUBCLINICAL infection
    P3 = 0
    # P3 is age dependent so is determined below. Probability of becoming a chronic carrier from a CLINICAL infection
    P5 = 0.05 #probability of typhoid death
    P6 = 0 #probability of sterile immunity after acute infection
    P7 = 00 #probability of clinical immunity after acute infection
    P8 = 00 #probability of sterile immunity from a subclinical infectin in the clinically immune
    P9 = 0 #probability of sterile immunity from a subclinical infection
    P10 = 0 #probability of clinical immunity from a subclinical infection

    #TREATMENT
    acute_symptoms_day = 5
    #PST = 0.9 #probability of successful treatment: note that santiago does not have MDR strains at this point
    CFRU = 0.1
    CFRH = 0.005
    #P5 is probability of typhoid death
    #The rest remain infected and have a high chance of dying

    #Trying a different mechanism for immunity
    #inf_protection = 0.1

    # Environmental amplification
    #amp = 15

    # death marker
    death = 0

        # Incubation period by transmission route (taken from Glynn's dose response analysis) assuming low dose for environmental
    mpe = 2.23
    spe = 0.05192995

    mpf = math.log(4.7)
    spf = 0.0696814

    #Subclinical infectious parameters: mean and standard deviation under and over 30
    mso30=3.430830
    sso30=0.922945
    msu30=3.1692211
    ssu30=0.5385523

    #Acute infectious parameters: mean and standard deviation under and over 30
    mao30=3.430830
    sao30=0.922945
    mau30=3.1692211
    sau30=0.5385523

#Right now I am assuming that subclinical and acute shed for the same but this is only based on acute data

    def __init__(self, new_id_in, new_mcw_in, new_age_in, new_sex_in ):
        PyIndividual.__init__( self, new_id_in, new_mcw_in, new_age_in, new_sex_in )
        self._id = new_id_in
        self._mcw = new_mcw_in
        self._age = new_age_in
        self._sex = new_sex_in
        self._route = "NA"
        self._infection_count = 0
        # trivial infection model
        self.acute_timer = -100
        self.prepatent_timer=-100
        self.subclinical_timer=-100
        self.chronic_timer=-100
        self.clinical_immunity_timer=-100
        self.sterile_immunity_timer=-100
        # trivial susceptibility model
        self.clinical_immunity = False
        self.sterile_immunity = False
        self.Ames_Chronic = 0

        self.treat = 0.9

        #self._acute_duration = int(numpy.random.lognormal(mean=math.log((self.ma**2)/float(math.sqrt((self.sa**2)+self.ma**2))), sigma=math.sqrt(math.log((self.sa**2)/float((self.ma**2))+1))))
        # this is taken from AMES 1943
        #self._subclinical_duration = int(numpy.random.lognormal(mean= math.log((self.ms**2)/float(math.sqrt((self.ss**2)+self.ms**2))), sigma=math.sqrt(math.log((self.ss**2)/float((self.ms**2))+1))))
        self._chronic_duration= 100000000
        self._clinical_immunity_duration = 160*30
        self._sterile_immunity_duration = 800*30

        if numpy.random.random() < tracking_sample_rate:
            # Let's track this person for reporting
            tracking_sample.append( self._id )
            tracking_report[ self._id ] = []

    #def __del__(self):
    #   if self._id in tracking_sample:
    #        #Dump this person's state to console
    #        print( "Trajectory for individual " + str( self._id ) + ": " + str( tracking_report[ self._id ] ) )
            #print (self._infection_count)

    def AcquireInfection(self):
        if self._route == "TRANSMISSIONROUTE_ENVIRONMENTAL":
            self._prepatent_duration = int(numpy.random.lognormal(mean= self.mpe, sigma= self.spe))
        else:
            self._prepatent_duration = int(numpy.random.lognormal(mean= self.mpf, sigma= self.spf))
        self._infection_count += 1
        self.prepatent_timer=self._prepatent_duration 
        #print self.prepatent_timer
        #print( str(self._id) + " got infected." )
        #print( "_prepatent_duration  = " + str( self._prepatent_duration ) )

    def processPrePatent( self, dt ):
        #print( "PREPATENT!!!!" )
        state_to_report='P'
        self.prepatent_timer -= dt
        if -100 < self.prepatent_timer <= 0:
            self.prepatent_timer=-100
            #array=[1,0]
            if self.clinical_immunity == 1:
                if self.ageInYearsAsInt() < 30:
                    self._subclinical_duration = int(numpy.random.lognormal(mean= self.msu30, sigma=self.ssu30))
                elif self.ageInYearsAsInt() >= 30:
                    self._subclinical_duration = int(numpy.random.lognormal(mean= self.mso30, sigma=self.sso30))
                self.subclinical_timer = self._subclinical_duration
                if self._subclinical_duration > 365:
                    self.Ames_Chronic = 1
            elif self.clinical_immunity == 0:
                shift=rand2choice(self.P1)
                if shift==1:
                    if self.ageInYearsAsInt() < 30:
                        self._acute_duration = int(numpy.random.lognormal(mean=self.mau30, sigma=self.sau30))
                    elif self.ageInYearsAsInt() >= 30:
                        self._acute_duration = int(numpy.random.lognormal(mean=self.mao30, sigma=self.sao30))
                    self.acute_timer = self._acute_duration
                    if self._acute_duration > 365:
                        self.Ames_Chronic = 1
                else:
                    if self.ageInYearsAsInt() < 30:
                        self._subclinical_duration = int(numpy.random.lognormal(mean= self.msu30, sigma=self.ssu30))
                    elif self.ageInYearsAsInt() >= 30:
                        self._subclinical_duration = int(numpy.random.lognormal(mean= self.mso30, sigma=self.sso30))
                    self.subclinical_timer = self._subclinical_duration
                    if self._subclinical_duration > 365:
                        self.Ames_Chronic = 1
        return state_to_report

    def processSubClinical( self, dt ):
        #print ("SUBCLINICALLLLLLL!!!!!!!!!!")
        state_to_report='B'
        self.subclinical_timer -= dt
        if -100 < self.subclinical_timer <= 0:
            #array = [1, 2]
            p2 = None
            ageInYrs = self.ageInYearsAsInt()
            if chronic == 'Saul':
                if self._sex == 'FEMALE':
                    p2 = agechronicfemale[ ageInYrs ]*0.05 #probability of infection turning chronic from subclinical
                elif self._sex =='MALE':
                    p2 = agechronicmale[ ageInYrs ]*0.05
                else:
                    p2 = 0
            elif chronic == 'Ames':
                if self.Ames_Chronic == 1:
                    p2 = 1
                elif self.Ames_Chronic == 0:
                    p2 = 0
            shift = rand2choice(p2)
            #print( "shift = {0} from prob = {1}." ).format( shift, p2 )
            if shift == 1:
                self.chronic_timer = self._chronic_duration
            elif shift == 0: # shift individuals who recovered into immunity states
                if self.clinical_immunity: #because probabilities differ depending on whether the individual is clinically immune
                    #shift = numpy.random.choice(array, p=[self.P8, 1-self.P8]) #probability of sterile immunity
                    shift = rand2choice(self.P8)
                    if shift == 1 :
                        self.sterile_immunity = True
                        self.sterile_immunity_timer = self._sterile_immunity_duration
                        self.clinical_immunity = False
                        self.clinical_immunity_timer = -100
                    elif shift == 0:
                        self.clinical_immunity_timer += self._clinical_immunity_duration
                else: #probabilities for the non immune class
                    shift = rand2choice(self.P9)
                    #shift = numpy.random.choice(array, p=[self.P9, 1-self.P9])
                    if shift == 1:
                        self.sterile_immunity = True
                        self.sterile_immunity_timer = self._sterile_immunity_duration
                    elif shift == 0:
                        shift = rand2choice(self.P10)
                        #shift = numpy.random.choice(array, p= [self.P10, 1-self.P10])
                        if shift ==1:
                            self.clinical_immunity = True
                            self.clinical_immunity_timer = self._clinical_immunity_duration
            self.subclinical_timer = -100
        return state_to_report

    def processAcute( self, dt ):
        #print( "Individual ACUTE,update timer(s)" )
        state_to_report = 'A'
        self.acute_timer -= dt
        #print(self.acute_timer)
        if self._acute_duration - self.acute_timer <= self.acute_symptoms_day:
            #array = [1,2]
            shift = rand2choice(self.treat)
            #shift = numpy.random.choice(array, p=[self.treat, 1-self.treat])
            if shift == 1:
                #if they don't die and seek treatment, we are assuming they recover
                shift = rand2choice(self.CFRH)
                #shift = numpy.random.choice(array, p=[self.CFRH, 1-self.CFRH])
                if shift == 1:
                    self.death = 1
                    self.acute_timer = -100
                elif shift == 0:
                    self.acute_timer = -100
            #elif shift == 2:

            #if they dont seek treatment, just keep them infectious until the end of their acute stage


        #Assume all acute individuals seek treatment since this is Mike's "reported" fraction
            #Some fraction are treated effectively, some become chronic carriers, some die, some remain infectious

            #assume all other risks are dependent on unsuccesful treatment


            #print(self.P3)
            #print( self.ageInYearsAsInt() )
            #print(self._sex)

        if self.acute_timer <= 0:
            #print ("END OF THE ROAAADDDDD!!!!!")
            #array = [1, 2, 3]
            #if self._sex == "FEMALE":
            #    self.P3 = agechronicfemale[self.ageInYearsAsInt()]
            #elif self._sex == "MALE":
            #    self.P3 = agechronicmale[self.ageInYearsAsInt()]
            #print(self.P3)
            #print( self.ageInYearsAsInt() )
            #print(self._sex)
            #shift = numpy.random.choice(array, p=[self.P3, self.P5, 1-self.P3-self.P5])
            #if shift == 1:
            #    self.chronic_timer = self._chronic_duration
            #elif shift == 2:
                #Add typhoid death
            #    self.death = 1
            #elif shift == 3:


            #array = [1,2]
            #shift = numpy.random.choice(array, p=[self.CFRU, 1-self.CFRU])
            shift = rand2choice(self.CFRU)
            if shift == 1:
                self.death = 1
                self.acute_timer = -100
            elif shift == 0: #if they survived, calculate probability of being a carrier
                if chronic == 'Saul':
                    if self._sex == "FEMALE":
                        self.P3 = agechronicfemale[self.ageInYearsAsInt()]
                    elif self._sex == "MALE":
                        self.P3 = agechronicmale[self.ageInYearsAsInt()]
                elif chronic == 'Ames':
                    if self.Ames_Chronic == 1:
                        self.P3 = 1
                    elif self.Ames_Chronic == 0:
                        self.P3 = 0
                shift = rand2choice(self.P3)
                #shift = numpy.random.choice(array, p=[self.P3, 1-self.P3])
                if shift == 1:
                    self.chronic_timer = self._chronic_duration
                    self.acute_timer = -100
                if shift == 0: #for other recovereds, calculate probability of becoming immune
                    #array = [1, 2]
                    #shift = numpy.random.choice(array, p = [self.P6, 1-self.P6])
                    shift = rand2choice(self.P6)
                    if shift == 1:
                        self.sterile_immunity = True
                        self.sterile_immunity_timer = self._sterile_immunity_duration
                    elif shift == 0:
                        #array = [1, 2]
                        #shift = numpy.random.choice(array, p = [self.P7, 1-self.P7])
                        shift = rand2choice(self.P7)
                        if shift == 1:
                            self.clinical_immunity = True
                            self.clinical_immunity_timer = self._clinical_immunity_duration
                    self.acute_timer = -100
        return state_to_report


    def Update( self, dt ):
        
        #self.age( dt )

        state_to_report='S'
        if self.prepatent_timer > -100:
            state_to_report = self.processPrePatent( dt )
            #print( "processPrePatent returned " + state_to_report );

        if self.subclinical_timer > -100:
            state_to_report = self.processSubClinical( dt )
            #print( "processSubClinical returned " + state_to_report );

        if self.acute_timer > -100:
            state_to_report = self.processAcute( dt )
            #print( "processAcute returned " + state_to_report );

        if self.chronic_timer > -100:
            #print ("BEWARE CHRONIC CARRIER!!!")
            state_to_report='C'
            self.chronic_timer -= dt
            #print(self.chronic_timer)

        if self.sterile_immunity == True:
            #print( "Sterile immune." )
            state_to_report='I'
            self.sterile_immunity_timer -= dt
            if -100 < self.sterile_immunity_timer <=0 :
                self.sterile_immunity = False
                self.sterile_immunity_timer = -100
                self.clinical_immunity = True
                self.clinical_immunity_timer = self._clinical_immunity_duration

        if self.clinical_immunity == True and self.subclinical_timer ==-100 and self.prepatent_timer ==-100:
            #print( "Clinically immune." )
            state_to_report='J'
            self.clinical_immunity_timer -= dt
            if -100 < self.clinical_immunity_timer <= 0:
                self.clinical_immunity = False
                self.clinical_immunity_timer = -100

        if self._id in tracking_sample:
            # store this individuals state in report
            tracking_report[ self._id ].append( state_to_report )

        #state_to_report_str = state_to_report
        if self.last_state_reported == state_to_report:
            #state_to_report_str += ":EXISTING"
            state_changed = False
        else:
            state_changed = True

        self.last_state_reported = state_to_report
        return state_to_report, state_changed

    def Expose( self, contagion_population, dt, route ):
        #return 

        # NOTE: route is TRANSMISSIONROUTE_CONTACT or TRANSMISSIONROUTE_ENVIRONMENTAL
        #print( str(self.__id) + " has chance of being infected." )
        # if random_number_draw < contagion_population * dt * individual_modifiers_based_on_immunity_and_ interventions
        # call AcquireInfection
        if self.sterile_immunity:
            return 0
        if self.chronic_timer>-100:
            return 0
        if self.subclinical_timer>-100:
            return 0
        if self.acute_timer>-100:
            return 0
        if self.prepatent_timer>-100:
            return 0
        if self.ageInYearsAsInt()<1:
            return 0
        pdb.set_trace()
        if route == 0:
            array = [0, 1]
            #exposed = numpy.random.choice(array, p = [1-self.exposure, self.exposure])
            #exposed = rand2choice(self.exposure)

            #if exposed == 0:
                #return 0
            #elif exposed == 1:
            if True:
                draw = numpy.random.random()
                day_of_year = (SimDay%365)
                if day_of_year > 274 | day_of_year < 91:
                    environment = contagion_population
                else:
                    environment = contagion_population * 0.2

                exposure = environment * self.amp
                infects = (1-(1 + exposure * (2**(1/self.alpha)-1)/self.N50)**-self.alpha)#DOES THIS INFECT INFECTED PEOPLE? ##took out dt
                #print(contagion_population)
                immunity= max(0.01, 1-(self._infection_count*self.inf_protection)) #change this later to distribution
                prob = min(1, (1-(1-(immunity * infects))**dt))
                #prob = 1-((1-0.1)**dt)

                #print(prob)
                #add probability reduction according to previous infection. 10% protection per infection. or clincial?
                if draw < prob:
                    #print( "Individual " + str(self._id) + " infected over route " + route )
                    self._route = route
                    return 1
                    #print (contagion_population)
                else:
                    return 0
        else:
            return 0

        #if route == "TRANSMISSIONROUTE_CONTACT"



    def GetInfectiousnessByRoute( self, route ):
        # NOTE: route is contact or environmental
        deposit = 0
        #if route == "contact":
            #self._route = "contact"
            #return 0

        if self.acute_timer >= 0 :
            deposit = self._acute_infectivity
            #print(deposit)
        elif self.prepatent_timer >=0:
            deposit = self._prepatent_infectivity
            #print(deposit)
        elif self.subclinical_timer >=0:
            deposit = self._subclinical_infectivity
            #print(deposit)
        elif self.chronic_timer >=0:
            deposit = self._chronic_infectivity
            #print(deposit)
        else:
            deposit = 0
        #print(deposit)
        return deposit

    def GetStateChange( self ):
        # None                  = 0,
        # DiedFromNaturalCauses = 1,
        # KilledByInfection     = 2,
        # KilledByCoinfection   = 3,
        # Migrating             = 10
        if self.death == 1:
            return 2

"""
This is the 'shim' layer between the C++ and Python code. It connects by individual id.
"""
def create( new_id, new_mcw, new_age, new_sex ):
    #print( "py: creating new individual: " + str(new_id) )
    new_id_int = int(new_id)
    new_mcw_flt = float(new_mcw)
    new_age_flt = float(new_age)
    population[new_id_int] = TyphoidIndividual( new_id_int, new_mcw_flt, new_age_flt, new_sex )
    PyIndividual.create_call_count += 1

def destroy( dead_id ):
    #pdb.set_trace()
    #print( "py: destroying individual: " + str(dead_id) )
    del population[ int(dead_id) ]
    PyIndividual.del_call_count += 1
    #print( "updates = " + str( PyIndividual.update_call_count ) )
    #print( "exposes = " + str( PyIndividual.expose_call_count ) )
    #print( "get_inf = " + str( PyIndividual.gibr_call_count ) )
    #print( "ai = " + str( PyIndividual.ai_call_count ) )

def update( update_id, dt ):
    #print( "py: updating individual: " + str(update_id) )
    PyIndividual.update_call_count += 1
    return population[int(update_id)].Update( float(dt) )

def update_and_return_infectiousness( update_id, route ):
    #print( "py: getting individual infectiousness by route for individual " + str(update_id) + " and route " + route )
    PyIndividual.gibr_call_count += 1
    return population[int(update_id)].GetInfectiousnessByRoute( route ) 

def acquire_infection( update_id):
    #print ('py: ACQUIRED')
    PyIndividual.ai_call_count += 1
    population[int(update_id)].AcquireInfection( )
    #return 

def expose( update_id, contagion_population, dt, route ):
    #print ('py: EXPOSE')
    PyIndividual.expose_call_count += 1
    return population[int(update_id)].Expose( float( contagion_population ), float( dt ), route )
    #print (contagion_population)
    #return

def start_timestep():
    #pdb.set_trace()
    global SimDay
    SimDay += TyphoidIndividual._dt
    #print (SimDay)
    #print ('py: BATCH UPDATE AGES')
    for i in population:
        # sucks using completely different way of accessing dt here.
        population[ i ].age( TyphoidIndividual._dt )


#on import
if os.path.exists( "config.json" ) == False:
    print( "Failed to find or open config.json. Exiting." )
    sys.exit()
config_json = json.loads( open( "config.json" ).read() )["parameters"]

for param in ["Typhoid_Acute_Infectivity","Typhoid_Prepatent_Infectivity","Typhoid_Subclinical_Infectivity","Typhoid_Chronic_Infectivity"]:
    if param not in config_json:
        print( "Failed to find key (parameter) " + param + " in config.json." )
        sys.exit()

TyphoidIndividual._acute_infectivity = config_json["Typhoid_Acute_Infectivity"]
TyphoidIndividual._prepatent_infectivity = config_json["Typhoid_Prepatent_Infectivity"]
TyphoidIndividual._subclinical_infectivity = config_json["Typhoid_Subclinical_Infectivity"]
TyphoidIndividual._chronic_infectivity = config_json["Typhoid_Chronic_Infectivity"]
TyphoidIndividual.amp = config_json["Typhoid_Environmental_Amplification"]
TyphoidIndividual.inf_protection = config_json["Typhoid_Protection_Per_Infection"]
#TyphoidIndividual.exposure = config_json["Typhoid_Environmental_Exposure"]
TyphoidIndividual._dt = config_json["Simulation_Timestep"]

