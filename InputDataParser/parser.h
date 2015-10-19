/***************************************************************************************************

Copyright (c) 2015 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <math.h>
#include "geodistance.h"

//basic community grid
class Community
{
public:
    unsigned long int id;
    double lat;
    double lon;
    double inpop; //inpop is the original population, in double format
    unsigned long int pop; //pop is the population in unsigned long int format
    int roadhub; //-1 if the community is not the road hub, otherwise put the settlement ID
    int airhub;  //-1 if the community is not the air hub, othersise put the settlement ID
    int voronoi_air; //put the assigned closest airport ID here, I didn't change the road migration network so for now there's only air available.
    double voronoi_dist; //used in when deciding fraction of links inside one basin
};

//main settlements in CIESIN, some of them will be road hub
class Settlement
{
public:
    std::string name;
    double lat;
    double lon;
    double pop;
    double traffic;
    double normalize; //used to normalize the distance between all communities
    //which ID it associates to
    unsigned long int comm_ID;
    int comm_row;
    int comm_col;
    double comm_lat;
    double comm_lon;
    int roadhub;
    int airhub;
};

//weather types
class Climate
{
public:
    double lat;
    double lon;
    int type;
};

//altitude files
class Altitude
{
public:
    double altitude;
    double altcounter;
};


//void ReadInFilesForDemo(char* =NULL, char* =NULL, char* =NULL, char* =NULL, char* =NULL, char* =NULL);
void ReadInFilesForDemo(char* =NULL, char* =NULL, char* =NULL, char* =NULL, char* =NULL, char* =NULL, char* =NULL, char* =NULL, char* =NULL);
void CreateUrbanExtent(char* =NULL, char* =NULL, char* =NULL);
void ConvertRegionToBin(char* =NULL);
double PDFNormalDist(double mu, double sigma, double x);

//void ReadInFilesForDemo(char* popinput, char* settlementinput, char* linksinput, char* climateinput, char* altitudeinput, char* urbanruralinput, char* start)
void ReadInFilesForDemo(char* popinput, char* settlementinput, char* linksinput, char* airportinput, char* airlinksinput, char* climateinput, char* altitudeinput, char* demographicinput, char* start)
{
    //define containers for community, road hubs and airports
    std::vector<std::vector<Community> > comm;
    std::vector<Settlement> region;
    std::vector<Settlement> airport;

    //define altitude and climate containers
    std::vector<Climate> climate;
    std::vector< std::vector<Altitude> > alt;

    //define link matrices
    std::vector<std::vector<double> > LinksMatrix;
    std::vector<int> sett_link_counter;
    std::map<unsigned long int,std::vector<unsigned long int> > reglink_id;
    std::map<unsigned long int,std::vector<double> > reglink_traffic;
    std::vector<std::vector<unsigned long int> > airlink_id;
    std::vector<std::vector<double> > airlink_traffic;

    std::ifstream settlefile;
    std::ifstream popfile;
    std::ifstream linksfile;
    std::ifstream airportfile;
    std::ifstream airlinksfile;
    std::ifstream climatefile;
    std::ifstream altitudefile;
    std::ifstream demoinfile;

    std::ofstream demofile;
//  std::ofstream aircommfile;
    std::ofstream loadbalancefile;
    std::ofstream localmigfile;
    std::ofstream regionmigfile;
    std::ofstream airmigfile;
    std::ofstream climate2binfile;

    settlefile.open(settlementinput);
    airportfile.open(airportinput);
    popfile.open(popinput);
    climatefile.open(climateinput);
    altitudefile.open(altitudeinput);
    demoinfile.open(demographicinput);

    char demoname[60];
//  char aircommname[60];
    char loadname[60];
    char localmigname[60];
    char regionname[60];
    char airmigname[60];
    char climatename[60];

    strcpy_s(demoname,start); 
//  strcpy_s(aircommname,start);
    strcpy_s(loadname,start); 
    strcpy_s(localmigname,start); 
    strcpy_s(regionname,start);
    strcpy_s(airmigname,start);
    strcpy_s(climatename,start); 

    strcat_s(demoname, "demo.dat");
//  strcat_s(aircommname, "airport_community.dat");
    strcat_s(loadname, "loadbalancing_comm.dat");
    strcat_s(localmigname, "localmig_bin.dat");
    strcat_s(regionname, "region_bin.dat");
    strcat_s(airmigname, "airmig_bin.dat");
    //strcat_s(regionname, "region.dat");
    strcat_s(climatename, "climate2_bin.dat");

    demofile.open(demoname);
//  aircommfile.open(aircommname);
    loadbalancefile.open(loadname, std::ios::binary);
    localmigfile.open(localmigname, std::ios::binary);
    regionmigfile.open(regionname, std::ios::binary);
    airmigfile.open(airmigname, std::ios::binary);
    //regionmigfile.open(regionname);
    climate2binfile.open(climatename, std::ios::binary);

    //make sure that input files opened properly
    if(settlefile.fail()){
        std::cout<<"ERROR: Settlement file open failed"<<std::endl;
        goto skip;
    }
    if(airportfile.fail()){
        std::cout<<"ERROR: Airport file open failed"<<std::endl;
        goto skip;
    }
    if(popfile.fail()){
        std::cout<<"ERROR: Population file open failed"<<std::endl;
        goto skip;
    }
    if(climatefile.fail()){
        std::cout<<"ERROR: Climate file open failed"<<std::endl;
        goto skip;
    }
    if(!altitudefile){
        std::cout<<"ERROR: Altitude input failed"<<std::endl;
        goto skip;
    }
    if(!demoinfile){
        std::cout<<"ERROR: Demographic details input failed - default values for all communities"<<std::endl;
    }

    std::cout<<"Opened all files successfully."<<std::endl<<std::endl;

    //Variables used multiple times
    int l=0;
    int k=0;
    int i=0;
    int j=0;
    int m=0;
    double dX=0; //distance in km of latitude
    double dY=0; //distance in km of longitude
    double dist=MAX_DIST;
    double tempval;
    char temp;

    l=0;
    while(!settlefile.eof() && !settlefile.fail()){
        Settlement temp_set;
        //LinksMatrix[l]=new double[MAX_SETTLEMENTS];
        settlefile>>temp_set.name;
        settlefile>>temp_set.lat;
        settlefile>>temp_set.lon;
        settlefile>>temp_set.pop;
        temp_set.traffic=0;
        temp_set.normalize=0;
        if (temp_set.lat!=0 && temp_set.lon!=0)
        {
            region.push_back(temp_set);
        }
    }
    int SettleNum=(int)region.size();

    for(i=0; i<SettleNum; i++){
        std::vector<double> temp_linkmatrix;
        sett_link_counter.push_back(0);
        for (j=0; j<SettleNum; j++)
        {
            temp_linkmatrix.push_back(0);
        }
        LinksMatrix.push_back(temp_linkmatrix);
    }

    std::cout<<"Finished reading in the settlement file with "<<SettleNum<<" settlements"<<std::endl<<std::endl;

    //Loading airports
    while (!airportfile.eof() && !airportfile.fail())
    {
        Settlement temp_air;
        airportfile>>temp_air.name;
        if (airportfile.eof()) break;
        airportfile>>temp_air.lat;
        airportfile>>temp_air.lon;
        temp_air.pop=0; //this will be calculated during Voronoi tessellation
        temp_air.roadhub=-1;
        temp_air.airhub=-1;
        temp_air.traffic=0;
        temp_air.normalize=0;
        if (temp_air.lat!=0 && temp_air.lon!=0)
        {
            airport.push_back(temp_air);
            std::vector<unsigned long int> temp_air_id;
            std::vector<double> temp_air_traffic;
            airlink_id.push_back(temp_air_id);
            airlink_traffic.push_back(temp_air_traffic);
        }
    }

    int AirportNum=(int)airport.size();

    std::cout <<"Finished reading in the airport file with "<<airport.size()<<" airports." << std::endl<< std::endl;
    
    //variables for heterogeneity in demographics
    unsigned long int Airport=0;
    unsigned long int Region=1;  //all regions participate in the regional migration by new regional migration
    unsigned long int Seaport=0;

    double birthrate[MAX_REGIONS];// prob that a person has a baby on a given day, needs to be gotten from data, chosen for birth_rate_dependence 2
    int agedistflag[MAX_REGIONS]; // Initial age distribution
    double agedist1[MAX_REGIONS];
    double agedist2[MAX_REGIONS];
    int prevdistflag[MAX_REGIONS]; // Initial prevalence distribution
    double prevdist1[MAX_REGIONS];
    double prevdist2[MAX_REGIONS];
    int immdistflag[MAX_REGIONS]; // Initial immunity distribution
    double immdist1[MAX_REGIONS];
    double immdist2[MAX_REGIONS];
    int riskdistflag[MAX_REGIONS];// Risk factor distribution
    double riskdist1[MAX_REGIONS];
    double riskdist2[MAX_REGIONS];
    int mighetdistflag[MAX_REGIONS];// Migration heterogeneity distribution
    double mighetdist1[MAX_REGIONS];
    double mighetdist2[MAX_REGIONS];
    int num_regions=1;
    double xll[MAX_REGIONS];
    double yll[MAX_REGIONS];
    double xru[MAX_REGIONS];
    double yru[MAX_REGIONS];
    int IDgroup[MAX_REGIONS];
    int tempregion=0;
    int num_demo_comms=0;


    birthrate[0]      = 5.2/38.6/365;// prob that a person has a baby on a given day, needs to be gotten from data, chosen for birth_rate_dependence 2
    agedistflag[0]    = 3; // Initial age distribution
    agedist1[0]       = 1/(38.6*365);
    agedist2[0]       = 0;
    prevdistflag[0]   = 1; // Initial prevalence distribution
    prevdist1[0]      = 0.1;
    prevdist2[0]      = 0.2;
    immdistflag[0]    = 0; // Initial immunity distribution
    immdist1[0]       = 1.0;
    immdist2[0]       = 0;
    riskdistflag[0]   = 0;// Risk factor distribution
    riskdist1[0]      = 1.0;
    riskdist2[0]      = 0;
    mighetdistflag[0] = 0;// Migration heterogeneity distribution
    mighetdist1[0]    = 1.0;
    mighetdist2[0]    = 0;

    if(demoinfile){
        demoinfile.get(temp);
        while(temp!=' '){demoinfile.get(temp);}
        demoinfile>>num_regions;
        if(num_regions>MAX_REGIONS){
            std::cout<<"ERROR redefine MAX_REGIONS in stdafx.h: "<<num_regions<<std::endl;
            goto skip;
        }
        for(i=0; i<num_regions; i++){
            IDgroup[i]=-1;
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>j;
            if(i!=j-1){std::cout<<"Check demographic details input file region "<<j<<std::endl;}
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>xll[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>yll[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>xru[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>yru[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>birthrate[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>agedistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp); }
            demoinfile>>agedist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>agedist2[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>prevdistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>prevdist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>prevdist2[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>immdistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>immdist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>immdist2[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>riskdistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>riskdist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>riskdist2[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>mighetdistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>mighetdist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>mighetdist2[i];

            std::cout<<j<<" : xll "<<xll[i]<<" yll "<<yll[i]<<" birthrate "<<birthrate[i]<<" age info "<<agedistflag[i]<<'\t'<<agedist1[i]<<'\t'<<agedist2[i]<<std::endl;
            std::cout<<" prev info "<<prevdistflag[i]<<'\t'<<prevdist1[i]<<'\t'<<prevdist2[i]<<" imm info "<<immdistflag[i]<<'\t'<<immdist1[i]<<'\t'<<immdist2[i]<<std::endl;
            std::cout<<" mighet info "<<mighetdistflag[i]<<'\t'<<mighetdist1[i]<<'\t'<<mighetdist2[i]<<std::endl<<std::endl;
        }

        demoinfile.get(temp);
        while(temp!=' '){demoinfile.get(temp);}
        demoinfile>>num_demo_comms;
        if(num_regions+num_demo_comms>MAX_REGIONS){
            std::cout<<"ERROR redefine MAX_REGIONS in stdafx.h to a minimum of "<<num_regions+num_demo_comms<<std::endl;
            goto skip;
        }
        for(i=num_regions; i<num_regions+num_demo_comms; i++){
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>IDgroup[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>birthrate[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>agedistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp); }
            demoinfile>>agedist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>agedist2[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>prevdistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>prevdist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>prevdist2[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>immdistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>immdist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>immdist2[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>riskdistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>riskdist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>riskdist2[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>mighetdistflag[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>mighetdist1[i];
            demoinfile.get(temp);
            while(temp!=' '){demoinfile.get(temp);}
            demoinfile>>mighetdist2[i];

            std::cout<<IDgroup[i]<<" community : birthrate "<<birthrate[i]<<" age info "<<agedistflag[i]<<'\t'<<agedist1[i]<<'\t'<<agedist2[i]<<std::endl;
            std::cout<<" prev info "<<prevdistflag[i]<<'\t'<<prevdist1[i]<<'\t'<<prevdist2[i]<<" imm info "<<immdistflag[i]<<'\t'<<immdist1[i]<<'\t'<<immdist2[i]<<std::endl;
            std::cout<<" mighet info "<<mighetdistflag[i]<<'\t'<<mighetdist1[i]<<'\t'<<mighetdist2[i]<<std::endl<<std::endl;
        }
    }

    //Variables for reading in the population file
    int nrows        = 0;
    int ncols        = 0;
    double xllcorner = 0;
    double yllcorner = 0;
    double xrucorner = 0;
    double yrucorner = 0;
    double cellsize  = 0;

    unsigned long int tempCommID=1;
    double localmigrate=LOCAL_MIGRATE;// number of people moving to each adjacent square for larger pops, if less than 800 in a square, then the rate is 1/8 to leave to each adjacent square, 
                            // so that sparse areas are more migratory.  This number can be tuned
    
    // Get demographic file characteristics
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>ncols;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>nrows;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>xllcorner;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>yllcorner;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>cellsize;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>tempval;
        
    //used for climate file construction
    xrucorner=xllcorner+cellsize*ncols;
    yrucorner=yllcorner+cellsize*nrows;
    
    std::cout<<"Started reading population file with the following characteristcs "<<std::endl;
    //verify all characteristics are inputed correctly
    std::cout<<ncols<<'\t'<<nrows<<'\t'<<xllcorner<<'\t'<<yllcorner<<'\t'<<cellsize<<'\t'<<tempval<<std::endl;
    std::cout<<xrucorner<<'\t'<<yrucorner<<std::endl<<std::endl;


    //reading altitude
    double templat;
    double templon;
    double tempalt;

    for(i=0;i<nrows; i++){
        std::vector<Altitude> temp_alt_v;
        for(j=0; j<ncols; j++){
            Altitude temp_alt;
            temp_alt.altitude=0;
            temp_alt.altcounter=0;
            temp_alt_v.push_back(temp_alt);
        }
        alt.push_back(temp_alt_v);
    }

    while(!altitudefile.eof()){
        altitudefile>>templon;
        if (altitudefile.eof()) break;
        altitudefile>>templat;
        altitudefile>>tempalt;
        i=(int)(abs(templat-yrucorner)/cellsize);
        j=(int)(abs(templon-xllcorner)/cellsize);
        if(tempalt<0){tempalt=0;}
        if(i<nrows && j<ncols)
        {
            if(alt[i][j].altcounter==0){ 
                alt[i][j].altcounter=1;
                alt[i][j].altitude=tempalt;
                //std::cout<<counter[i][j]<<'\t'<<i<<'\t'<<j<<'\t'<<altitude[i][j]<<std::endl;
            }//end if
            else{
                alt[i][j].altcounter++;
                alt[i][j].altitude=(alt[i][j].altitude*(alt[i][j].altcounter-1)+tempalt)/alt[i][j].altcounter;
                //std::cout<<counter[i][j]<<'\t'<<i<<'\t'<<j<<'\t'<<altitude[i][j]<<std::endl;
            }//end else
        }//end if
    }//end while
    std::cout<<"Finished reading in the altitude file."<<std::endl<<std::endl;
    

    //Variables for reading in the climate data
    
    templat          = 0;
    templon          = 0;
    int temptype     = 0;
    int TotClimTypes = 0;
    double min_total = 0;
    int min_index    = 0;
    double min_lat   = 0;
    double min_lon   = 0;

    // Record only the climate info for the latitude and longitude of the country in question
    while(climatefile>>templat){
        climatefile>>templon;
        climatefile>>temptype;
        if(templat<yrucorner && templat>yllcorner){
            if(templon<xrucorner && templon>xllcorner){
                Climate temp_climate;
                temp_climate.lat=templat;
                temp_climate.lon=templon;
                temp_climate.type=temptype;
                climate.push_back(temp_climate);
            }
        }
    }
    TotClimTypes=(int)climate.size();

    //construct the community
    for (i=0; i<nrows; i++)
    {
        std::vector<Community> temp_v_comm;
        for (j=0; j<ncols; j++)
        {
            Community temp_comm;
            temp_comm.id=0;
            temp_comm.pop=0;
            temp_comm.inpop=0;
            temp_comm.lat=0;
            temp_comm.lon=0;
            temp_comm.voronoi_air=-1;
            temp_comm.roadhub=-1;
            temp_comm.airhub=-1;
            temp_comm.voronoi_dist=-1;
            temp_v_comm.push_back(temp_comm);
        }
        comm.push_back(temp_v_comm);
    }

    //set the airport location into community cell
    for (l=0; l<AirportNum; l++)
    {
        int temp_row=nrows-(int)ceil((airport[l].lat-yllcorner)/cellsize);
        int temp_col=(int)floor((airport[l].lon-xllcorner)/cellsize);
        airport[l].comm_row=temp_row;
        airport[l].comm_col=temp_col;
        airport[l].pop=0;
        comm[temp_row][temp_col].airhub=l;
        comm[temp_row][temp_col].voronoi_air=l;
    }

    
    // Now process the population file and build the climate type 2 output file
    for(i=0; (i<nrows) ; i++)
    {
        for(j=0; j<ncols; j++)
        {
            Airport=0;
            double temp_pop;
            popfile>>temp_pop;
            if(temp_pop>0)
            {
                tempregion=0;
                min_total=MAX_DIST;
                dist=1000;
                min_index=0;
                min_lat=0;
                min_lon=0;
                comm[i][j].id=tempCommID;
                comm[i][j].pop=unsigned long int(temp_pop);
                comm[i][j].inpop=temp_pop;
                comm[i][j].lat=(nrows-i-0.5)*cellsize+yllcorner;
                comm[i][j].lon=(j+0.5)*cellsize+xllcorner;
                m=0;

                while(m<num_regions && tempregion==0){
                    if(comm[i][j].lat<=yru[m] && comm[i][j].lat>=yll[m]){
                        if(comm[i][j].lon<=xru[m] && comm[i][j].lon>=xll[m]){
                            tempregion=m;
                            //std::cout<<comm[i][j].id<<'\t'<<tempregion<<'\t'<<'\t';
                        }
                    }
                    m++;
                }

                while(m<num_regions+num_demo_comms){
                    if(comm[i][j].id==IDgroup[m]){
                        tempregion=m;
                    }
                    m++;
                }

                //Determine climate type's index 
                for(k=0; k<TotClimTypes; k++){
                    dX=abs(comm[i][j].lat-climate[k].lat)*LAT_CONVERTER;
                    dY=abs(comm[i][j].lon-climate[k].lon)*LON_CONVERTER;
                    dist=sqrt(dX*dX+dY*dY);
//                  dist=GeoDist(comm[i][j].lat, comm[i][j].lon, Climatelat[k], Climatelon[k])/1000.0;
                    //dist=sqrt((comm[i][j].lat-Climatelat[k])*(comm[i][j].lat-Climatelat[k])+(comm[i][j].lon-Climatelon[k])*(comm[i][j].lon-Climatelon[k]));
                    if(dist<min_total){
                        min_index=k;
                        min_total=dist;
                        min_lat=climate[k].lat;
                        min_lon=climate[k].lon;
                    }
                }

                if(min_total==1000){
                    std::cout<<"ERROR: Didn't find climate longitude/latitude for CommID "<<comm[i][j].id<<'\n';
                }

                climate2binfile.write((char *)(&comm[i][j].id),(sizeof(comm[i][j].id)));
                climate2binfile.write((char *)(&climate[min_index].type),(sizeof(climate[min_index].type)));
                
                //if( Climatetype[min_index]!=4 && Climatetype[min_index]!=15){
                //  std::cout<<comm[i][j].id<<'\t'<<Climatetype[min_index]<<std::endl;
                //}
                //std::cout<<comm[i][j].id<<std::endl;

                if (comm[i][j].airhub!=-1)
                {
                    int airportID=comm[i][j].airhub;
                    airport[airportID].comm_ID=comm[i][j].id;
                    airport[airportID].comm_lat=comm[i][j].lat;
                    airport[airportID].comm_lon=comm[i][j].lon;
                    Airport=1;
                }

                demofile<<comm[i][j].id<<'\t'<<comm[i][j].lat<<'\t'<<comm[i][j].lon<<'\t'<<alt[i][j].altitude<<'\t'<<Airport<<'\t'<<Region<<'\t'<<Seaport<<'\t'<<comm[i][j].pop<<'\t'<<birthrate[tempregion]<<'\t'<<agedistflag[tempregion]<<'\t'<<agedist1[tempregion]<<'\t'<<agedist2[tempregion]<<std::endl;
                demofile<<'\t'<<prevdistflag[tempregion]<<'\t'<<prevdist1[tempregion]<<'\t'<<prevdist2[tempregion]<<'\t'<<immdistflag[tempregion]<<'\t'<<immdist1[tempregion]<<'\t'<<immdist2[tempregion]<<'\t'<<riskdistflag[tempregion]<<'\t'<<riskdist1[tempregion]<<'\t'<<riskdist2[tempregion]<<'\t'<<mighetdistflag[tempregion]<<'\t'<<mighetdist1[tempregion]<<'\t'<<mighetdist2[tempregion]<<'\t';
                demofile<<std::endl;

                //std::cout<<comm[i][j].id<<'\t'<<urban[i][j]<<std::endl;
                
                //demofile<<comm[i][j].id<<'\t'<<comm[i][j].lat<<'\t'<<comm[i][j].lon<<'\t'<<altitude[i][j]<<'\t'<<Airport<<'\t'<<Region<<'\t'<<Seaport<<'\t'<<comm[i][j].pop<<'\t'<<birthrate<<'\t'<<agedistflag<<'\t'<<agedist1<<'\t'<<agedist2;
                //demofile<<'\t'<<prevdistflag<<'\t'<<prevdist1<<'\t'<<prevdist2<<'\t'<<immdistflag<<'\t'<<immdist1<<'\t'<<immdist2<<'\t'<<riskdistflag<<'\t'<<riskdist1<<'\t'<<riskdist2<<'\t'<<mighetdistflag<<'\t'<<mighetdist1<<'\t'<<mighetdist2<<'\t'<<Climatetype[min_index];
                //demofile<<std::endl;

                //constructs the regional network indexed by comm_ID
                std::vector<unsigned long int> temp_reg_id;
                std::vector<double> temp_reg_traffic;
                reglink_id[tempCommID]=temp_reg_id;
                reglink_traffic[tempCommID]=temp_reg_traffic;
                tempCommID++;
            }//end if
        }
    }

    // For debugging 
    std::cout<<"Finished writing demo and climate files."<<std::endl<<std::endl;

    //determines the closest community ID for each settlement (region, airport)
    for (l=0; l<SettleNum; l++)
    {
        int temp_row=nrows-(int)ceil((region[l].lat-yllcorner)/cellsize);
        int temp_col=(int)floor((region[l].lon-xllcorner)/cellsize);
        region[l].comm_row=temp_row;
        region[l].comm_col=temp_col;
        region[l].comm_ID=comm[temp_row][temp_col].id;
        region[l].comm_lat=comm[temp_row][temp_col].lat;
        region[l].comm_lon=comm[temp_row][temp_col].lon;
        region[l].pop=comm[temp_row][temp_col].pop;
        region[l].roadhub=-1;
    }
    
    settlefile.close();
    airportfile.close();
    popfile.close();
    climate2binfile.close();
    demofile.close();

    //Voronoi tessellation for airhubs, and calculate the population for each air basin

    double air_min_dist=MAX_DIST;
    int closest_airport=-1;
    for (i=0; i<nrows; i++)
    {
        for (j=0; j<ncols; j++)
        {
            if (comm[i][j].pop>0 && comm[i][j].airhub==-1)
            {
                air_min_dist=MAX_DIST;
                closest_airport=-1;
                for(k=0; k<AirportNum; k++)
                {
                    dX=abs(comm[i][j].lat-airport[k].lat)*LAT_CONVERTER;
                    dY=abs(comm[i][j].lon-airport[k].lon)*LON_CONVERTER;
                    dist=sqrt(dX*dX+dY*dY);
//                  dist=GeoDist(comm[i][j].lat,comm[i][j].lon,region[k].lat,region[k].lon)/1000.0;
                    if(dist<air_min_dist)
                    {
                        air_min_dist=dist;
                        closest_airport=k;
                    }//end if dist
                }//end for k
                if (air_min_dist<=DIST_THRESHOLD && closest_airport!=-1)
                {
                comm[i][j].voronoi_air=closest_airport;
                comm[i][j].voronoi_dist=air_min_dist;
                airport[closest_airport].pop=airport[closest_airport].pop+comm[i][j].pop;
                airport[closest_airport].normalize=airport[closest_airport].normalize+comm[i][j].pop*PDFNormalDist(0,SIGMA,air_min_dist); //this function can be changed
                }
            }//end if comm.pop
        }//end for j
    }//end for i


    // file for debugging
    //output airport community file
/*
    for (i=0; i<airport.size(); i++)
    {
        aircommfile << i << " " << airport[i].name.c_str() << " " << airport[i].comm_ID << " " << airport[i].lat << " " << airport[i].lon << std::endl;
    }

    aircommfile.close();
*/

    linksfile.open(linksinput);

    if(linksfile.fail()){
        std::cout<<"ERROR: Links file open failed."<<std::endl;
        goto skip;
    }
    
    l=0;
    int LinksNum=0;
    while(!linksfile.eof() && !linksfile.fail()){
        std::string LinkCity1, LinkCity2;
        int LinkCityIndex1=-1;
        int LinkCityIndex2=-1;
        int LinkType=-1;
        double LinkRate=-1;
        linksfile>>LinkCity1;
        if (linksfile.eof()) break;
        linksfile>>LinkCity2;
        linksfile>>LinkType;

        k=0;

        if(LinkType==1){LinkRate=ROAD_RATE1;}
        else if(LinkType==2){LinkRate=ROAD_RATE2;}
        else if(LinkType==3){LinkRate=ROAD_RATE3;}
        else if(LinkType==4){LinkRate=RAIL_RATE4;}
        else if(LinkType==5){LinkRate=RAIL_RATE5;}
        else{ std::cout<<"ERROR: Link type is "<<LinkType<<" for link "<<l<<std::endl;}
        
        while((k<SettleNum) && (LinkCityIndex1==-1 || LinkCityIndex2==-1)){
            if(region[k].name==LinkCity1){ 
                LinkCityIndex1=k;
                comm[region[k].comm_row][region[k].comm_col].roadhub=k;   //indicate that the Community is a Road Hub and which settlement that corresponds to
                region[k].roadhub=k;
            }
            if(region[k].name==LinkCity2){
                LinkCityIndex2=k;
                comm[region[k].comm_row][region[k].comm_col].roadhub=k;   //indicate that the Community is a Road Hub and which settlement that corresponds to
                region[k].roadhub=k;
            }
            k++;
        }
        if(LinkCityIndex1==-1){
            std::cout<<"ERROR: In link "<<l<<" the city "<<LinkCity1.c_str()<<" is not matched to a settlement"<<std::endl;
            goto nextlink;
        }
        if(LinkCityIndex2==-1){
            std::cout<<"ERROR: In link "<<l<<" the city "<<LinkCity2.c_str()<<" is not matched to a settlement"<<std::endl;
            goto nextlink;
        }

        double LinkRate12=0;
        double LinkRate21=0;

        if(region[LinkCityIndex1].pop > region[LinkCityIndex2].pop){
            LinkRate12= LinkRate;
            LinkRate21= LinkRate12 * (double(region[LinkCityIndex2].pop)/double(region[LinkCityIndex1].pop));
        }else{
            LinkRate21 = LinkRate;
            LinkRate12 = LinkRate * (double(region[LinkCityIndex1].pop)/double(region[LinkCityIndex2].pop));
        }
        LinksMatrix[LinkCityIndex1][LinkCityIndex2]=LinkRate12;
        LinksMatrix[LinkCityIndex2][LinkCityIndex1]=LinkRate21;
nextlink:
        l++;
    }
    LinksNum=l;

    //for debugging
    std::cout<<"Finished reading region link file with "<<LinksNum<<" links."<<std::endl<<std::endl;

    //Complete the LinksMatrix to include locations that you could go to through a middle city in one day
    double distAtoB=0;
    double WorstRoadRate=0;
    bool OneDayTravel;  //Is distance travelable in one day

    for(i=0; i<SettleNum; i++){
        for(j=i; j<SettleNum; j++){
            if(LinksMatrix[i][j]!=0){
                sett_link_counter[i]++;
            }
        }
    }

    for(i=0; i<SettleNum; i++){
        for(j=i; j<SettleNum; j++){
            if(LinksMatrix[i][j]!=0){
                for(k=0; k<SettleNum; k++){
                    if(LinksMatrix[j][k]!=0 && k!=i && LinksMatrix[i][k]==0){
                        dY=abs(region[i].comm_lat-region[k].comm_lat)*LAT_CONVERTER;
                        dX=abs(region[i].comm_lon-region[k].comm_lon)*LON_CONVERTER;
                        distAtoB=sqrt(dX*dX+dY*dY);
//                      distAtoB=GeoDist(comm[Settle_i[i]][Settle_j[i]].lat, comm[Settle_i[i]][Settle_j[i]].lon, comm[Settle_i[k]][Settle_j[k]].lat, comm[Settle_i[k]][Settle_j[k]].lon)/1000.0;
                        //std::cout<<distAtoB<<'\t';
                        //distAtoB=sqrt((latitude[Settle_i[i]][Settle_j[i]]-latitude[Settle_i[k]][Settle_j[k]])*(latitude[Settle_i[i]][Settle_j[i]]-latitude[Settle_i[k]][Settle_j[k]])+(longitude[Settle_i[i]][Settle_j[i]]-longitude[Settle_i[k]][Settle_j[k]])*(longitude[Settle_i[i]][Settle_j[i]]-longitude[Settle_i[k]][Settle_j[k]]));
                        OneDayTravel=0;
                        if(LinksMatrix[i][j]==ROAD_RATE1 || LinksMatrix[j][i]==ROAD_RATE1 || LinksMatrix[j][k]==ROAD_RATE1 ||LinksMatrix[k][j]==ROAD_RATE1){
                            WorstRoadRate=ROAD_RATE1;
                            //std::cout<<"road 1 "<<sett_link_counter[i]<<"  "<<sett_link_counter[k]<<std::endl;
                            if(distAtoB<=ONEDAY_DISTANCE1){
                                //if(LinksMatrix[i][j]!=RAIL_RATE4 && LinksMatrix[i][j]!=RAIL_RATE5 && LinksMatrix[j][i]==RAIL_RATE4 && LinksMatrix[j][i]==RAIL_RATE5 && LinksMatrix[j][k]==RAIL_RATE4 && LinksMatrix[j][k]==RAIL_RATE5 && LinksMatrix[k][j]==RAIL_RATE4 && LinksMatrix[k][j]==RAIL_RATE5){
                                    OneDayTravel=1;
                                //}
                            }
                        }else if(LinksMatrix[i][j]==ROAD_RATE2 || LinksMatrix[j][i]==ROAD_RATE2 || LinksMatrix[j][k]==ROAD_RATE2 ||LinksMatrix[k][j]==ROAD_RATE2){
                            WorstRoadRate=ROAD_RATE2;
                            //std::cout<<"road 2 "<<sett_link_counter[i]<<"  "<<sett_link_counter[k]<<std::endl;
                            if(distAtoB<=ONEDAY_DISTANCE2){
                                //if(LinksMatrix[i][j]!=RAIL_RATE4 && LinksMatrix[i][j]!=RAIL_RATE5 && LinksMatrix[j][i]==RAIL_RATE4 && LinksMatrix[j][i]==RAIL_RATE5 && LinksMatrix[j][k]==RAIL_RATE4 && LinksMatrix[j][k]==RAIL_RATE5 && LinksMatrix[k][j]==RAIL_RATE4 && LinksMatrix[k][j]==RAIL_RATE5){
                                    OneDayTravel=1;
                                //}
                            }
                        }else if(LinksMatrix[i][j]==RAIL_RATE4 || LinksMatrix[j][i]==RAIL_RATE4 || LinksMatrix[j][k]==RAIL_RATE4 ||LinksMatrix[k][j]==RAIL_RATE4){
                            WorstRoadRate=RAIL_RATE4;
                            //std::cout<<"rail 4 "<<sett_link_counter[i]<<"  "<<sett_link_counter[k]<<std::endl;
                            if(distAtoB<=ONEDAY_DISTANCE4){
                                //if(LinksMatrix[i][j]!=ROAD_RATE1 && LinksMatrix[i][j]!=ROAD_RATE2 && LinksMatrix[i][j]!=ROAD_RATE3 && LinksMatrix[j][i]==ROAD_RATE1 && LinksMatrix[j][i]==ROAD_RATE2 &&LinksMatrix[j][i]==ROAD_RATE3 && LinksMatrix[j][k]==ROAD_RATE1 && LinksMatrix[j][k]==ROAD_RATE2 && LinksMatrix[j][k]==ROAD_RATE3 && LinksMatrix[k][j]==ROAD_RATE1 && LinksMatrix[k][j]==ROAD_RATE2 && LinksMatrix[k][j]==ROAD_RATE3){
                                    OneDayTravel=1;
                                //}
                            }
                        }else if(LinksMatrix[i][j]==ROAD_RATE3 || LinksMatrix[j][i]==ROAD_RATE3 || LinksMatrix[j][k]==ROAD_RATE3 ||LinksMatrix[k][j]==ROAD_RATE3){
                            WorstRoadRate=ROAD_RATE3;
                            //std::cout<<"road 3 "<<sett_link_counter[i]<<"  "<<sett_link_counter[k]<<std::endl;
                            if(distAtoB<=ONEDAY_DISTANCE3){
                                //if(LinksMatrix[i][j]!=RAIL_RATE4 && LinksMatrix[i][j]!=RAIL_RATE5 && LinksMatrix[j][i]==RAIL_RATE4 && LinksMatrix[j][i]==RAIL_RATE5 && LinksMatrix[j][k]==RAIL_RATE4 && LinksMatrix[j][k]==RAIL_RATE5 && LinksMatrix[k][j]==RAIL_RATE4 && LinksMatrix[k][j]==RAIL_RATE5){
                                    OneDayTravel=1;
                                //}
                            }
                        }else if(LinksMatrix[i][j]==RAIL_RATE5 || LinksMatrix[j][i]==RAIL_RATE5 || LinksMatrix[j][k]==RAIL_RATE5 ||LinksMatrix[k][j]==RAIL_RATE5){
                            WorstRoadRate=RAIL_RATE5;
                            //std::cout<<"rail 5 "<<sett_link_counter[i]<<"  "<<sett_link_counter[k]<<std::endl;
                            if(distAtoB<=ONEDAY_DISTANCE5){
                                //if(LinksMatrix[i][j]!=ROAD_RATE1 && LinksMatrix[i][j]!=ROAD_RATE2 && LinksMatrix[i][j]!=ROAD_RATE3 && LinksMatrix[j][i]==ROAD_RATE1 && LinksMatrix[j][i]==ROAD_RATE2 &&LinksMatrix[j][i]==ROAD_RATE3 && LinksMatrix[j][k]==ROAD_RATE1 && LinksMatrix[j][k]==ROAD_RATE2 && LinksMatrix[j][k]==ROAD_RATE3 && LinksMatrix[k][j]==ROAD_RATE1 && LinksMatrix[k][j]==ROAD_RATE2 && LinksMatrix[k][j]==ROAD_RATE3){
                                    OneDayTravel=1;
                                //}
                            }
                        }
                        if(OneDayTravel && sett_link_counter[i]<REGION_0_PADDING && sett_link_counter[k]<REGION_0_PADDING){
                            if(region[i].pop>region[k].pop){
                                LinksMatrix[i][k]= WorstRoadRate;
                                LinksMatrix[k][i]= WorstRoadRate * (double(region[k].pop)/double(region[i].pop));
                            }else{
                                LinksMatrix[k][i]= WorstRoadRate;
                                LinksMatrix[i][k]= WorstRoadRate * (double(region[i].pop)/double(region[k].pop));
                            }
                            //std::cout<<"link added"<<std::endl;
                            sett_link_counter[i]++;
                            sett_link_counter[k]++;
                        }
                    }
                }
            }
        }
    }

    for(i=0; i<SettleNum; i++){
        if(sett_link_counter[i]!=0){
            std::cout<<"Settlement "<<i<<", which is community ID "<<region[i].comm_ID << ", has "<<sett_link_counter[i]<<" links."<<std::endl;
        }
    }
    std::cout << std::endl;

    //for debugging only
    //for(i=0; i<SettleNum; i++){
    //  for(j=0; j<SettleNum; j++){
    //      std::cout<<LinksMatrix[i][j]<<" ";
    //  }
    //  std::cout<<std::endl;
    //}

    //Air Migration
    airlinksfile.open(airlinksinput);

    if(airlinksfile.fail()){
        std::cout<<"ERROR: Air Links file open failed."<<std::endl;
        goto skip;
    }

    //load the flight migration links

    while(!airlinksfile.eof())
    {
        std::string code1,code2;
        double seats;
        double passengers;
        airlinksfile >> code1;
        if (airlinksfile.eof()) break;
        airlinksfile >> code2;
        airlinksfile >> seats;
        passengers=seats*(PERCENT_ANNUAL_SCALING*PERCENT_PASSENGER_FLIGHT); //rescaling the number of passengers
        unsigned long int ID1=-1;
        unsigned long int ID2=-1;

        //find the ID for two codes
        for (k=0; k<AirportNum; k++)
        {
            if (airport[k].name==code1)
            {
                ID1=k;
            }
            if (airport[k].name==code2)
            {
                ID2=k;
            }
            if (ID1!=-1 && ID2!=-1) break;
        }//end for k

        if (ID1==-1 ||  ID2==-1)
        {
            std::cout<<"Error in finding airport " << code1.c_str() << " and/or " << code2.c_str()  << " in the airport list, exit...."<< std::endl;
            goto skip;
        }
        else
        {
            //adds link
            airlink_id[ID1].push_back(ID2);
            airlink_traffic[ID1].push_back(passengers);
            airport[ID1].traffic=airport[ID1].traffic+passengers;
        }
    }//end while
    
    double min_dist;
    int closest_Sett;
    double mig_rate=0;
    unsigned long int migID=0;
    double mig_comm_rate[REGION_0_PADDING];
    unsigned long int mig_reg_ID[REGION_0_PADDING];
    double air_mig_comm_rate[AIR_0_PADDING];
    unsigned long int air_mig_reg_ID[AIR_0_PADDING];

    int IDfrom=0;
    for(i=0; i<nrows; i++)
    {
        for(j=0; j<ncols; j++)
        {
            IDfrom=comm[i][j].id;
            //write the regional road migration

            if (IDfrom>0 && comm[i][j].inpop>0) //populated cell
            {
                //clean up matrices for binary writing
                for (k=0; k<REGION_0_PADDING; k++)
                {
                    mig_comm_rate[k]=0;
                    mig_reg_ID[k]=0;
                }

                for (k=0; k<AIR_0_PADDING; k++)
                {
                    air_mig_reg_ID[k]=0;
                    air_mig_comm_rate[k]=0;
                }

                //if the community is a road hub
                if(comm[i][j].roadhub>=0 && comm[i][j].inpop>0){
                    l=0;
                    for(k=0; k<SettleNum; k++){ 
                        if(LinksMatrix[comm[i][j].roadhub][k]!=0)
                        {
                            mig_rate=LinksMatrix[comm[i][j].roadhub][k];
                            migID=region[k].comm_ID;
                            reglink_id[IDfrom].push_back(region[k].comm_ID);
                            reglink_traffic[IDfrom].push_back(LinksMatrix[comm[i][j].roadhub][k]);
                        }//end if
                    }//end for
                }//end if

                //non-road hub
                else if(comm[i][j].inpop>0)
                {
                    min_dist=MAX_DIST;
                    closest_Sett=-1;
                    for(k=0; k<SettleNum; k++){
                        if (region[k].roadhub!=-1) //Voronoi tessellation based only on road hubs
                        {
                            dX=abs(comm[i][j].lat-region[k].lat)*LAT_CONVERTER;
                            dY=abs(comm[i][j].lon-region[k].lon)*LON_CONVERTER;
                            dist=sqrt(dX*dX+dY*dY);
        //                  dist=GeoDist(comm[i][j].lat,comm[i][j].lon,Settlelat[k],Settlelon[k])/1000.0;
                            if(dist<min_dist)
                            {
                                min_dist=dist;
                                closest_Sett=k;
                            }//end if
                        }//end if
                    }//end for k
                    reglink_id[IDfrom].push_back(region[closest_Sett].comm_ID);
                    reglink_traffic[IDfrom].push_back(COMM_REGION * 1/sqrt(min_dist));
                }//end elseif

                //write the air migration for non-hubs (they will be traveling to the airhub first)
                if (comm[i][j].airhub==-1)
                {
                    int closest_air_ID=-1;
                    double closest_air_rate=-1;
                    closest_air_ID=comm[i][j].voronoi_air;
                    if (closest_air_ID!=-1)
                    {
                        if (AIRBASIN_LINK_DIST_FLAG==1) //for each community to airport, link is based on the proportion of population (flat rate)
                        {
                            closest_air_rate=(airport[closest_air_ID].traffic)/airport[closest_air_ID].pop;
                        }
                        else if (AIRBASIN_LINK_DIST_FLAG==2) //for each community to airport, rate is decayed with distance (normal distribution)
                        {
                            closest_air_rate=(PDFNormalDist(0,SIGMA, comm[i][j].voronoi_dist)*airport[closest_air_ID].traffic)/(airport[closest_air_ID].normalize);
                        }
                        reglink_id[IDfrom].push_back(airport[closest_air_ID].comm_ID);
                        reglink_traffic[IDfrom].push_back(closest_air_rate);
//                      fdebugair2 << IDfrom << " " << airport[closest_air_ID].comm_ID << " " << closest_air_rate << std::endl;
                    }//end if
                }//end if airhub==-1
                else //it's a air hub
                {
                    if (airlink_id[comm[i][j].airhub].size()>0) //there are flight links
                    {
                        for (k=0; k<airlink_id[comm[i][j].airhub].size(); k++)
                        {
                            air_mig_reg_ID[k]=airport[airlink_id[comm[i][j].airhub][k]].comm_ID;
                            air_mig_comm_rate[k]=airlink_traffic[comm[i][j].airhub][k]/airport[comm[i][j].airhub].pop;
//                          fdebugair << IDfrom << " " << air_mig_reg_ID[k] << " " << air_mig_comm_rate[k] << std::endl;
                        }
                    }//end if
                }//end else
            //write the regional migration and airport migration into binary file

            //truncate the links to the padding value by deleting the smallest weight of links
            if (reglink_id[IDfrom].size()>REGION_0_PADDING)
            {
                int number_link_delete=(int)reglink_id[IDfrom].size()-REGION_0_PADDING;
                std::cout << "Region padding value is " << REGION_0_PADDING << ", currently community " << IDfrom << " has " << reglink_id[IDfrom].size() << " links, needs to truncate " << number_link_delete << "." << std::endl;
                std::queue<int> min_id;
                std::queue<double> min_rate;

                for (k=0; k<reglink_id[IDfrom].size(); k++)
                {
                    std::cout << reglink_id[IDfrom][k] << " " << reglink_traffic[IDfrom][k] << std::endl;
                    if (min_rate.size()>0)
                    {
                        if (reglink_traffic[IDfrom][k]<=min_rate.back())
                        {
                            if (min_rate.size()==number_link_delete)
                            {
                                min_id.push(k);
                                min_rate.push(reglink_traffic[IDfrom][k]);
                                min_id.pop();
                                min_rate.pop();
                            }
                            else
                            {
                                min_id.push(k);
                                min_rate.push(reglink_traffic[IDfrom][k]);
                            }
                        }//end if
                    }//end if
                    else
                    {
                        min_id.push(k);
                        min_rate.push(reglink_traffic[IDfrom][k]);
                    }
                }//end for

                while (min_id.size()>0)
                {
                    int delete_id=min_id.front();
                    double delete_rate=min_rate.front();
                    min_id.pop();
                    min_rate.pop();

                    std::cout << "deleting the position " << delete_id << " with rate " << delete_rate << std::endl;
                    if (delete_rate!=reglink_traffic[IDfrom][delete_id])
                        std::cout << "delete elements don't match! please check." << std::endl;
                    reglink_id[IDfrom].erase(reglink_id[IDfrom].begin()+delete_id);
                    reglink_traffic[IDfrom].erase(reglink_traffic[IDfrom].begin()+delete_id);
                }//end while
                std::cout << "Now the degree of links is " << reglink_id[IDfrom].size() << "." << std::endl;
            }//end truncate

            for (k=0; k<reglink_id[IDfrom].size(); k++)
            {
                mig_reg_ID[k]=reglink_id[IDfrom][k];
                mig_comm_rate[k]=reglink_traffic[IDfrom][k];
//              fdebugreg << IDfrom << " " << mig_reg_ID[k] << " " << mig_comm_rate[k] << std::endl;
            }
            airmigfile.write((char*)air_mig_reg_ID, sizeof(unsigned long int)*AIR_0_PADDING);
            airmigfile.write((char*)air_mig_comm_rate, sizeof(double)*AIR_0_PADDING);
            regionmigfile.write((char*)mig_reg_ID, sizeof(unsigned long int)*REGION_0_PADDING);
            regionmigfile.write((char*)mig_comm_rate, sizeof(double)*REGION_0_PADDING);
            }//end for commID>0 and pop>0
        }//end for j
    }//end for i 

    std::cout<<"Finished creating the regional and airport migration file."<<std::endl<<std::endl;
    regionmigfile.close();

    // Now build load balancing file
    unsigned long int numComms=0;
    numComms=tempCommID-1;
    unsigned long int* Commarray;
    float* Sampleratearray;
    float* Procarray;
    double* CumulativePop;
    double* tempCumulativePop;
    double* tempdPop;

    // Hold the CommIDs, the sampling rates, and the processor assignment
    // CumulativePop holds the increasing total number of starting particles in the simulation
    Commarray=new unsigned long int[numComms];
    Sampleratearray=new float[numComms];
    Procarray=new float[numComms];
    CumulativePop=new double[numComms];
    tempCumulativePop=new double[numComms];
    tempdPop=new double[numComms];

    // Current version has weighted combination of comms, pop, and inf balancing
    // Have values for weights below
    double comm_weighting=0.0;
    double iter_weighting=1.0;
    double pop_weighting=0.0;
    // Building the file to balance population
    // go through, and get population of each valid grid location
    unsigned long int counter=0;
    for(int i=0; i<nrows; i++){
        for(int j=0; j<ncols; j++){
            if(comm[i][j].id){
                Commarray[counter]=comm[i][j].id;
                Sampleratearray[counter]=1.0;// can add differing sampling rates later
                CumulativePop[counter]=Sampleratearray[counter]*comm[i][j].pop;// want to track future particles, not the actual people
                counter++;
            }

        }
    }
    //std::cout<<counter<<std::endl;//debug check
    double totalPop=CumulativePop[0];
    for(i=1; i<counter; i++){
        totalPop+=CumulativePop[i];
        CumulativePop[i]+=CumulativePop[i-1];
    }
    if(totalPop>0){
        for(i=0; i<counter; i++){
            CumulativePop[i]/=(totalPop);
            Procarray[i]=pop_weighting*CumulativePop[i]*0.99999;
            //std::cout<<Procarray[i]<<std::endl;
        }
    
    }

    // Building the file to balance infections, empirically based on previous runs
    // Use CumulativePop and totalPop to weight Community objects based on empirical results from cluster runs

    // Commweighting is based on the number of infections at 365 days in the 80-core run of malaria Run 514 July 2, 2009
    //old way pre-09172009 double Commweighting[]={0.006181549,0.008734798,0.010884902,0.011624001,0.017268024,0.021836995,0.020425989,0.015521064,0.010011422,0.009070752,0.008398844,0.007726937,0.010347376,0.006517503,0.007458174,0.008264463,0.003426728,0.003225156,0.00356111,0.004636162,0.004770544,0.011153665,0.010884902,0.013438151,0.018544648,0.021097897,0.018141504,0.01511792,0.012967816,0.011489619,0.014916348,0.017603978,0.012564671,0.011422428,0.012766243,0.0159914,0.017536787,0.022912047,0.021433851,0.01424444,0.013639723,0.018544648,0.017402405,0.021232278,0.018007122,0.013035006,0.010414567,0.015252301,0.013169388,0.013169388,0.011422428,0.014580394,0.009877041,0.009205133,0.011959954,0.008801989,0.008331654,0.008734798,0.015252301,0.010616139,0.011288047,0.010884902,0.01243029,0.008667607,0.006315931,0.009070752,0.008801989,0.010548948,0.010616139,0.011019284,0.011355238,0.010817711,0.010347376,0.011019284,0.008331654,0.009406706,0.011086475,0.018880602,0.027010683,0.025330914};

    //iter1 09172009
    double Commweighting1[]={10780.6,9902.73,10567,10431.9,8735.52,5724.45,6066.27,7599.26,6900.96,7266.98,7524.95,8891.15,8012.73,7575.63,7351.54,5843.99,5852.64,7139.64,7009.64,7061.85,7471.78,6821.57,6721.91,10343.5};

    double Commweighting2[]={7562.32,7593.9,7936.15,7925.31,7837.35,8156.41,7451.37,7882.08,7793.34,7394.08,7886.99,8555.58,6851.38,8168.29,7759.47,7840.75,7832.15,7632,7695.02,7924.44,7812.63,7746.54,7415.99,8270.54};

    // keeps track of which processor this community was on in the 24-core run
    int tempproc=0;

    // Now incorporate iteration 1 to get iteration 2

    counter=0;
    for(int i=0; i<nrows; i++){
        for(int j=0; j<ncols; j++){
            if(comm[i][j].id){
                Commarray[counter]=comm[i][j].id;
                Sampleratearray[counter]=1.0;// can add differing sampling rates later
                /* old way
                // calculate which processor had this Community in 80-core Run 514 on July 2, 2009
                tempproc=(Commarray[counter]-1)/377;
                CumulativePop[counter]=1.0*Commweighting[tempproc];// each Community weighted
                counter++;
                */ 
                // new way, calculate based on previous iteration
                tempproc=24.0/30145.01*Commarray[counter];
                tempCumulativePop[counter]=1.0*Commweighting1[tempproc];// each Community weighted
                tempdPop[counter]=tempCumulativePop[counter];
                counter++;
            }

        }
    }
    //std::cout<<counter<<std::endl;//debug check
    totalPop=tempCumulativePop[0];
    for(int i=1; i<counter; i++){
        totalPop+=tempCumulativePop[i];
        tempCumulativePop[i]+=tempCumulativePop[i-1];
    }
    if(totalPop>0){
        for(int i=0; i<counter; i++){
            tempCumulativePop[i]/=(totalPop);// says what Cumulative Pop for iter weighting was on previous iteration

            // now calculate CumulativePop for this iteration
            tempproc=24.0*tempCumulativePop[i];
            CumulativePop[i]=Commweighting2[tempproc]*tempdPop[i];
        }
    }
    totalPop=CumulativePop[0];
    for(int i=1; i<counter; i++){
        totalPop+=CumulativePop[i];
        CumulativePop[i]+=CumulativePop[i-1];
    }
    if(totalPop>0){
        for(int i=0; i<counter; i++){
            CumulativePop[i]/=(totalPop);// says what Cumulative Pop for iter weighting was on previous iteration
            Procarray[i]+=iter_weighting*CumulativePop[i]*0.99999;
            //std::cout<<Procarray[i]<<std::endl;
        }

    }
