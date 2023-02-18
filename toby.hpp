#ifndef TOBY_HPP
#define TOBY_HPP

#include <string>
#include <vector>
#include <tins/tins.h>
#include <ndpi_main.h>

using namespace Tins;

//fonction qui converti les octets en ko, mo, go en arrondissant à 2 chiffres après la virgule
std::string convertSize(double size);

struct renifleur{

    void renifle(){
        // Create sniffer configuration object.
        SnifferConfiguration config;
        config.set_promisc_mode(true);

        // Use the default interface
        NetworkInterface iface = NetworkInterface::default_interface();

        // print Default interface
        std::cout << "Default interface name: " << iface.name();
        std::wcout << " (" << iface.friendly_name() << ")" << std::endl;

        // Now instantiate the sniffer
        Sniffer sniffer(iface.name(),config);
        sniffer.sniff_loop(make_sniffer_handler(this, &renifleur::lecture));

        // Initialisation de nDPI
        ndpi_init_prefs init_prefs = ndpi_no_prefs;
        struct ndpi_detection_module_struct* ndpi_struct = ndpi_init_detection_module(init_prefs);
        NDPI_PROTOCOL_BITMASK protos;
        NDPI_BITMASK_SET_ALL(protos);
        ndpi_set_protocol_detection_bitmask2(ndpi_struct, &protos);
        ndpi_finalize_initialization(ndpi_struct);
    }

    bool lecture(PDU& pdu){

        // On caste les données brutes dans un objet RawPDU
        RawPDU& raw = pdu.rfind_pdu<RawPDU>();
        //On récupère les données brutes
        const uint8_t *packet_data = raw.payload().data();
        //On récupère la taille des données
        unsigned int packet_size = raw.payload().size();
        unsigned short packetlen = packet_size;

        // Initialisation du flux
        ndpi_detection_module_struct *ndpi_struct = ndpi_init_detection_module( ndpi_no_prefs );  

        if (ndpi_struct == NULL) {
            std::cerr << "Erreur lors de l'initialisation de la structure ndpi_struct" << std::endl;
            exit(1);
        }

        // Initialisation du flow
        struct ndpi_flow_struct *flow = (struct ndpi_flow_struct*)malloc(sizeof(struct ndpi_flow_struct));

        // Utiliser ndpi_detection_process_packet() pour détecter le protocole et la catégorie du flux
        ndpi_protocol protocol = ndpi_detection_process_packet(ndpi_struct, flow, packet_data, packetlen, 0, 0);

        //détection du catégorie
        u_int16_t category = flow->guessed_category;

        // Afficher la catégorie
        ndpi_protocol_category_t category_trouve = ndpi_get_proto_category(ndpi_struct, protocol);
        char* name_category = ndpi_get_proto_name(ndpi_struct, protocol.category);
        std::string name_category_string(name_category != nullptr ? name_category : "pointeur null");
        std::cout << "Category: " << category_trouve << ' ' << protocol.category << std::endl;
        
        //std::cout << "Protocol: "  << ndpi_get_proto_name(ndpi_struct, protocol.category) << std::endl;

    return true;
    }

    // Libération de la mémoire
    //#TODO
};

#endif // TOBY_HPP