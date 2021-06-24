#!/bin/bash

# Note: InnoSetup is already installed on Azure images; so don't run this step
#       Running choco seems a bit slow; seems to save about 40-60 seconds here
#choco install InnoSetup

set PATH=%PATH%;"C:\Program Files (x86)\Inno Setup 6"

TOP_DIR=$(pwd)

# Download SWIG
cd ${TOP_DIR}
wget https://sourceforge.net/projects/swig/files/swigwin/swigwin-4.0.0/swigwin-4.0.0.zip
7z x swigwin-4.0.0.zip -oswig
cd swig/swigwin-4.0.0
cp * ..

# Download glog
cd ${TOP_DIR}
${TOP_DIR}/CI/appveyor/install_glog.bat "Release" "x64"
${TOP_DIR}/CI/appveyor/install_glog.bat "Release" "Win32"
