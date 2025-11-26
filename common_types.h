#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <string>
#include <vector>

// Configuration Constants
#define MAX_HOPS 30
#define PACKET_SIZE 64
#define TIMEOUT_SEC 2
#define PROBES_PER_HOP 1
#define BAR_REFERENCE_RTT 100.0 

// ANSI Color Codes
#define COLOR_RESET   "\033[0m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RED     "\033[31m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_GRAY    "\033[90m"
#define COLOR_WHITE   "\033[97m"
#define BOLD          "\033[1m"

// Geolocation Info
struct GeoInfo {
    std::string country;
    std::string country_code;
    std::string city;
    std::string region;
    double latitude;
    double longitude;
    bool valid;
    
    GeoInfo() : latitude(0), longitude(0), valid(false) {}
};

// ISP/ASN Info
struct NetworkInfo {
    std::string isp;
    std::string org;
    std::string asn;
    std::string as_name;
    bool valid;
    
    NetworkInfo() : valid(false) {}
};

// Hop Details
struct EnhancedHopInfo {
    std::string ip_address;
    std::string hostname;
    std::vector<double> rtts;
    double avg_rtt;
    double min_rtt;
    double max_rtt;
    double std_dev;
    double jitter;
    double packet_loss;
    bool is_destination;
    GeoInfo geo;
    NetworkInfo network;
    double distance_from_prev;

    EnhancedHopInfo() : avg_rtt(0), min_rtt(0), max_rtt(0), 
                        std_dev(0), jitter(0), packet_loss(0), 
                        is_destination(false), distance_from_prev(0) {}
};

#endif