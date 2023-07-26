#include <iostream>
#include <tins/tins.h>
#include "ndpi/ndpi_main.h"
#include "ndpi/ndpi_api.h"
#include <fstream>
#include <cassert>
#include <string>


#include <iomanip> // Inclure la bibliothèque pour manipuler le format de sortie 

int total_sizeip = 0;
int total_sizeother = 0;
int total_size = 0;

void affiche_protocol(const Tins::PDU &pdu);
int sum = 0;

struct Protocol_statistics{ // in bytes
    int safe = 0;
    int acceptable = 0;
    int fun = 0;
    int unsafe = 0;
};

struct Protocol{ // in bytes
    std::string name;
    int packets = 0;
    int bytes = 0;
    int flows = 0;
};

// Create a structure to store flow information
struct packet_info {
    int family; // AF_INET or AF_INET6

    //#TODO union
    uint32_t src_ip;
    Tins::IPv6::address_type src_ipv6;
    //#TODO union
    uint32_t dst_ip;
    Tins::IPv6::address_type dst_ipv6;

    uint16_t src_port;
    uint16_t dst_port;
    ndpi_protocol detected_protocol;
    uint64_t timestamp;
    const unsigned char* packet_data;
    uint16_t packetlen;
    const char* pdu_type = "";
    const char* pdu_subtype = "";
};

struct flow_input_info {
  struct ndpi_flow_struct *flow; // The flow to process
  void *flow_id; // The flow identifier
  unsigned char in_pkt_dir;
  unsigned char seen_flow_beginning;
};

struct flow_struct {
    ndpi_flow_struct ndpi_flow;
    packet_info packet_information;
};

// Global variable for ndpi_struct
extern struct ndpi_detection_module_struct* ndpi_struct = ndpi_init_detection_module(ndpi_no_prefs); // key word to remove  ? #TODO

// Protocols vector
std::vector<Protocol> protocols;

//Flow vector
std::vector<struct flow_struct *> flows;

uint32_t swapEndian(uint32_t value) {
    return ((value & 0xFF) << 24) | (((value >> 8) & 0xFF) << 16) | (((value >> 16) & 0xFF) << 8) | ((value >> 24) & 0xFF);
}

flow_struct& get_flow(packet_info &packet){
    // Check if the flow already exists else create it and add it to the vector
    bool found = false;
    for (int i = 0; i < flows.size(); i++) {
        if (flows[i]->packet_information.src_ip == packet.src_ip && flows[i]->packet_information.dst_ip == packet.dst_ip && flows[i]->packet_information.src_port == packet.src_port && flows[i]->packet_information.dst_port == packet.dst_port) {
            found = true;
            return *flows[i];
        }
    }
    if (!found) {
        struct flow_struct *flow = new struct flow_struct();
        flow->packet_information = packet;
        flows.push_back(flow);
        //std::cout << "Flow created " << flows.size() << std::endl;
        return *flow;
    }
}

void check_flow_aliveness(flow_struct &flow){ // use ndpi_detection_giveup
    uint8_t protocol_was_guessed = 0;
    ndpi_protocol guessed_protocol = ndpi_detection_giveup(ndpi_struct, &flow.ndpi_flow, 1, &protocol_was_guessed);
    if (protocol_was_guessed != 0) {
        //Delete flow
        std::cout << "Flow deleted " << flows.size() << std::endl;
        for (int i = 0; i < flows.size(); i++) {
            if (flows[i]->packet_information.src_ip == flow.packet_information.src_ip && flows[i]->packet_information.dst_ip == flow.packet_information.dst_ip && flows[i]->packet_information.src_port == flow.packet_information.src_port && flows[i]->packet_information.dst_port == flow.packet_information.dst_port) {
                flows.erase(flows.begin() + i);
                break;
            }
        }
        //delete &flow; //BUG
    } 
}


