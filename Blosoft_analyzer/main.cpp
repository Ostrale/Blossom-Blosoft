#include <iostream>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

#include <tins/tins.h>
#include "ndpi/ndpi_main.h"
#include <fstream>
#include <cassert>
#include <string>

#include <unordered_map> // for std::unordered_map (fonction de hachage)
#include <queue> // for std::queue (file d'attente)

#include <iomanip>
#include <memory>
#include <algorithm> // for std::remove

#include "../Blosoft_DatabaseManager/include/sqlite/sqlite3.h"
#include "../Blosoft_DatabaseManager/DatabaseManager.hpp"

#include <time.h>
#define TEMPS_DE_RUN 600 // en secondes

// Define comportement of the program
#define TEST
#define DEBUG

#ifdef DEBUG
    #include <chrono>
    #define DEBUG_PRINT(x) std::cout << x << std::endl;
    std::vector<std::chrono::steady_clock::duration> time_points_find_and_move_flow;
#else
    #define DEBUG_PRINT(x)
#endif

template <typename T>
T myMin(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template <typename T>
T myMax(const T& a, const T& b) {
    return (a > b) ? a : b;
}


// Define constants
#define MAX_IDLE_TIME 100000 // in milliseconds - 100 seconds

extern struct ndpi_detection_module_struct* ndpi_struct = ndpi_init_detection_module(ndpi_no_prefs); // key word to remove  ? #TODO

// Create a structure to store flow information
struct packet_struct {
    uint64_t timestamp;
    unsigned char* packet_data;
    uint16_t packetlen;
    uint32_t packetbytes; 
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
    std::chrono::system_clock::time_point creation_timestamp;
    struct Hash { //TODO : à tester il y a peut être mieux à faire
        std::size_t operator()(const flow_info& info) const {
            std::size_t hash = std::hash<int>()(info.family);

            uint16_t min_port = myMin(info.src_port, info.dst_port);
            uint16_t max_port = myMax(info.src_port, info.dst_port);
            
            if (info.family == AF_INET) {
                uint32_t min_ip = myMin(info.src_ip, info.dst_ip);
                uint32_t max_ip = myMax(info.src_ip, info.dst_ip);

                hash_combine(hash, min_ip);
                hash_combine(hash, max_ip);
            } else if (info.family == AF_INET6) {
                auto min_ipv6 = myMin(info.src_ipv6, info.dst_ipv6);
                auto max_ipv6 = myMax(info.src_ipv6, info.dst_ipv6);

                hash_combine(hash, min_ipv6);
                hash_combine(hash, max_ipv6);
            }

            hash_combine(hash, min_port);
            hash_combine(hash, max_port);
            hash_combine(hash, info.is_tcp);

            return hash;
        }

        template <typename T>
        void hash_combine(std::size_t& seed, const T& value) const {
            seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed = 0;
        }
    };
    bool operator==(const flow_info& other) const {
        if (family != other.family) {
            return false;
        }
        if (family == AF_INET) {
            return (src_ip == other.src_ip && dst_ip == other.dst_ip && src_port == other.src_port && dst_port == other.dst_port) ||
                   (src_ip == other.dst_ip && dst_ip == other.src_ip && src_port == other.dst_port && dst_port == other.src_port);
        } else if (family == AF_INET6) {
            return (src_ipv6 == other.src_ipv6 && dst_ipv6 == other.dst_ipv6 && src_port == other.src_port && dst_port == other.dst_port) ||
                   (src_ipv6 == other.dst_ipv6 && dst_ipv6 == other.src_ipv6 && src_port == other.dst_port && dst_port == other.src_port);
        }
        return false;
    }
};

struct flow_struct {
    flow_info info;
    ndpi_flow_struct ndpi_flow;
    std::vector<packet_struct> packets;
};

using FlowsMap = std::unordered_map<flow_info, std::shared_ptr<flow_struct>, flow_info::Hash>;
using FlowsQueue = std::queue<std::pair<flow_info, std::shared_ptr<flow_struct>>*>;

std::string detect_protocol(std::shared_ptr<flow_struct> &flow) {
    // use ndpi_get_proto_name to get the name of the protocol
    const char* appprotocol_name = ndpi_get_proto_name(ndpi_struct, flow->info.detected_protocol.app_protocol);

    if (flow->info.detected_protocol.app_protocol == NDPI_PROTOCOL_UNKNOWN) {
        u_int8_t* protocol_was_guessed = new u_int8_t; // Allouer de la mémoire pour la variable
        *protocol_was_guessed = 0; // Initialiser la valeur de la variable

        auto a = ndpi_detection_giveup(ndpi_struct, &flow->ndpi_flow, 1, protocol_was_guessed);
        appprotocol_name = ndpi_get_proto_name(ndpi_struct, flow->ndpi_flow.detected_protocol_stack[0]);

        delete protocol_was_guessed;
    }
    return appprotocol_name;
}

std::string detect_Category(std::shared_ptr<flow_struct> &flow) {
    // use ndpi_get_proto_name to get the name of the category
    const char* appprotocol_category = ndpi_category_get_name(ndpi_struct, flow->info.detected_protocol.category);
    if (flow->info.detected_protocol.category == NDPI_PROTOCOL_CATEGORY_UNSPECIFIED) {
        u_int8_t* category_was_guessed = new u_int8_t; // Allouer de la mémoire pour la variable
        *category_was_guessed = 0; // Initialiser la valeur de la variable

        auto a = ndpi_detection_giveup(ndpi_struct, &flow->ndpi_flow, 1, category_was_guessed);
        appprotocol_category = ndpi_category_get_name(ndpi_struct, flow->info.detected_protocol.category);

        delete category_was_guessed;
    }

    return appprotocol_category;
}

void save_to_database(std::shared_ptr<flow_struct>& flow) {
    DatabaseManager dbManager("BlosoftDB.db");

    std::string name_protocol = detect_protocol(flow);
    std::string name_category = detect_Category(flow);

    int categoryId = dbManager.getCategoryId(name_category);

    if (categoryId == -1) {
        categoryId = dbManager.insertCategory(name_category);
    }

    // Summing the bytes of all packets in the flow
    int bytes = 0;
    for (auto &packet : flow->packets) {
        bytes += packet.packetbytes;
    }
    // Taking the timestamp of the flow from flow->info.creation_timestamp convert in Unix Timestamp
    unsigned long timestamp = std::chrono::duration_cast<std::chrono::seconds>(flow->info.creation_timestamp.time_since_epoch()).count();

    // Creating a DataEntry object
    DataEntry dataEntry{timestamp, bytes};

    // Creating a vector of DataEntry objects
    std::vector<DataEntry> dataEntries;
    dataEntries.push_back(dataEntry);

    dbManager.insertDataEntry(categoryId, dataEntries);
}

void clean_flows_map(FlowsMap &flows_map) {
    std::vector<std::shared_ptr<flow_struct>> flows_to_delete = {};
    for (auto &flow : flows_map) {
        flows_to_delete.push_back(std::move(flow.second));
    }
    for (auto &flow : flows_to_delete) {
        save_to_database(flow);
        flows_map.erase(flow->info);
    }
}

void flow_heal_check(FlowsMap &flows_map, FlowsQueue &flows_queue, std::vector<std::shared_ptr<flow_struct>> &archived_flows) {
    auto current_time = std::chrono::system_clock::now(); 
    auto last_timestamp_to_delete = std::chrono::time_point_cast<std::chrono::milliseconds>(current_time - std::chrono::milliseconds(1)).time_since_epoch().count();
    std::vector<std::shared_ptr<flow_struct>> flows_to_delete = {};
    for (auto &flow : flows_map) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - flow.second->info.creation_timestamp).count() > MAX_IDLE_TIME) {
            flows_to_delete.push_back(std::move(flow.second));
        }
    }
    for (auto &flow : flows_to_delete) {
        save_to_database(flow);
        flows_map.erase(flow->info);
        archived_flows.push_back(std::move(flow));
    }
}