/* end of iteration weighting*/


    // now by comms
    counter=0;
    for(int i=0; i<nrows; i++){
        for(int j=0; j<ncols; j++){
            if(comm[i][j].id){
                Commarray[counter]=comm[i][j].id;
                Sampleratearray[counter]=1.0;// can add differing sampling rates later
                CumulativePop[counter]=1.0;// each Community weighted equally
                counter++;
            }

        }
    }
    //std::cout<<counter<<std::endl;//debug check
    totalPop=CumulativePop[0];
    for(int i=1; i<counter; i++){
        totalPop+=CumulativePop[i];
        CumulativePop[i]+=CumulativePop[i-1];
    }
    if(totalPop>0){
        for(int i=0; i<counter; i++){
            CumulativePop[i]/=(totalPop);
            //Procarray[i]=int(CumulativePop[i]*8192);
            Procarray[i]+=comm_weighting*CumulativePop[i]*0.99999;
            Procarray[i]/=(comm_weighting+iter_weighting+pop_weighting);
            //std::cout<<Procarray[i]<<std::endl;
        }
    }

    loadbalancefile.write((char*)&counter, sizeof(unsigned long int));
    loadbalancefile.write((char*)Sampleratearray, counter*sizeof(float));
    loadbalancefile.write((char*)Procarray, counter*sizeof(float));

    std::cout<<"Finished creating the load balancing file."<<std::endl<<std::endl;

    delete Commarray;
    delete Sampleratearray;
    delete Procarray;
    delete CumulativePop;

    // Now build local migration file
    int offsets[8][2]={{-1, -1}, {-1, 0}, {-1, +1}, {0, -1}, {0, +1}, {+1,-1}, {1, 0}, {1, 1}};
    // to hold information before writing to disk
    unsigned long int migCommIDs[8];
    double migCommrates[8];

    for(int i=0; i<nrows-1; i++)
    {
        for(int j=0; j<ncols-1; j++)
        {
            if(comm[i][j].id)
            {
                for(int k=0; k<8; k++)
                {
                    if(i+offsets[k][0]<0||i+offsets[k][0]>=nrows||j+offsets[k][1]<0||j+offsets[k][1]>=ncols)
                    {
                        migCommIDs[k]=0;
                        migCommrates[k]=0;
                    }else if(comm[i+offsets[k][0]][j+offsets[k][1]].id>0)
                    {
                        if(comm[i][j].inpop>localmigrate*8)
                        {
                            migCommIDs[k]=comm[i+offsets[k][0]][j+offsets[k][1]].id;
                            migCommrates[k]=localmigrate/comm[i][j].inpop;
                        }else
                        {
                            migCommIDs[k]=comm[i+offsets[k][0]][j+offsets[k][1]].id;
                            migCommrates[k]=0.125;
                        }
                    }else{
                        migCommIDs[k]=0;
                        migCommrates[k]=0;
                    }
                }
                localmigfile.write((char*)migCommIDs, 8*sizeof(unsigned long int));
                localmigfile.write((char*)migCommrates, 8*sizeof(double));
            }
        }
    }

    std::cout<<"Finished creating the local migration file."<<std::endl<<std::endl;

    comm.clear();
    region.clear();
    airport.clear();
    LinksMatrix.clear();
    reglink_id.clear();
    reglink_traffic.clear();
    airlink_id.clear();
    airlink_traffic.clear();