// Function to read a network packet and fill the packet structure
int read_packet(struct packet_info *packet, const Tins::PDU &pdu) {
    const Tins::Packet& tins_packet = static_cast<const Tins::Packet&>(pdu);
    const Tins::Timestamp& timestamp = tins_packet.timestamp();
    packet->timestamp = timestamp.seconds() * 1000 + timestamp.microseconds() / 1000; //[x] Is there a bug here ?

    if (packet->family == AF_INET) {
        Tins::IP ip = pdu.rfind_pdu<Tins::IP>(); // Get the IP layer of the packet
        packet->src_ip = pdu.rfind_pdu<Tins::IP>().src_addr(); 
        packet->dst_ip = pdu.rfind_pdu<Tins::IP>().dst_addr(); 
        std::vector<uint8_t> packet_vector = ip.serialize();
        // Allocate memory for packet->packet_data and copy packet data
        packet->packet_data = new unsigned char[packet_vector.size()];
        std::memcpy(const_cast<unsigned char*>(packet->packet_data), packet_vector.data(), packet_vector.size());
        packet->packetlen = packet_vector.size();
        packet->pdu_type = "IP";
    } else if (packet->family == AF_INET6) { 
        Tins::IPv6 ipv6 = pdu.rfind_pdu<Tins::IPv6>(); // Get the IPv6 layer of the packet
        packet->src_ipv6 = pdu.rfind_pdu<Tins::IPv6>().src_addr();
        packet->dst_ipv6 = pdu.rfind_pdu<Tins::IPv6>().dst_addr();
        std::vector<unsigned char> packet_vector = ipv6.serialize();
        // Allocate memory for packet->packet_data and copy packet data
        packet->packet_data = new unsigned char[packet_vector.size()];
        std::memcpy(const_cast<unsigned char*>(packet->packet_data), packet_vector.data(), packet_vector.size());
        packet->packetlen = packet_vector.size();
        packet->pdu_type = "IPv6";
    } else {
        std::cout << "Unknown family" << std::endl;
        return -1;
    }

    const Tins::TCP *tcp = pdu.find_pdu<Tins::TCP>(); // Get the TCP layer of the packet
    const Tins::UDP *udp = pdu.find_pdu<Tins::UDP>(); // Get the UDP layer of the packet
    if (udp != nullptr) {
        packet->src_port = udp->sport();
        packet->dst_port = udp->dport();
        packet->pdu_subtype = "UDP";
    } else if (tcp != nullptr) {
        packet->pdu_subtype = "TCP";
        packet->src_port = tcp->sport();
        packet->dst_port = tcp->dport();
    } else {
        std::cout << "Not TCP or UDP -> Unknown protocol" << std::endl;
        affiche_protocol(pdu);
        //TODO : add other protocols
    }

    //flow_struct* flow_ = new struct flow_struct();
    //flow_struct& flow = *flow_;
    flow_struct flow = get_flow(*packet);


    //// Use the same packet structure as the input structure
    //flow_input_info input;
    //memset(&input, 0, sizeof(input));
    //input.packet = packet->ndpi_flow; // Set the packet in the input structure
    //input.flow_id = packet->ndpi_flow; // Set the packet id in the input structure
    ////(const ndpi_flow_input_info *)&input) in function ndpi_detection_process_packet

    ndpi_finalize_initialization(ndpi_struct); // Finalize the nDPI initialization

    packet->detected_protocol = ndpi_detection_process_packet(ndpi_struct, &flow.ndpi_flow, packet->packet_data, packet->packetlen, packet->timestamp, NULL);
    
    check_flow_aliveness(flow);
    total_sizeip += pdu.size();
    return 0;
}

void detected_protocols(struct packet_info *packet) {
    const char *category_name = ndpi_category_get_name(ndpi_struct, packet->detected_protocol.category);
    const char *protocol_name = ndpi_get_proto_name(ndpi_struct, packet->detected_protocol.master_protocol);
    const char* appprotocol_name = ndpi_get_proto_name(ndpi_struct, packet->detected_protocol.app_protocol);
    const char *breed_name = ndpi_get_proto_breed_name(ndpi_struct, ndpi_get_proto_breed(ndpi_struct, packet->detected_protocol.master_protocol));
    bool found = false;
    for (int i = 0; i < protocols.size(); i++) {
        if (protocols[i].name == appprotocol_name) {
            found = true;
            protocols[i].packets++;
            protocols[i].bytes += packet->packetlen;
            break;
        }
    }
    if (!found) {
        Protocol p;
        p.name = appprotocol_name;
        p.packets = 1;
        p.bytes = packet->packetlen;
        protocols.push_back(p);
    }
    sum += packet->packetlen;
}

