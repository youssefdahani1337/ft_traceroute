#!/bin/bash

# Default
sudo ./ft_traceroute google.com

# First hop
sudo ./ft_traceroute -f 3 google.com

# Max hop
sudo ./ft_traceroute -m 5 google.com

# Probe count
sudo ./ft_traceroute -q 5 google.com

# Port
sudo ./ft_traceroute -p 40000 google.com

# Timeout
sudo ./ft_traceroute -w 1 google.com

# Combined
sudo ./ft_traceroute -f 2 -m 10 -p 40000 -q 5 -w 1 google.com

# Version
./ft_traceroute -V

# Invalid
sudo ./ft_traceroute -m 0 google.com
sudo ./ft_traceroute -q 0 google.com
sudo ./ft_traceroute -p 65536 google.com
sudo ./ft_traceroute -w 0 google.com
sudo ./ft_traceroute -f 0 google.com
sudo ./ft_traceroute --unknown google.com
sudo ./ft_traceroute invalid-host
sudo ./ft_traceroute