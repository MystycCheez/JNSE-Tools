@echo off
gcc -O0 -o rleToImg.exe rle_to_img.c -std=c99 -Wall -Wextra
gcc -O0 -o imgToRle.exe img_to_rle.c -std=c99 -Wall -Wextra