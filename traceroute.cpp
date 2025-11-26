#include "traceroute.h"
#include "utils.h"
#include "network.h"
#include "display.h"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/time.h>

using namespace std;

// Execute Enhanced Traceroute
vector<EnhancedHopInfo> enhanced_traceroute(const char *destination) {
    vector<EnhancedHopInfo> route;
    int send_sock, recv_sock;
    struct sockaddr_in dest_addr, recv_addr;
    char send_packet[PACKET_SIZE];
    char recv_packet[512];
    socklen_t addr_len = sizeof(recv_addr);
    struct timeval tv, send_time, recv_time;
    
    cout << COLOR_CYAN << "\nResolving target host..." << COLOR_RESET << endl;
    
    if (!resolve_hostname(destination, &dest_addr)) {
        cerr << COLOR_RED << "Error: Unable to resolve hostname " << destination << COLOR_RESET << endl;
        return route;
    }
    
    send_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    recv_sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    
    if (send_sock < 0 || recv_sock < 0) {
        cerr << COLOR_RED << "Error: Unable to create socket (Root privileges required)" << COLOR_RESET << endl;
        return route;
    }
    
    tv.tv_sec = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(recv_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    bool reached_destination = false;
    GeoInfo prev_geo;
    
    cout << COLOR_CYAN << "Starting traceroute, probing " << PROBES_PER_HOP << " times per hop...\n" << COLOR_RESET << endl;
    
    // PRINT HEADER ONCE BEFORE LOOP
    print_header();

    for (int ttl = 1; ttl <= MAX_HOPS && !reached_destination; ttl++) {
        // Print progress (will be overwritten by the full block)
        cout << COLOR_GRAY << "Probing hop " << ttl << "..." << COLOR_RESET << "\r" << flush;
        
        EnhancedHopInfo hop;
        hop.is_destination = false;
        
        if (setsockopt(send_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
            break;
        }
        
        int successful_probes = 0;
        string current_ip;
        
        for (int probe = 0; probe < PROBES_PER_HOP; probe++) {
            memset(send_packet, 0, PACKET_SIZE);
            create_icmp_packet(send_packet, ttl * PROBES_PER_HOP + probe);
            
            gettimeofday(&send_time, NULL);
            
            if (sendto(send_sock, send_packet, PACKET_SIZE, 0,
                      (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
                hop.rtts.push_back(-1);
                continue;
            }
            
            memset(recv_packet, 0, sizeof(recv_packet));
            ssize_t recv_len = recvfrom(recv_sock, recv_packet, sizeof(recv_packet), 0,
                                      (struct sockaddr*)&recv_addr, &addr_len);
            
            if (recv_len < 0) {
                hop.rtts.push_back(-1);
                continue;
            }
            
            gettimeofday(&recv_time, NULL);
            
            double rtt = (recv_time.tv_sec - send_time.tv_sec) * 1000.0 +
                         (recv_time.tv_usec - send_time.tv_usec) / 1000.0;
            
            hop.rtts.push_back(rtt);
            successful_probes++;
            
            struct ip *ip_hdr = (struct ip*)recv_packet;
            struct icmp *icmp_hdr = (struct icmp*)(recv_packet + (ip_hdr->ip_hl << 2));
            
            if (current_ip.empty()) {
                current_ip = inet_ntoa(recv_addr.sin_addr);
                hop.ip_address = current_ip;
                
                char hostname[NI_MAXHOST];
                if (getnameinfo((struct sockaddr*)&recv_addr, sizeof(recv_addr),
                               hostname, sizeof(hostname), NULL, 0, 0) == 0) {
                    hop.hostname = hostname;
                } else {
                    hop.hostname = current_ip;
                }
            }
            
            if (icmp_hdr->icmp_type == ICMP_ECHOREPLY) {
                reached_destination = true;
                hop.is_destination = true;
            }
        }
        
        // Calculate Stats
        hop.packet_loss = ((PROBES_PER_HOP - successful_probes) / (double)PROBES_PER_HOP) * 100.0;
        
        if (successful_probes > 0) {
            double sum = 0, min_val = 999999, max_val = 0;
            for (double rtt : hop.rtts) {
                if (rtt > 0) {
                    sum += rtt;
                    if (rtt < min_val) min_val = rtt;
                    if (rtt > max_val) max_val = rtt;
                }
            }
            hop.avg_rtt = sum / successful_probes;
            hop.min_rtt = min_val;
            hop.max_rtt = max_val;
            hop.std_dev = calculate_std_dev(hop.rtts, hop.avg_rtt);
            hop.jitter = calculate_jitter(hop.rtts);
            
            // Query Geo & Network info
            // Overwrite progress line with detailed progress
            cout << "\r" << string(80, ' ') << "\r"; // Clear
            cout << COLOR_GRAY << "Hop " << ttl << ": Querying geo/network info..." << COLOR_RESET << "\r" << flush;
            
            hop.geo = query_geo_info(hop.ip_address);
            hop.network = query_network_info(hop.ip_address);
            
            // Calculate distance
            if (prev_geo.valid && hop.geo.valid) {
                hop.distance_from_prev = calculate_distance(
                    prev_geo.latitude, prev_geo.longitude,
                    hop.geo.latitude, hop.geo.longitude
                );
            } else {
                hop.distance_from_prev = 0;
            }
            
            prev_geo = hop.geo;
        } else {
            hop.avg_rtt = hop.min_rtt = hop.max_rtt = -1;
            hop.std_dev = hop.jitter = 0;
            hop.distance_from_prev = 0;
        }
        
        // PRINT HOP IMMEDIATELY
        print_hop_entry(hop, ttl);
        
        route.push_back(hop);
    }
    
    close(send_sock);
    close(recv_sock);
    
    return route;
}