#include <iostream>
#include <tins/tins.h>
#include "ndpi_main.h"
#include "ndpi_api.h"
#include <fstream>

// Create a structure to store flow information
struct ndpi_flow_info {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    ndpi_protocol detected_protocol;
    struct ndpi_flow_struct *ndpi_flow;
};

struct flow_input_info {
  struct ndpi_flow_struct *flow; // The flow to process
  void *flow_id; // The flow identifier
  unsigned char in_pkt_dir;
  unsigned char seen_flow_beginning;
};

// Global variable for ndpi_struct
struct ndpi_detection_module_struct* ndpi_struct = ndpi_init_detection_module(ndpi_no_prefs);

// Function to read a network packet and fill the flow structure
int read_packet(struct ndpi_flow_info *flow, const Tins::PDU &pdu) {
    const Tins::IP &ip = pdu.rfind_pdu<Tins::IP>(); // Get the IP layer of the packet
    const Tins::TCP &tcp = pdu.rfind_pdu<Tins::TCP>(); // Get the TCP layer of the packet

    //copier ip dan ip2 qui est non const
    Tins::IP ip2 = pdu.rfind_pdu<Tins::IP>(); // Get the IP layer of the packet
    //void PDU::serialize(uint8_t* buffer, uint32_t total_sz)
    std::vector<uint8_t> packet_vector = ip2.serialize(); // Get the packet data
    const unsigned char* packet_data = packet_vector.data(); // Get the packet data
    const size_t packet_size = packet_vector.size(); // Get the packet size

    flow->src_ip = ip.src_addr();
    flow->dst_ip = ip.dst_addr();
    
    flow->src_port = tcp.sport();
    flow->dst_port = tcp.dport();

    flow->ndpi_flow = new struct ndpi_flow_struct();
    if (flow->ndpi_flow == nullptr) {
        std::cout << "Memory allocation error" << std::endl;
        return -1;
    }
    
    // Detect the protocol of the packet using nDPI
    //uint16_t packetlen = ip.tot_len();  // Get the packet length in bytes
    uint16_t packetlen = ip.size();  // Get the packet length in bytes = packet_size

    const Tins::Packet& packet = static_cast<const Tins::Packet&>(pdu);
    const Tins::Timestamp& timestamp = packet.timestamp();
    uint64_t time_ms = timestamp.seconds() * 1000 + timestamp.microseconds() / 1000;

    // Use the same flow structure as the input structure
    flow_input_info input;
    memset(&input, 0, sizeof(input));
    input.flow = flow->ndpi_flow; // Set the flow in the input structure
    input.flow_id = flow->ndpi_flow; // Set the flow id in the input structure


    ndpi_finalize_initialization (ndpi_struct); // Finalize the nDPI initialization

    flow->detected_protocol = ndpi_detection_process_packet(ndpi_struct, flow->ndpi_flow, packet_data, packet_size, time_ms, (const ndpi_flow_input_info *)&input);

    delete flow->ndpi_flow;
    return 0;
}

// Function to print the detected protocol category
void print_category(struct ndpi_flow_info *flow) {
    if (ndpi_struct == nullptr) {
        std::cout << "nDPI struct is null" << std::endl;
        return;
    }

    if (flow->ndpi_flow == nullptr) {
        std::cout << "nDPI flow is null" << std::endl;
        return;
    }

    if (flow->detected_protocol.category < 0 || flow->detected_protocol.category >= NDPI_PROTOCOL_NUM_CATEGORIES) {
        std::cout << "Invalid category: " << flow->detected_protocol.category << std::endl;
        return;
    }

    const char *category_name = ndpi_category_get_name(ndpi_struct, flow->detected_protocol.category);

    if (strcmp(category_name, "Unspecified") != 0){
        std::cout << "Flow between " << ((flow->src_ip >> 24) & 0xFF) << "." << ((flow->src_ip >> 16) & 0xFF) << "."
                << ((flow->src_ip >> 8) & 0xFF) << "." << (flow->src_ip & 0xFF) << ":" << flow->src_port << " and "
                << ((flow->dst_ip >> 24) & 0xFF) << "." << ((flow->dst_ip >> 16) & 0xFF) << "."
                << ((flow->dst_ip >> 8) & 0xFF) << "." << (flow->dst_ip & 0xFF) << ":" << flow->dst_port
                << " is classified as " << category_name << std::endl;
    }
}

int main() {
    // Initialize nDPI
    if (ndpi_struct == nullptr) {
        std::cout << "nDPI initialization failed" << std::endl;
        return -1;
    }

    // Read packets and detect protocols
    Tins::SnifferConfiguration config;
    config.set_promisc_mode(true); // Enable promiscuous mode
    config.set_filter("tcp"); // Set filter for TCP packets
    // Use the default interface
    Tins::NetworkInterface iface = Tins::NetworkInterface::default_interface();
    // print Default interface
    std::cout << "Default interface name: " << iface.name();
    std::wcout << " (" << iface.friendly_name() << ")" << std::endl;
    // Now instantiate the sniffer
    Tins::Sniffer sniffer(iface.name(),config);

    sniffer.sniff_loop([&](const Tins::PDU &pdu) -> bool {
        struct ndpi_flow_info flow;
        if (read_packet(&flow, pdu) == 0) {
            print_category(&flow);
        }
        return true;
    });

    
    ndpi_exit_detection_module(ndpi_struct);
    return 0;
}
