from distutils.core import setup, Extension

import os
os.environ['CC'] = 'g++'
os.environ['CXX'] = 'g++'
os.environ['CPP'] = 'g++'

module1 = Extension('dtk_mathfunc',
        sources = [
            'dtk_mathfunc_module.c',
            '../../utils/MathFunctions.cpp',
            '../../utils/Environment.cpp'
        ],
        extra_compile_args = ["-std=c++11", "-w", "-fpermissive" ], 
        include_dirs=[
            '../../interventions/',
            '../../campaign/',
            '../../utils/', 
            '../../cajun/include/', 
            '../../rapidjson/include/', 
            '../../Eradication/', 
            ],
        libraries = ['utils', 'campaign', 'cajun' ],
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
