#include <iostream>
#include "BMPImage.h"

using namespace std;


int main() {
    string filename;
    cout << "Enter BMP filename: ";
    cin >> filename;

    BMPImage image;
    if (image.load(filename)) {
        image.getDimensions();
        image.drawCross();
        image.display();
        image.saveImage();
    }

    return 0;
}
