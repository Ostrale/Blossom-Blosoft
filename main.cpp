#include <tins/tins.h>
#include <iostream>
#include <string>
#include <vector>
using namespace Tins;

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
        const IP &ip = pkt.rfind_pdu<IP>();
        int size_of_header = ip.header_size();
        data_size += size_of_header;
        std::cout << data_size << '\r' << std::flush ;
        //boucle infini
        return true;
    }
};


int main() {
    renifleur r;
    r.renifle();
    return 0;
}