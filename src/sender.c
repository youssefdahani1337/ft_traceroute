#include "../include/ft_traceroute.h"

static void reset_probe_state(t_probe *probe, int ttl)
{
    probe->ttl = ttl;
    probe->received = 0;
    probe->rtt.tv_sec = 0;
    probe->rtt.tv_usec = 0;
    memset(probe->received_from, 0, INET_ADDRSTRLEN);
}

static void send_all_probes(t_traceroute *traceroute, int ttl)
{
    for (int probe_index = 0; probe_index < traceroute->max_probes; probe_index++)
    {
        reset_probe_state(traceroute->probes[probe_index], ttl);
        send_probe(traceroute, probe_index);
    }
}

static void collect_responses(t_traceroute *traceroute)
{
    for (int probe_index = 0; probe_index < traceroute->max_probes; probe_index++)
    {
        if (receive_response(traceroute) > 0 && traceroute->reached_destination)
            break;
    }
}

void sender(t_traceroute *traceroute)
{
    for (int ttl = 1; ttl <= traceroute->max_hops; ttl++)
    {
        traceroute->reached_destination = 0;
        if (setsockopt(traceroute->sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
        {
            perror("setsockopt");
            exit(1);
        }

        send_all_probes(traceroute, ttl);
        collect_responses(traceroute);
        print_hop_results(traceroute, ttl);

        if (traceroute->reached_destination)
            break;
    }
}

void send_probe(t_traceroute *traceroute, int index)
{
    ssize_t bytes_sent;

    traceroute->target->sin_port = htons(traceroute->probes[index]->port);

    clock_gettime(
        CLOCK_MONOTONIC,
        &traceroute->probes[index]->send_time
    );

    bytes_sent = sendto(
        traceroute->sockfd,
        BUFFER,
        strlen(BUFFER),
        0,
        (struct sockaddr *)traceroute->target,
        sizeof(struct sockaddr_in)
    );

    if (bytes_sent < 0)
    {
        perror("sendto");
        exit(1);
    }
}

int receive_response(t_traceroute *traceroute)
{
    char buffer[512];
    struct sockaddr_in sender_addr;
    struct pollfd pfd;
    socklen_t addr_len;
    ssize_t bytes_received;
    struct timespec current_time;
    uint16_t original_port;
    int probe_index;
    int icmp_type;
    int icmp_code;
    char sender_ip[INET_ADDRSTRLEN];

    addr_len = sizeof(sender_addr);

    pfd.fd = traceroute->sockfd_icmp;
    pfd.events = POLLIN;
    pfd.revents = 0;
    if (poll(&pfd, 1, 1000) <= 0)
        return (0);

    bytes_received = recvfrom(
        traceroute->sockfd_icmp,
        buffer,
        sizeof(buffer),
        0,
        (struct sockaddr *)&sender_addr,
        &addr_len
    );

    if (bytes_received < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (0);
        perror("recvfrom");
        return (0);
    }

    if (parse_icmp(
            buffer,
            bytes_received,
            &original_port,
            &icmp_type,
            &icmp_code) < 0)
    {
        return (0);
    }

    probe_index = find_probe(
        traceroute,
        original_port
    );

    if (probe_index < 0)
        return (0);

    clock_gettime(
        CLOCK_MONOTONIC,
        &current_time
    );

    inet_ntop(
        AF_INET,
        &sender_addr.sin_addr,
        sender_ip,
        sizeof(sender_ip)
    );

    strcpy(
        traceroute->probes[probe_index]->received_from,
        sender_ip
    );

    traceroute->probes[probe_index]->received = 1;

    long long start_ns = (long long)traceroute->probes[probe_index]->send_time.tv_sec * 1000000000LL +
        traceroute->probes[probe_index]->send_time.tv_nsec;
    long long end_ns = (long long)current_time.tv_sec * 1000000000LL + current_time.tv_nsec;
    long long diff_us = (end_ns - start_ns) / 1000LL;

    traceroute->probes[probe_index]->rtt.tv_sec = diff_us / 1000000L;
    traceroute->probes[probe_index]->rtt.tv_usec = diff_us % 1000000L;

    if (icmp_type == ICMP_UNREACH && icmp_code == ICMP_UNREACH_PORT)
        traceroute->reached_destination = 1;

    return (1);
}

int find_probe(t_traceroute *traceroute, uint16_t port)
{
    for (int i = 0; i < traceroute->max_probes; i++)
    {
        if (traceroute->probes[i]->port == port)
            return i;
    }

    return -1;
}