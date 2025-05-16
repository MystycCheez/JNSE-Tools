#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
// #include <string.h>

#include "structs.h"
#include "enums.h"

SequenceType getSequenceType(uint8_t data[], size_t *index)
{
    return data[*index] == data[*index + 1];
}

// Returns struct containing rle data (type of sequence and count)
// Modifies the index
RptData ConvertSequence(uint8_t data[], size_t *index)
{
    RptData rptData = {0};
    SequenceData seqData = {SEQUENCE_REPEAT, SEQUENCE_UNIQUE};
    while (*index < 240 * 80 && rptData.count < 0x80) {
        if (seqData.seqType == seqData.seqTypeOld) {
            rptData.count++; *index++;
        } else break;
        seqData.seqTypeOld = seqData.seqType;
        seqData.seqType = getSequenceType(data, index);
    }
    rptData.seqType = seqData.seqTypeOld;
    return rptData;
}

int main(int argc, char *argv[])
{
    if (argc < 0 && argc > 3) {
        fprintf(stderr, "ERROR: Incorrect number of args!\n");
        exit(1);
    }
}