void print_detected_protocols() { // with iomanip
    std::cout << " --------------- Protocols ---------------" << std::endl;
    std::cout << std::setw(20) << std::left << "Name" << std::setw(15) << std::left << "Packets" << std::setw(15) << std::left << "Bytes" << std::endl;
    for (int i = 0; i < protocols.size(); i++) {
        std::cout << std::setw(20) << std::left << protocols[i].name << std::setw(15) << std::left << protocols[i].packets << std::setw(15) << std::left << protocols[i].bytes << std::endl;
    }
    std::cout << " ------------ End of Protocols ------------" << std::endl;
    std::cout << "Sum: " << sum << std::endl;
}

// Function to print the detected protocol category
void print_category(struct packet_info *packet) {
    const char *category_name = ndpi_category_get_name(ndpi_struct, packet->detected_protocol.category);
    const char *protocol_name = ndpi_get_proto_name(ndpi_struct, packet->detected_protocol.master_protocol);
    const char *subprotocol_name = ndpi_get_proto_name(ndpi_struct, packet->detected_protocol.app_protocol);
    const char *breed_name = ndpi_get_proto_breed_name(ndpi_struct, ndpi_get_proto_breed(ndpi_struct, packet->detected_protocol.master_protocol));
    if (packet->family == AF_INET) {
            int src = swapEndian(packet->src_ip);
            int dst = swapEndian(packet->dst_ip);
            std::cout << "Flow between " << ((src >> 24) & 0xFF) << "." << ((src >> 16) & 0xFF) << "."
            << ((src >> 8) & 0xFF) << "." << (src & 0xFF) << ":" << packet->src_port << " and "
            << ((dst >> 24) & 0xFF) << "." << ((dst >> 16) & 0xFF) << "."
            << ((dst >> 8) & 0xFF) << "." << (dst & 0xFF) << ":" << packet->dst_port
            << " is classified as " << category_name 
            << " | Detected protocol: " << protocol_name
            << " | Subprotocol: " << subprotocol_name
            << " | Breed: " << breed_name
            << " | Size: " << packet->packetlen << " bytes"
            << " | PDU type: " << packet->pdu_type
            << " | PDU subtype: " << packet->pdu_subtype
            << std::endl;
    } else if (packet->family == AF_INET6) {
            std::cout << "Flow between " << packet->src_ipv6 << ":" << packet->src_port << " and "
            << packet->dst_ipv6 << ":" << packet->dst_port
            << " is classified as " << category_name 
            << " | Detected protocol: " << protocol_name
            << " | Subprotocol: " << subprotocol_name
            << " | Breed: " << breed_name
            << " | Size: " << packet->packetlen << " bytes"
            << " | PDU type: " << packet->pdu_type
            << " | PDU subtype: " << packet->pdu_subtype
            << std::endl;
    } else {
        std::cout << "Unknown PDU type" << std::endl;
    }
    std::cout << "packet.packet_data: ";
    for (int i = 0; i < packet->packetlen; i++) {
        std::cout << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<unsigned>(packet->packet_data[i]) << " ";
    }
    std::cout << std::dec;
    std::cout << std::endl;
    std::cout << "packet.packetlen: " << packet->packetlen << std::endl;
    if (packet->family == AF_INET) {
        std::cout << "packet->src_ip: " << packet->src_ip << std::endl;
        std::cout << "packet->dst_ip: " << packet->dst_ip << std::endl;
    } else if (packet->family == AF_INET6) {
        std::cout << "packet->src_ipv6: " << packet->src_ipv6 << std::endl;
        std::cout << "packet->dst_ipv6: " << packet->dst_ipv6 << std::endl;
    }

    std::cout << "packet.src_port: " << packet->src_port << std::endl;
    std::cout << "packet.dst_port: " << packet->dst_port << std::endl;
    std::cout << "packet.timestamp: " << packet->timestamp << std::endl; 
    std::cout << std::endl;
    

    delete[] packet->packet_data; // Libérer la mémoire allouée pour packet_data
}

