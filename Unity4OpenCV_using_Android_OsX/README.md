This is a Unity project that uses OpenCV libraries created for Android and OsX platform.

** prerequisite** you have created required libraries for Android and MacOsX as explained in other sections and are ready with it.

## Creating OpenCV Plugin in Unity Android
Now in unity -
+ Create a new Unity project
- Create folder called Plugins inside the Assets folder. Then another called Android inside Plugins, and another called libs inside Android. Copy the folders “x86” and “armeabi-v7a” from app/build/intermediates/cmake/debug/obj
- These are the processor architectures that Android supports (ARMv8, ARMv7 and x86). Android also supports MIPS but it’s the least popular and not supported by Unity. Also, the 64 counterparts of ARM and x86 are not supported by Unity either.
- Also copy inside the corresponding architecture folder in Plugins the file libopencv_java3.so that can be found in OpenCV4Android/OpenCV-android-sdk/sdk/native/libs
- Once you've copied your .so in this folder, Unity will treat them as plugins.

## Creating OpenCV Plugin in Unity for OsX
- Create Osx folder under assets->Plugin.
- Insert "UnityPlugin.bundle" earlier into the created Plugin folder. At this time, you can drag and drop directly from Xcode's Products folder to Unity. (Goto Xcode editor and right click OpenCVPlugin.bundle and select show in folder. From there you can drag it into Unity)

## OpenCV application
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