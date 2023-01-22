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
    int streaming_size = 0;
    int web_size = 0;
    int mail_size = 0;
    int cloud_size = 0;
    int market_size = 0;
    int other_size = 0;

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
    }

    bool lecture(PDU& pkt){
        // Lookup the IP PDU
        if(pkt.find_pdu<IP>() && pkt.find_pdu<TCP>()){
            const IP &ip = pkt.rfind_pdu<IP>();
            const TCP &tcp = pkt.rfind_pdu<TCP>();
            int size_of_packet_ip = ip.tot_len();
            data_size += size_of_packet_ip;
            // Check the destination port
            if (tcp.dport() == 80 || tcp.dport() == 8080 || tcp.dport() == 443 || tcp.dport() == 5222) { //vérif 5222
                // Lookup the payload of the packet
                const RawPDU &raw = pkt.rfind_pdu<RawPDU>();
                // Get a string representation of the payload
                std::string payload = std::string(raw.payload().begin(), raw.payload().end());
                // Check if the payload starts with "GET" or "POST"
                if (payload.find("GET") == 0 || payload.find("POST") == 0) { // a comprendre #TODO 
                    web_size += size_of_packet_ip;
                } else {
                    other_size += size_of_packet_ip;
            }
            } else if (tcp.dport() == 25) {
                mail_size += size_of_packet_ip;
            } else {
                other_size += size_of_packet_ip;
            }
        }else if(pkt.find_pdu<IPv6>() && pkt.find_pdu<TCP>()){
            const IPv6 &ipv6 = pkt.rfind_pdu<IPv6>();
            const TCP &tcp = pkt.rfind_pdu<TCP>();
            int size_of_packet_ipv6 = ipv6.payload_length();
            data_size += size_of_packet_ipv6;
            // Check the destination port
            if (tcp.dport() == 80 || tcp.dport() == 8080 || tcp.dport() == 443 || tcp.dport() == 5222) { //vérif 5222
                web_size += size_of_packet_ipv6;
            } else if (tcp.dport() == 25) {
                mail_size += size_of_packet_ipv6;
            } else {
                other_size += size_of_packet_ipv6;
            }
        }
        std::cout << "Data usage:" << convertSize(data_size) <<
            "Web:" << convertSize(web_size) <<
            "Mail:" << convertSize(mail_size) <<
            "Streaming:" << convertSize(streaming_size) <<
            "Cloud:" << convertSize(cloud_size) <<
            "Market:" << convertSize(market_size) <<
            "Other:" << convertSize(other_size) <<
            '\r' << std::flush;
        //boucle infini
        return true;
    }
};

#endif // TOBY_HPP