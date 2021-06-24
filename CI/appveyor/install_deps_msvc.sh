#!/bin/bash

set -e
set PATH=%PATH%
TOP_DIR=$(pwd)

# Download SWIG
#cd /c/
#wget https://sourceforge.net/projects/swig/files/swigwin/swigwin-4.0.0/swigwin-4.0.0.zip
#7z x swigwin-4.0.0.zip -oswig
#cd swig/swigwin-4.0.0
#xcopy * .. /s /e /h /Q

# Download glog
#cd /c/
#${TOP_DIR}\CI\appveyor\install_glog.bat "Release" "x64"
#${TOP_DIR}\CI\appveyor\install_glog.bat "Release" "Win32"


MCS_EXECUTABLE_PATH="C:\Windows\Microsoft.NET\Framework\v4.0.30319"
OLD_PATH="$PATH"
DEST_LIBIIO="/c/libiio"

__get_libxml() {
	DEST_LIBXML="/c/libxml"
	mkdir -p $DEST_LIBXML
	cd $DEST_LIBXML

}

#__get_libiio_deps() {
#}



if ( "$ARCH" -eq "x64" ) {
	wget https://www.zlatkovic.com/pub/libxml/64bit/libxml2-2.9.3-win32-x86_64.7z -OutFile "libxml.7z"
} else {
	wget https://www.zlatkovic.com/pub/libxml/64bit/libxml2-2.9.3-win32-x86.7z -OutFile "libxml.7z"
}
7z x -y libxml.7z
rm libxml.7z

echo "Downloading deps..."
cd C:\
wget http://swdownloads.analog.com/cse/build/libiio-win-deps.zip -OutFile "libiio-win-deps.zip"
7z x -y "C:\libiio-win-deps.zip"

__build_libiio() {
	local PLATFORM="$1"
	local GENERATOR="$2"

	# Create the official build directory for this platform
	mkdir -p "$DEST_LIBIIO/build-$PLATFORM"
	cd "$DEST_LIBIIO/build-$PLATFORM"

	cmake -G "$GENERATOR" -DCMAKE_CONFIGURATION_TYPES=RELEASE ..
	cmake --build . --config Release

}

__mv_to_build_dir() {
	local PLATFORM="$1"

	DST_FOLDER="$DEST_LIBIIO-$PLATFORM/"
	cd "$DEST_LIBIIO/build-$PLATFORM"
	cp *.dll $DST_FOLDER
	cp *.exe $DST_FOLDER
	cp *.lib $DST_FOLDER
	cp *.iss $DST_FOLDER
	cd ..
	cp iio.h $DST_FOLDER
}

git clone https://github.com/analogdevicesinc/libiio $DEST_LIBIIO
cd $DEST_LIBIIO

__build_libiio win32 "Visual Studio 15"
__mv_to_build_dir win32

__build_libiio win64 "Visual Studio 15 Win64"
__mv_to_build_dir win64
