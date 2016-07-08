# -*- mode: python; -*-
# This Python script, SConstruct, requires
# 1. python V2.4 or above. you can get from http://www.python.org
# 2. scons V2.1 or above. you can get from  http://www.scons.org
#
# This file configures the build environment, and then delegates to
# several subordinate SConscript files, which describe specific build rules.
#
# Simply type scons to build everything in DTK
#
#
import datetime
import imp
import os
import re
import shutil
import stat
import sys
import types
import pdb
import platform


#later
#import libdeps

def findSettingsSetup():
    sys.path.append( "." )
    sys.path.append( ".." )
    sys.path.append( "../../" )

msarch = "amd64"

# --- options ---

options = {}

options_topass = {}

def add_option( name, help, nargs, contributesToVariantDir,
                dest=None, default = None, type="string", choices=None ):

    if dest is None:
        dest = name

    AddOption( "--" + name , 
               dest=dest,
               type=type,
               nargs=nargs,
               action="store",
               choices=choices,
               default=default,
               help=help )

    options[name] = { "help" : help ,
                      "nargs" : nargs , 
                      "contributesToVariantDir" : contributesToVariantDir ,
                      "dest" : dest } 

def get_option( name ):
    return GetOption( name )

def set_option( name, value ):
    return SetOption(name, value)

def _has_option( name ):
    x = get_option( name )
    if x is None:
        return False

    if x == False:
        return False

    if x == "":
        return False

    return True

def has_option( name ):
    x = _has_option(name)

    if name not in options_topass:
        # if someone already set this, don't overwrite
        options_topass[name] = x

    return x

def get_variant_dir():
    
    a = []
    
    for name in options:
        o = options[name]
        if not has_option( o["dest"] ):
            continue
        if not o["contributesToVariantDir"]:
            continue
        
        if o["nargs"] == 0:
            a.append( name )
        else:
            x = get_option( name )
            x = re.sub( "[,\\\\/]" , "_" , x )
            a.append( name + "_" + x )
            
    s = "#build/${PYSYSPLATFORM}/"
    if len(a) > 0:
        a.sort()
        s += "/".join( a ) + "/"

    return s

def get_build_var():
    
    bv = ""
    if has_option( "Release" ):
        bv = "Release"
    elif has_option( "Debug" ):
        bv = "Debug"
    else:
        bv = "Release"

    #if has_option( "Dlls" ):
        #bv = bv + "Dll"

    return bv

# General options
add_option( "MSVC" , "Generate Microsoft Visual Studio solution and project files" , 0 , False)

# compiling options
add_option( "Release" , "release build" , 0 , True)
add_option( "Debug" , "debug build" , 0 , True )

# module/linking options
#add_option( "Dlls" , "build all dlls" , 0 , True )
#add_option( "Interventions" , "build all intervention dlls" , 0 , True )
add_option( "DllDisease" , "build disease target dll" , 1 , True) #, Disease="Generic" )
add_option( "Disease" , "build only files for disease target " , 1 , True) #, Disease="Generic" )
add_option( "Report" , "build report target dll" , 1 , True) #, Report="Spatial" )
#add_option( "Campaign" , "build all campaign target dll" , 1 , True) #, Campaign=Bednet
 
# installation options
add_option( "Install" , "install target dll into given directory" , 1 , True) #, Install="install dir" )

# current default is Release
Dbg = has_option( "Debug" )
Rel = has_option( "Release" )

# print "Release = {0}".format(release)
# print "Debug = {0}".format(debug)


# --- environment setup ---

#variantDir = get_variant_dir()

s = "#build/${PYSYSPLATFORM}/"
bvar = get_build_var()
buildDir = s + bvar + "/"

def printLocalInfo():
    import sys, SCons
    print( "scons version: " + SCons.__version__ )
    #print( sys.version_info )
    print( "python version: " + " ".join( [ `i` for i in sys.version_info ] ) )

printLocalInfo()

pa = platform.architecture()
pi = os.sys.platform
if pa[0].find("64") != -1:
    pi = 'x64'
path = os.environ['PATH']
env = Environment( BUILD_DIR=buildDir,
                   DIST_ARCHIVE_SUFFIX='.tgz',
                   MSVS_ARCH=msarch ,
                   TARGET_ARCH=msarch ,
                   PYSYSPLATFORM=pi,
                   MSVSPROJECTSUFFIX='.vcxproj' ,
                   MSVC_VERSION='11.0'
                   )

if not(Dbg) and not(Rel):
    Rel = True
    env["Debug"] = False
    env["Release"] = True
else:
    env["Debug"] = True
    env["Release"] = False

print "Rel=" + str(Rel) + " Dbg=" + str(Dbg)

