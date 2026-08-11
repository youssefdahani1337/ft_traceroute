#include "../include/ft_traceroute.h"

static void configure_socket_timeout(int sockfd, long seconds, long microseconds)
{
	struct timeval timeout;

	timeout.tv_sec = seconds;
	timeout.tv_usec = microseconds;
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}
}

void resolve_host(t_traceroute *traceroute)
{
	struct addrinfo	hints;
	struct addrinfo	*result;
	char			ip_str[INET_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;

	if (getaddrinfo(traceroute->target_hostname, NULL, &hints, &result) != 0)
	{
		fprintf(stderr, "ft_traceroute: cannot resolve %s: unknown host\n",
			traceroute->target_hostname);
		exit(1);
	}

	traceroute->target = malloc(sizeof(struct sockaddr_in));
	if (!traceroute->target)
	{
		perror("malloc");
		freeaddrinfo(result);
		exit(1);
	}

	memset(traceroute->target, 0, sizeof(*traceroute->target));
	traceroute->target->sin_family = AF_INET;
	traceroute->target->sin_port = htons(traceroute->destination_port);
	traceroute->target->sin_addr = ((struct sockaddr_in *)result->ai_addr)->sin_addr;

	inet_ntop(AF_INET, &traceroute->target->sin_addr, ip_str, INET_ADDRSTRLEN);
	strncpy(traceroute->target_ip, ip_str, INET_ADDRSTRLEN - 1);
	traceroute->target_ip[INET_ADDRSTRLEN - 1] = '\0';

	freeaddrinfo(result);
}

static void configure_socket_options(int sockfd, t_traceroute *traceroute)
{
	(void)traceroute;
	configure_socket_timeout(sockfd, 2, 0);
}

void open_socket(t_traceroute *traceroute)
{
	int sockfd;

	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sockfd < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	traceroute->sockfd = sockfd;
	configure_socket_options(sockfd, traceroute);

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd < 0)
	{
		if (errno == EPERM)
			fprintf(stderr, "ft_traceroute: raw ICMP sockets require root privileges or CAP_NET_RAW\n");
		else
			perror("socket");
		exit(1);
	}
	configure_socket_options(sockfd, traceroute);
	traceroute->sockfd_icmp = sockfd;
}