#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>
#include <glob.h>
#include "dpxHeader.h"
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

using namespace std;

uint32_t* getPixelData(string dpxFilename);
void getComponentValues(uint16_t * buffer, uint32_t * valuePointer);
uint16_t codeValueInPQSpace(uint16_t full10bitvalue);
inline std::vector<std::string> glob(const std::string& pat);
pair<double,double> calculateMaxAndMeanNits(string fileName);

int main(int, char ** argv) {
    // result values
    double maxCLL = 0;
    double maxFALL = 0;
    string maxCLLFile="";
    string maxFALLFile="";

    // Get the list of files
    vector<string> fileNames = glob("*.dpx");
    cout << "There is " << fileNames.size() << " dpx files in current directory. ";
    cout << "The program will visit each file to calculate MaxCLL and MaxFALL." << endl;
    cout << "Note that, program requires all the files in 10bit PQ values, packed by dpx 0-1023 range method-A filling." << endl;
    cout << "(Press CTRL-C to stop running.)" << endl;
    cout << "====" << endl << "====" << endl; 

    // for each file, visit to return the values
    for (string file : fileNames) {
        pair<double, double> maxAndAvg = calculateMaxAndMeanNits(file);
        cout << file << ": max=" << maxAndAvg.first;
        cout << ", avg=" << maxAndAvg.second << endl;
        bool updated = false;

        if (maxAndAvg.first > maxCLL) {
            maxCLL = maxAndAvg.first;
            maxCLLFile = file;
            updated = true;
        }

        if (maxAndAvg.second > maxFALL) {
            maxFALL = maxAndAvg.second;
            maxFALLFile = file;
            updated = true;
        }

        if (updated) {
            cout << "\t(update occurred) Current Values: MaxCLL=" << maxCLL;
            cout << ", MaxFALL=" << maxFALL << endl;
        }
    }
    cout << "====" << endl << "====" << endl;
    cout << "All " << fileNames.size() << " files have been visited. Final values are:" << endl;
    cout << "maxCLL=" << maxCLL << " (from file " << maxCLLFile << ")" << endl;
    cout << "maxFALL=" << maxFALL << " (from file " << maxFALLFile << ")" << endl;

}

// void test() {
    // test for calculating for single file.
//    pair<double,double> result = calculateMaxAndMeanNits("../../00255.dpx");
//    cout << result.first << endl;
//    cout << result.second << endl;

    // test for unpacking and converting single value.
//    uint32_t * buffer = getPixelData("../PQ_1000nits.dpx");
//    uint16_t cValues[3];
//    getComponentValues(cValues, &buffer[0]);
//    cout << cValues[0] << " " << cValues[1] << " " << codeValueInPQSpace(cValues[2]) << endl;
//    delete[] buffer;
//    return 0;
// }

/**
 * Get pixel data as a buffer from a dpx file.
 *
 * @param dpxFilename DPX file name.
 * @return pixel data as a buffer of uint32*
 *         Note that, data are written as byte stream -- byte order of single
 *         pixel would be wrong if read as uint32.
 *         (e.g. pixel in file C0 70 1C 04 would read as-is, and will be
 *         read as one uint32 of 041c70c0, if accessed as a long. )
 *         So use it as 4-bytes, not as a uint32.
 */
uint32_t* getPixelData(string dpxFilename) {
    int pixelCountToRead;
    int dataOffset;
    DpxReader reader;

    reader.open(dpxFilename);
    dataOffset = reader.header().file.offset;
    pixelCountToRead = reader.header().image.image_lines * reader.header().image.line_pixels;
    reader.close();

    ifstream file;
    file.open(dpxFilename, ios::binary | ios::in);
    uint32_t* resultBuffer = new uint32_t[pixelCountToRead];

    file.seekg(dataOffset, ios::beg);
    file.read((char*)resultBuffer, pixelCountToRead * 4);

    file.close();
    return resultBuffer;
}

/**
 * Converts 0-1023 range-based 10bit value to PQ-space (64-940) value.
 *
 * @param full10bitValue 0-1023 range 10-bit code.
 * @return PQ-space (64-940) range 10 bit code.
 */
uint16_t codeValueInPQSpace(uint16_t full10bitValue) {
    double range;
    range  = full10bitValue / (double) 1023;
    double nearest = ((MAX_HDR_LEGAL_RANGE - MIN_HDR_LEGAL_RANGE) * range) + MIN_HDR_LEGAL_RANGE;
    return (uint16_t) round(nearest);
}

/**
 * get 4 bytes from uint32 value point
 * order of as they are written.
 *
 * @param buffer pass a uint8_t[4] here. Will be filled by BYTES of valuePointer (word)
 * @param valuePointer pass a single uint32_t pointer (not value) read from DPX file.
 */
void getBytes(uint8_t* buffer, uint32_t * valuePointer) {
    uint8_t* target = (uint8_t*) valuePointer;
    buffer[0] = target[0];
    buffer[1] = target[1];
    buffer[2] = target[2];
    buffer[3] = target[3];
}

