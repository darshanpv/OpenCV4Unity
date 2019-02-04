# Building OpenCV PlugIn for Unity with Example

Part-1 Building plugin for Android
----------------------------------
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

## Creating OpenCV Plugin in Unity
Now in unity -
+ Create a new Unity project
- Create folder called Plugins inside the Assets folder. Then another called Android inside Plugins, and another called libs inside Android. Copy the folders “x86” and “armeabi-v7a” from app/build/intermediates/cmake/debug/obj
- These are the processor architectures that Android supports (ARMv8, ARMv7 and x86). Android also supports MIPS but it’s the least popular and not supported by Unity. Also, the 64 counterparts of ARM and x86 are not supported by Unity either.
- Also copy inside the corresponding architecture folder in Plugins the file libopencv_java3.so that can be found in OpenCV4Android/OpenCV-android-sdk/sdk/native/libs
- Once you've copied your .so in this folder, Unity will treat them as plugins.
- Finally, to use the C++ functionality from Unity
1. Create a Dummy controller – cube object in Main Scene. 
2. Create “InImage” and “OutImage”as RawImages on canvas. (We will take camera imgae and apply to “InImage” and convert it into canny edge using OpenCV and render it to “OutImage”)

- Create Scripts folder in asset directory.
- Under Scripts folder create DummyController.cs scriot and attach it to DummyController object created earlier (cube)
```
using UnityEngine;
using UnityEngine.UI;

using System.Runtime.InteropServices;
using System;

/*
    DummyController provides examples for passing pointers for byte arrays back and forth
    between managed C# and native C++. When making your own classes, be sure to specify
    the Texture2D texture format before attempting to load raw bytes into it.
 */
public class DummyController : MonoBehaviour
{
    public float RotateSpeed = 0.5f;

    public RawImage InImage;
    public RawImage OutImage;

    WebCamTexture wct;
    Texture2D outTexture;

    void Awake()
    {
#if UNITY_EDITOR
        int width = 1280;
        int height = 720;
#else
        int width = 320;
        int height = 240;
#endif

        NativeLibAdapter.InitCV(width, height);

        outTexture = new Texture2D(width, height, TextureFormat.RGBA32, false);

        wct = new WebCamTexture(width, height);
        wct.Play();

        Debug.LogWarning("Foo Value in C++ is " + NativeLibAdapter.FooTest());
    }

    void Update()
    {
        this.transform.Rotate(Vector3.up, RotateSpeed * Time.deltaTime);

        if (wct.width > 100 && wct.height > 100)
        {
            Color32[] pixels = wct.GetPixels32();
            GCHandle pixelHandle = GCHandle.Alloc(pixels, GCHandleType.Pinned);

            IntPtr results = NativeLibAdapter.SubmitFrame(wct.width, wct.height, pixelHandle.AddrOfPinnedObject());
            int bufferSize = wct.width * wct.height * 4;
            byte[] rawData = new byte[bufferSize];

            if (results != IntPtr.Zero)
            {
                Marshal.Copy(results, rawData, 0, bufferSize);

                outTexture.LoadRawTextureData(rawData);
                outTexture.Apply();
            }
            
            InImage.texture = wct;
            OutImage.texture = outTexture;

            rawData = null;
            pixelHandle.Free();
        }
    }
} 
```
- Under Scripts folder create NativeLibAdapter.cs that acts as a layer between C# and C++ code.

```
using System.Runtime.InteropServices;
using System;

using UnityEngine;

/*
    NativeLibAdapter is an example communication layer between managed C# and native C++
 */
public class NativeLibAdapter
{
#if !UNITY_EDITOR
	[DllImport("native-lib")]
	private static extern int InitCV_Internal(int width, int height);

	[DllImport("native-lib")]
	private static extern IntPtr SubmitFrame_Internal(int width, int height, IntPtr bufferAddr);

	[DllImport("native-lib")]
	private static extern int FooTestFunction_Internal();
#elif UNITY_EDITOR
    [DllImport ("OpenCVPlugin")]
    private static extern int InitCV_Internal(int width, int height);

    [DllImport("OpenCVPlugin")]
    private static extern IntPtr SubmitFrame_Internal(int width, int height, IntPtr bufferAddr);

    [DllImport("OpenCVPlugin")]
    private static extern int FooTestFunction_Internal();
#endif


    public static int InitCV(int width, int height)
	{
#if !UNITY_EDITOR
		int result = InitCV_Internal(width, height);
#elif UNITY_EDITOR
        int result = InitCV_Internal(width, height);
#else
		int result = -1;
#endif
        Debug.LogWarning("[NativeLibAdapter] InitCV " + (result == 0 ? "No Errors" : "Error Code : " + result));

		
		return result;
	}

	public static IntPtr SubmitFrame(int width, int height, IntPtr bufferAddr)
	{
#if !UNITY_EDITOR
		IntPtr ret = SubmitFrame_Internal(width, height, bufferAddr);
#elif UNITY_EDITOR
        IntPtr ret = SubmitFrame_Internal(width, height, bufferAddr);
#else
		IntPtr ret = IntPtr.Zero;
#endif
        return ret;
	}

	public static int FooTest()
	{
#if !UNITY_EDITOR
		return FooTestFunction_Internal();
#elif UNITY_EDITOR
        return FooTestFunction_Internal();
#else
		return -1;
#endif
    }
} 
```
- Once the script is attached to Dummycontroller object pass “InImage” and “outImage” as an argument to script for In Image and Out Image respectively
- Now you build the project for android and you should be able to run the project. (this will not run on Unity Editor as the libraries are bundled for Android platform)
- More info at https://forum.unity.com/threads/tutorial-using-c-opencv-within-unity.459434/
- For more details please refer OpenCVBridge unity and UnityAR android project at git repo above.


Part-1 Building plugin for MacOsX
----------------------------------
To use OpenCV functions in MAC Os from Native Plugin without using 3rd party plugin from assets store (openCVforUnity)

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

## Creating OpenCV Plugin in Unity for OsX

- Please create a project with Unity and create a Plugin folder under the Asset folder. Create Osx folder under assets-Plugin.
- Insert "UnityPlugin.bundle" earlier into the created Plugin folder. At this time, you can drag and drop directly from Xcode's Products folder to Unity. (Goto Xcode editor and right click OpenCVPlugin.bundle and select show in folder. From there you can drag it into Unity)

-Follow other steps as mentioned in above section to complete OpenCV funcationlaities.