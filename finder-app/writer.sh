#!/bin/bash

#Assign CLI arguments to variables
writefile=$1
writestr=$2

#Check if both arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Error: Two arguments are required. Usage $0 <full_path_to_file> <text_string>"
    exit 1
fi

#Extract the directory path from the full path of the file
dirpath=$(dirname "$writefile")

#CHeck if the directory exists if not create it

if [ ! -d "$dirpath" ]; then
    mkdir -p "$dirpath"
    if [ $? -ne 0 ]; then   
        echo "Error: Failed to create directory path."
        exit 1
    fi
fi

#Attempt to write the string to the file
echo "$writestr" > "$writefile"
if [ $? -ne 0 ]; then
    echo "Error: Failed to write to the file."
    exit 1
fi

echo "Succesfully wrote to the file: $writefile"
