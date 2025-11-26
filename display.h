#ifndef DISPLAY_H
#define DISPLAY_H

#include "common_types.h"
#include <string>

const char* get_latency_color(double rtt);
std::string draw_latency_bar(double rtt, double max_rtt = BAR_REFERENCE_RTT);

void print_header();
void print_hop_entry(const EnhancedHopInfo &hop, int hop_num);
void print_summary(const std::vector<EnhancedHopInfo> &route);

#endif