/**
 * Get component values and fill it in the buffer of uint16_t[3]
 *
 * Component values are calculated from given uint32_t valuePointer, that holds
 * "file-order" of bytes in it.
 *
 * Logic: Method-A of DPX 10bit components with filling
 * component3 component2 component1 0 0
 * byte[0] byte[1]  byte[2] byte[3]
 * e.g.
 * 1100 0000   0111 0000   0001 1100   0000 0100
 * Becomes:
 * 1100000001  1100000001  1100000001  00
 *
 * @param buffer buffer of uint16_t[3]
 * @param valuePointer single uint32_t that is written (as-is) from method-A dpx pixel 4 bytes.
 */
void getComponentValues(uint16_t * buffer, uint32_t * valuePointer) {
    const uint16_t tenBitMask = 0x03FF;     // 0000 0011 1111 1111
    uint8_t fourBytes[4];
    uint16_t component0;
    uint16_t component1;
    uint16_t component2;

    uint16_t left;
    uint16_t right;

    // first, get bytes
    getBytes(fourBytes, valuePointer);

    // component2
    // component2 = byte1 << 2 + byte2 >> 6
    left = fourBytes[0];
    right = fourBytes[1];
    component0 = (left << 2) | (right >> 6);

    // component1
    // component1 = byte2 << 2 + byte3 >> 4;
    left = fourBytes[1];
    right = fourBytes[2];
    component1 = ((left << 4) | (right >> 4)) & tenBitMask;

    // component0
    left = fourBytes[2];
    right = fourBytes[3];
    component2 = ((left << 6) | (right >>2)) & tenBitMask;

    buffer[0] = component0;
    buffer[1] = component1;
    buffer[2] = component2;
}

bool checkFile(string fileName) {
    DpxReader reader;
    reader.open(fileName);
    bool fileIsOkay = true;

    // dpx file?
    if (!reader.is_dpx(fileName))
        fileIsOkay = false;

    // check bitsize, packing method, and 10bit range
    if (reader.header().image.element[0].bit_size != 10)
        fileIsOkay = false;
    if (reader.header().image.element[0].packing != 1)
        fileIsOkay = false;
    if (reader.header().image.element[0].ref_low_data != 0)
        fileIsOkay = false;
    if (reader.header().image.element[0].ref_high_data != 1023)
        fileIsOkay = false;

    reader.close();
    return fileIsOkay;
}

uint16_t maxVal(uint16_t a, uint16_t b, uint16_t c) {
    if (a > b) {
        if (c > a)
            return c;
        else
            return a;
    } else {
        if (c > b)
            return c;
        else
            return b;
    }
}

pair<double,double> calculateMaxAndMeanNits(string fileName) {
    // check file
    if (!checkFile(fileName)) {
        cout << "ERROR" << endl;
        cout << fileName << " is not a proper DPX file that this program can handle." << endl;
        cout << "DPX file MUST have 10bit, packing-1 (method-A), with 0-1023 range." << endl;
        exit(1);
    }

    // get data size, and get data.
    DpxReader reader;
    reader.open(fileName);
    int lines = reader.header().image.image_lines;
    int cols = reader.header().image.line_pixels;
    reader.close();
    uint32_t * buffer = getPixelData(fileName);

    // visit each pixel,
    double maxValue=0;
    double sum=0;
    int countPixel = lines*cols;
    uint16_t components[3];
    for(int i=0; i < countPixel; i++) {
        getComponentValues(components, &buffer[i]);
        uint16_t max = maxVal(components[0], components[1], components[2]);
        double actualVal = PQTableLookup(codeValueInPQSpace(max));
        sum+=actualVal;
        if(maxValue < actualVal)
            maxValue = actualVal;
    }

    pair<double,double> result = make_pair(maxValue, sum / countPixel);
    delete[] buffer;
    return result;
};

inline std::vector<std::string> glob(const std::string& pat){
    using namespace std;
    glob_t glob_result;
    glob(pat.c_str(),GLOB_TILDE,NULL,&glob_result);
    vector<string> ret;
    for(unsigned int i=0;i<glob_result.gl_pathc;++i){
        ret.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
    return ret;
}

//void dpxReaderTest() {
//    DpxReader reader;
//    if (reader.is_dpx("../PQ_1000nits.dpx"))
//        std::cout << "file is DPX.";
//
//    reader.open("../PQ_1000nits.dpx");
//
//    int offset = reader.header().file.offset;
//    offset = reader.header().image.element[0].offset;
//    int bitsize = reader.header().image.element[0].bit_size;
//    int minVal = reader.header().image.element[0].ref_low_data;
//    int maxVal = reader.header().image.element[0].ref_high_data;
//    int packing = reader.header().image.element[0].packing;
//    int lines = reader.header().image.image_lines;
//    int cols = reader.header().image.line_pixels;
//
//    unsigned long word;
//}
