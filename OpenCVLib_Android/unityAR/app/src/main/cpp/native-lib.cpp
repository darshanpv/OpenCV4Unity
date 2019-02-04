#include <jni.h>
#include <string>

#include "include/opencv2/core.hpp"
#include "include/opencv2/imgproc.hpp"

using namespace cv;

extern "C"
JNIEXPORT jstring JNICALL
Java_com_hmi_unityar_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C" {
uint8_t *resultBuffer;
int bufferWidth;
int bufferHeight;
// In order to save memory, we call this method to initialize our buffer to the
// size actually needed by Unity assuming that we will not resize our video buffer
int InitCV_Internal(int width, int height) {
    size_t size = width * height * 4;
    resultBuffer = new uint8_t[size];

    bufferWidth = width;
    bufferHeight = height;

    return 0;
}

// This method is called every frame to apply OpenCV processes to the buffer. Due
// to passing the whole buffer back and forth being expensive, it would be better
// to pull camera data from Android and only pass it up to Unity for debug purposes.

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
// This method only exists to ensure that we are getting correct values back from the native
// side using a simple integer return instead of a more complex byte array.
int FooTestFunction_Internal() {
    return 12345;
}

}