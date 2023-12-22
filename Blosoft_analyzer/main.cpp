#include <iostream>
#include <tins/tins.h>
#include "ndpi/ndpi_main.h"
#include <fstream>
#include <cassert>
#include <string>

#include <iomanip>
# include <memory>
#include <algorithm> // for std::remove

// Define comportement of the program
#define TEST
#define DEBUG

// Define constants
#define MAX_IDLE_TIME 300000 /* msec */

extern struct ndpi_detection_module_struct* ndpi_struct = ndpi_init_detection_module(ndpi_no_prefs); // key word to remove  ? #TODO

// Create a structure to store flow information
struct packet_struct {
    uint64_t timestamp;
    unsigned char* packet_data;
    uint16_t packetlen;
    ndpi_flow_input_info input_info = {NULL, NULL};
    /*
    ndpi_flow_input_info *input_info
    Public Members :
        unsigned char in_pkt_dir
        unsigned char seen_flow_beginning

    n_pkt_dir : un octet non signé qui indique la direction du paquet par rapport à l’observateur. Il peut prendre les valeurs 0 (entrant) ou 1 (sortant).
    seen_flow_beginning : un octet non signé qui indique si le début du flux a été observé. Il peut prendre les valeurs 0 (non) ou 1 (oui).
    */
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



void flow_heal_check(std::unique_ptr<flow_struct> &flow, Tins::PDU &pdu, std::vector<std::unique_ptr<flow_struct>> &flows, std::vector<std::unique_ptr<flow_struct>> &archived_flows) {
    // Fossayeur de flux

    //Si le flux est un flux TCP, il regarde si le paquet contient un flag FIN et ACK, qui indique que les deux parties veulent terminer la connexion. Dans ce cas, il met à jour le champ flow_fin_ack_seen du flux et le déplace dans la liste des flux inactifs.
    //Si le flux a dépassé un certain temps d’inactivité (défini par MAX_IDLE_TIME), il le déplace aussi dans la liste des flux inactifs.
    //Si le flux a atteint le nombre maximum de paquets à traiter pour la détection (défini par 0xFF), il arrête d’ajouter des paquets à ce flux et essaie de deviner son protocole avec la fonction ndpi_detection_giveup
    bool flow_is_done = false;
    // TCP ?
    if (flow->info.is_tcp) {
        const Tins::TCP *tcp = pdu.find_pdu<Tins::TCP>();
        if (tcp != nullptr) {
            if (tcp->get_flag(Tins::TCP::FIN) && tcp->get_flag(Tins::TCP::ACK)) {
                flow_is_done = true;
            }
        }
    }
    // time : MAX_IDLE_TIME 
    if (flow->packets.back().timestamp - flow->packets.front().timestamp > MAX_IDLE_TIME) {
        flow_is_done = true;
    }
    // number of packets : 0xFF
    if (flow->packets.size() > 0xFF) {
        flow_is_done = true;
    }

    if (flow_is_done) {
        // vérifiez si le protocole est connu
        uint8_t is_proto_guessed_value = 0; // Initialise avec une valeur par défaut
        uint8_t* is_proto_guessed = &is_proto_guessed_value;
        if (flow->info.detected_protocol.master_protocol == NDPI_PROTOCOL_UNKNOWN) {
            // Appel de la fonction ndpi_detection_giveup pour essayer de deviner le protocole en dernier recours
            ndpi_detection_giveup(ndpi_struct, &flow->ndpi_flow, 1, is_proto_guessed);
            // TODO: Gérer le cas où le protocole est inconnu
        }
   
#ifdef DEBUG        
        // Déplacez le flux vers archived_flows (attention on déplace le ownership) 
        archived_flows.push_back(std::move(flow)); 
#endif
    }else{
        // Déplacez le flux vers flows (attention on déplace le ownership)
        flows.push_back(std::move(flow));
    }
}


//compare function that compares IP addresses, ports and protocol of the packet with those of the stream
bool compare_flow(flow_struct &flow, const flow_info &info_packet) {
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

bool find_and_move_flow(const flow_info &info_packet, std::vector<std::unique_ptr<flow_struct>> &flows, std::unique_ptr<flow_struct> &flow_found_ptr) {
    auto it = std::find_if(flows.begin(), flows.end(), [&](const auto &flow) {
        return compare_flow(*flow, info_packet);
    });

    if (it != flows.end()) {
        // Flux trouvé, transfert de propriété
        flow_found_ptr = std::move(*it);
        flows.erase(it); // Supprime le flux du vecteur flows
        return true;
    }

    return false;  // Flux non trouvé
}

int read_packet(flow_info *info_packet, packet_struct &packet, const Tins::PDU &pdu, std::vector<std::unique_ptr<flow_struct>> &flows, std::vector<std::unique_ptr<flow_struct>> &archived_flows) {
    std::unique_ptr<flow_struct> flow_to_process_ptr;

    // Cherche si le flux existe déjà
    if (find_and_move_flow(*info_packet, flows, flow_to_process_ptr)) {
        // Flux trouvé, transfert de propriété effectué dans la fonction
        packet.input_info.seen_flow_beginning = 1;
    } else {
        // Flux non trouvé, crée un nouveau flux
        flow_to_process_ptr = std::make_unique<flow_struct>();
        flow_to_process_ptr->info = *info_packet;
        flow_to_process_ptr->info.family = info_packet->family;
        packet.input_info.seen_flow_beginning = 0;
    }
    flow_to_process_ptr->packets.push_back(packet);
    
    ndpi_protocol p_d = ndpi_detection_process_packet(ndpi_struct, &flow_to_process_ptr->ndpi_flow, packet.packet_data, packet.packetlen, packet.timestamp, &packet.input_info);

    flow_to_process_ptr->info.detected_protocol = p_d;

    // fossoyeur de flux : il va prendre le flux et le déplacer dans archived_flows ou flows en fonction de son état (flows et archived_flows sont passés par référence pour ne pas dupliquer les données)
    flow_heal_check(flow_to_process_ptr, const_cast<Tins::PDU &>(pdu), flows, archived_flows);

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

// Function to set packet family and source/destination IP addresses
void set_packet_info(const Tins::IP *ipv4_pdu, const Tins::IPv6 *ipv6_pdu, const Tins::TCP *tcp, const Tins::UDP *udp,
                     flow_info &packet_info, packet_struct &packet) {
    if (ipv4_pdu != nullptr) {
        Tins::IP ip = ipv4_pdu->rfind_pdu<Tins::IP>();
        packet_info.family = AF_INET;
        packet_info.src_ip = ip.src_addr();
        packet_info.dst_ip = ip.dst_addr();
        std::vector<uint8_t> packet_vector = ip.serialize();
        packet.packet_data = new unsigned char[packet_vector.size()];
        std::copy(packet_vector.begin(), packet_vector.end(), packet.packet_data);
        packet.packetlen = static_cast<uint16_t>(packet_vector.size());
    } else if (ipv6_pdu != nullptr) {
        Tins::IPv6 ipv6 = ipv6_pdu->rfind_pdu<Tins::IPv6>();
        packet_info.family = AF_INET6;
        packet_info.src_ipv6 = ipv6.src_addr();
        packet_info.dst_ipv6 = ipv6.dst_addr();
        std::vector<uint8_t> packet_vector = ipv6.serialize();
        packet.packet_data = new unsigned char[packet_vector.size()];
        std::copy(packet_vector.begin(), packet_vector.end(), packet.packet_data);
        packet.packetlen = static_cast<uint16_t>(packet_vector.size());
    }
}

// Function to set source/destination ports and packet type (TCP/UDP)
void set_packet_ports_and_type(const Tins::TCP *tcp, const Tins::UDP *udp, flow_info &packet_info) {
    if (tcp != nullptr) {
        packet_info.src_port = tcp->sport();
        packet_info.dst_port = tcp->dport();
        packet_info.is_tcp = true;
    } else if (udp != nullptr) {
        packet_info.src_port = udp->sport();
        packet_info.dst_port = udp->dport();
        packet_info.is_tcp = false;
    }
}

int Incoming_outcoming(const Tins::IP *ipv4_pdu, const Tins::IPv6 *ipv6_pdu, const Tins::TCP *tcp, const Tins::UDP *udp) {
    if (ipv4_pdu) {
        if (ipv4_pdu->src_addr() == ipv4_pdu->dst_addr()) {
            return -2; // Erreur : adresse source et adresse de destination identiques
        } else if (ipv4_pdu->src_addr() == ipv4_pdu->dst_addr()) {
            return 0; // Paquet sortant
        } else if (ipv4_pdu->dst_addr() == ipv4_pdu->dst_addr()) {
            return 1; // Paquet entrant
        }
    } else if (ipv6_pdu) {
        if (ipv6_pdu->src_addr() == ipv6_pdu->dst_addr()) {
            return -3; // Erreur : adresse source et adresse de destination identiques
        } else if (ipv6_pdu->src_addr() == ipv6_pdu->dst_addr()) {
            return 0; // Paquet sortant
        } else if (ipv6_pdu->dst_addr() == ipv6_pdu->dst_addr()) {
            return 1; // Paquet entrant
        }
    }

    if (! ipv4_pdu & ! ipv6_pdu) { // si ce n'est pas un paquet IPv4 ou IPv6
        return -4; // Erreur : paquet non pris en charge
    }
    return -1; // Erreur : paquet non pris en charge ou informations manquantes
}


int main() {
    // Initialize nDPI
    std::vector<App_Protocol> protocols;
    if (ndpi_struct == nullptr) {
        std::cout << "nDPI initialization failed" << std::endl;
        return -1;
    }

    // Vector for storing packet flows
    std::vector<std::unique_ptr<flow_struct>> flows;
    std::vector<std::unique_ptr<flow_struct>> archived_flows;

    // Read packets and detect protocols
    Tins::SnifferConfiguration config;
    config.set_promisc_mode(true);  // Enable promiscuous mode

    // Use the default network interface
    Tins::NetworkInterface iface = Tins::NetworkInterface::default_interface();
    std::cout << "Default interface name: " << iface.name();
    std::wcout << " (" << iface.friendly_name() << ")" << std::endl;

#if defined TEST
    Tins::FileSniffer sniffer("data/a.pcap");  // Open the pcap file for reading
#elif !defined TEST
    Tins::Sniffer sniffer(iface.name(), config);  // Instantiate the sniffer
#endif

    // Set the detection bitmask: all protocols
    NDPI_PROTOCOL_BITMASK all;
    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_struct, &all); 
    ndpi_finalize_initialization(ndpi_struct);

    // Sniff packets in a loop
    sniffer.sniff_loop([&](const Tins::PDU &pdu) -> bool {
        const Tins::IP *ipv4_pdu = pdu.find_pdu<Tins::IP>();
        const Tins::IPv6 *ipv6_pdu = pdu.find_pdu<Tins::IPv6>();
        const Tins::TCP *tcp = pdu.find_pdu<Tins::TCP>();
        const Tins::UDP *udp = pdu.find_pdu<Tins::UDP>();

        // Packet information structure
        packet_struct packet;

        const Tins::Packet &tins_packet = static_cast<const Tins::Packet &>(pdu);
        const Tins::Timestamp &timestamp = tins_packet.timestamp();
        packet.timestamp = timestamp.seconds() * 1000 + timestamp.microseconds() / 1000;

        // Check if the packet is incoming or outgoing 
        if (Incoming_outcoming(ipv4_pdu, ipv6_pdu, tcp, udp) < 0) {
#ifdef DEBUG
            //std::cout << "error : " << Incoming_outcoming(ipv4_pdu, ipv6_pdu, tcp, udp) << std::endl;
#endif
        }else if (Incoming_outcoming(ipv4_pdu, ipv6_pdu, tcp, udp) == 0) {
            packet.input_info.in_pkt_dir = 0;
        }else if (Incoming_outcoming(ipv4_pdu, ipv6_pdu, tcp, udp) == 1) {
            packet.input_info.in_pkt_dir = 1;
        }
        //TODO : doit on garder cette vérification ?

        if ((ipv4_pdu != nullptr || ipv6_pdu != nullptr) && (tcp != nullptr || udp != nullptr)) {
            flow_info packet_info;

            // Set packet family and source/destination IP addresses
            set_packet_info(ipv4_pdu, ipv6_pdu, tcp, udp, packet_info, packet);

            // Set source/destination ports and packet type (TCP/UDP)
            set_packet_ports_and_type(tcp, udp, packet_info);

            // Read and process the packet
            read_packet(&packet_info, packet, pdu, flows, archived_flows);

            // Use smart pointers to manage memory
            delete[] packet.packet_data;
        } else {
            // TODO: Handle other protocols like ICMP or ICMPv6 individually
        }
        return true;
    });

    // Print size of flows
    std::cout << "Size of flows: " << flows.size() << std::endl;

#ifdef DEBUG
    // Print size of archived flows (if DEBUG is defined)
    std::cout << "Size of archived flows: " << archived_flows.size() << std::endl;
#endif

    // Move flows to archived flows
    for (auto &flow : flows) {
        archived_flows.push_back(std::move(flow));
    }

    // Detect protocols for archived flows
    for (auto &flow : archived_flows) {
        detected_protocols(*flow, protocols);
    }

    // Print the detected protocols
    print_detected_protocols(protocols);

    ndpi_exit_detection_module(ndpi_struct);

    return 0;
}
