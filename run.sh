#!/bin/bash

for input_file in inputs/*
do
    echo "Running RepCRec with $input_file..."
    
    ./RepCRec "$input_file"
    
    echo "-------------------------------------------------"
done
