/* Change Log 

 * 15-September-2026
   Initial release.
 */

/* Attribution
 * Adapted and extended from https://github.com/sol-prog/cpp-bmp-images.
 */

#pragma once
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace std;

#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t fileType = 0x4D42;             // File type always BM which is 0x4D42 (stored as hex uint16_t in little endian)
    uint32_t fileSize = 0;                  // Size of the file (in bytes)
    uint16_t reserved1 = 0;                 // Reserved, always 0
    uint16_t reserved2 = 0;                 // Reserved, always 0
    uint32_t offsetData = 0;                // Start position of pixel data (bytes from the beginning of the file)
};

struct BMPInfoHeader {
    uint32_t size = 0;                      // Size of this header (in bytes)
    int32_t width = 0;                      // width of bitmap in pixels
    int32_t height = 0;                     // height of bitmap in pixels
                                            //       (if positive, bottom-up, with origin in lower left corner)
                                            //       (if negative, top-down, with origin in upper left corner)
    uint16_t planes = 1;                    // No. of planes for the target device, this is always 1
    uint16_t bitCount = 0;                  // No. of bits per pixel
    uint32_t compression = 0;               // 0 or 3 - uncompressed; THIS PROGRAM CONSIDERS ONLY UNCOMPRESSED BMP images
    uint32_t sizeImage = 0;                 // 0 - for uncompressed images
    int32_t xPixelsPerMeter = 0;
    int32_t yPixelsPerMeter = 0;
    uint32_t colorsUsed = 0;                // No. color indexes in the color table. Use 0 for the max number of colors allowed by bitCount
    uint32_t colorsImportant = 0;           // No. of colors used for displaying the bitmap. If 0 all colors are required
};

struct BMPColorHeader {
    uint32_t redMask = 0x00ff0000;          // Bit mask for the red channel
    uint32_t greenMask = 0x0000ff00;        // Bit mask for the green channel
    uint32_t blueMask = 0x000000ff;         // Bit mask for the blue channel
    uint32_t alphaMask = 0xff000000;        // Bit mask for the alpha channel
    uint32_t colorSpaceType = 0x73524742;   // Default "sRGB" (0x73524742)
    uint32_t unused[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Unused data for sRGB color space
};
#pragma pack(pop)

struct BMP {
    BMPFileHeader fileHeader;
    BMPInfoHeader bmpInfoHeader;
    BMPColorHeader bmpColorHeader;
    std::vector<uint8_t> data;

    /** Create a new BMP object from the given file
     */
    BMP(const char *fname) {
        read(fname);
    }

    /** Create an "empty" BMP object given the properties of a given BMP
     */
    BMP(BMP *ref) {
        // copy header info
        fileHeader = ref->fileHeader;
        bmpInfoHeader = ref->bmpInfoHeader;
        bmpColorHeader = ref->bmpColorHeader;

        data.resize(ref->data.size());
    }

