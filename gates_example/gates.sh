#!/bin/bash

# Compile C files
gcc -o parent gates_parent.c -Wall
gcc -o child gates_child.c -Wall

# Function to cleanup executables
cleanup() {
    rm -f parent child
    echo "Executables deleted"
}

# Trap SIGINT and SIGTERM signals to perform cleanup
trap 'cleanup' SIGINT SIGTERM

# Run the parent program with user input
read -p "Enter gate states: " gate_states
./parent "$gate_states"

# Cleanup after the process terminates
cleanup
