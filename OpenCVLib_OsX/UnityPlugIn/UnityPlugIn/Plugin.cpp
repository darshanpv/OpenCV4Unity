//
//  Plugin.cpp
//  UnityPlugIn
//
//  Created by Purushottam Darshankar on 21/01/19.
//  Copyright © 2019 Purushottam Darshankar. All rights reserved.
//

#include "Plugin.pch"
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

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
