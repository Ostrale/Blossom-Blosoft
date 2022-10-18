#ifndef TOBY_HPP
#define TOBY_HPP

#include <string>
#include <vector>
#include <tins/tins.h>
using namespace Tins;

//fonction qui converti les octets en ko, mo, go en arrondissant à 2 chiffres après la virgule
std::string convertSize(double size);

struct renifleur{

    int data_size = 0;

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
        Sniffer sniffer(iface.name()); // Ready to sniff!
        sniffer.sniff_loop(make_sniffer_handler(this, &renifleur::lecture));
    }

    bool lecture(PDU& pkt){
        // Lookup the UDP PDU

        if(pkt.find_pdu<IP>()){
            const IP &ip = pkt.rfind_pdu<IP>();
            int size_of_packet_ip = ip.tot_len();
            data_size += size_of_packet_ip;
        }else if(pkt.find_pdu<IPv6>()){
            const IPv6 &ipv6 = pkt.rfind_pdu<IPv6>();
            int size_of_packet_ipv6 = ipv6.payload_length();
            data_size += size_of_packet_ipv6;
        }

        std::cout << convertSize(data_size) << '\r' << std::flush ;

        //std::cout << ip.tos() << std::endl ;
        //boucle infini
        return true;
    }
};

#endif // TOBY_HPP