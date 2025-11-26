#include "display.h"
#include <iostream>
#include <iomanip>

using namespace std;

// Get Color based on Latency
const char* get_latency_color(double rtt) {
    if (rtt < 0) return COLOR_RED;
    if (rtt < 30) return COLOR_GREEN;
    if (rtt < 100) return COLOR_YELLOW;
    return COLOR_RED;
}

// Draw Latency Bar
string draw_latency_bar(double rtt, double max_rtt) {
    if (rtt < 0) return "[" COLOR_RED "██████████" COLOR_RESET "] TIMEOUT";
    
    int bar_length = 10;
    int filled = (int)((rtt / max_rtt) * bar_length);
    if (filled > bar_length) filled = bar_length;
    
    const char* color = get_latency_color(rtt);
    
    string bar = "[";
    bar += color;
    for (int i = 0; i < bar_length; i++) {
        if (i < filled) bar += "█";
        else bar += "░";
    }
    bar += COLOR_RESET;
    bar += "]";
    
    return bar;
}

void print_header() {
    cout << "\n";
    cout << COLOR_CYAN << BOLD;
    cout << "╔════════════════════════════════════════════════════════════════════════════════╗\n";
    cout << "║            ENHANCED VISUAL TRACEROUTE WITH GEO & ISP INFO                      ║\n";
    cout << "╚════════════════════════════════════════════════════════════════════════════════╝";
    cout << COLOR_RESET << "\n\n";
    
    cout << COLOR_BLUE << BOLD << "Source: Your Computer" << COLOR_RESET << "\n";
    cout << COLOR_GRAY << "  ↓\n" << COLOR_RESET;
}

void print_hop_entry(const EnhancedHopInfo &hop, int hop_num) {
    // Clear the "Probing..." line before printing the hop block
    cout << "\r" << string(80, ' ') << "\r";

    // Draw connector
    cout << COLOR_GRAY << "  │\n" << COLOR_RESET;
    
    if (hop.avg_rtt < 0) {
        // Timeout hop
        cout << COLOR_RED << "  ├──[" << BOLD << setw(2) << hop_num << COLOR_RESET << COLOR_RED << "]";
        cout << "──────────────────────────────────────────────────────────────────────\n";
        cout << "  │        " << BOLD << "* * * Request Timeout * * *" << COLOR_RESET << "\n";
    } else {
        // Valid hop
        const char* color = get_latency_color(hop.avg_rtt);
        
        // Header: Hop Number + Hostname
        cout << color << "  ├──[" << BOLD << setw(2) << hop_num << COLOR_RESET << color << "]";
        cout << "──────────────────────────────────────────────────────────────────────\n";
        cout << "  │     " << BOLD << COLOR_WHITE << "▌ " << hop.hostname << COLOR_RESET << "\n";
        
        // IP Address
        cout << COLOR_GRAY << "  │     " << COLOR_RESET;
        cout << "IP: " << COLOR_CYAN << hop.ip_address << COLOR_RESET << "\n";
        
        // Geo Info
        if (hop.geo.valid) {
            cout << COLOR_GRAY << "  │     " << COLOR_RESET;
            cout << "Location: " << COLOR_YELLOW;
            
            if (!hop.geo.city.empty()) {
                cout << hop.geo.city << ", ";
            }
            if (!hop.geo.region.empty() && hop.geo.region != hop.geo.city) {
                cout << hop.geo.region << ", ";
            }
            cout << hop.geo.country << " " << hop.geo.country_code;
            cout << COLOR_RESET << "\n";
            
            cout << COLOR_GRAY << "  │       " << COLOR_RESET;
            cout << "Coords: " << fixed << setprecision(4) 
                 << hop.geo.latitude << ", " << hop.geo.longitude << "\n";
            
            if (hop.distance_from_prev > 0) {
                cout << COLOR_GRAY << "  │       " << COLOR_RESET;
                cout << "Dist from prev: " << COLOR_MAGENTA 
                         << fixed << setprecision(1) << hop.distance_from_prev 
                         << " km" << COLOR_RESET << "\n";
            }
        }
        
        // ISP/ASN Info
        if (hop.network.valid) {
            cout << COLOR_GRAY << "  │     " << COLOR_RESET;
            cout << "ISP: " << COLOR_GREEN << hop.network.isp << COLOR_RESET << "\n";
            
            if (!hop.network.org.empty() && hop.network.org != hop.network.isp) {
                cout << COLOR_GRAY << "  │       " << COLOR_RESET;
                cout << "Org: " << hop.network.org << "\n";
            }
            
            if (!hop.network.asn.empty()) {
                cout << COLOR_GRAY << "  │       " << COLOR_RESET;
                cout << "ASN: " << COLOR_CYAN << hop.network.asn << COLOR_RESET;
                if (!hop.network.as_name.empty()) {
                    cout << " (" << hop.network.as_name << ")";
                }
                cout << "\n";
            }
        }
        
        // Latency Stats
        cout << COLOR_GRAY << "  │     " << COLOR_RESET;
        cout << "Latency: " << color << BOLD 
             << fixed << setprecision(2) << hop.avg_rtt << " ms" 
             << COLOR_RESET << " " << draw_latency_bar(hop.avg_rtt) << "\n";
        
        cout << COLOR_GRAY << "  │       " << COLOR_RESET;
        cout << "Range: " << hop.min_rtt << " ~ " << hop.max_rtt << " ms  |  ";
        cout << "StdDev: " << hop.std_dev << " ms  |  ";
        cout << "Jitter: " << hop.jitter << " ms\n";
        
        // Packet Loss
        if (hop.packet_loss > 0) {
            cout << COLOR_GRAY << "  │       " << COLOR_RESET;
            cout << COLOR_RED << "Packet Loss: " << hop.packet_loss << "%" << COLOR_RESET << "\n";
        }
    }
    
    if (hop.is_destination) {
        cout << COLOR_GRAY << "  ↓\n" << COLOR_RESET;
        cout << COLOR_GREEN << BOLD << "  ╰──→ Destination Reached" << COLOR_RESET << "\n\n";
    }
}

