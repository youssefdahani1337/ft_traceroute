# ft_traceroute

A lightweight implementation of the classic Unix traceroute utility in C. The project is designed to explore how network packets travel across routers and how ICMP/UDP-based probe responses can reveal each hop on the path to a remote host.

## Project goal

The goal of this project is to re-implement the behavior of traceroute in a minimal way, while learning:

- socket programming
- raw ICMP sockets
- UDP packet sending
- TTL (Time-To-Live) behavior
- IP routing and hop discovery
- RTT calculation
- parsing of ICMP responses
- argument handling and command-line tool design

## What this program does

Traceroute works by sending packets with increasing TTL values. Each router receiving the packet decrements the TTL; when TTL reaches zero, the router sends an ICMP Time Exceeded message back to the sender. By observing which IP responds at each hop, we can reconstruct the route to the destination.

When the packet eventually reaches the target host, the host may respond with an ICMP Port Unreachable message if the destination port is closed, which confirms the target was reached.

## Features

- resolves hostnames to IP addresses
- sends UDP probes with increasing TTL values
- waits for ICMP replies
- prints hop responses with RTTs
- supports common options such as:
  - `-f` / `--first-hop`
  - `-m` / `--max-hop`
  - `-q`
  - `-p`
  - `-w` / `--wait`
  - `-V` / `--version`
  - `-h` / `--help`

## Build

```bash
make
```

This compiles the project and creates the executable named `ft_traceroute`.

## Usage

```bash
./ft_traceroute <hostname>
```

Examples:

```bash
./ft_traceroute google.com
./ft_traceroute 8.8.8.8
./ft_traceroute -f 1 -m 20 github.com
./ft_traceroute -w 2 -q 3 example.com
./ft_traceroute -V
./ft_traceroute -h
```

## Options

```bash
Usage: ft_traceroute [options] <hostname>
Options:
  -h, --help            Show this help message
  -f, --first-hop <hops>   Set the first hop to probe
  -m, --max-hop <hops>     Set the maximum number of hops
  -q <probes>           Set the number of probes per hop
  -p <port>             Set the destination UDP port
  -w, --wait=NUM        wait NUM seconds for response (default: 3)
  -V, --version         Print program version
```

## How it works internally

### 1. Host resolution
The program resolves the target hostname using `getaddrinfo()` and stores the destination IP address.

### 2. Socket creation
It creates:

- a UDP socket to send packets
- a raw ICMP socket to receive ICMP responses

### 3. TTL control
Each probe is sent with a different TTL value. A router receiving the packet decreases TTL and, if it becomes zero, sends an ICMP Time Exceeded message.

### 4. RTT measurement
The program records the time when the probe is sent and compares it with the receiving time to compute RTT.

### 5. Hop detection
Each probe is assigned a UDP port range based on TTL and probe number, so replies can be matched to the correct packet.

### 6. Output
For each TTL, the program prints the responding IP(s) and the measured time for each probe.

## Concepts learned

This project helps practice and understand several core networking concepts:

- TCP/IP model basics
- IPv4 packet structure
- UDP and ICMP protocols
- TTL and hop counting
- Routing and intermediary network devices
- Round-trip time estimation
- socket APIs in C
- raw packet handling and privilege requirements
- network debugging and packet analysis

## Notes

This is a simplified educational implementation of traceroute, not a full production-grade traceroute. It is ideal for learning how the protocol works and how to build a network tool in C.

To run traceroute successfully, the program may require root privileges or CAP_NET_RAW depending on your environment, because it opens a raw ICMP socket.