int read_packet(flow_info *info_packet, packet_struct &packet, const Tins::PDU &pdu, FlowsMap &flows_map, std::vector<std::shared_ptr<flow_struct>> &archived_flows, FlowsQueue &flows_queue) {
    std::shared_ptr<flow_struct> flow_to_process_ptr;

    // TIME
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    // Cherche si le flux existe déjà
    auto it = flows_map.find(*info_packet);
    if (it != flows_map.end()) {
        // Flux trouvé, transfert de propriété
        flow_to_process_ptr = std::move(it->second);
        flows_map.erase(it); // Supprime le flux du vecteur flows
        packet.input_info.seen_flow_beginning = 1;
    } else {
        // Flux non trouvé, crée un nouveau flux
        flow_to_process_ptr = std::make_unique<flow_struct>();
        flow_to_process_ptr->info = *info_packet;
        flow_to_process_ptr->info.family = info_packet->family;
        packet.input_info.seen_flow_beginning = 0;
        flow_to_process_ptr->info.creation_timestamp = std::chrono::system_clock::now();
    }
    // TIME
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    //push_back end - begin to time_points_find_and_move_flow
    time_points_find_and_move_flow.push_back(end - begin);

    flow_to_process_ptr->packets.push_back(packet);

    //BUG filter :
    std::vector<packet_struct> packets1 = flow_to_process_ptr->packets;
    int count_packets1 = flow_to_process_ptr->packets.size();
    ndpi_protocol p_d = ndpi_detection_process_packet(ndpi_struct, &flow_to_process_ptr->ndpi_flow, packet.packet_data, packet.packetlen, packet.timestamp, &packet.input_info);
    //BUG filter :
    int count_packets2 = flow_to_process_ptr->packets.size();
    if (count_packets1 != count_packets2) {
        // on remet l'ancien vecteur de paquets
        //flow_to_process_ptr->packets = packets1;
        return -1;
    }

    flow_to_process_ptr->info.detected_protocol = p_d;

    flows_map.insert(std::make_pair(flow_to_process_ptr->info, std::move(flow_to_process_ptr)));
    

    // fossoyeur de flux : il va prendre le flux et le déplacer dans archived_flows ou flows en fonction de son état (flows et archived_flows sont passés par référence pour ne pas dupliquer les données)
    flow_heal_check(flows_map, flows_queue, archived_flows);
    

    return 0;
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
    if (ndpi_struct == nullptr) {
        std::cout << "nDPI initialization failed" << std::endl;
        return -1;
    }

    // Vector for storing packet flows
    std::vector<std::shared_ptr<flow_struct>> flows;
    FlowsMap flows_map;
    FlowsQueue flows_queue;
    std::vector<std::shared_ptr<flow_struct>> archived_flows;

    // Read packets and detect protocols
    Tins::SnifferConfiguration config;
    config.set_promisc_mode(true);  // Enable promiscuous mode

    // Use the default network interface
    Tins::NetworkInterface iface = Tins::NetworkInterface::default_interface();
    std::cout << "Default interface name: " << iface.name();
    std::wcout << " (" << iface.friendly_name() << ")" << std::endl;

#if defined TEST
    std::cout << "Répertoire de travail : " << std::experimental::filesystem::current_path() << std::endl;
    Tins::FileSniffer sniffer("Blosoft_analyzer/data/big.pcap");  //XXX Open the pcap file for reading
#elif !defined TEST
    Tins::Sniffer sniffer(iface.name(), config);  // Instantiate the sniffer
#endif

    // Set the detection bitmask: all protocols
    NDPI_PROTOCOL_BITMASK all;
    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_struct, &all); 
    ndpi_finalize_initialization(ndpi_struct);

    int start_time = time(NULL);
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

            // Set packet length
            packet.packetbytes = pdu.size();

            // Read and process the packet
            read_packet(&packet_info, packet, pdu, flows_map, archived_flows, flows_queue);

            // Use smart pointers to manage memory
            delete[] packet.packet_data;
        } else {
            // TODO: Handle other protocols like ICMP or ICMPv6 individually
        }
        
