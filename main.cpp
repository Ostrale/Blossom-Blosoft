#include <iostream>
#include <tins/tins.h>
#include "ndpi/ndpi_main.h"
#include "ndpi/ndpi_api.h"
#include "ndpi/ndpi_protocols.h"
#include "ndpi/ndpi_protocol_ids.h"
#include <fstream>
#include <cassert>
#include <string>

#include <iomanip>

// Define comportement of the program
#define TEST
#define DEBUG

// Define constants
#define MAX_IDLE_TIME 300000 /* msec */

extern struct ndpi_detection_module_struct* ndpi_struct = ndpi_init_detection_module(ndpi_no_prefs); // key word to remove  ? #TODO

// Create a structure to store flow information
struct packet_struct {
    uint64_t timestamp;
    const unsigned char* packet_data;
    uint16_t packetlen;
};

struct flow_info{
    int family; // AF_INET ou AF_INET6
        uint32_t src_ip; // Make union if possible
        Tins::IPv6::address_type src_ipv6;
        uint32_t dst_ip;
        Tins::IPv6::address_type dst_ipv6;
    uint16_t src_port;
    uint16_t dst_port;
    ndpi_protocol detected_protocol; 
    bool is_tcp;
};

struct flow_struct {
    flow_info info;
    ndpi_flow_struct ndpi_flow;
    std::vector<packet_struct> packets;
};

struct App_Protocol{ // in bytes
    std::string name;
    int packets = 0;
    int bytes = 0;
    int flows = 0;
};

std::vector<struct flow_struct *> flows;
std::vector<struct flow_struct *> archived_flows; // In Debug file

//compare function that compares IP addresses, ports and protocol of the packet with those of the stream
bool compare_flow(const flow_struct &flow, const flow_info &info_packet) {
    if (flow.info.family == info_packet.family) {
        if (flow.info.family == AF_INET) {
            if (flow.info.src_ip == info_packet.src_ip && flow.info.dst_ip == info_packet.dst_ip && flow.info.src_port == info_packet.src_port && flow.info.dst_port == info_packet.dst_port) {
                return true;
            }
        } else if (flow.info.family == AF_INET6) {
            if (flow.info.src_ipv6 == info_packet.src_ipv6 && flow.info.dst_ipv6 == info_packet.dst_ipv6 && flow.info.src_port == info_packet.src_port && flow.info.dst_port == info_packet.dst_port) {
                return true;
            }
        }
    }
    return false;
}

void flow_heal_check(flow_struct &flow, Tins::PDU &pdu) {
    //Si le flux est un flux TCP, il regarde si le paquet contient un flag FIN et ACK, qui indique que les deux parties veulent terminer la connexion. Dans ce cas, il met à jour le champ flow_fin_ack_seen du flux et le déplace dans la liste des flux inactifs.
    //Si le flux a dépassé un certain temps d’inactivité (défini par MAX_IDLE_TIME), il le déplace aussi dans la liste des flux inactifs.
    //Si le flux a atteint le nombre maximum de paquets à traiter pour la détection (défini par 0xFF), il arrête d’ajouter des paquets à ce flux et essaie de deviner son protocole avec la fonction ndpi_detection_giveup
    bool flow_is_done = false;
    // TCP ?
    if (flow.info.is_tcp) { 
        const Tins::TCP *tcp = pdu.find_pdu<Tins::TCP>();
        if (tcp != nullptr) {
            if (tcp->get_flag(Tins::TCP::FIN) && tcp->get_flag(Tins::TCP::ACK)) {
                flow_is_done = true;
            }
        }
    }
    // time : MAX_IDLE_TIME 
    if (flow.packets.back().timestamp - flow.packets.front().timestamp > MAX_IDLE_TIME) {
        flow_is_done = true;
    }
    // number of packets : 0xFF
    if (flow.packets.size() > 0xFF) {
        flow_is_done = true;
    }

    if (flow_is_done) {
        // verify if the protocol is known
        uint8_t* is_proto_guessed = 0;
        if (flow.info.detected_protocol.master_protocol == NDPI_PROTOCOL_UNKNOWN) {
            ndpi_detection_giveup(ndpi_struct, &flow.ndpi_flow, 1, is_proto_guessed);
            //TODO : verifier si le protocole est connu ou
        }

        archived_flows.push_back(&flow);
        flows.erase(std::remove(flows.begin(), flows.end(), &flow), flows.end());
    }

}

