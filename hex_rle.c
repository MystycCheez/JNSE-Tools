#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

FILE *logfile;
FILE *terrain;
FILE *heightmap;

bool hex_output = false;

typedef struct RptData {
    uint8_t count;
    bool isRepeating;
} RptData;

const char* hexToTile(uint8_t hex)
{
    switch (hex) {
        case 0x00:
            return "HEAVY ROUGH";
        case 0x01:
            return "TEE";
        case 0x02:
            return "BUNKER";
        case 0x03:
            return "WATER";
        case 0x04:
            return "ROUGH";
        case 0x05:
            return "FAIRWAY";
        case 0x06:
            return "GREEN";
        case 0x07:
            return "CART PATH";
        default:
            fprintf(stderr, "ERROR: Expected tile hex, got %02X! (0x00:0x07)\n", hex);
            exit(1);
    }
}

void hexToPrintColor(uint8_t hex)
{
    switch (hex) {
        case 0x00:
            printf("\033[38;5;22m");
            break;
        case 0x01:
            printf("\033[38;5;148m");
            break;
        case 0x02:
            printf("\033[38;5;223m");
            break;
        case 0x03:
            printf("\033[0;34m");
            break;
        case 0x04:
            printf("\033[38;5;28m");
            break;
        case 0x05:
            printf("\033[38;5;34m");
            break;
        case 0x06:
            printf("\033[38;5;76m");
            break;
        case 0x07:
            printf("\033[38;5;8m");
            break;
        default:
            printf("\033[38;5;1m");
        break;
    }
}

const char *typeToColor(uint8_t hex)
{
    switch (hex) {
        case 0x00:
            return "32 128 32";
        case 0x01:
            return "64 192 64";
        case 0x02:
            return "255 255 192";
        case 0x03:
            return "32 32 192";
        case 0x04:
            return "48 160 48";
        case 0x05:
            return "64 192 64";
        case 0x06:
            return "80 224 80";
        case 0x07:
            return "128 128 128";
        default:
            fprintf(stderr, "ERROR: Expected terrain, got 0x%02X!\n", hex);
            exit(1);
    }
}

char *heightToColor(uint8_t hex)
{
    char *color = malloc(12);
    sprintf(color, "%d %d %d", hex, hex, hex);
    return color;
}

int hexToCount(uint8_t hex)
{
    if (hex == 0x01) return 1;
    return 0x100 - hex + 1;
}

int hexToLitCount(uint8_t hex)
{
    if (hex < 0x02 || hex > 0x7F) {
        fprintf(stderr, "ERROR: Unexpected hex! Got %02X\n", hex);
    }
    return hex;
}

bool isRepeating(uint8_t data[], size_t *index)
{
    return data[*index] == data[*index + 1];
}

// Returns the first byte in an rle string(?), rptCount
// Modifies the index
RptData countTiles(uint8_t data[], size_t *index)
{
    RptData returnVal = {0};
    bool repeating = false;
    bool repeatingOld = repeating;
    while (*index < 240 * 80 && returnVal.count < 0x80) {
        if (repeating == repeatingOld) {returnVal.count++; *index++;} else break;
        repeatingOld = repeating;
        repeating = isRepeating(data, index);
    }
    returnVal.isRepeating = repeatingOld;
    return returnVal;
}

// Returns the first byte in an rle string(?), litCount
// Modifies the index
uint8_t countNonrepeatingTiles(uint8_t data[], size_t *index)
{
    uint8_t count = 0;
    while (
        *index < 240 * 80
        && data[*index] != data[*index + 1]
        && count < 0x80
    ) {count++; *index++;}
    return count;
}

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

int main(int argc, char *argv[])
{
    if (argc < 0 && argc > 3) {
        fprintf(stderr, "ERROR: Incorrect number of args!\n");
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

    char *chunk = strchr(filename, '/');
    size_t nameLen = strlen(chunk) - strlen(dot);
    char *name = malloc(sizeof(char) * nameLen);
    snprintf(name, nameLen, chunk);

    char *terrain_ppm = malloc(30);
    sprintf(terrain_ppm, "output/%s_%s_T.ppm", name, dot);
    terrain = fopen(terrain_ppm, "w");
    char *heightmap_ppm = malloc(30);
    sprintf(heightmap_ppm, "output/%s_%s_H.ppm", name, dot);
    heightmap = fopen(heightmap_ppm, "w");

    const char *headerText = "P3\n240 80\n255\n";
    fwrite(headerText, sizeof(char), strlen(headerText), terrain);
    fwrite(headerText, sizeof(char), strlen(headerText), heightmap);

    char *rle_log = malloc(30);
    sprintf(rle_log, "terrain/%s_%s.txt", name, dot);
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