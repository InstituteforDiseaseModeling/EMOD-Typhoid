##Simple GIT Commands:
     1. download dtk master code set
         git clone https://github.com/InstituteforDiseaseModeling/DtkTrunk
     2. switch to the IHME branch (ensure you use caps for IHME)
         git checkout IHME
     3. ensure you're on the IHME branch
         git branch         
         * IHME           '*' indicated you are on the IHME branch
           master
           
           
##Software Documentation:
    http://idmod.org/idmdoc/
    
##Targeted Linux Enviornment:
    CentOS Linux release 7.1.1503 (Core)
    SVN (subversion.x86_64, 1.8.11-1)
    Python:
    python.x86_64, 2.7.5-16.el7
    python-devel.x86_64, 2.7.5-16.el7
    pip 6.1.1 from /usr/lib/python2.7/site-packages (python 2.7)
    numpy.x86_64, 1:1.7.1-10.el7
    python-matplotlib.x86_64 0:1.2.0-15.el7
    Boost:
    boost.x86_64, 1.53.0-23.el7
    boost-mpich.x86_64, 1.53.0-23.el7
    boost-mpich-devel.x86_64, 1.53.0-23.el7
    mpich:
    mpich.x86_64, 3.0.4-8.el7
    mpich-devel.x86_64, 3.0.4-8.el7
    gcc-c++ (gcc-c++.x86_64, 4.8.3-9.el7)
    SCons (scons.noarch, 2.3.4-1)
   

##Installation of key software components:
    sudo yum install python-2.7.5
    sudo yum install python-devel-2.7.5
    sudo curl -O https://bootstrap.pypa.io/get-pip.py
    sudo python get-pip.py
    sudo yum install numpy-1.7.1
    sudo yum install python-matplotlib
     
    sudo yum install boost-1.53.0
    sudo yum install boost-mpich-1.53.0
    sudo yum install boost-mpich-devel-1.53.0
     
    sudo yum install mpich-3.0.4
    sudo yum install mpich-devel-3.0.4-8
     
    sudo yum install gcc-c++-4.8.3
     
    sudo yum install scons.noarch-2.3.4
     
    export PATH=$PATH:/usr/lib/mpich/bin/

##Building with scons:
    Go to the root of the git branch.
    
    To build
       scons --jobs=4

    To clean
      scons --clean

    Build Release
       scons --Release --jobs==4
       
     Build Debug
       scons --Debug --jobs==4
   

##Executable Location:
    ../build/x64/Release/Eradication      
    ../build/x64/Debug/Eradication

##Running regressions:
    
    Note: you need the set of input files to run the regressions. Ensure
          the files are in a directory that you will identify for 
          local_input_root in the configuration.
    
    EDIT the Regression/regression_test.cfg
        vi regression_test.cfg
        
        1. edit the ENVIRONMENT section modifying the input_root to match the POSIX section local_input_root

        
        [ENVIRONMENT]
        input_root = /geneShare/Eradication/emod_input/

        2. change the three locations to a path accessible with your account permissions
        
        [POSIX]
        local_sim_root = /geneShare/Eradication/simulations/
        local_input_root = /geneShare/Eradication/emod_input/
        local_bin_root = /geneShare/Eradication/bin/

    Try a few regressions:   
         
         cd to the Regresssion directory
         
         python ./regression_test.py --config-constraints=Num_Cores:1 27 ../build/x64/Release/Eradication/Eradication
                        
         python ./regression_test.py --config-constraints=Num_Cores:1 vector ../build/x64/Release/Eradication/Eradication
          will output on stdout indicating it passed
                   .
                   .
                   .
            14 out of 25 completed.
            54_Vector_Release_Wolbachia passed (0:00:02.063240) - InsetChart.linux.json
            15 out of 25 completed.
            58_Vector_Release_HEGs_multipuddle passed (0:00:01.669593) - InsetChart.linux.json
            16 out of 25 completed.
            27_Vector_Sandbox passed (0:00:05.624805) - InsetChart.linux.json
            17 out of 25 completed.
            56_Vector_InsectKillingFence passed (0:00:13.087057) - InsetChart.linux.json
                   .
                   .
                   .

