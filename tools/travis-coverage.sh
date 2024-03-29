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

FILES=$(find ./ -type f -name '*.gcno')

for FILE in ${FILES}; do
	FILENAME=$(basename ${FILE} .gcno)
	TARGETDIR=$(dirname ${FILE})
	# This is a big hack...
	SOURCE=$(find ../ -type f -name "${FILENAME}" -print0)
	if [ -f "${SOURCE}" ]; then
		gcov ${SOURCE} --object-file ${FILE}
		mv ${FILENAME}.gcov ${TARGETDIR}/${FILENAME}.gcov
		# if any .h need to do otherwise... They can exist multiple times...
		OTHERS=$(find ./ -maxdepth 1 -type f -name '*.gcov')
		for OTHER in ${OTHERS}; do
			OTHER=$(basename ${OTHER} .gcov)
			mv ${OTHER}.gcov ${TARGETDIR}/${OTHER}.$(cmake -E md5sum ${OTHER}.gcov | awk '{ print $1 }').gcov
		done
	fi
done

pipx run codecov