#print "BUILD_DIR=" + env['BUILD_DIR'] + " pi=" + pi
env['BUILD_VARIANT'] = bvar

#libdeps.setup_environment( env )

env['EXTRACPPPATH'] = []
if os.sys.platform == 'win32':
    if Dbg:
        print( "----------------------------------------------------" )
        print( "Visual Studio debug build not supported using SCONS." )
        print( "Please use the IDE for debugging." )
        print( "----------------------------------------------------" )
        Exit(-1)

    env['OS_FAMILY'] = 'win'
    env.Append( EXTRACPPPATH=[
                          "#/Eradication",
                          "#/interventions",
                          "#/campaign",
                          "#/baseReportLib",
                          "#/utils",
                          "#/libgeneric_static",
                          "#/C:/boost/boost_1_51_0",
                          "#/C:/Python27/Include",
                          "#/Dependencies/ComputeClusterPack/Include",
                          "#/cajun/include",
                          "#/rapidjson/include",
                          "#/rapidjson/modp",
                          "#/snappy",
                          "#/unittest/UnitTest++/src"])
else:
    env['ENV']['PATH'] = path
    env['OS_FAMILY'] = 'posix'
    env['CC'] = "mpicxx"
    env['CXX'] = "mpicxx"
    env.Append( CCFLAGS=["-fpermissive"] )
    env.Append( CCFLAGS=["--std=c++0x"] )
    env.Append( CCFLAGS=["-w"] )
    env.Append( CCFLAGS=["-ffloat-store"] )
    env.Append( CCFLAGS=["-Wno-unknown-pragmas"] )
    #env.Append( CCFLAGS=["-save-temps"] )
    env.Append( EXTRACPPPATH=[
                          "#/Eradication",
                          "#/interventions",
                          "#/campaign",
                          "#/baseReportLib",
                          "#/utils",
                          "#/libgeneric_static",
                          "/usr/include/python2.7/",
                          "#/cajun/include",
                          "#/rapidjson/include",
                          "#/rapidjson/modp",
                          "#/snappy",
                          "#/unittest/UnitTest++/src"])

#if has_option( "cxx" ):
#    env["CC"] = get_option( "cxx" )
#    env["CXX"] = get_option( "cxx" )
#if has_option( "cc" ):
#    env["CC"] = get_option( "cc" )

env["LIBPATH"] = []
#
#if has_option( "libpath" ):
#    env["LIBPATH"] = [get_option( "libpath" )]

env['EXTRALIBPATH'] = []


# ---- other build setup -----

platform = os.sys.platform
if "uname" in dir(os):
    processor = os.uname()[4]
else:
#    processor = "i386"
    processor = "x86_64"

env['PROCESSOR_ARCHITECTURE'] = processor

nixLibPrefix = "lib"

dontReplacePackage = False
isBuildingLatest = False

def findVersion( root , choices ):
    if not isinstance(root, list):
        root = [root]
    for r in root:
        for c in choices:
            if ( os.path.exists( r + c ) ):
                return r + c
    raise RuntimeError("can't find a version of [" + repr(root) + "] choices: " + repr(choices))


# platform independend compile and linker setting
# This has to be added in the static build
#env.Append( CPPDEFINES=["ENABLE_TB" ] )
#env.Append( CPPDEFINES=["ENABLE_POLIO" ] )

if os.sys.platform.startswith("linux"):
    linux = True
    static = True
    platform = "linux"

    env.Append( LIBS=['m'] )

    if os.uname()[4] == "x86_64":
        linux64 = True
        nixLibPrefix = "lib64"
        env.Append( EXTRALIBPATH=["/usr/lib64" , "/lib64" ] )

    env.Append( LIBS=["pthread", "python2.7", "dl" ] ) 
    env.Append( EXTRALIBPATH=[ "/usr/local/lib", "/usr/lib64/mpich/lib" ] )

    if static:
        #env.Append( LINKFLAGS=" -static " )
        env.Append( LINKFLAGS=" -rdynamic " )
    if Dbg:
        env.Append( CCFLAGS=["-O0", "-g"] )
        env.Append( CPPDEFINES=["_DEBUG"] )
    else:
        env.Append( CCFLAGS=["-O3"] )
        