skip:

    if(settlefile.is_open()){settlefile.close();}
    if(popfile.is_open()){popfile.close();}
    if(linksfile.is_open()){linksfile.close();}
    if(airlinksfile.is_open()){airlinksfile.close();}
    if(climatefile.is_open()){climatefile.close();}
    if(demofile.is_open()){demofile.close();}
//  if(aircommfile.is_open()){aircommfile.close();}
    if(loadbalancefile.is_open()){loadbalancefile.close();}
    if(localmigfile.is_open()){localmigfile.close();}
    if(regionmigfile.is_open()){regionmigfile.close();}
    if(airmigfile.is_open()){airmigfile.close();}
    if(climate2binfile.is_open()){climate2binfile.close();}

    return;
}

double PDFNormalDist(double mu, double sigma, double x)
{
    return (1/(sqrt(2*PI)*sigma))*exp(-(x-mu)*(x-mu)/(2*sigma*sigma));
}

void CreateUrbanExtent(char* popinput, char* urbanruralinput, char* start){
    
    std::ifstream popfile;
    std::ifstream urbanruralfile;

    std::ofstream urbanfile; 
    std::ofstream urbanbinfile;

    popfile.open(popinput);
    urbanruralfile.open(urbanruralinput);

    char urbanname[60];
    char urbanbinname[60];

    strcpy_s(urbanname,start);
    strcpy_s(urbanbinname, start);
    strcat_s(urbanname, "urbanextent.dat");
    strcat_s(urbanbinname, "urbanextent_bin.dat");
    urbanfile.open(urbanname);
    urbanbinfile.open(urbanbinname, std::ios::binary);

    //make sure that input files opened properly
    if(popfile.fail()){
        std::cout<<"ERROR: Population file open failed"<<std::endl;
        goto skip;
    }
    if(!urbanruralfile){
        std::cout<<"ERROR: Urban extent input failed"<<std::endl;
        goto skip;
    }
    std::cout<<"Urban extent and population files opened successfully."<<std::endl<<std::endl;

    //Variables used multiple times
    int l      = 0;
    int k      = 0;
    int i      = 0;
    int j      = 0;
    float dX   = 0; //distance in km of latitude
    float dY   = 0; //distance in km of longitude
    float dist = 1000000;
    float tempval;
    char temp;

    //Variables for reading in the population file
    int nrows                    = 0;
    int ncols                    = 0;
    float xllcorner              = 0;
    float yllcorner              = 0;
    float xrucorner              = 0;
    float yrucorner              = 0;
    float cellsize               = 0;
    unsigned long int tempCommID = 1;

    unsigned long int* CommIDs[10000];
    float* latitude[10000];
    float* longitude[10000];
    float* inPopulation[10000];
    
    // Get demographic file characteristics
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>ncols;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>nrows;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>xllcorner;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>yllcorner;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>cellsize;
    popfile.get(temp);
    while(temp!=' '){popfile.get(temp);}
    popfile>>tempval;
        
    //used for climate file construction
    xrucorner=xllcorner+cellsize*ncols;
    yrucorner=yllcorner+cellsize*nrows;
    
    std::cout<<"Started reading population file for urban extent creation with the following characteristcs "<<std::endl;
    //verify all characteristics are inputed correctly
    std::cout<<ncols<<'\t'<<nrows<<'\t'<<xllcorner<<'\t'<<yllcorner<<'\t'<<cellsize<<'\t'<<tempval<<std::endl;
    std::cout<<xrucorner<<'\t'<<yrucorner<<std::endl<<std::endl;
    
    int* urban[10000];
    
    for(i=0; i<nrows; i++){
        urban[i]=new int[ncols];
        for(j=0; j<ncols; j++){
            urban[i][j]=0;
        }
    }

    int urbnrows=0;
    int urbncols=0;
    float urbxllcorner=0;
    float urbyllcorner=0;
    float urbcellsize=0;
    float templat=0;
    float templon=0;
    int urbanindicator=0;

    // Get demographic file characteristics
    urbanruralfile.get(temp);
    while(temp!=' '){urbanruralfile.get(temp);}
    urbanruralfile>>urbncols;
    urbanruralfile.get(temp);
    while(temp!=' '){urbanruralfile.get(temp);}
    urbanruralfile>>urbnrows;
    urbanruralfile.get(temp);
    while(temp!=' '){urbanruralfile.get(temp);}
    urbanruralfile>>urbxllcorner;
    urbanruralfile.get(temp);
    while(temp!=' '){urbanruralfile.get(temp);}
    urbanruralfile>>urbyllcorner;
    urbanruralfile.get(temp);
    while(temp!=' '){urbanruralfile.get(temp);}
    urbanruralfile>>urbcellsize;
    urbanruralfile.get(temp);
    while(temp!=' '){urbanruralfile.get(temp);}
    urbanruralfile>>tempval;

    if(urbxllcorner!=xllcorner){
        std::cout<<"WARNING: Longitudes for urban extent and demographics may be unmatched; check files."<<std::endl;
    }
    if(urbyllcorner!=yllcorner){
        std::cout<<"WARNING: Latitudes for urban extent and demographics may be unmatched; check files."<<std::endl;
    }

    std::cout<<"Started reading urban extent file with the following characteristcs "<<std::endl;
    std::cout<<urbncols<<'\t'<<urbnrows<<'\t'<<urbxllcorner<<'\t'<<urbyllcorner<<'\t'<<urbcellsize<<std::endl<<std::endl;

    for(k=0; k<urbnrows; k++){
        for(l=0; l<urbncols; l++){
            urbanruralfile>>urbanindicator;
            templat=(urbnrows-k-0.5)*urbcellsize+urbyllcorner;
            templon=(l+0.5)*urbcellsize+urbxllcorner;
            i=abs(templat-yrucorner)/cellsize;
            j=abs(templon-xllcorner)/cellsize;
            if(i<nrows && j<ncols){
                if(urbanindicator==2){ 
                    urban[i][j]=1;
                    //std::cout<<i<<'\t'<<j<<'\t'<<"city"<<std::endl;
                }
            }
        }
    }
    
    std::cout<<"Finished reading urban extent file."<<std::endl<<std::endl;
    urbanruralfile.close();

    // Now process the population file and build the climate type 2 output file
    for(i=0; (i<nrows) ; i++)
    {
        latitude[i]=new float[ncols];
        longitude[i]=new float[ncols];
        inPopulation[i]=new float[ncols];
        CommIDs[i]=new unsigned long int[ncols];
        for(j=0; j<ncols; j++)
        {
            //std::cout<<"Read in column "<<j<<" row "<<i<<std::endl;
            popfile>>inPopulation[i][j];
            if(inPopulation[i][j]>0){

                CommIDs[i][j]=tempCommID;
                latitude[i][j]=(nrows-i-0.5)*cellsize+yllcorner;
                longitude[i][j]=(j+0.5)*cellsize+xllcorner;

                urbanfile<<CommIDs[i][j]<<'\t'<<latitude[i][j]<<'\t'<<longitude[i][j]<<'\t'<<inPopulation[i][j]<<'\t'<<urban[i][j]<<std::endl;
                //std::cout<<CommIDs[i][j]<<'\t'<<urban[i][j]<<std::endl;
                urbanbinfile.write((char *)(&CommIDs[i][j]),(sizeof(CommIDs[i][j])));
                urbanbinfile.write((char *)(&urban[i][j]),(sizeof(urban[i][j])));
                
                tempCommID++;

            }else{
                CommIDs[i][j]=0;
                latitude[i][j]=0;
                longitude[i][j]=0;
            }
        }
    }
    for(int i=0; i<nrows; i++)
    {
        delete [] inPopulation[i];
        delete [] latitude[i];
        delete [] longitude[i];
        delete [] CommIDs[i];
        delete [] urban[i];
    }


skip:

    if(popfile.is_open()){popfile.close();}
    if(urbanruralfile.is_open()){urbanruralfile.close();}
    if(urbanbinfile.is_open()){urbanbinfile.close();}
    if(urbanfile.is_open()){urbanfile.close();}
}

