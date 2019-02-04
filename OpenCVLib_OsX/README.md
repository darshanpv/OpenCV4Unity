This project is used to create OpenCV libraries used as Unity Plugin for OsX platform.

** Prerequisite ** – Xcode and CMake

## Build and Install of opencv 
1.	Download openCV and unzip it somewhere on your computer. Create build folder inside it
2.	Open CMake
3.	Click Browse Source and navigate to your openCV folder.
4.	Click Browse Build and navigate to your build Folder.
5.	Click the configure button. You will be asked how you would like to generate the files. Choose Unix-Makefile from the Drop Down menu and Click OK. CMake will perform some tests and return a set of red boxes appear in the CMake Window.
6.	Click Configure again, then Click Generate.
7.	Goto build folder

```
# cd build
# make
# sudo make install
```

8.	This will install the opencv libraries on your computer.

## Create Plug-In using Xcode
We will create a Native Plugin for Mac used in Unity as Plugin. 
1.	Please start Xcode and choose 
- MacOS> Framework & Library> Bundle. "Product Name" is set to "UnityPlugin".
- Use packagae “com.hmi”
- Change bundle identifier to “com.hmi.opencvplugin”
- Select UnityPlugin in the navigation view and enter "/usr/local/include" in the "Build Settings" → "Header Search Path" field (enter the details without collapsing the “Header Search Paths”)
- In the "Build Settings" → "Library Search Path" field, enter “/usr/local/lib”
- In the "Build Settings" → "Other Linker Flags" field, enter “-lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core”

These are the linked libraries obtained from command
```
# pkg-config --libs --cflags opencv

-I/usr/local/include/opencv -I/usr/local/include -L/usr/local/lib -lopencv_dnn -lopencv_ml -lopencv_objdetect -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_imgproc -lopencv_flann -lopencv_core
```

- Next, create a new cpp file (OpenCVPlugin.cpp) (deslect “Also create header file” for use as a Native Plugin.  
OpenCVPlugin.cpp
```
#include <stdio.h>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

extern "C" {
    
    void conv(unsigned char* arr, int w, int h);
    
    int InitCV_Internal(int width, int height);
    uint8_t *SubmitFrame_Internal(int width, int height, uint8_t *buffer);
    int FooTestFunction_Internal();
    
}

uint8_t *resultBuffer;
int bufferWidth;
int bufferHeight;

void conv(unsigned char* arr, int w, int h)
{
    Mat img(Size(w, h), CV_8UC4, cv::Scalar(0, 0, 0));
    Mat gray(Size(w, h), CV_8UC1, cv::Scalar(0, 0, 0));
    
    // char* -> Mat
    int i = 0;
    for(int y = 0; y < img.rows; ++y){
        for(int x = 0; x < img.cols; ++x){
            for(int c = 0; c < img.channels(); ++c){
                img.data[ y * img.step + x * img.elemSize() + c ] = arr[i++];
            }
        }
    }
    
    // RGBA -> GRAY
    cvtColor(img, gray, CV_RGBA2GRAY);
    cvtColor(gray, img, CV_GRAY2RGBA);
    
    // Mat -> char*
    i = 0;
    for(int y = 0; y < img.rows; ++y){
        for(int x = 0; x < img.cols; ++x){
            for(int c = 0; c < img.channels(); ++c){
                arr[i++] = img.data[ y * img.step + x * img.elemSize() + c ];
            }
        }
    }
}

int InitCV_Internal(int width, int height) {
    size_t size = width * height * 4;
    resultBuffer = new uint8_t[size];
    
    bufferWidth = width;
    bufferHeight = height;
    
    return 0;
}

uint8_t *SubmitFrame_Internal(int width, int height, uint8_t *buffer) {
    
    Mat inFrame(height, width, CV_8UC4, buffer);
    Mat outFrame(inFrame.rows, inFrame.cols, CV_8UC4, Scalar(0, 0, 0));
    
    Mat processingFrame;
    
    cvtColor(inFrame, processingFrame, CV_RGBA2GRAY);
    Canny(processingFrame, processingFrame, 0, 30, 3);
    cvtColor(processingFrame, outFrame, CV_GRAY2RGBA);
    
    size_t size = bufferWidth * bufferHeight * 4;
    memcpy(resultBuffer, outFrame.data, size);
    
    inFrame.release();
    outFrame.release();
    processingFrame.release();
    
    
    return resultBuffer;
}

int FooTestFunction_Internal() {
    return 12345;
}
```
- Here we are creating OpenCV function to perform color to Canny edge conversion. Ultimately, these functions will be called from the Unity side. 
e.g. conv function - In the argument, we received the array of pixels and the vertical and horizontal widths of the image. Because Unity can not use cv :: Mat type, here we are exchanging pixel data of the image as an array of unsigned char. Inside the conv function, I convert it to grayscale using OpenCV, then write it back to pixel array again.
- Select "Product" → "Build" from the menu bar and build the project, UnityPlugin.bundle will be created in the Products folder.