void affiche_protocol(const Tins::PDU &pdu){
    const Tins::IP *ip = pdu.find_pdu<Tins::IP>(); // Get the IP layer of the packet
    const Tins::TCP *tcp = pdu.find_pdu<Tins::TCP>(); // Get the TCP layer of the packet
    const Tins::UDP *udp = pdu.find_pdu<Tins::UDP>(); // Get the UDP layer of the packet
    const Tins::STP *stp = pdu.find_pdu<Tins::STP>(); // Get the STP layer of the packet
    const Tins::ICMP *icmp = pdu.find_pdu<Tins::ICMP>(); // Get the ICMP layer of the packet
    const Tins::ARP *arp = pdu.find_pdu<Tins::ARP>(); // Get the ARP layer of the packet
    const Tins::BootP *bootp = pdu.find_pdu<Tins::BootP>(); // Get the BootP layer of the packet
    const Tins::DHCP *dhcp = pdu.find_pdu<Tins::DHCP>(); // Get the DHCP layer of the packet
    const Tins::DHCPv6 *dhcpv6 = pdu.find_pdu<Tins::DHCPv6>(); // Get the DHCPv6 layer of the packet
    const Tins::DNS *dns = pdu.find_pdu<Tins::DNS>(); // Get the DNS layer of the packet
    const Tins::EthernetII *ethernet = pdu.find_pdu<Tins::EthernetII>(); // Get the Ethernet layer of the packet
    const Tins::ICMPv6 *icmpv6 = pdu.find_pdu<Tins::ICMPv6>(); // Get the ICMPv6 layer of the packet
    const Tins::IPv6 *ipv6 = pdu.find_pdu<Tins::IPv6>(); // Get the IPv6 layer of the packet
    const Tins::LLC *llc = pdu.find_pdu<Tins::LLC>(); // Get the LLC layer of the packet
    const Tins::MPLS *mpls = pdu.find_pdu<Tins::MPLS>(); // Get the MPLS layer of the packet
    const Tins::RawPDU *raw = pdu.find_pdu<Tins::RawPDU>(); // Get the Raw layer of the packet
    const Tins::Dot11 *dot11 = pdu.find_pdu<Tins::Dot11>(); // Get the Dot11 layer of the packet
    const Tins::SNAP *snap = pdu.find_pdu<Tins::SNAP>(); // Get the SNAP layer of the packet
    const Tins::Dot1Q *dot1q = pdu.find_pdu<Tins::Dot1Q>(); // Get the Dot1Q layer of the packet
    const Tins::Dot3 *dot3 = pdu.find_pdu<Tins::Dot3>(); // Get the Dot3 layer of the packet
    const Tins::EAPOL *eapol = pdu.find_pdu<Tins::EAPOL>(); // Get the EAPOL layer of the packet
    const Tins::IPSecAH *ipsec_ah = pdu.find_pdu<Tins::IPSecAH>(); // Get the IPSecAH layer of the packet
    const Tins::IPSecESP *ipsec_esp = pdu.find_pdu<Tins::IPSecESP>(); // Get the IPSecESP layer of the packet
    const Tins::PPPoE *pppoe = pdu.find_pdu<Tins::PPPoE>(); // Get the PPPoE layer of the packet
    const Tins::RadioTap *radiotap = pdu.find_pdu<Tins::RadioTap>(); // Get the RadioTap layer of the packet
    const Tins::SLL *sll = pdu.find_pdu<Tins::SLL>(); // Get the SLL layer of the packet
    std :: cout << " --- Protocol :  ---" << std :: endl;
    if (tcp != nullptr) {
        std :: cout << "TCP" << std :: endl;
    }
    if (udp != nullptr) {
        std :: cout << "UDP" << std :: endl;
    }
    if (stp != nullptr) {
        std :: cout << "STP" << std :: endl;
    }
    if (icmp != nullptr) {
        std :: cout << "ICMP" << std :: endl;
    }
    if (arp != nullptr) {
        std :: cout << "ARP" << std :: endl;
    }
    if (bootp != nullptr) {
        std :: cout << "BootP" << std :: endl;
    }
    if (dhcp != nullptr) {
        std :: cout << "DHCP" << std :: endl;
    }
    if (dhcpv6 != nullptr) {
        std :: cout << "DHCPv6" << std :: endl;
    }
    if (dns != nullptr) {
        std :: cout << "DNS" << std :: endl;
    }
    if (ethernet != nullptr) {
        std :: cout << "Ethernet" << std :: endl;
    }
    if (icmpv6 != nullptr) {
        std :: cout << "ICMPv6" << std :: endl;
    }
    if (ipv6 != nullptr) {
        std :: cout << "IPv6" << std :: endl;
    }
    if (llc != nullptr) {
        std :: cout << "LLC" << std :: endl;
    }
    if (mpls != nullptr) {
        std :: cout << "MPLS" << std :: endl;
    }
    if (raw != nullptr) {
        std :: cout << "Raw" << std :: endl;
    }
    if (dot11 != nullptr) {
        std :: cout << "Dot11" << std :: endl;
    }
    if (snap != nullptr) {
        std :: cout << "SNAP" << std :: endl;
    }
    if (dot1q != nullptr) {
        std :: cout << "Dot1Q" << std :: endl;
    }
    if (dot3 != nullptr) {
        std :: cout << "Dot3" << std :: endl;
    }
    if (eapol != nullptr) {
        std :: cout << "EAPOL" << std :: endl;
    }
    if (ipsec_ah != nullptr) {
        std :: cout << "IPSecAH" << std :: endl;
    }
    if (ipsec_esp != nullptr) {
        std :: cout << "IPSecESP" << std :: endl;
    }
    if (pppoe != nullptr) {
        std :: cout << "PPPoE" << std :: endl;
    }
    if (radiotap != nullptr) {
        std :: cout << "RadioTap" << std :: endl;
    }
    if (sll != nullptr) {
        std :: cout << "SLL" << std :: endl;
    }
    std :: cout << " --- End of Protocol ---" << std :: endl;
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
    //config.set_filter("stp"); // Set filter for IP packets
    // Use the default interface
    Tins::NetworkInterface iface = Tins::NetworkInterface::default_interface();
    // print Default interface
    std::cout << "Default interface name: " << iface.name();
    std::wcout << " (" << iface.friendly_name() << ")" << std::endl;
    // Now instantiate the sniffer
    //Tins::Sniffer sniffer(iface.name(),config);
    Tins::FileSniffer sniffer("data/a.pcap"); // Open the pcap file for reading

    // Set the detection bitmask
    NDPI_PROTOCOL_BITMASK all;
    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_struct, &all);
    int nb = 0;

    sniffer.sniff_loop([&](const Tins::PDU &pdu) -> bool {
        struct packet_info packet;
        total_size += pdu.size();
        const Tins::IP* ip_test = pdu.find_pdu<Tins::IP>(); // Get the IP layer of the packet
        const Tins::IPv6* ipv6_test = pdu.find_pdu<Tins::IPv6>(); // Get the IPv6 layer of the packet
        if (ip_test != nullptr) {
            packet.family = AF_INET;
            if (read_packet(&packet, pdu) == 0) {
                //print_category(&packet);
                detected_protocols(&packet);
            }else{
                throw std::runtime_error("Error reading packet");
            }
        } else if (ipv6_test != nullptr){
            packet.family = AF_INET6;
            if (read_packet(&packet, pdu) == 0) {
                //print_category(&packet);
                detected_protocols(&packet);
            }else{
                throw std::runtime_error("Error reading packet");
            }
        }else{
            const Tins::STP* stp_test = pdu.find_pdu<Tins::STP>(); // Get the STP layer of the packet
            const Tins::Dot3* dot3_test = pdu.find_pdu<Tins::Dot3>(); // Get the Dot3 layer of the packet
            const Tins::LLC* llc_test = pdu.find_pdu<Tins::LLC>(); // Get the LLC layer of the packet
            if (stp_test != nullptr || dot3_test != nullptr || llc_test != nullptr) {
                //std::cout << " it's a STP or Dot3 or LLC packet" << std::endl;
                total_sizeother += pdu.size();
                //TODO Look to classify this 3 protocols
            } else {
                std::cout << "Not an IP or IPv6 packet, it's a " << pdu.pdu_type() << std::endl;
                affiche_protocol(pdu);
                //TODO ?
            }
        }
        nb++;
        if (nb == -1) {
            std::cout << "Total size: " << total_size << std::endl;
            std::cout << "Total sizeip: " << total_sizeip << std::endl;
            std::cout << "Total sizestp: " << total_sizeother << std::endl;
            std::cout << "sum size: " << total_sizeip + total_sizeother << std::endl;
            return false;
        }
        return true;
    });
    print_detected_protocols();
    std::cout << "Total sizeip: " << total_sizeip << std::endl;
    std::cout << "Total size: " << total_size << std::endl;
    ndpi_exit_detection_module(ndpi_struct);
    return 0;
}