    /** Read a 24- or 32-bit depth BMP at fname
     * BMP file must have positive width; positive or negative height is allowed.
     */
    void read(const char *fname) {
        std::ifstream inp(fname, std::ios_base::binary);
        if (inp) {
            inp.read((char*)&fileHeader, sizeof(fileHeader));
            if(fileHeader.fileType != 0x4D42) {
                throw std::runtime_error("Read: Unrecognized file format.");
            }
            inp.read((char*)&bmpInfoHeader, sizeof(bmpInfoHeader));

            // The BMPColorHeader is used only for transparent images
            if(bmpInfoHeader.bitCount == 32) {
                // Check if the file has bit mask color information
                if(bmpInfoHeader.size >= (sizeof(BMPInfoHeader) + sizeof(BMPColorHeader))) {
                    inp.read((char*)&bmpColorHeader, sizeof(bmpColorHeader));
                    // Check if the pixel data is stored as BGRA and if the color space type is sRGB
                    checkColorHeader(bmpColorHeader);
                } else {
                    std::cerr << "Read: The file \"" << fname << "\" does not seem to contain bit mask information\n";
                    throw std::runtime_error("Read: Unrecognized file format.");
                }
            }

            // Jump to the pixel data location
            inp.seekg(fileHeader.offsetData, inp.beg);

            // Adjust the header fields for output.
            // Some editors will put extra info in the image file, we only save the headers and the data.
            if(bmpInfoHeader.bitCount == 32) {
                bmpInfoHeader.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
                fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
            } else {
                bmpInfoHeader.size = sizeof(BMPInfoHeader);
                fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);
            }
            fileHeader.fileSize = fileHeader.offsetData;

            // if (bmpInfoHeader.height < 0) {
            //     throw std::runtime_error("The program can treat only BMP images with the origin in the bottom left corner!");
            // }

            data.resize(bmpInfoHeader.width * abs(bmpInfoHeader.height) * bmpInfoHeader.bitCount / 8);

            // Here we check if we need to take into account row padding
            if (bmpInfoHeader.width % 4 == 0) {
                // 4B pixels: no problem
                inp.read((char*)data.data(), data.size());
                fileHeader.fileSize += static_cast<uint32_t>(data.size());
            }
            else {
                // 3B pixels, row width in bytes must be divisible by 4
                rowStride = bmpInfoHeader.width * bmpInfoHeader.bitCount / 8;
                uint32_t newStride = makeStrideAligned(4);
                std::vector<uint8_t> paddingRow(newStride - rowStride);

                for (int y = 0; y < abs(bmpInfoHeader.height); ++y) {
                    inp.read((char*) (data.data() + rowStride * y), rowStride);
                    inp.read((char*) paddingRow.data(), paddingRow.size());
                }
                fileHeader.fileSize += static_cast<uint32_t>(data.size()) + abs(bmpInfoHeader.height) * static_cast<uint32_t>(paddingRow.size());
            }
        }
        else {
            throw std::runtime_error("Read: Unable to open the input image file.");
        }
    } // read

