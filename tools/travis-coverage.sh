#!/bin/bash

# This script executes the coverage step when running under travis-ci

# Set-up some error handling
set -o nounset   ## set -u : exit the script if you try to use an uninitialised variable
set -o errexit   ## set -e : exit the script if any statement returns a non-true return value
set -o pipefail  ## Fail on error in pipe

function exit_handler ()
{
	local exit_code="$?"
	
	test ${exit_code} == 0 && return;

	echo -e "\nPrepare coverage failed !!!\nLast command at line ${BASH_LINENO}: ${BASH_COMMAND}";
	exit "${exit_code}"
}
trap exit_handler EXIT
trap exit ERR

find ./ -type f -name '*.gcno' | awk '{ print "mv "$0" $(dirname "$0")/$(basename "$0" .c.gcno).gcno" }' | bash
find ./ -type f -name '*.gcda' | awk '{ print "mv "$0" $(dirname "$0")/$(basename "$0" .c.gcda).gcda" }' | bash
