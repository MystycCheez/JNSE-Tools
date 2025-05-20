#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "structs.h"
#include "enums.h"
#include "seq.c"
#include "hex.c"
#include "globals.h"

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "ERROR: Incorrect number of args!\n");
        exit(1);
    }

    char *filename = malloc(30);
    sprintf(filename, argv[1]);
    char *extension = strchr(filename, '.');
    if (extension == NULL) {
        fprintf(stderr, "ERROR: Not a valid file, missing extension!\n");
        exit(1);
    }
    extension++;
    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        fprintf(stderr, "ERROR: Cannot open file: %s!\n", filename);
        exit(1);
    }

    char *chunk = strchr(filename, '/');
    size_t nameLen = strlen(chunk) - strlen(extension);
    char *name = malloc(sizeof(char) * nameLen);
    snprintf(name, nameLen, chunk);

    // char *seq_log = malloc(30);
    // sprintf(seq_log, "log/%s_%s.txt", name, extension);
    // seqlog = fopen(seq_log, "w");

    // if (seqlog == NULL) {
    //     fprintf(stderr, "ERROR: Cannot open file: %s!\n", seq_log);
    //     exit(1);
    // }

    char *rle_log = malloc(30);
    sprintf(rle_log, "log/%s_log.txt", name);
    rlelog = fopen(rle_log, "w");

    if (rlelog == NULL) {
        fprintf(stderr, "ERROR: Cannot open file: %s!\n", rle_log);
        exit(1);
    }

    char *rle_out = malloc(30);
    sprintf(rle_out, "rle-out/%s.rle", name);
    rleOut = fopen(rle_out, "wb");

    if (rleOut == NULL) {
        fprintf(stderr, "ERROR: Cannot open file: %s!\n", rle_out);
        exit(1);
    }

    char delims[2] = {'\r', '\n'};

    char* header = malloc(20);
    fgets(header, 20, file);
    header = strtok(header, delims);
    size_t headerLen = strlen(header);
    free(header);

    fseek(file, headerLen, SEEK_SET);

    char *data = malloc(240 * 80);
    char c;

    size_t iteration = 0;

    while (iteration < 240 * 80) {
        c = fgetc(file);
        if (c != '\r' && c != '\n') data[iteration++] = c;
    }

    uint8_t *convData = malloc(sizeof(uint8_t) * 240 * 80);
    for (size_t i = 0; i < 240 * 80; i++) {
        convData[i] = (uint8_t)(data[i] - '0');
        // printf("%lld: %d\n", i, convData[i]);
    }
    // printf("\n");
    // exit(0);

    iteration = 0;
    size_t rleBlockSize = 80;
    RleData *rleData = malloc(sizeof(RleData) * rleBlockSize);
    size_t accumulator = 0;
    while (accumulator < 240 * 80) {
        rleData[iteration] = ConvertSequence(convData, accumulator);
        fprintf(seqlog, "count: %d\n", rleData[iteration].count);
        fprintf(seqlog, "type:  %s\n", (rleData[iteration].seqType ? "Repeat": "Unique"));
        for (size_t i = 0; i < rleData[iteration].count; i++) {
            fprintf(seqlog, "convData[%lld]: %02X\n", i + accumulator, convData[i + accumulator]);
        }
        fprintf(seqlog, "\n");
        accumulator += rleData[iteration].count;
        iteration++;
        if (iteration >= rleBlockSize) {
            rleBlockSize *= 1.5;
            // printf("rleBlockSize: %lld\n", rleBlockSize);
            RleData *tmp = realloc(rleData, sizeof(RleData) * rleBlockSize);
            if (tmp == NULL) {
                fprintf(stderr, "ERROR: Failed to realloc!\n");
                exit(1);
            }
            rleData = tmp;
        }
        // printf("Iteration: %lld\n", iteration);
    }

    accumulator = 0;
    iteration = 0;

    while (accumulator < 240 * 80) {
        uint8_t *tmp2 = convData + accumulator;
        fprintf(rlelog, "Iteration: %lld\n", iteration);
        if (rleData[iteration].seqType == SEQUENCE_REPEAT) {
            uint8_t tmp = hexToRepeatCount(rleData[iteration].count);
            fwrite(&tmp, sizeof(uint8_t), 1, rleOut);
            fwrite(tmp2, sizeof(uint8_t), 1, rleOut);
            fprintf(rlelog, "REPEAT: %u\n%u\n\n", rleData[iteration].count, *tmp2);
        } else {
            fwrite(&rleData[iteration].count, sizeof(uint8_t), 1, rleOut);
            fwrite(tmp2, sizeof(uint8_t), rleData[iteration].count, rleOut);
            fprintf(rlelog, "UNIQUE: %u\n", rleData[iteration].count);
            for (size_t i = 0; i < rleData[iteration].count; i++) {
                fprintf(rlelog, "%u\n", tmp2[i]);
            }
            fprintf(rlelog, "\n");
        }
        accumulator += rleData[iteration].count;
        iteration++;
    }

    // fclose(seqlog);
    
    return 0;
}