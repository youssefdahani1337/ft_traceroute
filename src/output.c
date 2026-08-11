#include "../include/ft_traceroute.h"

static void format_probe_label(const t_probe *probe, char *buffer, size_t size)
{
    if (!probe->received || probe->received_from[0] == '\0')
    {
        snprintf(buffer, size, "*");
        return;
    }
    snprintf(buffer, size, "%s (%s)", probe->received_from, probe->received_from);
}

double get_probe_rtt_ms(const t_probe *probe)
{
    return (double)probe->rtt.tv_sec * 1000.0 +
        (double)probe->rtt.tv_usec / 1000.0;
}

void print_hop_results(const t_traceroute *traceroute, int ttl)
{
    char current_display[INET_ADDRSTRLEN + 16] = {0};
    int current_display_set = 0;
    int first_output = 1;

    for (int probe_index = 0; probe_index < traceroute->max_probes; probe_index++)
    {
        const t_probe *probe = traceroute->probes[probe_index];
        char display[INET_ADDRSTRLEN + 16];

        if (probe->received)
        {
            double rtt_ms = get_probe_rtt_ms(probe);
            format_probe_label(probe, display, sizeof(display));

            if (!current_display_set || strcmp(display, current_display) != 0)
            {
                if (!first_output)
                    printf("\n    ");
                else
                    printf(" %2d  ", ttl);
                printf("%-24s", display);
                strncpy(current_display, display, sizeof(current_display) - 1);
                current_display[sizeof(current_display) - 1] = '\0';
                current_display_set = 1;
                first_output = 0;
            }
            printf("  %.3f ms", rtt_ms);
        }
        else
        {
            if (!current_display_set)
            {
                printf(" %2d  ", ttl);
                printf("%-24s", "*");
                strncpy(current_display, "*", sizeof(current_display) - 1);
                current_display[sizeof(current_display) - 1] = '\0';
                current_display_set = 1;
            }
            else
                printf("  *");
            first_output = 0;
        }
    }
    printf("\n");
}
