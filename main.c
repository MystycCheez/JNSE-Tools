#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
        break;
    }
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
        break;
    }
}

int main(int argc, char *argv[])
{
    (void)argc; // dummy

    char *filename = malloc(30);
    sprintf(filename, argv[1]);
    FILE *file = fopen(filename, "r");

    FILE *export = fopen("out.ppm", "w");
    const char *headerText = "P3\n240 80\n255\n";
    fwrite(headerText, sizeof(char), strlen(headerText), export);

    FILE *log = fopen("log.txt", "w");

    if (file == NULL) {
        fprintf(stderr, "ERROR: Cannot open file: %s!\n", filename);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    size_t fileLen = ftell(file);
    rewind(file);

    char **raw = malloc(sizeof(char*) * fileLen);
    uint8_t *data = malloc(sizeof(int8_t) * fileLen / 2);

    int tilesRemaining = 240 * 80;

    uint8_t runRpt = 0;
    uint8_t runLit = 0;

    for (size_t i = 0; i < fileLen / 2; i++) {
        raw[i] = malloc(3);
        memset(raw[i], 0, 3);
        fread(raw[i], sizeof(char), 2, file);
        data[i] = (int8_t)strtol(raw[i], NULL, 16);
        
        // for (size_t i = 0; i < (size_t)runRpt; i++) {
        //     const char *color = typeToColor(data[3 + i]);
        //     fwrite(color, sizeof(char), strlen(color), export);
        //     fwrite("\n", sizeof(char), 1, export);
        //     // printf("%s\n", color);
        // }

        printf("\033[38;5;1m");
        if (data[i] == 0x00) printf("\033[38;5;22m");
        if (data[i] == 0x01) printf("\033[38;5;148m");
        if (data[i] == 0x02) printf("\033[38;5;223m");
        if (data[i] == 0x03) printf("\033[0;34m");
        if (data[i] == 0x04) printf("\033[38;5;28m");
        if (data[i] == 0x05) printf("\033[38;5;34m");
        if (data[i] == 0x06) printf("\033[38;5;76m");
        if (data[i] == 0x07) printf("\033[38;5;8m");
        printf("%02X ", data[i] & 0xFF);
        if ((i + 1) % 18 == 0) {
            printf("\n");
        }
    }

    printf("\033[0;37m");
    printf("\n");

    for (size_t i = 2; i < fileLen / 2; i++) {
        if (data[i] >= 0x80 || data[i] == 0x01) {
            runRpt = hexToCount(data[i]);
            fprintf(log, "data[%lld]: 0x%02X, runRpt: %d\n", i, data[i], runRpt);
            for (size_t j = 0; j < runRpt; j++) {
                const char *color = typeToColor(data[i + 1]);
                fwrite(color, sizeof(char), strlen(color), export);
                fwrite("\n", sizeof(char), 1, export);
            }
            i++;
        } else if (data[i] < 0x80 && data[i] > 0x01) {
            runLit = hexToLitCount(data[i]);
            fprintf(log, "data[%lld]: 0x%02X, runLit: %d\n", i, data[i], runLit);
            for (size_t j = 1; j <= runLit; j++) {
                fprintf(log, "data[%lld]: 0x%02X\n", i + j, data[i + j]);
                const char *color = typeToColor(data[i + j]);
                fwrite(color, sizeof(char), strlen(color), export);
                fwrite("\n", sizeof(char), 1, export);
            }
            i += runLit;
        }
    }
    
    printf("\n");
    printf("\033[0;37m");
    
    int x = 2;
    for (size_t i = 0; i < 1000; i++) {
        if (data[i + x - 1] == 0x00) printf("\033[38;5;22m");
        if (data[i + x - 1] == 0x01) printf("\033[38;5;148m");
        if (data[i + x - 1] == 0x02) printf("\033[38;5;223m");
        if (data[i + x - 1] == 0x03) printf("\033[0;34m");
        if (data[i + x - 1] == 0x04) printf("\033[38;5;28m");
        if (data[i + x - 1] == 0x05) printf("\033[38;5;34m");
        if (data[i + x - 1] == 0x06) printf("\033[38;5;76m");
        if (data[i + x - 1] == 0x07) printf("\033[38;5;8m");
        if ((x + i) % 2 == 0) {
            // printf("%s: %d", hexToTile(data[x + i + 1] & 0xFF), hexToCount(data[x + i] & 0xFF));
            tilesRemaining -= hexToCount(data[x + i] & 0xFF);
            // printf("\033[0;37m");
            // printf("\nTiles Remaining: %d\n", tilesRemaining);
        }
        if ((i + 1) % 2 == 0) {
            // printf("\n");
        } else if ((i + 1) % 2 == 0) printf(" - ");

        if (tilesRemaining <= 0) break;
    }
    
    printf("\n");

    return 0;
}