int read_packet(flow_info *info_packet, packet_struct &packet, const Tins::PDU &pdu) {
    flow_struct* flow_to_process_;
    bool flow_found = false;
    for (auto &flow : flows) { // Is there a better way to do this ? #TODO
        if (compare_flow(*flow, *info_packet)) {
            flow_found = true;
            flow_to_process_ = flow;
            break;
        }
    }
    if (!flow_found) {
        flow_struct *new_flow = new struct flow_struct();
        new_flow->info = *info_packet;
        new_flow->info.family = info_packet->family;
        flows.push_back(new_flow);
        flow_to_process_ = new_flow;
    }
    //std::cout << flow_to_process_->packets.size() << std::endl;
    flow_to_process_->packets.push_back(packet);//BUG
    flow_struct& flow_to_process = *flow_to_process_;

    ndpi_finalize_initialization(ndpi_struct);

    
    ndpi_protocol p_d = ndpi_detection_process_packet(ndpi_struct, &flow_to_process.ndpi_flow, packet.packet_data, packet.packetlen, packet.timestamp, NULL);
    flow_to_process.info.detected_protocol = p_d;
    //flow_heal_check(flow_to_process, const_cast<Tins::PDU&>(pdu)); //FIXME 
    return 0;
}

std::vector<App_Protocol> protocols;

void detected_protocols( flow_struct &flow, std::vector<App_Protocol> &protocols) {
    const char* appprotocol_name = ndpi_get_proto_name(ndpi_struct, flow.info.detected_protocol.app_protocol);
    //const char* appprotocol_name = ndpi_get_proto_name(ndpi_struct, flow.ndpi_flow.detected_protocol_stack[0]); //
    // si c'est un QUIC, il faut regarder le ALPN
    if (flow.ndpi_flow.detected_protocol_stack[0] == NDPI_PROTOCOL_QUIC) {
        // pour tous les paquets du flux
    }
    bool found = false;
    for (int i = 0; i < protocols.size(); i++){
        if (protocols[i].name == appprotocol_name){
            protocols[i].flows++;
            for (auto &packet : flow.packets){
                protocols[i].packets++;
                protocols[i].bytes += packet.packetlen;
            }
            found = true;
        }
    }
    if (!found){
        App_Protocol new_protocol;
        new_protocol.name = appprotocol_name;
        new_protocol.flows = 1;
        for (auto &packet : flow.packets){
            new_protocol.packets++;
            new_protocol.bytes += packet.packetlen;
        }
        protocols.push_back(new_protocol);
    }
}

void QUIC_detection(const flow_struct &flow) {
    flow.ndpi_flow.protos.tls_quic.advertised_alpns;
}

void print_detected_protocols(std::vector<App_Protocol> &protocols){
    std::cout << " ---------------------- Protocols ----------------------" << std::endl;
    std::cout << std::setw(20) << std::left << "Name" << std::setw(15) << std::left << "Packets" << std::setw(15) << std::left << "Bytes" << std::setw(15) << std::left << "Flows" << std::endl;
    for (int i = 0; i < protocols.size(); i++) {
        std::cout << std::setw(20) << std::left << protocols[i].name << std::setw(15) << std::left << protocols[i].packets << std::setw(15) << std::left << protocols[i].bytes << std::setw(15) << std::left << protocols[i].flows << std::endl;
    }
    std::cout << " ------------------- End of Protocols -------------------" << std::endl;
}

