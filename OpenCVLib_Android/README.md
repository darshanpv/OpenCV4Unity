This Android project is used to build openCV libraries that are used in Unity Project as openCV Plugin

Prerequisite
-	Android Studio
-	CMake
-	Android NDK

## Build opencv library using Android studio
+ Download OpenCV4Android (Android Package) from this page. (https://opencv.org/releases.html) – this has been tested on 3.4.5 and should work for latest 4 release also.
- sdk/java folder is key if you're going to use Java OpenCV code. (but Opencv using JAVA is too slow hence ruled out)
- sdk/native folder contains OpenCV C++ headers (for JNI code) and native Android libraries (*.so and *.a) for ARM-v8a, ARM-v7a and x86 architectures. Header files are in sdk/native/jni/include and libraries in sdk/native/libs.

+ Create Android project with C/C++ support
- Check “Phone and tablet”, leave the recommended minimum SDK version.
- Include an Empty Activity (this will create files necessary for the C++ functionality)
- Keep the rest of the options as they come by default.
- Android Studio has now created a project folder. Inside it, in app/src/main/cpp/, Android Studio will have created a file called native-lib.cpp, which contains an example function called stringfromJNI() that we can ignore. We will write our C++ OpenCV code in this file.
- In the app folder inside your project folder, a file called CMakeLists.txt will be created. This is the file with all the instructions on how to compile your native C++ code. Leave it as it is right now, we'll be modifying it in a bit.
- Copy the libraries of OpenCV4Android folder, contained in sdk/native/libs (all folders for different architectures) in app/main/jniLibs (create jniLibs folder) inside your Android Studio project. Delete all static libraries (.a) from all folders, only leaving libopencv_java3.so in each of the architecture's folders. The other ones are not needed.

- Copy the header files in OpenCV4Android (sdk/native/jni/include) to a folder in app/src/main/cpp/include folder (create include folder). I think this folder could be anywhere, the only important thing is that it needs to be in a location accessible by Android Studio

- Edit the CMakeLists.txt file so that 
1. It imports the OpenCV library (libopencv_java3.so) as a shared library; 
2. It adds the library as a target; 
3. It includes the path to OpenCV’s header files. This is a copy of the file, which does all of this.

```
cmake_minimum_required(VERSION 3.4.1)

add_library( # Sets the name of the library.
            native-lib            
# Sets the library as a shared library.
            SHARED
# Provides a relative path to your source file(s).
            src/main/cpp/native-lib.cpp)

add_library(opencv-lib SHARED IMPORTED)

set_target_properties(opencv-lib PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so)

include_directories(src/main/cpp/include)

find_library( # Sets the name of the path variable.
             log-lib            
# Specifies the name of the NDK library that
# you want CMake to locate.
             log)


target_link_libraries( # Specifies the target library.
                      native-lib
                      opencv-lib
                      
# Links the target library to the log library
# included in the NDK.
                      ${log-lib})

```
- Modify native-lib.cpp code to handle opencv functions that Unity project can call

```
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
//global variables
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

```
- Now the project is ready to be built in Android Studio: Build->Make project. This will generate a shared library (.so file, in this case called libnative-lib.so) for native code for each architecture in jniLibs. The generated libraries can be found in the folder app/build/intermediates/cmake/debug/obj.