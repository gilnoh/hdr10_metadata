//
// Created by tailblues on 1/11/17.
//

#ifndef HDR10_METADATA_PQTABLE_H
#define HDR10_METADATA_PQTABLE_H

#define MIN_HDR_LEGAL_RANGE 64
#define MAX_HDR_LEGAL_RANGE 940

double PQTableLookup(int value);

extern const double g_ST2084_PQTable[MAX_HDR_LEGAL_RANGE - MIN_HDR_LEGAL_RANGE + 1];

#endif //HDR10_METADATA_PQTABLE_H
