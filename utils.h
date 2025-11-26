#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>

// Math helpers
double calculate_distance(double lat1, double lon1, double lat2, double lon2);
double calculate_std_dev(const std::vector<double>& values, double mean);
double calculate_jitter(const std::vector<double>& rtts);

// Network raw helpers
unsigned short calculate_checksum(unsigned short *buf, int len);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

#endif