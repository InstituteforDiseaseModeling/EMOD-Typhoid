  ___           _   _ _         _          __
 |_ _|_ __  ___| |_(_) |_ _   _| |_ ___   / _| ___  _ __
  | || '_ \/ __| __| | __| | | | __/ _ \ | |_ / _ \| '__|
  | || | | \__ \ |_| | |_| |_| | ||  __/ |  _| (_) | |
 |___|_|_|_|___/\__|_|\__|\__,_|\__\___|_|_|  \___/|_|      _ _
 |  _ \(_)___  ___  __ _ ___  ___  |  \/  | ___   __| | ___| (_)_ __   __ _
 | | | | / __|/ _ \/ _\ / __|/ _ \ | |\/| |/ _ \ / _\ |/ _ \ | | '_ \ / _\ |
 | |_| | \__ \  __/ (_| \__ \  __/ | |  | | (_) | (_| |  __/ | | | | | (_| |
 |____/|_|___/\___|\__,_|___/\___| |_|  |_|\___/ \__,_|\___|_|_|_| |_|\__, |
                                                                      |___/

Coupled with the PrepareLinuxEnvironment.sh script.

This document is best read using a monotype.

This readme.txt file outlines the flow of the EMODprepare.sh script.  This
script is designed and tested to run on an Azure CentOS 7 virtual machine
with the sole purpose of readying the environment for the Institute for
Disease Modeling EMOD software.

It will...
  1. Ask the user to elevate their permissions through sudo. (exit)
  2. Update the system base packages. (optional)
  3. Check for dependency packages, prompting to install them. (exit)
  4. Add necessary environment variables. (optional)
  5. Prompt to download the EMOD software (optional)
    a. Create a directory to copy the EMOD software source into.
    b. Prompt for GitHub permissions.
    c. Download the source, including the large files using lfs support.

To run the script, chmod to 755 and execute the following at the command
prompt:

     ./EMODprepare.sh

The user may enter a test mode.  This will run through the script workflow,
but make no permanent system changes or download any files.

     ./EMODprepare.sh test

The source is documented inside, but the program flow is as follows:

                      +-------+
                      | Start |
                      +-------+
                          |
                  +----------------+
                  | Display Banner |
                  +----------------+
                          |
                  +----------------+
                  | CentOS 7 check |->--no-----------------------+
                  +----------------+                             |
                          |                                      |
                         yes                                     |
                          |                                      |
                  +-----------------+                            |
                  | Prompt to begin |->--no----------------------+
                  +-----------------+                            |
                          |                                      |
                         yes                                     |
                          |                                      |
              +------------------------+                         |
              | Enter sudo credentials |->--no/fail--------------+
              +------------------------+                         |
                          |                                      |
                         yes                                     |
                          |                                      |
                 +------------------+                            |
                 | Prompt to update |->--no--------------+       |
                 +------------------+                    |       |
                          |                           warning    |
                         yes                             |       |
                          |                              |       |
                          +<-----------------------------+       |
                          |                                      |
         +----------------------------------+                    |
         | Check for EMOD required packages |                    |
         +----------------------------------+                    |
                          |                                      |
     +------------------------------------------+                |
     | Prompt to install packages and libraries |->--no--+       |
     +------------------------------------------+        |       |
                          |                              |       |
                         yes                             |       |
                          |                              |       |
          +-------------------------------+              |       |
          | Add git-lfs repository        |              |       |
          | Upgrade pip                   |              |       |
          | yum -y install [package name] |              |       |
          | pip install [library name]    |              |       |
          +-------------------------------+              |       |
                          |                              |       |
                          +<-----------------------------+       |
                          |                                      |
         +----------------------------------+                    |
         | Prompt to download EMOD software |->--no----------+   |
         +----------------------------------+                |   |
                          |                                  |   |
                         yes                                 |   |
                          |                                  |   |
                          +<-----------------------------+   |   |
                          |                              |   |   |
        +--------------------------------------+         |   |   |
        | Prompt and create a source directory |->--fail-+   |   |
        +--------------------------------------+             |   |
                          |                                  |   |
                       created                               |   |
                          |                                  |   |
   +------------------------------------------------+        |   |
   | Download EMOD software from GitHub (git clone) |        |   |
   | LFS fetch the input files                      |        |   |
   +------------------------------------------------+        |   |
                          |                                  |   |
                          +<---------------------------------+   |
                          |                                      |
          +---------------------------------+                    |
          | Check for environment variables |                    |
          +---------------------------------+                    |
                          |                                      |
         +-----------------------------------+                   |
         | Prompt to update the .bashrc file |->--no-----+       |
         +-----------------------------------+           |       |
                          |                              |       |
                         yes                             |       |
                          |                              |       |
             +-------------------------+                 |       |
             | Update the .bashrc file |                 |       |
             +-------------------------+                 |       |
                          |                              |       |
                          +<-----------------------------+       |
                          |                                      |
              +-----------------------+                          |
              | Present final message |                          |
              +-----------------------+                          |
                          |                                      |
                          +<-------------------------------------+
                          |
                       +------+
                       | Done |
                       +------+

[end of the readme.txt file]
