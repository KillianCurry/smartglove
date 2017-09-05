using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;


public class HandController:MonoBehaviour
{
    [Tooltip("Maximum degrees the finger can rotate per second.")]
    public float rotationSpeed = 180f;
	//list of finger rotation ranges
	public List<float> rotationMinimum;
    public List<float> rotationMaximum;
	//list of each joint
	private List<Transform> joints;
	
	//TODO roll this calibration into the API
	private Quaternion zeroRotation;
	
	//is there one sensor per finger?
	public bool fiveSensor = false;
	[HideInInspector]
	//-1 is left-handed, 1 is right-handed
	public int handedness = 1;

    //list of each joint's rotation
    public List<double> fingerRotations;
	//vector of the x, y, and z rotation
	public Vector3 palmOrientation;
	private float fingerCurl;
	
	//is the BLE connected?
	[HideInInspector]
	public bool connected = false;
	public int ID;
	
	//import functions from the DLL
	[DllImport("smartglove", EntryPoint="establishConnection", SetLastError = true)]
	public static extern bool openConnection(int gloveID);
	[DllImport("smartglove", EntryPoint="closeConnection")]
	public static extern bool closeConnection(int gloveID);
	[DllImport("smartglove", EntryPoint="getData", CallingConvention = CallingConvention.Cdecl)]
	public static extern IntPtr readGlove(int gloveID);
	[DllImport("smartglove", EntryPoint="clearCalibration")]
	public static extern void clearCalibration(int gloveID);
	
	public bool GloveConnect()
	{
		if (connected) return true;
		connected = openConnection(ID);
        if (!connected)
        {
            //write the exception from the DLL if the connection doesn't work
            Debug.Log("CONNECTION ERROR: 0x" + Marshal.GetLastWin32Error().ToString("X"));
        }
		return connected;
	}
	
	public bool GloveDisconnect()
	{
		if (!connected) return true;
		connected = !closeConnection(ID);
		//if (!connected) StopCoroutine("GloveRead");
		return !connected;
	}
	
	public void GloveClear()
	{
		zeroRotation = Quaternion.Inverse(Quaternion.Euler(palmOrientation.x, palmOrientation.y, palmOrientation.z));
		clearCalibration(ID);
	}
	
	void Start()
	{
		joints = new List<Transform>();
		fingerRotations = new List<double>();
		rotationMinimum = new List<float>();
		rotationMaximum = new List<float>();
		//set rotation limits
		rotationMinimum = new List<float>()
		{
			0f,0f,0f,	 //thumb
			-10f,-15f,0f,//index
			-10f,-15f,0f,//middle
			-10f,-15f,0f,//ring
			-10f,-15f,0f //pinky
		};
		rotationMaximum = new List<float>()
		{
			90f,90f,90f, //thumb
			75f,110f,80f,//index
			75f,110f,80f,//middle
			75f,110f,80f,//ring
			75f,110f,80f,//pinky
		};
		//instantiate the hand prefab and transfer its contents to this
		GameObject gloveObject = (GameObject)Instantiate(Resources.Load("models/glove", typeof(GameObject)));
		gloveObject.transform.GetChild(0).SetParent(transform, false);
		gloveObject.transform.GetChild(0).SetParent(transform, false);
		//mark all the joints for easier reference later
		for (int i = 0; i < 5; i++)
		{
			joints.Add(transform.GetChild(0).GetChild(0).GetChild(i));
			fingerRotations.Add(0d);
			joints.Add(transform.GetChild(0).GetChild(0).GetChild(i).GetChild(0));
			fingerRotations.Add(0d);
			joints.Add(transform.GetChild(0).GetChild(0).GetChild(i).GetChild(0).GetChild(0));
			fingerRotations.Add(0d);
		}
		//destroy the now empty prefab
		Destroy(gloveObject);
		UpdateTexture();
	}
	
	void Update()
	{
		transform.localScale = new Vector3(-handedness, 1, 1);
        //update joint angles and palm orientation to match input
        //otherwise curl the fingers (so it unfolds when connected)
        fingerCurl = 1f;
		if (!connected) fingerRotations[0] = rotationMinimum[0] + (fingerCurl * rotationMaximum[0]);
		joints[0].localRotation = Quaternion.Euler(joints[0].localRotation.eulerAngles.x, -(float)fingerRotations[0], joints[0].localRotation.eulerAngles.z);
		for (int r = 1; r < fingerRotations.Count; r++)
		{
			if (!connected) fingerRotations[r] = rotationMinimum[r] + (fingerCurl * rotationMaximum[r]);
			float spread = 0f;
			if (r % 3 == 0) spread = ((((r/3)-2)*5)*((90f-(float)fingerRotations[r])/90f));
            joints[r].localRotation = Quaternion.RotateTowards(joints[r].localRotation, Quaternion.Euler((float)fingerRotations[r], 0f, spread), rotationSpeed * Time.deltaTime);
		}
		if (connected) this.transform.localRotation = zeroRotation * Quaternion.Euler(palmOrientation.x, palmOrientation.y, palmOrientation.z);
	}
	
	public void UpdateTexture()
	{
		string texName = "textures/logo";
		if (handedness == -1) texName += "R";
		else texName += "L";
		transform.GetChild(1).GetComponent<SkinnedMeshRenderer>().material.mainTexture = (Texture)Resources.Load(texName, typeof(Texture));
	}
	
	void OnApplicationQuit()
	{
		if (connected) GloveDisconnect();
	}
}
