from distutils.core import setup, Extension

import os

compiler_args = None
extralibs = []

if os.name == "posix":
    os.environ['CC'] = 'g++'
    os.environ['CXX'] = 'g++'
    os.environ['CPP'] = 'g++'
    compiler_args = [ "-std=c++11", "-w", "-fpermissive" ]
else:
    extralibs = [ "ws2_32", "DbgHelp" ]
    compiler_args = [ 
        "/c", "/TP", "/nologo", "/EHsc", "/W3", "/bigobj", "/errorReport:none", "/fp:strict", "/GS-", "/Oi", "/Ot", "/Zc:forScope", "/Zc:wchar_t", "/Z7", "/DIDM_EXPORT", "/O2", "/MD", "/DWIN32", "/D_UNICODE", "/DUNICODE", "/DBOOST_ALL_NO_LIB"
        ]

module1 = Extension('dtk_mathfunc',
        sources = [
            'dtk_mathfunc_module.cpp',
            '../../utils/MathFunctions.cpp',
            '../../utils/Environment.cpp'
        ],
        extra_compile_args = compiler_args,
        include_dirs=[
            '../../interventions/',
            '../../campaign/',
            '../../utils/', 
            '../../cajun/include/', 
            '../../rapidjson/include/', 
            '../../Eradication/', 
            'C:/boost/boost_1_51_0',
            ],
        libraries = ['utils', 'campaign', 'cajun' ] + extralibs,
        library_dirs = [
            '../../build/x64/Release/utils/',
            '../../build/x64/Release/campaign/',
            '../../build/x64/Release/cajun/',
            ]
        )

setup (name = 'PackageName',
        version = '1.0',
        description = 'This is a demo package',
        ext_modules = [module1])
