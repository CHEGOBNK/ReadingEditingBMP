#include "BMPImage.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>

using namespace std;

// Задание требует обрабатывать изображения только с черными и с белыми цветами
bool BMPImage::isValidPixel(const Pixel& p) const {
    if (dibHeader.biBitCount == 24) {
        return (p.r == 255 && p.g == 255 && p.b == 255) || (p.r == 0 && p.g == 0 && p.b == 0);
    }
    else {
        return (p.r == 255 && p.g == 255 && p.b == 255 && p.a == 0) || (p.r == 0 && p.g == 0 && p.b == 0 && p.a == 0);
    }
}

bool BMPImage::load(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Cannot open file!" << endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(&bmpHeader), sizeof(bmpHeader));
    file.read(reinterpret_cast<char*>(&dibHeader), sizeof(dibHeader));

    // Задание требует обработки только 24/32 битных форматов, остальное игнорируем
    if (bmpHeader.bfType != 0x4D42 || (dibHeader.biBitCount != 24 && dibHeader.biBitCount != 32)) {
        cerr << "Error: Unsupported BMP format!" << endl;
        return false;
    }

    file.seekg(bmpHeader.bfOffBits, ios::beg);
    pixels.resize(abs(dibHeader.biHeight), vector<Pixel>(dibHeader.biWidth));
    rowPadding = (4 - (dibHeader.biWidth * (dibHeader.biBitCount / 8)) % 4) % 4;
    /*
    * Объяснение для себя же, я не вспомню что делает эта строка через месяц.
    * BMP хранит данные в строках кратных 4 байтам, значит нам нужно это учесть.
    * biWidth - пикселей в ряду,
    * (biBitCount / 8) - байт на пиксель. В biBitCount лежит или 24 или 32 в зависимости от режима,
    * разделив на 8 получим или 3 или 4.
    * X = (biWidth * (biBitCount / 8)) - сколько памяти занимает хранение одной строки
    * X % 4 - сколько байт идет излишком от кратности 4
    * (4 - (X % 4)) - сколько байт нужно добавить, чтобы строка стала кратна 4
    * ((4 - (X % 4)) % 4) - если (X % 4 = 0) то выражение выше добавит 4 байта, а это лишнее, строка уже кратна 4
    */

    for (int i = 0; i < abs(dibHeader.biHeight); ++i) {
        for (int j = 0; j < dibHeader.biWidth; ++j) {
            Pixel pixel = {};
            file.read(reinterpret_cast<char*>(&pixel), dibHeader.biBitCount / 8);
            if (!isValidPixel(pixel)) {
                cerr << "Error: BMP contains unsupported colors!" << endl;
                file.close();
                return false;
            }
            pixels[i][j] = pixel;
        }
        file.ignore(rowPadding);
    }

    if (dibHeader.biHeight > 0) {
        reverse(pixels.begin(), pixels.end());
    }

    file.close();
    return true;
}

void BMPImage::display() const {
    for (const auto& row : pixels) {
        for (const auto& pixel : row) {
            cout << (pixel.r == 0 ? "#" : " ");
        }
        cout << endl;
    }
}

// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
void BMPImage::drawLine(int x1, int y1, int x2, int y2) {
    // Мы программно задаем координаты рисования крестика, так что
    // проверка излишняя. Использовалась при отладке
    if (x1 < 0 || x1 >= dibHeader.biWidth || y1 < 0 || y1 >= abs(dibHeader.biHeight) ||
        x2 < 0 || x2 >= dibHeader.biWidth || y2 < 0 || y2 >= abs(dibHeader.biHeight)) {
        cerr << "Error: Line coordinates out of bounds!" << endl;
        return;
    }

    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        pixels[y1][x1] = { 0, 0, 0, 0 };

        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

// Рисует две линии в форме крестика от углов картинки,
// отступая 5 пикселей. Задание не уточняет, так что почему бы нет
void BMPImage::drawCross() {
    int x1 = 5, y1 = 5;
    int x2 = dibHeader.biWidth - 6, y2 = abs(dibHeader.biHeight) - 6;
    drawLine(x1, y1, x2, y2);
    drawLine(x1, y2, x2, y1);
}

void BMPImage::getDimensions() const {
    cout << "Dimensions. Width:" << dibHeader.biWidth << ". Height: " << abs(dibHeader.biHeight) << endl;
}

void BMPImage::saveImage() {
    string filename;
    cout << "Enter the filename to save the image (e.g., output.bmp): ";
    cin >> filename;

    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Cannot open file for saving!" << endl;
        return;
    }

    // Мы не меняем размер изображения, так что можем сохранить header как у исходника
    file.write(reinterpret_cast<const char*>(&bmpHeader), sizeof(bmpHeader));
    file.write(reinterpret_cast<const char*>(&dibHeader), sizeof(dibHeader));

    // В зависимости от знака biHeight строки сохраняются или сверху вниз или снизу вверх, учтем это
    int startRow, endRow, rowStep;
    if (dibHeader.biHeight < 0)
    {
        startRow = 0;
        endRow = abs(dibHeader.biHeight);
        rowStep = 1;
    }
    else
    {
        startRow = abs(dibHeader.biHeight) - 1;
        endRow = -1;
        rowStep = -1;
    }

    for (int i = startRow; i != endRow; i += rowStep) {
        for (int j = 0; j < dibHeader.biWidth; ++j) {
            file.write(reinterpret_cast<const char*>(&pixels[i][j]), dibHeader.biBitCount / 8);
        }
        file.write(reinterpret_cast<const char*>(&rowPadding), rowPadding);
    }

    file.close();
    cout << "File saved successfully!" << endl;
}