/*
Blosoft project
Vert-y-good
http://libtins.github.io/docs/latest/
*/
#include <tins/tins.h>
#include <ndpi_main.h>
#include <iostream>
#include <string>
#include <vector>
using namespace Tins;

#include "toby.hpp"

int main() {
    //test nPDI npdi_main.h
    ndpi_detection_module_struct *ndpi = ndpi_init_detection_module(ndpi_no_prefs);

    if (ndpi != NULL) {
        std::cout << "nDPI initialized successfully!" << std::endl;
        ndpi_exit_detection_module(ndpi);
    } else {
        std::cout << "nDPI initialization failed!" << std::endl;
    }
    
    renifleur r;
    r.renifle();
    return 0;
}