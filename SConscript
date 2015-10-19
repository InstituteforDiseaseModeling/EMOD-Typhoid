# -*- mode: python; -*-
# This Python script, SConscript, invoked by the SConstruct in this directory,
#
# 1. delegates to other per-module SConscript files for executable and library 
# (static and/or dynamic)
import os
import sys
import shutil

Import('env')

def InstallEmodules(src, dst):
    
    print "\nInstalling from " + src + " to " + dst + "..."
    if os.path.isfile(dst):
        print "Warning: " + dst + " is a file\n";
        return;

    if os.path.exists(dst) != True:
        print "Creating " + dst;
        os.mkdir(dst)

    srcfiles = os.path.join(src,'*.dll')
    for root, dirs, files in os.walk(src):
        for file in files:
            if file.endswith('.dll') or file.endswith('.exe'):
                full_fn = os.path.join(root,file);
                print "copying: " + full_fn;
                shutil.copy2(full_fn, dst);


        
    
# if --install is on, just copy the dlls (assumed there already) and finish
dst_path = env['Install']
if dst_path != "":
    InstallEmodules(Dir('.').abspath, dst_path)
    #InstallEmodules(Dir('#').abspath, dst_path)
    print("Finished installing.\n")
    sys.exit(0)

# set the common libraries
env.Append(LIBPATH = ["$BUILD_DIR/cajun", "$BUILD_DIR/campaign", "$BUILD_DIR/utils"])

env.Append(LIBS=["cajun", "campaign", "utils"])

#print "builddir is " + env["BUILD_DIR"]

# First static libs
SConscript( [ 'cajun/SConscript',
              'campaign/SConscript',
              'utils/SConscript' ])

# If DLL=true, build libgeneric_static.lib
# to be used by other dlls

if env['AllDlls'] or ( 'DllDisease' in env and env[ 'DllDisease' ] !="" ) or env[ 'Report' ] != "" or env[ 'Campaign' ] != "":
    print "Build libgeneric_static.lib for dll...."
    SConscript( 'libgeneric_static/SConscript' )

# Then build dlls

if env['AllDlls']:

    print "Build all dlls..."
    SConscript( 'libgeneric/VectorSConscriptStatic' )
    SConscript( 'libgeneric/MalariaSConscriptStatic' )
    # NOT YET SConscript( 'libgeneric/EnvironmentalSConscriptStatic' )
    SConscript( 'libgeneric/GenericSConscript' )
    SConscript( 'libgeneric/VectorSConscript' )
    SConscript( 'libgeneric/MalariaSConscript' )
    # NOT YET SConscript( 'libgeneric/EnvironmentalSConscript' )
    #SConscript( 'libgeneric/PolioSConscript' )
elif env[ 'DiseaseDll' ] != "":
    dtype = env['Disease']
    if dtype == 'Generic':
        SConscript( 'libgeneric/GenericSConscript' )
    elif dtype == 'Vector':
        SConscript( 'libgeneric/VectorSConscriptStatic' )
        SConscript( 'libgeneric/VectorSConscript' )
    elif dtype == 'Malaria':
        SConscript( 'libgeneric/VectorSConscriptStatic' )
        SConscript( 'libgeneric/MalariaSConscriptStatic' )
        SConscript( 'libgeneric/MalariaSConscript' )
    elif dtype == 'Environmental':
        SConscript( 'libgeneric/EnvironmentalSConscriptStatic' )
        SConscript( 'libgeneric/EnvironmentalSConscript' )
    elif dtype == 'Polio':
        SConscript( 'libgeneric/EnvironmentalSConscriptStatic' )
        SConscript( 'libgeneric/PolioSConscript' )
    else:
        print "Unspecified or unknown disease type: " + dtype

# intervention dlls
if env['AllDlls'] or env['AllInterventions'] or env[ 'Disease' ] != "" or env[ 'Report' ]:

    # this vector and malaria static is needed for MalariaDrugTypeParameters 
    # should be cleared out once it is done correctly
    SConscript( 'libgeneric/VectorSConscriptStatic' )
    SConscript( 'libgeneric/MalariaSConscriptStatic' )


    SConscript( 'libgeneric/AntimalarialdrugSConscript' )
    # NOT YET SConscript( 'libgeneric/AntitbdrugSConscript' )
    # NOT YET SConscript( 'libgeneric/BCGVaccineSConscript' )
    SConscript( 'libgeneric/BednetSConscript' )
    SConscript( 'libgeneric/BirthtriggeredSConscript' )
    SConscript( 'libgeneric/CalendarSConscript' )
    SConscript( 'libgeneric/DiagnosticsSConscript' )
    SConscript( 'libgeneric/HealthseekingbehaviorSConscript' )
    SConscript( 'libgeneric/HealthtriggeredSConscript' )
    SConscript( 'libgeneric/HousingmodSConscript' )
    SConscript( 'libgeneric/HumanhostseekingtrapSConscript' )
    SConscript( 'libgeneric/ImmunoglobulinSConscript' )
    SConscript( 'libgeneric/OutbreakSConscript' )
    # NOT YET SConscript( 'libgeneric/PoliovaccineSConscript' )
    SConscript( 'libgeneric/SimplevaccineSConscript' )
    SConscript( 'libgeneric/VcntSConscript' )

# report dlls
# NOT YET if env['AllDlls'] or env['Report'] != "":
# NOT YET SConscript( 'libgeneric/TajikSConscript' )
    
# Finally executable
SConscript('Eradication/SConscript')