    /** Create a new BMP with given width and height, and optional transperancy channel
     * Width must be positive; height may be positive or negative.
     */
    BMP(int32_t width, int32_t height, bool hasAlpha = true) {
        if (width <= 0) {
            throw std::runtime_error("BMP: Image width must be positive.");
        }

        bmpInfoHeader.width = width;
        bmpInfoHeader.height = height;
        if (hasAlpha) {
            bmpInfoHeader.size = sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);
            fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(BMPColorHeader);

            bmpInfoHeader.bitCount = 32;
            bmpInfoHeader.compression = 3;
            rowStride = width * 4;
            data.resize(rowStride * abs(height));
            fileHeader.fileSize = fileHeader.offsetData + data.size();
        }
        else {
            bmpInfoHeader.size = sizeof(BMPInfoHeader);
            fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

            bmpInfoHeader.bitCount = 24;
            bmpInfoHeader.compression = 0;
            rowStride = width * 3;
            data.resize(rowStride * abs(height));

            uint32_t newStride = makeStrideAligned(4);
            fileHeader.fileSize = fileHeader.offsetData + static_cast<uint32_t>(data.size()) + abs(bmpInfoHeader.height) * (newStride - rowStride);
        }
    } // BMP

    /** Write a BMP to given fname 
     */
    void write(const char *fname) {
        std::ofstream of(fname, std::ios_base::binary);
        if (of) {
            if (bmpInfoHeader.bitCount == 32) {
                writeHeadersAndData(of);
            }
            else if (bmpInfoHeader.bitCount == 24) {
                if (bmpInfoHeader.width % 4 == 0) {
                    writeHeadersAndData(of);
                }
                else {
                    uint32_t newStride = makeStrideAligned(4);
                    std::vector<uint8_t> paddingRow(newStride - rowStride);

                    writeHeaders(of);

                    for (int y = 0; y < abs(bmpInfoHeader.height); ++y) {
                        of.write((const char*)(data.data() + rowStride * y), rowStride);
                        of.write((const char*)paddingRow.data(), paddingRow.size());
                    }
                }
            }
            else {
                throw std::runtime_error("Write: Only BMP with 24 or 32 bits per pixel are currently supported.");
            }
        }
        else {
            throw std::runtime_error("Write: Unable to open the output image file.");
        }
    } // write

    /** Set the color of the pixel at (x0, y0)
     */
    void setPixel(uint32_t x0, int32_t y0, uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
        // check that the requested pixel is within image boundariesm
        bool negativeY = bmpInfoHeader.height < 0 ? true: false;

        if (x0 >= (uint32_t)bmpInfoHeader.width || x0 < 0 ||
                    (negativeY) && (y0 > 0 || y0 <= bmpInfoHeader.height) ||
                    (!negativeY) && (y0 < 0 || y0 >= bmpInfoHeader.height) )
            throw std::runtime_error("Set pixel: The point is outside the image boundaries!");

        uint32_t channels = bmpInfoHeader.bitCount / 8;
        data[channels * (abs(y0) * bmpInfoHeader.width + x0) + 0] = B;
        data[channels * (abs(y0) * bmpInfoHeader.width + x0) + 1] = G;
        data[channels * (abs(y0) * bmpInfoHeader.width + x0) + 2] = R;
        if (channels == 4) {
            data[channels * (abs(y0) * bmpInfoHeader.width + x0) + 3] = A;
        }
    } // setPixel

    /** Get all color channels in a single word for the pixel at (x0, y0) 
     */
    uint32_t getPixel(uint32_t x0, int32_t y0) {
         // check that the requested pixel is within image boundariesm
        bool negativeY = bmpInfoHeader.height < 0 ? true: false;

        if (x0 >= (uint32_t)bmpInfoHeader.width || x0 < 0 ||
                    (negativeY) && (y0 > 0 || y0 <= bmpInfoHeader.height) ||
                    (!negativeY) && (y0 < 0 || y0 >= bmpInfoHeader.height) )
            throw std::runtime_error("Get pixel: The point is outside the image boundaries!");

        uint32_t pixel = 0;
        uint8_t B=0, G=0, R=0, A=0;
        uint32_t channels = bmpInfoHeader.bitCount / 8;
        B = data[channels * (abs(y0) * bmpInfoHeader.width + x0) + 0];
        G = data[channels * (abs(y0) * bmpInfoHeader.width + x0) + 1];
        R = data[channels * (abs(y0) * bmpInfoHeader.width + x0) + 2];
        if (channels == 4)
            A = data[channels * (abs(y0) * bmpInfoHeader.width + x0) + 3];

        pixel = B + (G << 8) + (R << 16) + (A << 24);

        return pixel;
    } // getPixel

    /** Print a pixel's value to standard out
     */
    void printPixel(BMP *img, uint32_t x, int32_t y) {
        uint32_t pixel = img->getPixel(x, y);

        uint8_t B=0, G=0, R=0, A=0;
        // get each individual channel using bitwise AND of the pixel 
        // with each channel mask and shift if needed
        B = (uint8_t)  (pixel & img->bmpColorHeader.blueMask);
        G = (uint8_t) ((pixel & img->bmpColorHeader.greenMask) >> 8);
        R = (uint8_t) ((pixel & img->bmpColorHeader.redMask) >> 16);
        A = (uint8_t) ((pixel & img->bmpColorHeader.alphaMask) >> 24);

        cout << "( " << x << ", " << y << " ) : [ " << (int) B << ", " << (int) G << ", " << (int) R;
        if (img->bmpInfoHeader.bitCount / 8 == 4)
            cout << ", " << (int) A;
        cout << " ]" << endl; 
    } // printPixel


    /** Get a pointer to the data for a given channel at a given pixel (x0, y0)
     */
    uint8_t* getPixelChannelPtr(uint32_t x0, int32_t y0, uint32_t channel) {
         // check that the requested pixel is within image boundariesm
        bool negativeY = bmpInfoHeader.height < 0 ? true: false;

        if (x0 >= (uint32_t)bmpInfoHeader.width || x0 < 0 ||
                    (negativeY) && (y0 > 0 || y0 <= bmpInfoHeader.height) ||
                    (!negativeY) && (y0 < 0 || y0 >= bmpInfoHeader.height) )
            throw std::runtime_error("Get pixel channel pointer: The point is outside the image boundaries!");

        uint32_t channels = bmpInfoHeader.bitCount / 8;
        return &(data[channels * (abs(y0) * bmpInfoHeader.width + x0) + channel]);
    } // getPixelChannel

    /** Get an n x n patch centered at (x0, y0) from a single channel of the image
     * - (x0, y0) is the center of the patch
     * - n includes the center cell, and must be odd
     * - returned vector has a single channel
     * - returned vector is of pointers to elements of the image's data vector
     */
    std::vector<uint8_t*> getPatch(uint32_t x0, int32_t y0, uint32_t n, uint32_t channel) {
        // ensure n is odd
        if (n % 2 != 1)
            throw std::runtime_error("Get patch: n must be odd.");

        // if the BMP has alpha, stride is 4; otherwise stride is 3
        uint32_t channels = bmpInfoHeader.bitCount / 8;
        uint32_t stride = (channels == 4) ? 4 : 3;

        std::vector<uint8_t*> patch;

        // set the direction of y-axis iteration accordingly
        //bool negativeY = bmpInfoHeader.height < 0 ? true: false;

        // top to bottom
        int32_t yi = y0 + (n/2);
        while (yi > y0-((int32_t) n/2)-1) {
            //std::cout << "While " << yi;
            // left to right
            for (uint32_t xi = x0-(n/2); xi<x0+(n/2)+1; xi++) {
                //std::cout << " ~for~ " << xi;
                uint8_t *ptr = getPixelChannelPtr(xi, yi, channel); 
                patch.push_back(ptr);
            } // for xi

            yi--;
            //std::cout << std::endl;
        } // while yi

        return patch;
    } // getPatch

    /** Get a vector of values (NOT references) from a single image channel
     * - channel 0: B; 1: G; 2: R; A: 3
     */
    std::vector<uint8_t> getChannel(uint32_t channel) {
        uint32_t channels = bmpInfoHeader.bitCount / 8;
        if (channel == 3 && channels / 8 == 3)
            throw std::runtime_error("Get channel: alpha channel not available in this image.");

        // start the vector we will return
        std::vector<uint8_t> channelData;

        // get an iterator
        std::vector<uint8_t>::iterator it = data.begin();
        // get it to the first element we care about
        it += channel;

        // collect the channel
        int32_t *width = &bmpInfoHeader.width;
        int32_t *height = &bmpInfoHeader.height;

        for (int pixels=0; pixels<abs((*width) * (*height)); pixels++, it+=channels) {
            channelData.push_back(*it);
        } // for

        return channelData;
    } // getChannel

    /** Sets the given channel to the values in channelData 
     * - channel 0: B; 1: G; 2: R; A: 3
     */
    void setChannel(std::vector<uint8_t> channelData, uint32_t channel) {
        // first check that channelData has enough data
        uint32_t channels = bmpInfoHeader.bitCount / 8;
        if (data.size() / channels != channelData.size())
            throw std::runtime_error("Set channel: channel data size does not match.");

       if (channel == 3 && bmpInfoHeader.bitCount / 8 == 3)
            throw std::runtime_error("Set channel: alpha channel not available in this image.");

        // get a couple iterators
        std::vector<uint8_t>::iterator dataIt = data.begin();
        std::vector<uint8_t>::iterator it = channelData.begin();

        // get dataIt into position for the first bit of data
        dataIt += channel;

        // set the channel
        int32_t *width = &bmpInfoHeader.width;
        int32_t *height = &bmpInfoHeader.height;

        for (int pixels=0; pixels<abs((*width) * (*height)); pixels++, dataIt+=channels, it++) {
            *dataIt = *it;
        } // for
    } // setChannel

    /** Add one or more rows of padding to the top and bottom of the image
     * - rows will be added to both top and bottom
     * - height grows by 2*rows
     */
    void padHeight(uint32_t rows,
                    uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
        // this only works for 4B pixels and positive widths at the moment
        uint32_t channels = bmpInfoHeader.bitCount / 8;
        if (channels != 4)
            throw std::runtime_error("Pad height: Only works for 4-channel images.");
        if (bmpInfoHeader.width < 0)
            throw std::runtime_error("Pad height: Only works for images with positive width.");

        // make the vector to insert at the start and end of the image
        uint32_t width = (uint32_t) bmpInfoHeader.width;
        std::vector<uint8_t> padding;

        for (uint32_t pad=0; pad<width*rows; pad++) {
            // each iteration inserts a pixel
            padding.push_back(B);
            padding.push_back(G);
            padding.push_back(R);
            padding.push_back(A);
        } // for

        // pad top and bottom
        data.insert(data.begin(), padding.begin(), padding.end());
        data.insert(data.end(), padding.begin(), padding.end());

        // update header info so the image can be decoded properly
        fileHeader.fileSize += 2*padding.size();

        int32_t *height = &bmpInfoHeader.height;
            if (*height > 0)
                (*height) += 2*rows;
            else
                (*height) -= 2*rows;

        return;
    } // padHeight

    /** Add one or more columns of padding to the left and right of the image
     * - columns will be added to both left and right
     * - width grows by 2*cols
     */
    void padWidth(uint32_t cols,
                    uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
        // this only works for 4B pixels and positive widths at the moment
        uint32_t channels = bmpInfoHeader.bitCount / 8;
        if (channels != 4)
            throw std::runtime_error("Pad height: Only works for 4-channel images.");
        if (bmpInfoHeader.width < 0)
            throw std::runtime_error("Pad height: Only works for images with positive width.");
    
        // create the pixel that will be used for padding
        std::vector<uint8_t> pixel;
        pixel.push_back(B);
        pixel.push_back(G);
        pixel.push_back(R);
        pixel.push_back(A);

        int32_t *width = &bmpInfoHeader.width;
        int32_t *height = &bmpInfoHeader.height;

        std::vector<uint8_t>::iterator it = data.begin();

        for (uint32_t row = 0; row < (uint32_t) abs(*height); row++) {
            // insert cols pixels on the left
            for (uint32_t col = 0; col < cols; col++) {
                // each iteration inserts a pixel
                // it points to the location of the first inserted element on return
                it = data.insert(it, pixel.begin(), pixel.end());
                //fileHeader.fileSize += channels;
            } // for

            // it must move to the end of the current line, 
            // which now includes the new pixels on the left
            it += (*width)*channels+channels*cols;

            // insert cols pixels on the right
            for (uint32_t col = 0; col < cols; col++) {
                it = data.insert(it, pixel.begin(), pixel.end());
                //fileHeader.fileSize += channels;
            } // for

            // it must be moved to the end of the current line, 
            // past the new pixels on the right
            it += channels*cols;
        } // for

        // update header info so the image can be decoded properly
        fileHeader.fileSize += abs(*height)*channels*cols*2;
        (*width) += cols*2;

        return;
    } // padWidth

    /** Add one or more pixels of padding to the edge of the image
     * - pixels rows and columns will be added around the image
     * - height and width grow by pixels*2
     */
    void pad(uint32_t pixels,
                uint8_t B, uint8_t G, uint8_t R, uint8_t A) {
    
        padHeight(pixels, B, G, R, A);
        padWidth(pixels, B, G, R, A);

        return;
    } // pad

