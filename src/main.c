#include "../include/ft_traceroute.h"


int main(int argc, char **argv)
{
    t_traceroute *traceroute = calloc(1, sizeof(*traceroute));

    if (!traceroute)
    {
        perror("calloc");
        return (1);
    }
    parse_arguments(argc, argv, traceroute);
    resolve_host(traceroute);
    open_socket(traceroute);
    sender(traceroute);
    
    free_all(traceroute);
    return (0);
}