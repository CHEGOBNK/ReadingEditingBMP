#pragma once

#ifndef BMPIMAGE_H
#define BMPIMAGE_H

#include <string>
#include <vector>

using namespace std;

// Заголовок BMP файла должен быть выровнен, по байтам
#pragma pack(push, 1)
struct BMPHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct DIBHeader {
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)

struct Pixel {
    uint8_t b, g, r, a;
};

class BMPImage {
private:
    BMPHeader bmpHeader;
    DIBHeader dibHeader;
    vector<vector<Pixel>> pixels;
    int rowPadding;

    bool isValidPixel(const Pixel& p) const;

public:
    bool load(const string& filename);
    void display() const;
    void drawLine(int x1, int y1, int x2, int y2);
    void drawCross();
    void getDimensions() const; // Задание не требует такого метода, так было проще в отладке
    void saveImage();
};

#endif