#if !defined TEST
        if (time(NULL) - start_time > TEMPS_DE_RUN) {
            return false;
        } else {
            return true;
        }
#else defined TEST
        return true;
#endif

    });

    // Print size of flows
    std::cout << "Size of flows: " << flows.size() << std::endl;

    // Print size of flows_map
    std::cout << "Size of flows_map: " << flows_map.size() << std::endl;

#ifdef DEBUG
    // Print size of archived flows (if DEBUG is defined)
    std::cout << "Size of archived flows: " << archived_flows.size() << std::endl; 

    // Print min max and average time of find_and_move_flow
    std::chrono::steady_clock::duration min = time_points_find_and_move_flow[0];
    std::chrono::steady_clock::duration max = time_points_find_and_move_flow[0];
    std::chrono::steady_clock::duration sum = time_points_find_and_move_flow[0];

    for (int i = 1; i < time_points_find_and_move_flow.size(); i++) {
        if (time_points_find_and_move_flow[i] < min) {
            min = time_points_find_and_move_flow[i];
        }
        if (time_points_find_and_move_flow[i] > max) {
            max = time_points_find_and_move_flow[i];
        }
        sum += time_points_find_and_move_flow[i];
    }

    std::cout << "Min time of find_and_move_flow: " << std::chrono::duration_cast<std::chrono::microseconds>(min).count() << " microseconds" << std::endl;
    std::cout << "Max time of find_and_move_flow: " << std::chrono::duration_cast<std::chrono::microseconds>(max).count() << " microseconds" << std::endl;
    std::cout << "Average time of find_and_move_flow: " << std::chrono::duration_cast<std::chrono::microseconds>(sum / time_points_find_and_move_flow.size()).count() << " microseconds" << std::endl;
#endif

    // Move flows to archived flows
    for (auto &flow : flows) {
        archived_flows.push_back(std::move(flow));
    }
    // Move flows_map to archived flows
    clean_flows_map(flows_map);
    for (auto &flow : flows_map) {
        archived_flows.push_back(std::move(flow.second));
    }

    ndpi_exit_detection_module(ndpi_struct);

    return 0;
}
