#include <iostream>
#include <dpxHeader.h>
#include "PQTable.h"
#include "dpx.h"

/**
 * Simple program that reads all DPX files in the current directory,
 * and calculates maxCLL and maxFALL.
 *
 * Requires the DPX files to be exported from video stream with PQ coding,
 * and 64-960 range 10 bit files.
 *
 * The program assumes a fixed packing format (filling, method A) of DPX,
 * and if the format is not what it expects, it stops processing.
 *
 */

void dpxReaderTest();

int main(int, char ** argv) {
//    double testVal = PQTableLookup(509);
//    std::cout << testVal << std::endl;
//    DpxReader reader;

    // Get the list of files

    // for each file, visit to return the values

    dpxReaderTest();

    return 0;
}


void dpxReaderTest() {
    DpxReader reader;
    if (reader.is_dpx("../PQ_1000nits.dpx"))
        std::cout << "file is DPX.";

    reader.open("../PQ_1000nits.dpx");

    int offset = reader.header().file.offset;
    offset = reader.header().image.element[0].offset;
    int bitsize = reader.header().image.element[0].bit_size;
    int minVal = reader.header().image.element[0].ref_low_data;
    int maxVal = reader.header().image.element[0].ref_high_data;
    int packing = reader.header().image.element[0].packing;
    int lines = reader.header().image.image_lines;
    int cols = reader.header().image.line_pixels;


}