#ifndef NETWORK_H
#define NETWORK_H

#include "common_types.h"
#include <netinet/in.h>

// API Queries
GeoInfo query_geo_info(const std::string& ip_address);
NetworkInfo query_network_info(const std::string& ip_address);

// Packet & DNS
void create_icmp_packet(char *packet, int seq);
bool resolve_hostname(const char *hostname, struct sockaddr_in *addr);

#endif