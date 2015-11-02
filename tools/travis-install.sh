#!/bin/bash

# This script executes the install step when running under travis-ci

# Set-up some error handling
set -o nounset   ## set -u : exit the script if you try to use an uninitialised variable
set -o errexit   ## set -e : exit the script if any statement returns a non-true return value
set -o pipefail  ## Fail on error in pipe

function exit_handler ()
{
	local exit_code="$?"
	
	test ${exit_code} == 0 && return;

	echo -e "\nInstall failed !!!\nLast command at line ${BASH_LINENO}: ${BASH_COMMAND}";
	exit "${exit_code}"
}
trap exit_handler EXIT
trap exit ERR

# We need a newer version of cmake than travis-ci provides
if [ -d "${HOME}/cmake-install" ]; then
	echo "Using $(${HOME}/cmake-install/bin/cmake --version | head -1) from cache"
else
	echo "Retrieving CMake 3.3.2"
	if [ "${TRAVIS_OS_NAME:-}" == "linux" ] || uname -s | grep -i Linux &> /dev/null; then
		wget https://cmake.org/files/v3.3/cmake-3.3.2-Linux-x86_64.tar.gz
		tar -xzf cmake-3.3.2-Linux-x86_64.tar.gz
		#wget -qO - https://cmake.org/files/v3.3/cmake-3.3.2-Linux-x86_64.tar.gz | tar -xz
		# copy to a directory that will not changed every version
		mv cmake-3.3.2-Linux-x86_64 ${HOME}/cmake-install
	elif [ "${TRAVIS_OS_NAME:-}" == "osx" ] || uname -s | grep -i Darwin &> /dev/null; then
		wget -qO - https://cmake.org/files/v3.3/cmake-3.3.2-Darwin-x86_64.tar.gz | tar -xz
		# copy to a directory that will not changed every version
		mv cmake-3.3.2-Darwin-universal/CMake.app/Contents ${HOME}/cmake-install
	else
		echo "OS not supported" && exit 1
	fi
fi
