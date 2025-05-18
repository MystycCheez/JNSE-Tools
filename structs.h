#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>

#include "enums.h"

typedef struct SequenceData {
    SequenceType seqType : 1;
    SequenceType seqTypeOld : 1;
} SequenceData;

typedef struct RleData {
    uint8_t count;
    SequenceType seqType;
} RleData;

#endif