#include "utils.h"
#include <cmath>
#include <string>

using namespace std;

// CURL Write Callback
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Calculate geographical distance (Haversine formula)
double calculate_distance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0; // Earth radius (km)
    double dLat = (lat2 - lat1) * M_PI / 180.0;
    double dLon = (lon2 - lon1) * M_PI / 180.0;
    
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    
    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1) * cos(lat2) *
               sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    
    return R * c;
}

// Calculate Standard Deviation
double calculate_std_dev(const vector<double>& values, double mean) {
    if (values.empty()) return 0.0;
    
    double sum_squared_diff = 0.0;
    int count = 0;
    
    for (double val : values) {
        if (val > 0) {
            sum_squared_diff += (val - mean) * (val - mean);
            count++;
        }
    }
    
    if (count == 0) return 0.0;
    return sqrt(sum_squared_diff / count);
}

// Calculate Jitter
double calculate_jitter(const vector<double>& rtts) {
    if (rtts.size() < 2) return 0.0;
    
    double sum = 0.0;
    int count = 0;
    
    for (size_t i = 1; i < rtts.size(); i++) {
        if (rtts[i] > 0 && rtts[i-1] > 0) {
            sum += fabs(rtts[i] - rtts[i-1]);
            count++;
        }
    }
    
    return (count > 0) ? (sum / count) : 0.0;
}

// Calculate ICMP Checksum
unsigned short calculate_checksum(unsigned short *buf, int len) {
    unsigned long sum = 0;
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    if (len == 1) {
        sum += *(unsigned char*)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}