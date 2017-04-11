// on OpenGL ES there is no way to query texture extents from native texture id
#if (UNITY_IPHONE || UNITY_ANDROID) && !UNITY_EDITOR
	#define UNITY_GLES_RENDERER
#endif


using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;


public class UseRenderingPlugin : MonoBehaviour
{
	// Native plugin rendering events are only called if a plugin is used
	// by some script. This means we have to DllImport at least
	// one function in some active script.
	// For this example, we'll call into plugin's SetTimeFromUnity
	// function and pass the current time so the plugin can animate.

#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin")]
#endif
	private static extern void SetTimeFromUnity(float t);


	// We'll also pass native pointer to a texture in Unity.
	// The plugin will fill texture data from native code.
#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport ("RenderingPlugin")]
#endif
#if UNITY_GLES_RENDERER
	private static extern void SetTextureFromUnity(System.IntPtr texture, int w, int h);
#else
	private static extern void SetTextureFromUnity(System.IntPtr texture);
#endif


#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport("RenderingPlugin")]
#endif
	private static extern void SetUnityStreamingAssetsPath([MarshalAs(UnmanagedType.LPStr)] string path);


#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
	[DllImport("RenderingPlugin")]
#endif
	private static extern IntPtr GetRenderEventFunc();

	//native function for open web cam.
	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin")]
	#endif
	private static extern Boolean OpenWebCam();

	//native function call for destroying web cam
	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin")]
	#endif
	private static extern void DestroyWebCam();

	//native function call for getting return imu quaterion vector length
	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin")]
	#endif
	private static extern int GetIMUQuaterionLength();

	//native function for getting return imu quaterion array
	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin", EntryPoint="GetQuaterionVector")]
	#endif
	private static extern IntPtr GetQuaterionVector();

	//native function call for getting return imu acceleration vector length
	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin", EntryPoint="GetIMUAccelerationLength")]
	#endif
	private static extern int GetIMUAccelerationLength();

	//native function for getting return imu quaterion array
	#if UNITY_IPHONE && !UNITY_EDITOR
	[DllImport ("__Internal")]
	#else
	[DllImport ("RenderingPlugin", EntryPoint="GetAccelerationVector")]
	#endif
	private static extern IntPtr GetAccelerationVector();

	private float[] imu_q_array = new float[4];
	private float[] imu_acce_array = new float[3];
	public GameObject main_camera;

	IEnumerator Start()
	{
		if (OpenWebCam ()) {
			Debug.Log("init success!");
		}else {
			Debug.Log("init false");
		}

		main_camera = GameObject.Find ("Main Camera");

		SetUnityStreamingAssetsPath(Application.streamingAssetsPath);

		CreateTextureAndPassToPlugin();
		yield return StartCoroutine("CallPluginAtEndOfFrames");
	}

	private void CreateTextureAndPassToPlugin()
	{
		// Create a texture
		Texture2D tex = new Texture2D(256,256,TextureFormat.ARGB32,false);
		// Set point filtering just so we can see the pixels clearly
		tex.filterMode = FilterMode.Point;
		// Call Apply() so it's actually uploaded to the GPU
		tex.Apply();

		// Set texture onto our matrial
		GetComponent<Renderer>().material.mainTexture = tex;

		// Pass texture pointer to the plugin
	#if UNITY_GLES_RENDERER
		SetTextureFromUnity (tex.GetNativeTexturePtr(), tex.width, tex.height);
	#else
		SetTextureFromUnity (tex.GetNativeTexturePtr());
	#endif
	}

	private IEnumerator CallPluginAtEndOfFrames()
	{
		while (true) {
			// Wait until all frame rendering is done
			yield return new WaitForEndOfFrame();

			// Set time for the plugin
			SetTimeFromUnity (Time.timeSinceLevelLoad);

			// Issue a plugin event with arbitrary integer identifier.
			// The plugin can distinguish between different
			// things it needs to do based on this ID.
			// For our simple plugin, it does not matter which ID we pass here.
			GL.IssuePluginEvent(GetRenderEventFunc(), 1);


			int quaterion_vec_length = GetIMUQuaterionLength ();
//			Debug.Log ("quaterion vector length" + quaterion_vec_length);

			if (quaterion_vec_length > 4) {
				IntPtr ptr = GetQuaterionVector ();
				Marshal.Copy (ptr, imu_q_array, 0, 4);
//				Debug.Log ("w:" + imu_q_array [0]);
//				Debug.Log ("x:" + imu_q_array [1]);
//				Debug.Log ("y:" + imu_q_array [2]);
//				Debug.Log ("z:" + imu_q_array [3]);

				Quaternion imu_q = new Quaternion (imu_q_array [1], -imu_q_array [3], imu_q_array [2], imu_q_array [0]);

				main_camera.transform.rotation = imu_q;

				main_camera.transform.Rotate (Vector3.right * -90.0f);
			}


			int acceleartion_vec_length = GetIMUAccelerationLength();
//			Debug.Log ("acceleration vector length" + acceleartion_vec_length);

			if (acceleartion_vec_length > 3) {
				IntPtr ptr = GetAccelerationVector ();
				Marshal.Copy (ptr, imu_acce_array, 0, 3);

				Debug.Log ("x:" + imu_acce_array [0]);
				Debug.Log ("y:" + imu_acce_array [1]);
				Debug.Log ("z:" + imu_acce_array [2]);

			}
        }
	}

	void OnApplicationQuit()
	{
		DestroyWebCam ();
		Debug.Log("quit");
	}

}
