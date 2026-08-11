#include "../include/ft_traceroute.h"

int parse_icmp(
    char *buffer,
    ssize_t len,
    uint16_t *original_port,
    int *icmp_type,
    int *icmp_code
)
{
    struct ip *outer_ip;
    struct icmphdr *icmp;
    struct ip *original_ip;
    struct udphdr *original_udp;

    int outer_ip_len;
    int original_ip_len;

    if (len < (ssize_t)sizeof(struct ip))
        return (-1);

    outer_ip = (struct ip *)buffer;
    if (outer_ip->ip_v != 4)
        return (-1);
    outer_ip_len = (outer_ip->ip_hl & 0x0F) << 2;
    if (outer_ip_len < (int)sizeof(struct ip) || outer_ip_len > len)
        return (-1);

    if (len < outer_ip_len + (int)sizeof(struct icmphdr))
        return (-1);

    icmp = (struct icmphdr *)(buffer + outer_ip_len);
    *icmp_type = icmp->type;
    *icmp_code = icmp->code;

    if (icmp->type != ICMP_TIMXCEED && icmp->type != ICMP_UNREACH)
        return (-1);

    if (len < outer_ip_len + (int)sizeof(struct icmphdr) + (int)sizeof(struct ip))
        return (-1);

    original_ip = (struct ip *)((char *)icmp + sizeof(struct icmphdr));
    if (original_ip->ip_v != 4)
        return (-1);
    original_ip_len = (original_ip->ip_hl & 0x0F) << 2;
    if (original_ip_len < (int)sizeof(struct ip) ||
        original_ip_len > (len - outer_ip_len - (int)sizeof(struct icmphdr)))
        return (-1);

    if (len < outer_ip_len + (int)sizeof(struct icmphdr) + original_ip_len +
        (int)sizeof(struct udphdr))
        return (-1);

    original_udp = (struct udphdr *)((char *)original_ip + original_ip_len);
    *original_port = ntohs(original_udp->uh_dport);

    return (0);
}

void free_all(t_traceroute *traceroute)
{
    free(traceroute->target);
    for (int i = 0; i < traceroute->max_probes; i++)
        free(traceroute->probes[i]);
    free(traceroute->probes);
    free(traceroute);
}