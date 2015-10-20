# -*- mode: python; -*-
# This Python script, SConscript, invoked by the SConstruct in this directory,
#
# 1. delegates to other per-module SConscript files for executable and library 
# (static and/or dynamic)
import os
import sys
import shutil
import pdb

Import('env')

def InstallEmodules(src, dst):
    
    print "\nInstalling from " + src + " to " + dst + "..."
    if os.path.isfile(dst):
        print "Warning: " + dst + " is a file\n";
        return;

    if os.path.exists(dst) != True:
        print "Creating " + dst + " in " + os.getcwd();
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

print( "Link executable against cajun, campaign, and utils lib's." )
env.Append(LIBS=["cajun", "campaign", "utils"])

#print "builddir is " + env["BUILD_DIR"]

# First static libs
SConscript( [ 'cajun/SConscript',
              'campaign/SConscript',
              'utils/SConscript' ])

# If DLL=true, build libgeneric_static.lib
# to be used by other dlls

# not sure yet exactly right set of conditions for this
if env['AllDlls'] or env['AllInterventions'] or ( 'DiseaseDll' in env and env[ 'DiseaseDll' ] != "" ) or env[ 'Report' ] != "" or env[ 'Campaign' ] != "":
    print "Build libgeneric_static.lib for dll...."
    SConscript( 'libgeneric_static/SConscript' )

# Then build dlls
if env['AllDlls']:
    print "Build all dlls..."
    SConscript( 'libgeneric/VectorSConscriptStatic' )
    SConscript( 'libgeneric/MalariaSConscriptStatic' )
    SConscript( 'libgeneric/EnvironmentalSConscriptStatic' )
    SConscript( 'libgeneric/GenericSConscript' )
    SConscript( 'libgeneric/VectorSConscript' )
    SConscript( 'libgeneric/MalariaSConscript' )
    SConscript( 'libgeneric/EnvironmentalSConscript' )
    SConscript( 'libgeneric/TBSConscriptStatic' )
    SConscript( 'libgeneric/TBSConscript' )
    SConscript( 'libgeneric/STISConscriptStatic' )
    SConscript( 'libgeneric/HIVSConscript' )
    #SConscript( 'libgeneric/PolioSConscript' )
elif env[ 'DiseaseDll' ] != "":
    print( "Build specific disease dll." )
    dtype = env['DiseaseDll']
    if dtype == 'Generic':
        SConscript( 'libgeneric/GenericSConscript', variant_dir="Generic/disease_plugins" )
    elif dtype == 'Vector':
        SConscript( 'libgeneric/VectorSConscriptStatic' )
        SConscript( 'libgeneric/VectorSConscript', variant_dir="Vector/disease_plugins" )
    elif dtype == 'Malaria':
        SConscript( 'libgeneric/VectorSConscriptStatic' )
        SConscript( 'libgeneric/MalariaSConscriptStatic' )
        SConscript( 'libgeneric/MalariaSConscript', variant_dir="Malaria/disease_plugins" )
    elif dtype == 'Environmental':
        SConscript( 'libgeneric/EnvironmentalSConscriptStatic' )
        SConscript( 'libgeneric/EnvironmentalSConscript', variant_dir="Environmental/disease_plugins" )
    elif dtype == 'Polio':
        SConscript( 'libgeneric/EnvironmentalSConscriptStatic' )
        SConscript( 'libgeneric/PolioSConscript', variant_dir="Polio/disease_plugins" )
    elif dtype == 'TB':
        SConscript( 'libgeneric/TBSConscriptStatic' )
        SConscript( 'libgeneric/TBSConscript', variant_dir="TB/disease_plugins" )
    elif dtype == 'STI':
        SConscript( 'libgeneric/STISConscriptStatic' )
        SConscript( 'libgeneric/STISConscript', variant_dir="STI/disease_plugins" )
    elif dtype == 'HIV':
        SConscript( 'libgeneric/STISConscriptStatic' )
        SConscript( 'libgeneric/HIVSConscriptStatic' )
        SConscript( 'libgeneric/HIVSConscript', variant_dir="HIV/disease_plugins" )
    else:
        print "Unspecified or unknown disease type: " + dtype