elif "win32" == os.sys.platform:
    windows = True

    env['DIST_ARCHIVE_SUFFIX'] = '.zip'

    env.Append( CPPDEFINES=[ "WIN32" ] )
    env.Append( CPPDEFINES=[ "_UNICODE" ] )
    env.Append( CPPDEFINES=[ "UNICODE" ] )
    env.Append( CPPDEFINES=[ "BOOST_ALL_NO_LIB" ] )

    # this is for MSVC <= 10.0
    #winSDKHome = findVersion( [ "C:/Program Files/Microsoft SDKs/Windows/", "C:/Program Files (x86)/Microsoft SDKs/Windows/" ] , [ "v7.0A", "v7.0"] )
    #env.Append( EXTRACPPPATH=[ winSDKHome + "/Include" ] )
    #env.Append( EXTRALIBPATH=[ winSDKHome + "/Lib/x64" ] )

    # this is for MSVC >= 11.0
    winSDKHome = "C:/Program Files (x86)/Windows Kits/8.0/"
    env.Append( EXTRACPPPATH=[ winSDKHome + "/Include/um" ] )
    env.Append( EXTRALIBPATH=[ winSDKHome + "Lib/win8/um/x64" ] )
    env.Append( EXTRALIBPATH=[ "C:/Python27/libs" ] )
    env.Append( EXTRALIBPATH=[ "#/Dependencies/ComputeClusterPack/Lib/amd64" ] )

    print( "Windows SDK Root '" + winSDKHome + "'" )

    #print( "Windows MSVC Root '" + winVCHome + "'" )
    #env.Append( EXTRACPPPATH=[ winVCHome + "/Include" ] )

    # /EHsc exception handling style for visual studio
    # /W3 warning level
    # /WX abort build on compiler warnings
    env.Append(CCFLAGS=["/EHsc","/W3"])

    # /bigobj for an object file bigger than 64K
    # DMB It is in the VS build parameters but it doesn't show up in the Command Line view
    #env.Append(CCFLAGS=["/bigobj"])

    # some warnings we don't like:
    # c4355
    # 'this' : used in base member initializer list
    #    The this pointer is valid only within nonstatic member functions. It cannot be used in the initializer list for a base class.
    # c4800
    # 'type' : forcing value to bool 'true' or 'false' (performance warning)
    #    This warning is generated when a value that is not bool is assigned or coerced into type bool. 
    # c4267
    # 'var' : conversion from 'size_t' to 'type', possible loss of data
    # When compiling with /Wp64, or when compiling on a 64-bit operating system, type is 32 bits but size_t is 64 bits when compiling for 64-bit targets. To fix this warning, use size_t instead of a type.
    # c4244
    # 'conversion' conversion from 'type1' to 'type2', possible loss of data
    #  An integer type is converted to a smaller integer type.
    
    # PSAPI_VERSION relates to process api dll Psapi.dll.
    env.Append( CPPDEFINES=["_CONSOLE"] )

    # this would be for pre-compiled headers, could play with it later  
    #env.Append( CCFLAGS=['/Yu"pch.h"'] )

    # docs say don't use /FD from command line (minimal rebuild)
    # /Gy function level linking (implicit when using /Z7)
    # /Z7 debug info goes into each individual .obj file -- no .pdb created 
    # /Zi debug info goes into a PDB file
    env.Append( CCFLAGS= ["/errorReport:none"] )

    # Not including debug information for SCONS builds
    env.Append( CCFLAGS=["/fp:strict", "/GS-", "/Oi", "/Ot", "/Zc:forScope", "/Zc:wchar_t" ])

    env.Append( CCFLAGS=["/DIDM_EXPORT"] )
    env.Append( LIBS=["python27.lib"] )

    # /MD  : Causes your application to use the multithread, dll version of the run-time library (LIBCMT.lib)
    # /MT  : use static lib
    # /O2  : optimize for speed (as opposed to size)
    # /MP  : build with multiple processes
    # /Gm- : No minimal build
    # /WX- : Do NOT treat warnings as errors
    # /Gd  : the default setting, specifies the __cdecl calling convention for all functions
    env.Append( CCFLAGS= ["/O2", "/MD", "/MP", "/Gm-", "/WX-", "/Gd" ] )
    env.Append( CPPDEFINES= ["NDEBUG"] )

    # Disable these two for faster generation of codes
    #env.Append( CCFLAGS= ["/GL"] ) # /GL whole program optimization
    #env.Append( LINKFLAGS=" /LTCG " )         # /LTCG link time code generation
    #env.Append( ARFLAGS=" /LTCG " ) # for the Library Manager
    # /DEBUG will tell the linker to create a .pdb file
    # which WinDbg and Visual Studio will use to resolve
    # symbols if you want to debug a release-mode image.
    # Note that this means we can't do parallel links in the build.
    # NOTE: /DEBUG and Dbghelp.lib go together with changes in Exception.cpp which adds
    #       the ability to print a stack trace.
    env.Append( LINKFLAGS=" /DEBUG " )
    # For MSVC <= 10.0
    #env.Append( LINKFLAGS=[ "/NODEFAULTLIB:LIBCPMT", "/NODEFAULTLIB:LIBCMT", "/MACHINE:X64"] )
        
    # For MSVC >= 11.0
    # /OPT:REF : eliminates functions and data that are never referenced
    # /OPT:ICF : to perform identical COMDAT folding
    # /DYNAMICBASE:NO : Don't Use address space layout randomization
    # /SUBSYSTEM:CONSOLE : Win32 character-mode application.
    env.Append( LINKFLAGS=[ "/MACHINE:X64", "/MANIFEST", "/HEAP:\"100000000\"\",100000000\" ", "/OPT:REF", "/OPT:ICF ", "/DYNAMICBASE:NO", "/SUBSYSTEM:CONSOLE"] )

    # This causes problems with the report DLLs.  Don't have time right now to figure out
    # how to remove flags from the DLL build.
    #env.Append( LINKFLAGS=[ "/STACK:\"100000000\"\",100000000\"" ] )

    winLibString = "Dbghelp.lib psapi.lib ws2_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib"

    winLibString += ""

    env.Append( LIBS=Split(winLibString) )

    env.Append( LIBS=["msmpi.lib"] )

