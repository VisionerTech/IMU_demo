
## IMU DEMO

This is an example of VMG-PROV IMU 3-dof rendering. IMU information is syned and encoded into image frame by hardware. "/RenderingPLuginExample52/RenderingPlgun/VisualStudio2013" holds a Visual Studio 2013 project building a dll render call of Unity. "/RenderingPLuginExample52/UnityProject" holds a unity project the camera rotates exactly like the VMG-PROV headset. This project could be use as a start of 3-dof VR.

## Requirement:

1.  Recommended specs: Intel Core i5-4460/8G RAM/GTX 660/at least two USB3.0/
2.  Windows x64 version.(tested on win7/win10)

## Installation

1.  Install [Unity](https://unity3d.com/) 64bit editor(tested on Unity 5.4.0f3 (64-bit))

2.  (Optional)Download and install Visual Studio 2013. Compiled and minimum dependencies dlls is provided in "/RenderingPluginExample52/UnityProject/Assets/Plugins/x86_64". The Unity project could run directly. If you would like to build the dlls, visual studio 2013 is needed.

3.  (Optional) Download and install OpenCV(version 2.4.X) as: http://docs.opencv.org/2.4.11/doc/tutorials/introduction/windows_install/windows_install.html. Minimum opencv dlls is provided in "/Assets/Plugins/" in the Unity project. However, we strongly recomend a full installation of opencv and config opencv dlls into the environment variable.

4.  (Optional) "/RenderingPluginExample52/RenderingPlugin/VisualStudio2013" folder holds a Visual Studio 2013 project, which builds a RenderingPlugin.dll and copy to Unity plugin. If you need to compile and build it, open "/RenderingPluginExample52/RenderingPlugin/VisualStudio2013/RenderingPlugin.sln" with Visual Studio 2013 and config it to your opencv environment following:http://docs.opencv.org/2.4.11/doc/tutorials/introduction/windows_visual_studio_Opencv/windows_visual_studio_Opencv.html. Besieds opencv, this project needs [videoInput](https://github.com/ofTheo/videoInput) to retrieve camera name and "/RenderingPluginExample52/RenderingPlugin/VMG_IMU_Paritycheck" to parity check bitcode. Prebuild lib for vs2013 x64 is provided.

## How to Run

1.  Make sure VMG-PROV device is running.

2.  Open this project with Unity editor and open the scene "scene".

3.  Run the project and the camera would rotates as VMG-PROV headset.
