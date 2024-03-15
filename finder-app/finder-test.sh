#!/bin/sh
# Modified tester script for assignment integration with Buildroot
# Author: Siddhant Jajoo

set -e
set -u

NUMFILES=10
WRITESTR=AELD_IS_FUN
WRITEDIR=/tmp/aeld-data
username=$(cat /etc/finder-app/conf/username.txt)  # Assuming conf files are moved to /etc/finder-app/conf

if [ $# -lt 3 ]; then
    echo "Using default value ${WRITESTR} for string to write"
    if [ $# -lt 1 ]; then
        echo "Using default value ${NUMFILES} for number of files to write"
    else
        NUMFILES=$1
    fi    
else
    NUMFILES=$1
    WRITESTR=$2
    WRITEDIR=/tmp/aeld-data/$3
fi

MATCHSTR="The number of files are ${NUMFILES} and the number of matching lines are ${NUMFILES}"

echo "Writing ${NUMFILES} files containing string ${WRITESTR} to ${WRITEDIR}"

rm -rf "${WRITEDIR}"

# Assumes assignment.txt is also moved to a global location
# assignment=$(cat ../conf/assignment.txt)

mkdir -p "$WRITEDIR"

if [ -d "$WRITEDIR" ]; then
    echo "$WRITEDIR created"
else
    exit 1
fi

for i in $(seq 1 $NUMFILES); do
    writer "$WRITEDIR/${username}$i.txt" "$WRITESTR"  # Assuming writer is in PATH
done

OUTPUTSTRING=$(finder "$WRITEDIR" "$WRITESTR")  # Assuming finder.sh is renamed to finder and in PATH

# Writing output to /tmp/assignment4-result.txt
echo "${OUTPUTSTRING}" > /tmp/assignment4-result.txt

# Check for success or failure and append to the result file
echo ${OUTPUTSTRING} | grep "${MATCHSTR}"
if [ $? -eq 0 ]; then
    echo "success" >> /tmp/assignment4-result.txt
    exit 0
else
    echo "failed: expected  ${MATCHSTR} in ${OUTPUTSTRING} but instead found" >> /tmp/assignment4-result.txt
    exit 1
fi
