#include <string>
#include <vector>

//fonction qui converti les octets en ko, mo, go en arrondissant à 2 chiffres après la virgule
std::string convertSize(double size)
{
    std::string unit = " o";
    if (size > 1024){
        size /= 1024;
        unit = " Ko";
    }
    if (size > 1024){
        size /= 1024;
        unit = " Mo";
    }
    if (size > 1024){
        size /= 1024;
        unit = " Go";
    }
    return std::to_string((int)(size*100)/100.0) + unit+ "  ";
}