int main() {
    // Initialize nDPI
    std::vector<App_Protocol> protocols;
    if (ndpi_struct == nullptr) {
        std::cout << "nDPI initialization failed" << std::endl;
        return -1;
    }

    // Read packets and detect protocols
    Tins::SnifferConfiguration config;
    config.set_promisc_mode(true);                                              // Enable promiscuous mode
    Tins::NetworkInterface iface = Tins::NetworkInterface::default_interface(); // Use the default interface
    std::cout << "Default interface name: " << iface.name();
    std::wcout << " (" << iface.friendly_name() << ")" << std::endl;
#if defined TEST
    Tins::FileSniffer sniffer("data/a.pcap"); // Open the pcap file for reading
#elif !defined TEST
    Tins::Sniffer sniffer(iface.name(), config); // Now instantiate the sniffer
#endif

    NDPI_PROTOCOL_BITMASK all; // Set the detection bitmask : all protocols
    NDPI_BITMASK_SET_ALL(all); 
    ndpi_set_protocol_detection_bitmask2(ndpi_struct, &all);

    sniffer.sniff_loop([&](const Tins::PDU &pdu) -> bool {
        const Tins::IP *is_ip = pdu.find_pdu<Tins::IP>();
        const Tins::IPv6 *is_ipv6 = pdu.find_pdu<Tins::IPv6>();
        const Tins::TCP *tcp = pdu.find_pdu<Tins::TCP>();
        const Tins::UDP *udp = pdu.find_pdu<Tins::UDP>();

        packet_struct packet;
        const Tins::Packet& tins_packet = static_cast<const Tins::Packet&>(pdu);
        const Tins::Timestamp& timestamp = tins_packet.timestamp();
        packet.timestamp = timestamp.seconds() * 1000 + timestamp.microseconds() / 1000; //[x] Is there a bug here ?

        if ((is_ip != nullptr || is_ipv6 != nullptr) && (tcp != nullptr || udp != nullptr)) {
            flow_info packet_info;
            if (is_ip != nullptr) {
                Tins::IP ip = pdu.rfind_pdu<Tins::IP>();
                packet_info.family = AF_INET;
                packet_info.src_ip = ip.src_addr();
                packet_info.dst_ip = ip.src_addr();
                std::vector<uint8_t> packet_vector = ip.serialize(); // a list can be used instead of a vector #TODO
                packet.packet_data = new unsigned char[packet_vector.size()]; //TODO need to be deleted
                std::memcpy(const_cast<unsigned char*>(packet.packet_data), packet_vector.data(), packet_vector.size());
                packet.packetlen = packet_vector.size();
            } else if (is_ipv6 != nullptr) {
                Tins::IPv6 ipv6 = pdu.rfind_pdu<Tins::IPv6>();
                packet_info.family = AF_INET6;
                packet_info.src_ipv6 = ipv6.src_addr();
                packet_info.dst_ipv6 = ipv6.dst_addr();
                std::vector<uint8_t> packet_vector = ipv6.serialize(); // a list can be used instead of a vector #TODO
                packet.packet_data = new unsigned char[packet_vector.size()]; //TODO need to be deleted
                std::memcpy(const_cast<unsigned char*>(packet.packet_data), packet_vector.data(), packet_vector.size());
                packet.packetlen = packet_vector.size(); 
            }
            if (tcp != nullptr) {
                packet_info.src_port = tcp->sport();
                packet_info.dst_port = tcp->dport();
                packet_info.is_tcp = true;
            } else if (udp != nullptr) {
                packet_info.src_port = udp->sport();
                packet_info.dst_port = udp->dport();
                packet_info.is_tcp = false;
            }

            //read_packet(packet_info, packet, pdu);
            read_packet(&packet_info, packet, pdu);
            

        } else{
            //TODO
            //Pour les autres protocoles, comme ICMP ou ICMPv6, il n’y a pas de notion de flux, donc on peut traiter chaque paquet individuellement.
        }
        return true;
    });
    // print size of flows and archived flows
    std::cout << "Size of flows : " << flows.size() << std::endl;
    std::cout << "Size of archived flows : " << archived_flows.size() << std::endl;
    // move flows to archived flows
    for (auto &flow : flows) {
        archived_flows.push_back(flow);
    }
    
    for (auto &flow : archived_flows) {
        detected_protocols(*flow, protocols);
    }
    print_detected_protocols(protocols);
    return 0;
}