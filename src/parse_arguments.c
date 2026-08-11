#include "../include/ft_traceroute.h"

static int  parse_positive_int(const char *str, int min_value, int max_value, const char *option_name)
{
    char *endptr;
    long value;

    if (str == NULL || *str == '\0')
    {
        fprintf(stderr, "ft_traceroute: option -%s requires a value\n", option_name);
        exit(1);
    }
    errno = 0;
    value = strtol(str, &endptr, 10);
    if (errno != 0 || endptr == str || *endptr != '\0' || value < min_value || value > max_value)
    {
        fprintf(stderr, "ft_traceroute: invalid value for -%s\n", option_name);
        exit(1);
    }
    return (int)value;
}

static void free_probe_array(t_traceroute *traceroute, int probe_count)
{
    if (!traceroute->probes)
        return;
    for (int i = 0; i < probe_count; i++)
        free(traceroute->probes[i]);
    free(traceroute->probes);
    traceroute->probes = NULL;
}

static void allocate_probe_array(t_traceroute *traceroute, int probe_count)
{
    traceroute->probes = malloc(sizeof(t_probe *) * probe_count);
    if (!traceroute->probes)
    {
        perror("malloc");
        exit(1);
    }

    for (int i = 0; i < probe_count; i++)
    {
        traceroute->probes[i] = calloc(1, sizeof(t_probe));
        if (!traceroute->probes[i])
        {
            perror("calloc");
            exit(1);
        }
    }
}

static void validate_probe_port_range(const t_traceroute *traceroute)
{
    unsigned long hop_count;
    unsigned long highest_port;

    hop_count = (unsigned long)(traceroute->max_hops - traceroute->first_hop + 1);
    highest_port = (unsigned long)traceroute->destination_port +
        (hop_count * (unsigned long)traceroute->max_probes) - 1;
    if (highest_port > 65535UL)
    {
        fprintf(stderr, "ft_traceroute: probe port range overflows 16-bit UDP ports\n");
        exit(1);
    }
}

void init_traceroute(t_traceroute *traceroute)
{
    memset(traceroute, 0, sizeof(*traceroute));
    traceroute->first_hop = 1;
    traceroute->max_hops = MAX_HOPS;
    traceroute->max_probes = MAX_PROBES;
    traceroute->destination_port = DEFAULT_PORT;
    allocate_probe_array(traceroute, traceroute->max_probes);
}

void print_help(FILE *stdfile)
{
    fprintf(stdfile, "Usage: ft_traceroute [options] <hostname>\n");
    fprintf(stdfile, "Options:\n");
    fprintf(stdfile, "  -h, --help\t\tShow this help message\n");
    fprintf(stdfile, "  -f <hops>\t\tSet the first hop to probe\n");
    fprintf(stdfile, "  -m <hops>, --max-hop <hops>\tSet the maximum number of hops\n");
    fprintf(stdfile, "  -q <probes>\t\tSet the number of probes per hop\n");
    fprintf(stdfile, "  -p <port>\t\tSet the destination UDP port\n");

    exit(stdfile == stdout ? 0 : 1);
}

void parse_arguments(int argc, char **argv, t_traceroute *traceroute)
{
    int opt;
    int long_index;
    int previous_probe_count;
    struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"max-hop", required_argument, NULL, 'm'},
        {0, 0, 0, 0}
    };

    init_traceroute(traceroute);
    previous_probe_count = traceroute->max_probes;
    opterr = 0;

    while ((opt = getopt_long(argc, argv, "hf:m:q:p:", long_opts, &long_index)) != -1)
    {
        switch (opt)
        {
            case 'h':
                print_help(stdout);
                break ;
            case 'f':
                traceroute->first_hop = parse_positive_int(optarg, 1, 255, "f");
                break;
            case 'm':
                traceroute->max_hops = parse_positive_int(optarg, 1, 255, "m");
                break;
            case 'q':
                traceroute->max_probes = parse_positive_int(optarg, 1, 10, "q");
                break;
            case 'p':
                traceroute->destination_port = parse_positive_int(optarg, 1, 65535, "p");
                break;
            default:
                print_help(stderr);
                break;
        }
    }

    if (traceroute->first_hop > traceroute->max_hops)
    {
        fprintf(stderr, "ft_traceroute: first hop cannot be greater than max hop\n");
        exit(1);
    }

    validate_probe_port_range(traceroute);

    if (optind >= argc)
    {
        fprintf(stderr, "Error: Missing hostname argument\n");
        print_help(stderr);
    }
    if (optind < argc - 1)
    {
        fprintf(stderr, "Error: Too many arguments\n");
        print_help(stderr);
    }

    traceroute->target_hostname = argv[optind];

    free_probe_array(traceroute, previous_probe_count);
    allocate_probe_array(traceroute, traceroute->max_probes);
}
