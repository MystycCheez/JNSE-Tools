#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "structs.h"
#include "globals.h"
#include "hex.c"
#include "rle.c"

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

    if (argc == 3) {
        if (strcmp(argv[2], "-h") == 0) {
            hex_output = true;
        } else {
            fprintf(stderr, "ERROR: Not a valid argument!\n");
            exit(1);
        }
    }

    char *chunk = strchr(filename, '/');
    size_t nameLen = strlen(chunk) - strlen(extension);
    char *name = malloc(sizeof(char) * nameLen);
    snprintf(name, nameLen, chunk);

    char *terrain_ppm = malloc(30);
    sprintf(terrain_ppm, "ppm-out/%s_T.ppm", name);
    terrain = fopen(terrain_ppm, "w");
    char *heightmap_ppm = malloc(30);
    sprintf(heightmap_ppm, "ppm-out/%s_H.ppm", name);
    heightmap = fopen(heightmap_ppm, "w");

    const char *headerText = "P3\n240 80\n255\n";
    fwrite(headerText, sizeof(char), strlen(headerText), terrain);
    fwrite(headerText, sizeof(char), strlen(headerText), heightmap);

    char *rle_log = malloc(30);
    sprintf(rle_log, "log/%s_%s.txt", name, extension);
    logfile = fopen(rle_log, "w");

    fseek(file, 0, SEEK_END);
    size_t fileLen = ftell(file);
    rewind(file);

    uint8_t *data = malloc(sizeof(uint8_t) * fileLen);
    uint8_t terrainHeader[2] = {0x89, 0x80};

    for (size_t i = 0; i < fileLen; i++) {
        data[i] = (uint8_t)fgetc(file);
    }

    for (size_t i = 0; i < fileLen; i++) {
        if (memcmp(terrainHeader, data + i, sizeof(uint8_t) * 2) == 0) {
            size_t offset = ParseTerrain(data + i);
            ParseHeightmap(data + i + offset);
        } 
    }

    return 0;
}