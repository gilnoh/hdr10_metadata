#include <iostream>
#include <Magick++.h>
#include "PQTable.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    double testVal = PQTableLookup(939);
    std::cout << testVal << std::endl;
    return 0;
}


