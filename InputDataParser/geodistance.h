/***************************************************************************************************

Copyright (c) 2016 Intellectual Ventures Property Holdings, LLC (IVPH) All rights reserved.

EMOD is licensed under the Creative Commons Attribution-Noncommercial-ShareAlike 4.0 License.
To view a copy of this license, visit https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode

***************************************************************************************************/

/* 
Added by Hao Hu, 5/4/2010
Calculate the Geographic Distance giving the latitude and longitude data of two points on earth. Adopted from java code at
http://www.movable-type.co.uk/scripts/latlong-vincenty.html
http://www.movable-type.co.uk/scripts/LatLongVincentyDirect.html
Using Vincenty inverse formula.
*/
#include <float.h>

//calculating distance between two positions, result in meters
double GeoDist(double lat1, double lon1, double lat2, double lon2)
{
    //convert to radians
    lat1 = lat1/57.29577951;
    lon1 = lon1/57.29577951;
    lat2 = lat2/57.29577951;
    lon2 = lon2/57.29577951;

    //WGS-84 ellipsiod
    double a=6378137;
    double b=6356752.3142;
    double f=1/298.257223563;

    double L=lon2 - lon1;

    double U1=atan((1-f)*tan(lat1));
    double U2=atan((1-f)*tan(lat2));

    double sinU1 = sin(U1);
    double cosU1 = cos(U1);
    double sinU2 = sin(U2);
    double cosU2 = cos(U2);

    double lambda = L;
    double lambdaP = 2 * 3.1415926;

    //used in while iterations
    double sinlambda;
    double coslambda;
    double sinsigma;
    double cossigma;
    double sinalpha;
    double cossqalpha;
    double cos2sigmam;
    double C;
    double sigma;

    int iterlimit=20;

    while(abs(lambda - lambdaP) > 1e-12 && --iterlimit>0)
    {
        sinlambda = sin(lambda);
        coslambda = cos(lambda);
        sinsigma = sqrt((cosU2*sinlambda) * (cosU2*sinlambda) + (cosU1*sinU2-sinU1*cosU2*coslambda) * (cosU1*sinU2 - sinU1*cosU2*coslambda));
        if (sinsigma == 0) return 0; //co-incident points
        cossigma = sinU1*sinU2 + cosU1 * cosU2 * coslambda;
        sigma = atan2(sinsigma, cossigma);
        sinalpha = cosU1 * cosU2 * sinlambda / sinsigma;
        cossqalpha = 1 - sinalpha * sinalpha;
        cos2sigmam = cossigma - 2*sinU1*sinU2/cossqalpha;
        if (_isnan(cos2sigmam)) cos2sigmam=0; //equatorial line: cossqalpha=0
        C = f/16 * cossqalpha * (4+f*(4-3*cossqalpha));
        lambdaP = lambda;
        lambda = L + (1-C) * f * sinalpha * (sigma + C*sinsigma*(cos2sigmam+C*cossigma*(-1+2*cos2sigmam*cos2sigmam)));
    }

    if (iterlimit == 0) 
    {
        std::cout << "Vincenty's Formula failed to converge, exit!" << std::endl;
        exit(1);
    }

    double uSq = cossqalpha * (a*a - b*b)/(b*b);
    double A = 1 + uSq / 16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
    double B = uSq / 1024 * (256 + uSq * (-128 + uSq * (74-47 * uSq)));
    double deltasigma = B * sinsigma * (cos2sigmam + B/4 * (cossigma * (-1 + 2*cos2sigmam * cos2sigmam) - B/6*cos2sigmam * (-3+4*sinsigma*sinsigma)*(-3+4*cos2sigmam*cos2sigmam)));
    double s = b*A*(sigma-deltasigma);
    return s;
}

//calculate the new position, based on the existing position with a certain distance and angle
//use pointers to return both latitude and longitude
void GeoDist2(double *lat1, double *lon1, double dist, double angle)
{
    //convert to radians
    double reslat2 = *lat1/57.29577951;
    double reslon2 = *lon1/57.29577951;

    double a=6378137;
    double b=6356752.3142;
    double f=1/298.257223563;

    double s=dist;
    double alpha1 = angle/57.29577951;
    double sinAlpha1 = sin(alpha1);
    double cosAlpha1 = cos(alpha1);

    double tanU1 = (1-f) * tan(reslat2);
    double cosU1 = 1 / sqrt(1+tanU1*tanU1);
    double sinU1 = tanU1 * cosU1;
    double sigma1 = atan2(tanU1, cosAlpha1);
    double sinAlpha = cosU1 * sinAlpha1;
    double cosSqAlpha = 1 - sinAlpha * sinAlpha;
    double uSq = cosSqAlpha * (a*a-b*b)/(b*b);
    double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
    double B = uSq / 1024 * (256 + uSq * (-128+uSq*(74-47*uSq)));
    double sigma = s/(b*A);
    double sigmaP = 2*3.1415926;

    //used in while iterations
    double cos2SigmaM;
    double sinSigma;
    double cosSigma;
    double deltaSigma;

    while (abs(sigma-sigmaP)>1e-12)
    {
        cos2SigmaM = cos(2*sigma1 + sigma);
        sinSigma = sin(sigma);
        cosSigma = cos(sigma);
        deltaSigma = B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
        sigmaP=sigma;
        sigma=s/(b*A)+deltaSigma;
    }

    double tmp = sinU1*sinSigma - cosU1*cosSigma*cosAlpha1;
    double lat2 = atan2(sinU1*cosSigma + cosU1*sinSigma*cosAlpha1, (1-f)*sqrt(sinAlpha*sinAlpha + tmp*tmp));
    double lambda = atan2(sinSigma*sinAlpha1, cosU1*cosSigma - sinU1*sinSigma*cosAlpha1);
    double C=(f/16)*cosSqAlpha*(4+f*(4-3*cosSqAlpha));
    double L=lambda-(1-C)*f*sinAlpha*(sigma+C*sinSigma*(cos2SigmaM+C*cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)));
    double revAz=atan2(sinAlpha, -tmp);
    reslat2=lat2*180/3.1415926;
    reslon2=(reslon2+L)*180/3.1415926;
    *lat1=reslat2;
    *lon1=reslon2;
//        printf("lat=%lf, lon=%lf\n",*lat1,*lon1);
}
