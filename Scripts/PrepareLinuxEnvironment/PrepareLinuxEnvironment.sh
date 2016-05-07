#!/bin/bash

# Variable declaration section.
FontReset="\x1b[39;49;00m"
FontRed="\x1b[31;01m"
FontGreen="\x1b[32;01m"
FontBlink="\e[5;32;40m"
FontYellow="\x1b[33;01m"
FontBlue="\x1b[34;01m"
FontMagenta="\x1b[35;01m"
FontCyan="\x1b[36;01m"
OSTypeCheck=$(cat /etc/*-release | grep 'ID="centos"')
OSVersionCheck=$(cat /etc/*-release | grep 'VERSION_ID="7"')
declare -a EMODPackageRequired=("wget" "curl" "epel-release" "python-pip" "python-devel" "gcc-c++" "numpy" "scons" "python-matplotlib" "mpich" "mpich-devel" "boost-mpich" "boost-mpich-devel" "git" "xorg-x11-xauth" "git-lfs" "PyYAML")
declare -a EMODPythonLibraryRequired=("numpy" "xlrd")
EMODGitHubURL="https://github.com/InstituteforDiseaseModeling/"
declare -a EMODSoftware=("EMOD" "EMOD-InputData")
WarningMessage="\n${FontRed}Warning: $FontReset"
IDMSupportEmail="idm-support@intven.com"
LineBreak="${FontGreen}********************************************************************************$FontReset\n"

# Clear the screen for better presentation.
stty sane
clear

# Display an informational banner if the "test" argument is passed.
# This allows a user to check the script flow while disabling functionality that would modify the environment.
TestState=0
if [ $1 ] && [ $1 == "test" ]
then
  TestState=1
  printf "${FontRed}********************************************************************************
* TEST STATE ENABLED. Permanent system changes will not occur.                 *
********************************************************************************${FontReset}\n"
fi

# Change to the user's home directory and present an IDM banner to the user.
cd ~
printf "
  ___           _   _ _         _          __                               
 |_ _|_ __  ___| |_(_) |_ _   _| |_ ___   / _| ___  _ __                    
  | || '_ \/ __| __| | __| | | | __/ _ \ | |_ / _ \| '__|                   
  | || | | \__ \ |_| | |_| |_| | ||  __/ |  _| (_) | |                      
 |___|_|_|_|___/\__|_|\__|\__,_|\__\___|_|_|  \___/|_|      _ _             
 |  _ \(_)___  ___  __ _ ___  ___  |  \/  | ___   __| | ___| (_)_ __   __ _ 
 | | | | / __|/ _ \/ _\` / __|/ _ \ | |\/| |/ _ \ / _\` |/ _ \ | | '_ \ / _\` |
 | |_| | \__ \  __/ (_| \__ \  __/ | |  | | (_) | (_| |  __/ | | | | | (_| |
 |____/|_|___/\___|\__,_|___/\___| |_|  |_|\___/ \__,_|\___|_|_|_| |_|\__, |
                                                                      |___/ 


"

# Eject the user if they attempt to run the script as root.
if [ "$EUID" -eq 0 ]
then
  printf "${WarningMessage}This script should not be executed as root.  Exiting.\n\n"
  exit 0
fi

# Display a welcome message to the user.
printf "Welcome!  Inspired by a collaborative and multidisciplinary effort from the scientific community, IDM's innovative EMOD software tools provide a qualitative and analytical means to model infectious disease.  Our software source and input data files are provided to the scientific community to accelerate the exploration of disease eradication through the use of computational modeling.

The purpose of this script is to ready a CentOS 7 machine for EMOD software."

# Check the user's version, displaying a message and exiting the script if not found to be supported.
if [ ${#OSTypeCheck} -eq 0 ] || [ ${#OSVersionCheck} -eq 0 ]
then
  printf "\n${WarningMessage}CentOS 7 is currently the only version supported by this script and the EMOD software.  This script cannot proceed further.  Exiting.\n\n"
  exit 0
fi

# Provide an imformational message about the script's process to the user.
printf "\n\nThe script will:
1. Attempt to update your system.
2. Install packages needed by the EMOD software.
3. Modify your \$PATH variable.
4. Download the EMOD software from our GitHub repository.

You'll need:
1. sudo privileges to install packages.
2. 6GB free in your home directory.
3. Your computer connected to the Internet.
4. A GitHub account with permissions to download the EMOD software.  Contact ${IDMSupportEmail} for details.

If you're not ready with the prerequisites, answer 'n' to the next question.\n\n"

# Prompt the user to update their system, providing a warning if they decline.
read -p "Are you ready to begin this process (y/n)? " AnswerYN
case ${AnswerYN:0:1} in
  y|Y )
    printf "\nGreat!  Let's get started.\n\n"
  ;;
  * )
    printf "${WarningMessage}OK.  Exiting the EMOD software environment preparation script now.\n\n"
    exit 0
  ;;
esac

# Elevate the user to root level to update the system and install dependencies.
# The user is prompted and the script will then execute a sudo command.
# The system will then prompt for a password.
read -p "First, you'll need to elevate your permissions.  Are you ready to sudo to root (y/n)? " AnswerYN
case ${AnswerYN:0:1} in
  y|Y )
    if [ $TestState -eq 0 ]
    then
      sudoCheck=$(sudo sh -c 'echo $UID')
      if [ $sudoCheck -eq 0 ]
      then
        printf "\n${FontRed}********************************************************************************
* Caution!  To install software on this system you're now sudo'd to root.  If  *
* you escape this script, remember to reduce your permissions to prevent       *
* accidental and catastrophic damage to your system.                           *
********************************************************************************${FontReset}\n\n"
      elif [ -z $sudoCheck ]
      then
        printf "${WarningMessage}Without sudo permissions you cannot install the packages required by the EMOD software.  Exiting.\n\n"
        exit 0
      else
        printf "${WarningMessage}Elevation to sudo failed.  This script cannot proceed further with the installation of required software packages.  Exiting.\n\n"
        exit 0
      fi
    else
      printf "\n${FontRed}TEST STATE ENABLED: sudo option disabled${FontReset}\n\n"
    fi
  ;;
  * )
    printf "${WarningMessage}The prerequisite libraries cannot be installed if you lack sudo permission.  Exiting.\n\n"
    exit 0
  ;;
esac

# Prompt the user to udpate their system, providing a warning if they decline.
read -p "Are you ready update on your system?  This is not required, but recommended. (y/n) " AnswerYN
case ${AnswerYN:0:1} in
  y|Y )
    printf "\nThe system is silently updating now (yum -y update) and may take some time.  Your patience is appreciated.\n\n"
    if [ ${TestState} -eq 0 ]
    then
      sudo yum -y update
    else
      printf "${FontRed}TEST STATE ENABLED: sudo yum -y update${FontReset}\n"
    fi
    printf "\nThe system update check has completed.  We're now going to continue by installing third-party software packages that are required by the EMOD software.\n"
  ;;
  * )
    printf "\nOK.  You may need to manually update your system to ensure any specific dependency versions are up-to-date.\n\nWe're now going to attempt to install the packages required by the EMOD software.\n"
  ;;
esac

# Four loops to search for packages and libraries.
# The first two loops set the length of the status bar while the last two increase the user's status bar.
# Initially these tasks look like they could be combined, but the printf statement for status
# is needed and interrupts the flow.
printf "\nThe EMOD software requires the following packages and their dependencies to be installed:"
Counter=1
StatusBar=""
for PackageRequired in "${EMODPackageRequired[@]}"
do
  printf "\n     ${Counter}. ${PackageRequired}"
  Counter=$((Counter + 1))
  StatusBar=${StatusBar}"\\u178C"
done
printf "\n\nIn addition, the following Python packages need to be installed using pip:"
Counter=1
for PIPRequired in "${EMODPythonLibraryRequired[@]}"
do
  printf "\n     ${Counter}. ${PIPRequired}"
  Counter=$((Counter + 1))
  StatusBar=${StatusBar}"\\u178C"
done

printf "\n\nWe're checking now to see if the required packages and libraries are present on this system.\n\nStatus:\n${FontYellow}${StatusBar}${FontReset}\n"
declare -a EMODMissing
for PackageRequired in "${EMODPackageRequired[@]}"
do
  printf "${FontGreen}\\u178C${FontReset}"
  if ! rpm -qa | grep -qw ${PackageRequired}; then
    EMODMissing+=(${PackageRequired}) 
  fi
done

declare -a EMODPIPMissing
for PIPRequired in "${EMODPythonLibraryRequired[@]}"
do
  printf "${FontGreen}\\u178C${FontReset}"
  PIPResult=`pip list | grep "$PIPRequired"`
  if [ -z "$PIPResult" ]
  then
    EMODPIPMissing+=(${PIPRequired})
  fi
done

# Once the EMODMissing array exists, present them to the user in an ordered list.
if [ ${#EMODMissing[@]} -gt 0 ]; then
  printf "\n\nThe following software is missing on this system and required for the EMOD software to work properly:\n"
  printf "\nPackages:\n"
  Counter=1
  for PackageMissing in "${EMODMissing[@]}"
  do
    printf "\n     ${Counter}. ${PackageMissing}"
    Counter=$((Counter + 1))
  done
  printf "\n"
fi
if [ ${#EMODPIPMissing[@]} -gt 0 ]; then
  printf "\nPython Libraries:\n"
  Counter=1
  for PIPMissing in "${EMODPIPMissing[@]}"
  do
    printf "\n     ${Counter}. ${PIPMissing}"
    Counter=$((Counter + 1))
  done
  printf "\n"
fi
if [ ${#EMODMissing[@]} -gt 0 ] || [ ${#EMODPIPMissing[@]} -gt 0 ]; then
  # Prompt the user and automatically install the missing packages.
  # After the packages are installed pip is then used to install the required Python numby library.
  # Note the curl command to add the repository.  This is for the git-lfs packaging.
  printf "\n"
  read -p "Are you ready to install these dependencies (packages and libraries) now?  This is a required step. (y/n) " AnswerYN
  case ${AnswerYN:0:1} in
    y|Y )
      if [ ${TestState} -eq 0 ]
      then
        printf "\n"
        sudo curl -s https://packagecloud.io/install/repositories/github/git-lfs/script.rpm.sh | sudo bash
        sudo pip install --upgrade pip
      else
        printf "\n"
        printf "${LineBreak}"
      fi
      for Package in "${EMODMissing[@]}"
      do
        if [ ${TestState} -eq 0 ]
        then
          printf "${LineBreak}"
          sudo yum -y install ${Package}
        else
          printf "${FontRed}TEST STATE ENABLED: sudo yum -y ${Package}\n"
        fi
      done
      for MissingPIP in "${EMODPIPMissing[@]}"
      do
        if [ ${TestState} -eq 0 ]
        then
          printf "${LineBreak}"
          sudo pip install ${MissingPIP}
        else
          printf "${FontRed}TEST STATE ENABLED: sudo pip install ${MissingPIP}\n"
        fi
      done
      printf "${LineBreak}" 
      printf "\nIf an error was reported by the package manager, [CTRL]+[C] from this script and investigate.  Otherwise, we're continuing with the remaining steps.\n"
    ;;
    * )
      printf "${WarningMessage}Without the required packages installed, the EMOD software will not run.  Exiting.\n\n"
      exit 0
    ;;
  esac
else
  printf "\n\nAll of the required EMOD software dependencies are found on your system.  No additional software needs to be installed.\n"
fi


# Ask the user if they want to download the EMOD source.
# Create a directory within their home and then prompt for GitHub credentials.
# The git client will then prompt the user for their credentials.
# Visual feedback on the clone status is displayed by the git client.
# Note the special case for the EMOD-InputData repository.  This is the only
# one that uses lfs support.
printf "\nYour environment is now ready to get the EMOD source code (~4GB) from GitHub.  If you want to stop now, answer 'n' at the next prompt.  Otherwise, you'll be prompted for your GitHub credentials.

If you lack sufficient permissions to download the EMOD software, please contact ${IDMSupportEmail}.\n\n"
read -p "Do you want to download the EMOD software now? (y/n)? " AnswerYN
case ${AnswerYN:0:1} in
  y|Y )
    printf "\nBefore we begin the download a new directory to house the files should be created.  This directory will be located in your home directory and must not currently exist.\n\n"
    DirectoryExists=0
    until [ ${DirectoryExists} -eq 1 ]
    do
      read -p "What is the name of the directory you would like to create? (Default: IDM) " NewDirectory
      if [ -z ${NewDirectory} ]
      then
        NewDirectory="IDM"
      else
        NewDirectory="$(echo -e "${NewDirectory}" | tr -d '[[:space:]]')"
      fi
      if [ ! -d "${NewDirectory}" ]
      then
        DirectoryExists=1
        if [ ${TestState} -eq 0 ]
        then
          mkdir ~/${NewDirectory}
          printf "\nA directory named ${FontYellow}${NewDirectory}${FontReset} was created in your home directory.\n"
          cd ~/${NewDirectory}
        else
          printf "\n${FontRed}TEST STATE ENABLED: Directory ${NewDirectory} not created.${FontReset}\n"
        fi
      else
        DirectoryExists=0
        printf "\nA directory named ${FontRed}${NewDirectory}${FontReset} already exists in your home directory.  Please enter another name.\n"
      fi
    done

    for ToDownload in "${EMODSoftware[@]}"
    do
      if [ ${TestState} -eq 0 ]
      then
        printf "\n"
        git clone ${EMODGitHubURL}${ToDownload}
        if [ ${ToDownload}=="EMOD-InputData" ]
        then
          git lfs fetch
        fi
      else
        printf "${FontRed}TEST STATE ENABLED: git clone ${EMODGitHubURL}${ToDownload}${FontReset}\n"
      fi
    done
    cd ~
    printf "\nThe download from GitHub has finished and the software is located on this system at ${FontGreen}~/${NewDirectory}${FontReset}.\n"
  ;;
  * )
    printf "\nYour environment is ready for the EMOD software, but you selected to not download EMOD source and support GitHub.\n\nTo complete this process manually, execute the following from your command prompt:"
    for ToDownload in "${EMODSoftware[@]}"
    do
      printf "\n     ${FontGreen}git clone ${EMODGitHubURL}${ToDownload}${FontReset}"
    done
    printf "\n"
  ;;
esac

# Define an environment variable for the .bashrc file.
# This is dynamic based upon the user's selection to download the source.
if [ ${NewDirectory} ]
then
  declare -a BashChanges=("export EMOD_ROOT=~/${NewDirectory}/EMOD/DtkTrunk/" "export PATH=\$PATH:/usr/lib/mpich/bin/" "export PATH=\$PATH:.:\$EMOD_ROOT/Scripts/")
  ln -s ~/${NewDirectory}/EMOD-InputData ~/${NewDirectory}/EMOD/DtkTrunk/
else
  declare -a BashChanges=("export PATH=\$PATH:.:/usr/lib/mpich/bin/")
fi

# Check that environment variables are in the .bashrc file.
if [ $(grep "EMOD SOFTWARE CHANGES" ~/.bashrc | wc -l) -gt 0 ]
then
  ENVChangesNeeded=0
else
  ENVChangesNeeded=1
fi

# Add variables to the .bashrc file if they aren't found.
# Display commands to the user and remind them to source the file.
if [ ${ENVChangesNeeded} -eq 1 ]
then
  printf "\nThe following PATH values need to be included into your environment file:"
  for AddToBash in "${BashChanges[@]}"
  do
    printf "\n     ${FontYellow}${AddToBash}${FontReset}"
  done
  printf "\n\n"
  read -p "Are you OK with this change to your .bashrc file? (y/n) " AnswerYN
  case ${AnswerYN:0:1} in
    y|Y )
      if [ ${TestState} -eq 0 ]
      then
        echo "" >> ~/.bashrc
        echo "# BEGIN EMOD SOFTWARE CHANGES HERE" >> ~/.bashrc
        for AddToBash in "${BashChanges[@]}"
        do
          echo ${AddToBash} >> ~/.bashrc 
        done
        echo "# END EMOD SOFTWARE CHANGES HERE" >> ~/.bashrc
      else
        printf "\n${FontRed}TEST STATE ENABLED: ${BashChange}${FontReset}\n"
      fi
      printf "\nA new environment variable has been added to your .bashrc file.  You'll need to source this file for these changes to take effect during your current session.\n"
    ;;
    * )
      printf "${WarningMessage}You will need to add the following to your .bashrc file to ensure the EMOD software runs:\n\n${FontGreen}${BashChange}${FontReset}\n\n"
    ;;
  esac
else
  if [ ${TestState} -eq 0 ]
  then
    printf "\nYour .bashrc file appears to be up-to-date.  No additional changes were needed.\n"
  else
    printf "${FontRed}TEST STATE ENABLED: .bashrc file already contains values${FontReset}\n"
  fi
fi

# Display the final message, completing the script run.
printf "\n${FontGreen}Your EMOD software environment set-up is now complete!${FontReset}

Remember to source your .bashrc files to make environment changes available for this session.

If you have any questions you may refer to our documentation found at ${FontGreen}http://idmod.org/idmdoc${FontReset}.

Thank you for your interest in our software.\n\n"