//used for debugging
void ConvertRegionToBin(char* start){
    std::ifstream regionmigfile;
    std::ofstream regionmigbinfile; 

    char regionname[60];
    char regionbinname[60];

    strcpy_s(regionname,start); 
    strcpy_s(regionbinname,start); 

    strcat_s(regionname, "region.dat");
    strcat_s(regionbinname, "region_bin.dat");

    regionmigfile.open(regionname);
    regionmigbinfile.open(regionbinname, std::ios::binary);

    if(regionmigfile.fail()||regionmigbinfile.fail()){
        std::cout<<"Region migration input or output file open failed"<<std::endl;
        goto skip;
    }
    
    unsigned long int tempID[REGION_0_PADDING];
    double rate[REGION_0_PADDING];

    while(!regionmigfile.eof()){
        for(int i=0; i<REGION_0_PADDING; i++){
            regionmigfile>>tempID[i];
            regionmigfile>>rate[i];
        }
        regionmigbinfile.write((char*)tempID, REGION_0_PADDING*sizeof(unsigned long int));
        regionmigbinfile.write((char*)rate, REGION_0_PADDING*sizeof(double));
    }

skip:
    if(regionmigfile.is_open()){regionmigfile.close();}
    if(regionmigbinfile.is_open()){regionmigbinfile.close();}
    return;
}
