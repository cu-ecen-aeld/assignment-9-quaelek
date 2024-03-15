#!/bin/bash

# Assinging command line arguments to variables
filesdir=$1
searchstr=$2

#Check if both arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Error: Two arguments are required. Usage $0 <directory> <search_string>"
    exit 1
fi

#Check if the girst argument is a directory
if [ ! -d "$filesdir" ]; then
    echo "Error: The specified directory does not exist."
    exit 1
fi

#Finding the total number of files
num_files=$(find "$filesdir" -type f | wc -l)

#Finding the total number of matching lines
num_lines=$(grep -r "$searchstr" "$filesdir" | wc -l)

#Output

echo "The number of files are $num_files and the number of matching lines are $num_lines"
