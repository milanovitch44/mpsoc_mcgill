#include <iostream>
#include "systemc.h"

#include "bmp.h"
#include "hw1.h"

using namespace std;

BMP *img;

void usage(char* cmd) {
    cerr << "Usage: " << cmd << " <input filename>" << endl; 
} // void usage

// define a processing element called a hw1Module
SC_MODULE(hw1Module) {
    // the constructor of hw1Module
    SC_CTOR(hw1Module) {
        // the functionality of the module is run as a thread based on
        // the function toGrayscaleWM
        SC_THREAD(toGrayscaleWM);
    }

    /** Convert the BMP, pixel by pixel, to grayscale using the weighted method: 
     * - Grayscale = 0.299R + 0.587G + 0.114B
     */
    void toGrayscaleWM() {
        // set the direction of y-axis iteration accordingly
        bool negativeY = img->bmpInfoHeader.height < 0 ? true: false;
        
        int32_t y = 0;
        while (abs(y) < abs(img->bmpInfoHeader.height)) {
            for (uint32_t x = 0; x<img->bmpInfoHeader.width; x++) {
                uint32_t pixel = img->getPixel(x, y);
                
                uint8_t B, G, R, A;
                // get each individual channel using bitwise AND of the pixel 
                // with each channel mask and shift if needed
                B = (uint8_t)  (pixel & img->bmpColorHeader.blueMask);
                G = (uint8_t) ((pixel & img->bmpColorHeader.greenMask) >> 8);
                R = (uint8_t) ((pixel & img->bmpColorHeader.redMask) >> 16);
                A = (uint8_t) ((pixel & img->bmpColorHeader.alphaMask) >> 24);

                uint8_t gray = (uint8_t) ( 0.299*(float) R + 0.587*(float) G + 0.114*(float) B);

                img->setPixel(x, y, gray, gray, gray, A);
            } // for x

            // update y based on whether or not BMP height is negative
            y = negativeY ? y-1: y+1;
        } // while y

        // a placeholder wait statement to confirm we're writing and compiling
        // SystemC correctly; remove, replace, etc, as necessary
        wait(1, SC_NS);
    } // to GrayscaleWM
};

// the main function of a SystemC similation
int sc_main(int argc, char* argv[]) {
    // get the input file to manipulate
    if (argc != 2) {
        usage(argv[0]);
        return 1;
    } // if

    // read an image
    img = new BMP(argv[1]);
    // sanity check
    img->write("before.bmp");

    // instantiate a processing element with the functionality of hw1Module
    hw1Module hw1("hw1Instance");

    /* run the simulation */
    sc_time startTime = sc_time_stamp();
    sc_start();
    sc_time stopTime = sc_time_stamp();

    cout << "Simulated for " << (stopTime - startTime) << endl;

    // output grayscale image
    img->write("after.bmp");
    delete img;

    return 0;
} // int sc_main