void print_summary(const vector<EnhancedHopInfo> &route) {
    // Summary Stats
    cout << COLOR_CYAN << BOLD;
    cout << "\n╔════════════════════════════════════════════════════════════════════════════════╗\n";
    cout << "║                           ROUTE SUMMARY STATISTICS                             ║\n";
    cout << "╚════════════════════════════════════════════════════════════════════════════════╝";
    cout << COLOR_RESET << "\n\n";
    
    double total_latency = 0;
    int successful_hops = 0;
    int timeout_hops = 0;
    double total_jitter = 0;
    double max_packet_loss = 0;
    double total_distance = 0;
    
    for (const auto &hop : route) {
        if (hop.distance_from_prev > 0) {
            total_distance += hop.distance_from_prev;
        }

        if (hop.avg_rtt > 0) {
            total_latency += hop.avg_rtt;
            total_jitter += hop.jitter;
            successful_hops++;
            if (hop.packet_loss > max_packet_loss) {
                max_packet_loss = hop.packet_loss;
            }
        } else {
            timeout_hops++;
        }
    }
    
    cout << BOLD << "Hop Stats:" << COLOR_RESET << "\n";
    cout << "   Total Hops: " << route.size() << "\n";
    cout << "   Successful: " << COLOR_GREEN << successful_hops << COLOR_RESET << "\n";
    cout << "   Timed Out:  " << COLOR_RED << timeout_hops << COLOR_RESET << "\n\n";
    
    cout << BOLD << "Latency Stats:" << COLOR_RESET << "\n";
    cout << "   Total End-to-End Latency: " << COLOR_YELLOW << fixed << setprecision(2) 
              << total_latency << " ms" << COLOR_RESET << "\n";
    cout << "   Avg Latency per Hop:       " 
              << (successful_hops > 0 ? total_latency / successful_hops : 0) << " ms\n";
    cout << "   Avg Jitter:                " 
              << (successful_hops > 0 ? total_jitter / successful_hops : 0) << " ms\n";
    cout << "   Max Packet Loss:           " << max_packet_loss << "%\n\n";
    
    if (total_distance > 0) {
        cout << BOLD << "Geographic Stats:" << COLOR_RESET << "\n";
        cout << "   Total Physical Distance: " << COLOR_MAGENTA << fixed << setprecision(1) 
                  << total_distance << " km" << COLOR_RESET << "\n";
        cout << "   Approx. Propagation Speed: " 
                  << (total_latency > 0 ? (total_distance / (total_latency / 1000.0)) : 0) 
                  << " km/s\n";
        cout << "   (Speed of light in fiber is approx. 200,000 km/s)\n\n";
    }
}