else:
    print( "No special config for [" + os.sys.platform + "] which probably means it won't work" )

env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME'] = 1

env.Append( CPPPATH=['$EXTRACPPPATH'] )
env.Append( LIBPATH=['$EXTRALIBPATH'] )

#print env['EXTRACPPPATH']

# --- check system ---

def doConfigure(myenv):
    conf = Configure(myenv)

    if 'CheckCXX' in dir( conf ):
        if  not conf.CheckCXX():
            print( "c++ compiler test failed!" )
            print( "This sometimes happens even though the compiler is fine and can be resolved by performing a 'scons -c' followed by manually removing the .sconf_temp folder and .sconsign.dblite. It can also be because mpich_devel is not installed." )
            Exit(1)
            
    return conf.Finish()


def setEnvAttrs(myenv):

    diseasedlls = ['Generic', 'Vector', 'Malaria', 'Environmental', 'TB', "STI", "HIV" ]
    diseases = ['Generic', 'Vector', 'Malaria', 'Polio', 'TB', 'STI', 'HIV', 'Py' ]
    reportdlls = ['Spatial', 'Binned']
    campaigndlls = ['Bednet', 'IRSHousing']

    myenv['AllDlls'] = False
    dlldisease = has_option('DllDisease')
    dllreport = has_option('Report')
    #dllcampaign = has_option('Campaign')
    monodisease = has_option('Disease')

    #if has_option('Dlls'):
        #myenv['AllDlls'] = True

    myenv['AllInterventions'] = False
    #if has_option('Interventions'):
    #    myenv['AllInterventions'] = True

    #if has_option('Dlls') or dlldisease or dllreport or dllcampaign:
    if dlldisease or dllreport:
        myenv.Append( CPPDEFINES=["_DLLS_" ] )

    if dlldisease:
        myenv['DiseaseDll'] = get_option( 'DllDisease' ) # careful, tricky
        print "DiseaseDll=" + myenv['DiseaseDll']
        if myenv['DiseaseDll'] not in diseasedlls:
            print "Unknown disease (EMODule) type: " + myenv['DiseaseDll']
            exit(1)
    else:
        myenv['DiseaseDll'] = ""

    if monodisease:
        myenv['Disease'] = get_option( 'Disease' )
        print "Disease=" + myenv['Disease']
        if myenv['Disease'] not in diseases:
            print "Unknown disease type: " + myenv['Disease']
            exit(1)
    else:
        myenv['Disease'] = ""

    if dllreport:
        myenv['Report'] = get_option( 'Report' )
        print "Report=" + myenv['Report']
        if myenv['Report'] not in reportdlls:
            print "Unknown report type: " + myenv['Report']
            exit(1)
    else:
        myenv['Report'] = ""

    #if dllcampaign:
    #    myenv['Campaign'] = get_option( 'Campaign' )
    #    print "Campaign=" + myenv['Campaign']
    #    if myenv['Campaign'] not in campaigndlls:
    #        print "Unknown campaign type: " + myenv['Campaign']
    #        exit(1)
    #else:
    #    myenv['Campaign'] = ""
    
    if has_option('Install'):
        myenv['Install'] = get_option( 'Install' )
    else:
        myenv['Install'] = ""

    print "DLL=" + str(myenv['AllDlls'])
    print "Install=" + myenv['Install']

# Main starting  point
env = doConfigure( env )

# set evn based on cmdline options
setEnvAttrs( env )

# Export the following symbols for them to be used in subordinate SConscript files.
Export("env")


# pass the build_dir as the variant directory
env.SConscript( 'SConscript', variant_dir='$BUILD_DIR', duplicate=False )

env.SConscript( 'MSVCSConscript', duplicate=False )

#env.SConscript( 'unittest/SConscript', variant_dir='$BUILD_DIR/unittest', duplicate=False )

