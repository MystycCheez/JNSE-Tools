#include <stdio.h>
#include <stdint.h>

const char* hexToTerrainText(uint8_t hex)
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

const char *typeToRGB_Text(uint8_t hex)
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

int hexToRepeatCount(uint8_t hex)
{
    if (hex == 0x01) return 1;
    return 0x100 - hex + 1;
}

int hexToUniqueCount(uint8_t hex)
{
    if (hex < 0x02 || hex > 0x7F) {
        fprintf(stderr, "ERROR: Unexpected hex! Got %02X\n", hex);
    }
    return hex;
}

char *heightToGrayscaleText(uint8_t hex)
{
    char *color = malloc(12);
    sprintf(color, "%d %d %d", hex, hex, hex);
    return color;
}