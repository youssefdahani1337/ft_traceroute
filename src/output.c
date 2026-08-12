#include "../include/ft_traceroute.h"

double get_probe_rtt_ms(const t_probe *probe)
{
    return (double)probe->rtt.tv_sec * 1000.0 +
        (double)probe->rtt.tv_usec / 1000.0;
}

void print_traceroute_header(const t_traceroute *traceroute)
{
    size_t packet_size;

    packet_size = strlen(BUFFER) + sizeof(struct iphdr) + sizeof(struct udphdr);
    printf(
        "traceroute to %s (%s), %d hops max, %zu byte packets\n",
        traceroute->target_hostname ? traceroute->target_hostname : traceroute->target_ip,
        traceroute->target_ip[0] ? traceroute->target_ip : "unknown",
        traceroute->max_hops,
        packet_size
    );
}

void print_hop_results(const t_traceroute *traceroute, int ttl)
{
    char last_ip[INET_ADDRSTRLEN] = {0};
    int printed_any_valid = 0;

    printf(" %2d", ttl);
    for (int probe_index = 0; probe_index < traceroute->max_probes; probe_index++)
    {
        const t_probe *probe = traceroute->probes[probe_index];
        double rtt_ms;

        if (!probe->received || probe->received_from[0] == '\0')
        {
            printf("  *");
            last_ip[0] = '\0';
            continue;
        }

        rtt_ms = get_probe_rtt_ms(probe);

        if (!printed_any_valid)
        {
            printf("  %s (%s)  %.3f ms", probe->received_from, probe->received_from, rtt_ms);
            printed_any_valid = 1;
            snprintf(last_ip, sizeof(last_ip), "%s", probe->received_from);
        }
        else if (strcmp(probe->received_from, last_ip) == 0)
        {
            printf("  %.3f ms", rtt_ms);
        }
        else
        {
            printf("   %s (%s)  %.3f ms", probe->received_from, probe->received_from, rtt_ms);
            snprintf(last_ip, sizeof(last_ip), "%s", probe->received_from);
        }
    }
    printf("\n");
}
