#ifndef TRACEROUTE_H
#define TRACEROUTE_H

#include "common_types.h"
#include <vector>

std::vector<EnhancedHopInfo> enhanced_traceroute(const char *destination);

#endif