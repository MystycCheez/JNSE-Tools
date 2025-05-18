#include "includes.h"
#include "structs.h"
#include "enums.h"

SequenceType getSequenceType(uint8_t data[])
{
    // printf("%d, %d\n", data[0], data[1]);
    return data[0] == data[1];
}

// Returns struct containing rle data (type of sequence and count)
RleData ConvertSequence(uint8_t data[])
{
    RleData rleData = {0, SEQUENCE_REPEAT};
    SequenceData seqData = {getSequenceType(data), 0};
    seqData.seqTypeOld = !seqData.seqType;
    do {
        seqData.seqTypeOld = seqData.seqType;
        seqData.seqType = getSequenceType(data + rleData.count);
        rleData.count++;
    } while (rleData.count < 0x80 && seqData.seqType == seqData.seqTypeOld);
    // Look ahead to see if the last item is equal to the first item of the next iteration
    if (seqData.seqTypeOld == SEQUENCE_UNIQUE && rleData.count < 0x80) {
        if (getSequenceType(data + rleData.count - 1) == SEQUENCE_REPEAT) {
            rleData.count--;
        }
    }
    // The while loop breaks if it detects a change, This means:
    // The type for all but the last iteration needs to be returned
    rleData.seqType = seqData.seqTypeOld;
    return rleData;
}