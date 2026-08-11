#ifndef FT_TRACEROUTE_H
#define FT_TRACEROUTE_H
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
// MACROS 

#define DEFAULT_PORT 33434
#define BUFFER  "HELLO"
#define MAX_HOPS 30
#define MAX_PROBES 3

typedef struct sprobe
{
    int ttl;
    uint16_t port;
    struct timespec send_time;
    char received_from[INET_ADDRSTRLEN];
    struct timeval rtt;
    int received;
} t_probe;

typedef struct s_traceroute
{
    struct sockaddr_in *target;
    char target_ip[INET_ADDRSTRLEN];
    char *target_hostname;
    char *interface_name;
    int max_hops;
    int max_probes;
    int destination_port;
    int tos;
    int sockfd;
    int sockfd_icmp;
    int reached_destination;
    t_probe **probes;
} t_traceroute;

void        resolve_host(t_traceroute *traceroute);
void        parse_arguments(int argc, char **argv, t_traceroute *traceroute);
void        init_traceroute(t_traceroute *traceroute);
void        print_help(FILE *stdfile);
void        open_socket(t_traceroute *traceroute);
void        send_probe(t_traceroute *traceroute, int probe_num);
void        sender(t_traceroute *traceroute);
int         receive_response(t_traceroute *traceroute);
int         find_probe(t_traceroute *traceroute, uint16_t port);
int         parse_icmp(char *buffer, ssize_t len, uint16_t *original_port, int *icmp_type, int *icmp_code);
void        print_hop_results(const t_traceroute *traceroute, int ttl);
double      get_probe_rtt_ms(const t_probe *probe);
void        free_all(t_traceroute *traceroute);


#endif



