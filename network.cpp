#include "network.h"
#include "utils.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
#include <cstring>

using namespace std;
using json = nlohmann::json;

// Query Geolocation (using ip-api.com free API)
GeoInfo query_geo_info(const string& ip_address) {
    GeoInfo geo;
    
    if (ip_address.empty() || ip_address == "*") {
        return geo;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) return geo;
    
    string url = "http://ip-api.com/json/" + ip_address + "?fields=status,country,countryCode,region,city,lat,lon";
    string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res == CURLE_OK && !response.empty()) {
        try {
            json j = json::parse(response);
            
            if (j["status"] == "success") {
                geo.country = j.value("country", "");
                geo.country_code = j.value("countryCode", "");
                geo.city = j.value("city", "");
                geo.region = j.value("region", "");
                geo.latitude = j.value("lat", 0.0);
                geo.longitude = j.value("lon", 0.0);
                geo.valid = true;
            }
        } catch (...) {
            // JSON parse error - ignore
        }
    }
    
    return geo;
}

// Query ISP/ASN Info
NetworkInfo query_network_info(const string& ip_address) {
    NetworkInfo network;
    
    if (ip_address.empty() || ip_address == "*") {
        return network;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) return network;
    
    string url = "http://ip-api.com/json/" + ip_address + "?fields=status,isp,org,as";
    string response;
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res == CURLE_OK && !response.empty()) {
        try {
            json j = json::parse(response);
            
            if (j["status"] == "success") {
                network.isp = j.value("isp", "");
                network.org = j.value("org", "");
                
                string as_info = j.value("as", "");
                if (!as_info.empty()) {
                    // Format: "AS15169 Google LLC"
                    size_t space_pos = as_info.find(' ');
                    if (space_pos != string::npos) {
                        network.asn = as_info.substr(0, space_pos);
                        network.as_name = as_info.substr(space_pos + 1);
                    } else {
                        network.asn = as_info;
                    }
                }
                
                network.valid = true;
            }
        } catch (...) {
            // JSON parse error - ignore
        }
    }
    
    return network;
}

// Create ICMP Packet
void create_icmp_packet(char *packet, int seq) {
    struct icmp *icmp_hdr = (struct icmp*)packet;
    
    icmp_hdr->icmp_type = ICMP_ECHO;
    icmp_hdr->icmp_code = 0;
    icmp_hdr->icmp_id = getpid();
    icmp_hdr->icmp_seq = seq;
    icmp_hdr->icmp_cksum = 0;
    
    for (int i = sizeof(struct icmp); i < PACKET_SIZE; i++) {
        packet[i] = i;
    }
    
    icmp_hdr->icmp_cksum = calculate_checksum((unsigned short*)packet, PACKET_SIZE);
}

// Resolve Hostname
bool resolve_hostname(const char *hostname, struct sockaddr_in *addr) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    
    if (getaddrinfo(hostname, NULL, &hints, &result) != 0) {
        return false;
    }
    
    memcpy(addr, result->ai_addr, sizeof(struct sockaddr_in));
    freeaddrinfo(result);
    return true;
}