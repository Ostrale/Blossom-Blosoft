#define TINS_STATIC

#include <iostream>
#include <tins/arp.h>
#include <tins/ip.h>
#include <tins/tcp.h>
#include <tins/ethernetII.h>

using namespace Tins;

int main() {
    //do helo world
    std::cout << "Hello, World!123" << std::endl;
    EthernetII eth;
    IP* ip = new IP();
    TCP* tcp = new TCP();

    // tcp is ip's inner pdu
    ip->inner_pdu(tcp);

    // ip is eth's inner pdu
    eth.inner_pdu(ip);
}