using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;


public class HandController:MonoBehaviour
{
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
	[DllImport("smartglove", EntryPoint="establishConnection")]
	public static extern bool openConnection(int gloveID);
	[DllImport("smartglove", EntryPoint="closeConnection")]
	public static extern bool closeConnection(int gloveID);
	[DllImport("smartglove", EntryPoint="getData", CallingConvention = CallingConvention.Cdecl)]
	public static extern IntPtr readGlove(int gloveID);
	[DllImport("smartglove", EntryPoint="clearCalibration")]
	public static extern void clearCalibration(int gloveID);
	
	//TODO support for multiple tinyTILES
	//TODO initialize plugin to remove lingering variables??
	
	public bool GloveConnect()
	{
		if (connected) return true;
		connected = openConnection(ID);
		//if (connected) StartCoroutine("GloveRead");
		zeroRotation = Quaternion.Inverse(Quaternion.Euler(palmOrientation.x, palmOrientation.y, palmOrientation.z));
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
		//otherwise let the user control the hand via interface
		fingerCurl = Mathf.Sin(Time.time)*0.5f + 0.5f;
		if (!connected) fingerRotations[0] = rotationMinimum[0] + (fingerCurl * rotationMaximum[0]);
		joints[0].localRotation = Quaternion.Euler(joints[0].localRotation.eulerAngles.x, -(float)fingerRotations[0], joints[0].localRotation.eulerAngles.z);
		for (int r = 1; r < fingerRotations.Count; r++)
		{
			if (!connected) fingerRotations[r] = rotationMinimum[r] + (fingerCurl * rotationMaximum[r]);
			float spread = 0f;
			if (r % 3 == 0) spread = ((((r/3)-2)*5)*((90f-(float)fingerRotations[r])/90f));
			joints[r].localRotation = Quaternion.Euler((float)fingerRotations[r], 0f, spread);
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
	
	//coroutine to read data from arduino into fingerRotations, avoiding lag from waiting to read serial
	IEnumerator GloveRead()
	{
		while(true)
		{
			//copy data from the DLL's unmanaged memory into a managed array
			double[] data = new double[13];
			IntPtr ptr = readGlove(ID);
			Marshal.Copy(ptr, data, 0, 13);
			
			string dat = "";
			for (int i = 0; i < 13; i++)
			{
				dat += data[i].ToString() + " ";
			}
			Debug.Log(dat);
			
			//copy the orientation data (y and z negated to remove mirroring)
			palmOrientation = new Vector3((float)data[0],(float)data[2],(float)data[1]);
			
			//TODO more robust matching of sensors to finger representation
			//loop through the calibrated finger rotation data
			for (int i = 0; i < 15; i++)
			{
				//TODO move all of this into the library
				//TODO make the array the library passes 15 values long, even if the last joint values are duplicates
				//get the stretch data from the BLE (3 to 12)
				double val = data[(i/3)*2+3];
				if ((i%3) != 0) val = data[(i/3)*2+4];
				//clean out NaN values
				if (double.IsNaN(val)) val = 0d;
				//clamp just in case it exceeds extremes
				if (val < 0d) val = 0d;
				else if (val > 1d) val = 1d;
				//if the glove is ten-sensor, just directly translate the value, splitting it between the two extreme joints
				if (!fiveSensor)
				{
					fingerRotations[i] = rotationMinimum[i] + (val * rotationMaximum[i]);
				}
				//otherwise, split even values among all joints of a finger
				else
				{
					//TODO five sensor
				}
			}
			
			//DEV return null means this will be called again next frame
			//DEV return new WaitForSeconds(t) means this will wait t seconds before being called again
			yield return null;//new WaitForSeconds(0.1f);
		}
	}
	
	void OnApplicationQuit()
	{
		if (connected) GloveDisconnect();
	}
}