private:
    uint32_t rowStride = 0;

    void writeHeaders(std::ofstream &of) {
        of.write((const char*)&fileHeader, sizeof(fileHeader));
        of.write((const char*)&bmpInfoHeader, sizeof(bmpInfoHeader));
        if(bmpInfoHeader.bitCount == 32) {
            of.write((const char*)&bmpColorHeader, sizeof(bmpColorHeader));
        }
    } // writeHeaders

    void writeHeadersAndData(std::ofstream &of) {
        writeHeaders(of);
        of.write((const char*)data.data(), data.size());
    } // writeHeadersAndData

    // Add 1 to the rowStride until it is divisible with align_stride
    uint32_t makeStrideAligned(uint32_t align_stride) {
        uint32_t newStride = rowStride;
        while (newStride % align_stride != 0) {
            newStride++;
        }
        return newStride;
    } // makeStrideAligned

    // Check if the pixel data is stored as BGRA and if the color space type is sRGB
    void checkColorHeader(BMPColorHeader &bmpColorHeader) {
        BMPColorHeader expected_color_header;
        if(expected_color_header.redMask != bmpColorHeader.redMask ||
            expected_color_header.blueMask != bmpColorHeader.blueMask ||
            expected_color_header.greenMask != bmpColorHeader.greenMask ||
            expected_color_header.alphaMask != bmpColorHeader.alphaMask) {
            throw std::runtime_error("Check color header: Unexpected color mask format! The program expects the pixel data to be in the BGRA format");
        }
        if(expected_color_header.colorSpaceType != bmpColorHeader.colorSpaceType) {
            throw std::runtime_error("Check color header: Unexpected color space type! The program expects sRGB values");
        }
    } // checkColorHeader
};
