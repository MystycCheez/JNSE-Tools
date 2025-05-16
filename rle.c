#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "globals.h"

size_t ParseTerrain(uint8_t data[])
{
    int tilesRemaining = 240 * 80;
    size_t totalHex = 0;
    size_t offset = 2;
    uint8_t runRpt = 0;
    uint8_t runLit = 0;
    for (size_t i = 2; tilesRemaining > 0; i++) {
        if (data[i] >= 0x80 || data[i] == 0x01) {
            runRpt = hexToCount(data[i]);
            tilesRemaining -= runRpt;
            fprintf(logfile, "%s: %d\n", hexToTile(data[i + 1]), runRpt);
            for (size_t j = 0; j < runRpt; j++) {
                const char *color = typeToColor(data[i + 1]);
                fprintf(terrain, "%s\n", color);
            }
            i++;
            totalHex += 2;
        } else if (data[i] < 0x80 && data[i] > 0x01) {
            runLit = hexToLitCount(data[i]);
            tilesRemaining -= runLit;
            fprintf(logfile, "Literal Count: %d\n", runLit);
            for (size_t j = 1; j <= runLit; j++) {
                const char *color = typeToColor(data[i + j]);
                fprintf(logfile, "  %s\n", hexToTile(data[i + j]));
                fprintf(terrain, "%s\n", color);
            }
            i += runLit;
            totalHex += runLit + 1;
        }
        offset = i;
    }
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
    uint8_t runRpt = 0;
    uint8_t runLit = 0;
    for (size_t i = 0; tilesRemaining > 0; i++) {
        if (data[i] >= 0x80 || data[i] == 0x01) {
            runRpt = hexToCount(data[i]);
            tilesRemaining -= runRpt;
            for (size_t j = 0; j < runRpt; j++) {
                const char *color = heightToColor(data[i + 1]);
                fprintf(heightmap, "%s\n", color);
            }
            i++;
        } else if (data[i] < 0x80 && data[i] > 0x01) {
            runLit = hexToLitCount(data[i]);
            tilesRemaining -= runLit;
            fprintf(logfile, "Literal Count: %d\n", runLit);
            for (size_t j = 1; j <= runLit; j++) {
                const char *color = heightToColor(data[i + j]);
                fprintf(heightmap, "%s\n", color);
            }
            i += runLit;
        }
    }
    return true;
}