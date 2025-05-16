#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

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
    RptData rptData = {1, SEQUENCE_REPEAT};
    SequenceData seqData = {SEQUENCE_REPEAT, SEQUENCE_UNIQUE};
    while (*index < 240 * 80 && rptData.count < 0x80) {
        if (seqData.seqType == seqData.seqTypeOld) {
            rptData.count++; *index = *index + 1;
        } else if (*index) break;
        seqData.seqTypeOld = seqData.seqType;
        seqData.seqType = getSequenceType(data, index);
    }
    // The while loop breaks if it detects a change, This means:
    // The type for all but the last iteration needs to be returned
    rptData.seqType = seqData.seqTypeOld;
    return rptData;
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "ERROR: Incorrect number of args!\n");
        exit(1);
    }

    char *filename = malloc(30);
    sprintf(filename, argv[1]);
    char *dot = strchr(filename, '.');
    if (dot == NULL) {
        fprintf(stderr, "ERROR: Not a valid file, missing extension!\n");
        exit(1);
    }
    dot++;
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        fprintf(stderr, "ERROR: Cannot open file: %s!\n", filename);
        exit(1);
    }

    char delims[2] = {'\r', '\n'};

    char* header = malloc(20);
    fgets(header, 20, file);
    header = strtok(header, delims);
    size_t headerLen = strlen(header);
    free(header);

    fseek(file, 0, SEEK_END);
    size_t fileLen = ftell(file) - headerLen;
    rewind(file);

    char *fileData = malloc(fileLen);

    for (size_t i = 0; i < fileLen; i++) {
        fileData[i] = fgetc(file);
    }
    fileData = fileData + headerLen;

    char *data = malloc(240 * 80);
    char *buf = malloc(240);
    size_t iteration = 0;
    buf = strtok(fileData, delims);
    while (buf != NULL) {
        strncpy(data + iteration * 240, buf, 240);
        buf = strtok(NULL, delims);
        iteration++;
    }
    uint8_t *convData = malloc(sizeof(uint8_t) * 240 * 80);
    for (size_t i = 0; i < 240 * 80; i++) {
        convData[i] = data[i] - '0';
        // printf("%d", convData[i]);
    }
    // printf("\n");

    size_t index = 0;
    RptData rptData = ConvertSequence(convData, &index);
    printf("count: %d\n", rptData.count);
    printf("type:  %s\n", (rptData.seqType ? "Repeat": "Unique"));
    return 0;
}