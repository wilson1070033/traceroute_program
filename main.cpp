#include "traceroute.h"
#include "display.h"
#include "common_types.h"
#include <iostream>
#include <unistd.h>
#include <curl/curl.h>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <hostname or IP>\n";
        cerr << "Example: sudo " << argv[0] << " www.google.com\n";
        return 1;
    }
    
    if (geteuid() != 0) {
        cerr << COLOR_RED << "Error: Root privileges required.\n" << COLOR_RESET;
        cerr << "Please run with: sudo " << argv[0] << " " << argv[1] << "\n";
        return 1;
    }
    
    // Init CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    cout << COLOR_MAGENTA << BOLD << "\n╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║    Enhanced Visual Traceroute - w/ Geo & ISP Information       ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝";
    cout << COLOR_RESET << "\n";
    
    cout << "\nTarget: " << COLOR_CYAN << argv[1] << COLOR_RESET << "\n";
    cout << "Probes per hop: " << PROBES_PER_HOP << "\n";
    cout << "API Provider: ip-api.com (Free Service)\n";
    
    vector<EnhancedHopInfo> route = enhanced_traceroute(argv[1]);
    
    if (!route.empty()) {
        print_summary(route);
    }
    
    curl_global_cleanup();
    
    return 0;
}