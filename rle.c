#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "globals.h"

size_t ParseTerrain(uint8_t data[])
{
    int tilesRemaining = 240 * 80;
    size_t totalHex = 0;
    size_t offset = 2;
    uint8_t runRepeat = 0;
    uint8_t runUnique = 0;
    for (size_t i = 2; tilesRemaining > 0; i++) {
        if (data[i] >= 0x80 || data[i] == 0x01) {
            runRepeat = hexToRepeatCount(data[i]);
            tilesRemaining -= runRepeat;
            fprintf(logfile, "%s: %d\n", hexToTerrainText(data[i + 1]), runRepeat);
            for (size_t j = 0; j < runRepeat; j++) {
                const char *color = typeToRGB_Text(data[i + 1]);
                fprintf(terrain, "%s\n", color);
            }
            i++;
            totalHex += 2;
        } else if (data[i] < 0x80 && data[i] > 0x01) {
            runUnique = hexToUniqueCount(data[i]);
            tilesRemaining -= runUnique;
            fprintf(logfile, "Unique Count: %d\n", runUnique);
            for (size_t j = 1; j <= runUnique; j++) {
                const char *color = typeToRGB_Text(data[i + j]);
                fprintf(logfile, "  %s\n", hexToTerrainText(data[i + j]));
                fprintf(terrain, "%s\n", color);
            }
            i += runUnique;
            totalHex += runUnique + 1;
        }
        offset = i;
    }
    printf("%lld\n", totalHex);
    if (hex_output) {
        for (size_t i = 2; i < totalHex + 2; i++) {
            hexToPrintColor(data[i]);
            printf("%02X ", data[i] & 0xFF);
            if ((i + 1 - 2) % 12 == 0) {
                printf("\n");
            }
        }
    }
    return offset + 1;
}

bool ParseHeightmap(uint8_t data[])
{
    int tilesRemaining = 240 * 80;
    uint8_t runRepeat = 0;
    uint8_t runUnique = 0;
    for (size_t i = 0; tilesRemaining > 0; i++) {
        if (data[i] >= 0x80 || data[i] == 0x01) {
            runRepeat = hexToRepeatCount(data[i]);
            tilesRemaining -= runRepeat;
            for (size_t j = 0; j < runRepeat; j++) {
                const char *color = heightToGrayscaleText(data[i + 1]);
                fprintf(heightmap, "%s\n", color);
            }
            i++;
        } else if (data[i] < 0x80 && data[i] > 0x01) {
            runUnique = hexToUniqueCount(data[i]);
            tilesRemaining -= runUnique;
            fprintf(logfile, "Unique Count: %d\n", runUnique);
            for (size_t j = 1; j <= runUnique; j++) {
                const char *color = heightToGrayscaleText(data[i + j]);
                fprintf(heightmap, "%s\n", color);
            }
            i += runUnique;
        }
    }
    return true;
}