# intervention dlls
if env['AllDlls'] or env['AllInterventions'] or env[ 'DiseaseDll' ] != "" or env[ 'Report' ]:
    print( "Building dlls." )

    # this vector and malaria static is needed for MalariaDrugTypeParameters 
    # should be cleared out once it is done correctly
    #SConscript( 'libgeneric/VectorSConscriptStatic' )
    #SConscript( 'libgeneric/MalariaSConscriptStatic' )

    dll_op_path = env['DiseaseDll'] + "/interventions"
    # Vector
    if env['DiseaseDll'] == "Vector" or env['DiseaseDll'] == "Malaria":
        SConscript( 'libgeneric/BednetSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/HousingmodSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/HumanhostseekingtrapSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/IndividualrepellentSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/IvermectinSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/MosquitoreleaseSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/ScalelarvalhabitatSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/VcntSConscript', variant_dir=dll_op_path )

    # Malaria
    if env['DiseaseDll'] == "Malaria":
        SConscript( 'libgeneric/AntimalarialdrugSConscript', variant_dir=dll_op_path  )
        SConscript( 'libgeneric/InputEIRSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/MalariaChallengeSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/RTSSVaccineSConscript', variant_dir=dll_op_path )

    # TB
    if env['DiseaseDll'] == "TB":
        SConscript( 'libgeneric/ActivediagnosticsSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/AntitbdrugSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/AntitbpropdepdrugSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/BCGVaccineSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/DiagnosticstreatnegSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/HealthSeekingBehaviorUpdateSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/HealthSeekingBehaviorUpdateableSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/NodeLevelHealthTriggeredIVScaleUpSwitchSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/ResistancediagnosticsSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/SmearDiagnosticsSConscript', variant_dir=dll_op_path )

    if env['DiseaseDll'] == "STI" or env['DiseaseDll'] == "HIV":
        SConscript( 'libgeneric/StibarrierSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/StiispostdebutSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/ModifysticoinfectionstatusSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/SticoinfectiondiagnosticSConscript', variant_dir=dll_op_path )

    if env['DiseaseDll'] == "HIV":
        SConscript( 'libgeneric/ArtbasicSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/ArtdropoutSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/BroadcasteventSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/Cd4diagnosticSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/AgediagnosticSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/Hivartstagingbycd4diagnosticSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/Hivartstagingcd4agnosticdiagnosticSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/HivdelayedinterventionSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/HivdrawbloodSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/HivpiecewisebyyearandsexdiagnosticSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/HivpreartnotificationSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/HivrandomchoiceSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/HivrapidhivdiagnosticSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/HivsetcascadestateSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/HivsigmoidbyyearandsexdiagnosticSConscript', variant_dir=dll_op_path ) 
        SConscript( 'libgeneric/HivsimplediagnosticSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/HivmuxerSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/MalecircumcisionSConscript', variant_dir=dll_op_path )
        SConscript( 'libgeneric/PmtctSConscript', variant_dir=dll_op_path )

    # Polio
    # NOT YET SConscript( 'libgeneric/PoliovaccineSConscript' )
    SConscript( 'libgeneric/BirthtriggeredSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/CalendarSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/DelayedInterventionSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/DiagnosticsSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/HealthseekingbehaviorSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/MultiinterventiondistributorSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/NodeLevelHealthtriggeredSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/ImmunoglobulinSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/ImportPressureSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/OutbreakSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/OutbreakIndividualSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/PropertyvaluechangerSConscript', variant_dir=dll_op_path )
    SConscript( 'libgeneric/SimplevaccineSConscript', variant_dir=dll_op_path )

# report dlls
# NOT YET if env['AllDlls'] or env['Report'] != "":
# NOT YET SConscript( 'libgeneric/TajikSConscript' )

# Finally executable
SConscript('Eradication/SConscript')
