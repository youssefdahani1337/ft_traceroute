#include "../include/ft_traceroute.h"

#define RESPONSE_TIMEOUT_MS 2000

static void reset_probe_state(t_probe *probe, int ttl)
{
    probe->ttl = ttl;
    probe->received = 0;
    probe->rtt.tv_sec = 0;
    probe->rtt.tv_usec = 0;
    memset(probe->received_from, 0, INET_ADDRSTRLEN);
}

static void print_probe_result(const t_probe *probe, char *last_ip, int *printed_any_valid)
{
    double rtt_ms;

    if (!probe->received || probe->received_from[0] == '\0')
    {
        write(1, "  *", 3);
        last_ip[0] = '\0';
        return;
    }

    rtt_ms = get_probe_rtt_ms(probe);
    if (!*printed_any_valid)
    {
        printf("  %s (%s)  %.3f ms", probe->received_from, probe->received_from, rtt_ms);
        *printed_any_valid = 1;
    }
    else if (strcmp(probe->received_from, last_ip) == 0)
    {
        printf("  %.3f ms", rtt_ms);
    }
    else
    {
        printf("   %s (%s)  %.3f ms", probe->received_from, probe->received_from, rtt_ms);
    }

    snprintf(last_ip, INET_ADDRSTRLEN, "%s", probe->received_from);
}

static void send_probe_and_collect(
    t_traceroute *traceroute,
    int ttl,
    int probe_index,
    char *last_ip,
    int *printed_any_valid
)
{
    traceroute->probes[probe_index]->port =
        traceroute->destination_port +
        ((ttl - traceroute->first_hop) * traceroute->max_probes) + probe_index;
    reset_probe_state(traceroute->probes[probe_index], ttl);
    send_probe(traceroute, probe_index);

    while (traceroute->probes[probe_index]->received == 0)
    {
        if (receive_response(traceroute) <= 0)
            break;
        if (traceroute->reached_destination)
            break;
    }

    print_probe_result(traceroute->probes[probe_index], last_ip, printed_any_valid);
}

void sender(t_traceroute *traceroute)
{
    char last_ip[INET_ADDRSTRLEN] = {0};
    int printed_any_valid;

    for (int ttl = traceroute->first_hop; ttl <= traceroute->max_hops; ttl++)
    {
        int hop_index = ttl - traceroute->first_hop + 1;

        traceroute->reached_destination = 0;
        printed_any_valid = 0;
        last_ip[0] = '\0';

        char ttl_str[4];
        snprintf(ttl_str, sizeof(ttl_str), " %2d", hop_index);
        write(1, ttl_str, strlen(ttl_str));
        if (setsockopt(traceroute->sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0)
        {
            perror("setsockopt");
            exit(1);
        }

        for (int probe_index = 0; probe_index < traceroute->max_probes; probe_index++)
        {
            send_probe_and_collect(
                traceroute,
                ttl,
                probe_index,
                last_ip,
                &printed_any_valid
            );
            if (traceroute->reached_destination)
                break;
        }
        printf("\n");

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
    // struct pollfd pfd;
    socklen_t addr_len;
    ssize_t bytes_received;
    struct timespec current_time;
    uint16_t original_port;
    int probe_index;
    int icmp_type;
    int icmp_code;
    char sender_ip[INET_ADDRSTRLEN];

    addr_len = sizeof(sender_addr);

    // pfd.fd = traceroute->sockfd_icmp;
    // pfd.events = POLLIN;
    // pfd.revents = 0;
    // if (poll(&pfd, 1, RESPONSE_TIMEOUT_MS) <= 0)
    //     return (0);

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

    probe_index = find_probe(traceroute, original_port);
    if (probe_index < 0)
        return (0);

    if (traceroute->probes[probe_index]->ttl != 0 &&
        traceroute->probes[probe_index]->received)
        return (0);

    clock_gettime(CLOCK_MONOTONIC, &current_time);

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