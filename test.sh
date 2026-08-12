#!/bin/bash

tests=(
    "-m 0 google.com"
    "-m -1 google.com"
    "-m abc google.com"
    "-q 0 google.com"
    "-q -1 google.com"
    "-q abc google.com"
    "-p 0 google.com"
    "-p 65536 google.com"
    "-p abc google.com"
    "-w 0 google.com"
    "-w -1 google.com"
    "-w abc google.com"
    "-f 0 google.com"
    "-f -1 google.com"
    "-f abc google.com"
    "--unknown google.com"
    "invalid-host-123456789.com"
)

for test in "${tests[@]}"; do
    echo "================================"
    echo "TEST: ./ft_traceroute $test"
    echo "================================"
    timeout 10 sudo ./ft_traceroute $test
    echo "exit code: $?"
done