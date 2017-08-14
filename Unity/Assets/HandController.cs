using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System.Runtime.InteropServices;
using System.Linq;
using UnityEngine;


public class HandController:MonoBehaviour
{	
	//the height of the hand
	public float palmHeight = 0.25f;
	//the width of the hand, excluding the thumb
	public float palmWidth = 1f;
	//the length of the palm from wrist to finger base
	public float palmLength = 1f;
	//list of each segment's length, for proportions
	private List<float> fingerLengths;
	//list of finger rotation ranges
	private List<float> rotationMinimum;
	private List<float> rotationMaximum;
	//list of each joint
	private List<GameObject> joints;
	
	//TODO roll this calibration into the API
	private Quaternion zeroRotation;
	
	//is there one sensor per finger?
	public bool fiveSensor = false;
	[HideInInspector]
	//-1 is left-handed, 1 is right-handed
	public int handedness = 1;
	
	//list of each joint's rotation
	private List<double> fingerRotations;
	//vector of the x, y, and z rotation
	private Vector3 palmOrientation;
	[Range(-0.1f, 1.1f)]
	public float fingerCurl;
	
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
		connected = openConnection(ID);
		if (connected) StartCoroutine("GloveRead");
		return connected;
	}
	
	public bool GloveDisconnect()
	{
		connected = !closeConnection(ID);
		if (!connected) StopCoroutine("GloveRead");
		return !connected;
	}
	
	public void GloveClear()
	{
		clearCalibration(ID);
	}
	
	void Start()
	{
		//finger proportions
		fingerLengths = new List<float>();
		rotationMinimum = new List<float>();
		rotationMaximum = new List<float>();
		//thumb
		fingerLengths.Add(palmWidth/2f);
		rotationMinimum.Add(0f);
		rotationMaximum.Add(90f);
		
		fingerLengths.Add(palmWidth/2f);
		rotationMinimum.Add(0f);
		rotationMaximum.Add(90f);
		
		fingerLengths.Add(palmWidth/4f);
		rotationMinimum.Add(0f);
		rotationMaximum.Add(90f);
		
		//index
		fingerLengths.Add(0.5f);
		rotationMinimum.Add(-10f);
		rotationMaximum.Add(75f);
		
		fingerLengths.Add(0.3f);
		rotationMinimum.Add(-15f);
		rotationMaximum.Add(110f);
		
		fingerLengths.Add(0.2f);
		rotationMinimum.Add(0f);
		rotationMaximum.Add(80f);
		
		//middle
		fingerLengths.Add(0.6f);
		rotationMinimum.Add(-10f);
		rotationMaximum.Add(75f);
		
		fingerLengths.Add(0.35f);
		rotationMinimum.Add(-15f);
		rotationMaximum.Add(110f);
		
		fingerLengths.Add(0.2f);
		rotationMinimum.Add(0f);
		rotationMaximum.Add(80f);
		
		//ring
		fingerLengths.Add(0.5f);
		rotationMinimum.Add(-10f);
		rotationMaximum.Add(75f);
		
		fingerLengths.Add(0.3f);
		rotationMinimum.Add(-15f);
		rotationMaximum.Add(110f);
		
		fingerLengths.Add(0.2f);
		rotationMinimum.Add(0f);
		rotationMaximum.Add(80f);
		
		//index
		fingerLengths.Add(0.35f);
		rotationMinimum.Add(-10f);
		rotationMaximum.Add(75f);
		
		fingerLengths.Add(0.2f);
		rotationMinimum.Add(-15f);
		rotationMaximum.Add(110f);
		
		fingerLengths.Add(0.15f);
		rotationMinimum.Add(0f);
		rotationMaximum.Add(80f);
		
		GenerateHand();
	}
	
	void Update()
	{
		//update joint angles and palm orientation to match input
		if (connected)
		{
			joints[0].transform.localRotation = Quaternion.Euler(joints[0].transform.localRotation.eulerAngles.x, joints[0].transform.localRotation.eulerAngles.y, handedness*(float)fingerRotations[0]);
			for (int r = 1; r < fingerRotations.Count; r++)
			{
				float spread = 0f;
				if (r % 3 == 0) spread = handedness*((((r/3)-2)*5)*((90f-(float)fingerRotations[r])/90f));
				joints[r].transform.localRotation = Quaternion.Euler((float)fingerRotations[r], spread, 0f);
			}
			this.transform.localRotation = Quaternion.Euler(palmOrientation.x, palmOrientation.y, palmOrientation.z);
		}
		//otherwise let the user control the hand via interface
		else
		{
			fingerRotations[0] = fingerCurl*90f;
			joints[0].transform.localRotation = Quaternion.Euler(joints[0].transform.localRotation.eulerAngles.x, joints[0].transform.localRotation.eulerAngles.y, handedness*(float)fingerRotations[0]);
			for (int r = 1; r < fingerRotations.Count; r++)
			{
				fingerRotations[r] = fingerCurl*90f;
				float spread = 0f;//joints[r].transform.localRotation.eulerAngles.y;
				if (r % 3 == 0) spread = handedness*((((r/3)-2)*5)*((90f-(float)fingerRotations[r])/90f));
				joints[r].transform.localRotation = Quaternion.Euler((float)fingerRotations[r], spread, 0f);
			}
		}
	}
	
	void OnDrawGizmos()
	{
		if (!Application.isPlaying) return;
		Gizmos.color = Color.black;
		foreach (GameObject e in joints)
		{
			Gizmos.DrawSphere(e.transform.position, 0.02f);
		}
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
			palmOrientation = new Vector3((float)data[0],(float)data[1],(float)data[2]);
			
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
	
	void GenerateHand()
	{
		//TODO final pass of hand generation for tidiness
		fingerRotations = new List<double>();
		
		//create the palm geometry, a cube
		GameObject obj = GameObject.CreatePrimitive(PrimitiveType.Cube);
		//name the geometry for easy identification
		obj.name = "palm";
		//scale to match specifications
		obj.transform.localScale = new Vector3(palmWidth, palmHeight, palmLength+0.05f);
		obj.transform.localPosition = new Vector3(0f, 0f, obj.transform.localScale.z/2f);
		//child to the heirarchy root
		obj.transform.SetParent(this.transform, false);
		
		//previous joint transform to use as the parent of the next joint in a finger
		Transform prev;
		
		joints = new List<GameObject>();
		
		//generate first thumb joint
		fingerRotations.Add(0d);
		GameObject empty = new GameObject("joint0_0");
		empty.transform.localPosition = new Vector3(handedness*-palmWidth/4f, palmHeight/2f, 0f);
		empty.transform.localRotation = Quaternion.Euler(0f, handedness*-20f, handedness*45f);
		empty.transform.SetParent(this.transform, false);
		joints.Add(empty);
		prev = empty.transform;
		
		obj = GameObject.CreatePrimitive(PrimitiveType.Cube);
		obj.name = "finger0_0";
		obj.transform.localScale = new Vector3(palmWidth/2f, palmHeight, fingerLengths[0]);
		obj.transform.localPosition = new Vector3(handedness*-palmWidth/4f, -palmHeight/2f, fingerLengths[0]/2f);
		obj.transform.SetParent(empty.transform, false);
		
		//generate rest of thumb
		prev = AddJoint(prev, new Vector3(handedness*-3f*palmWidth/8f, 0f, fingerLengths[0]), 0, 1);
		prev = AddSegment(0, 1, prev);
		AddSegment(0, 2, prev);
		
		//generate fingers
		for (int i = 1; i < 5; i++)
		{
			prev = AddJoint(this.transform, new Vector3(handedness*(((float)(i-1) * (palmWidth/4f)) - ((palmWidth / 2f) - (palmWidth / 8f))), 0f, palmLength), i, 0);
			for (int j = 0; j < 3; j++)
			{
				prev = AddSegment(i, j, prev);
			}
		}
	}
	
	Transform AddJoint(Transform parent, Vector3 position, int finger, int joint)
	{
		fingerRotations.Add((joint+1)*3);
		GameObject empty = new GameObject("joint" + finger.ToString() + "_" + joint.ToString());
		empty.transform.SetParent(parent, false);
		if (joint == 0) position += Vector3.up*palmHeight/2f;
		empty.transform.localPosition = position;
		float spread = 0f;
		if (joint == 0) spread = handedness*((finger-2)*5f);
		empty.transform.localRotation = Quaternion.Euler((float)fingerRotations.Last(), spread, 0f);
		joints.Add(empty);
		return empty.transform;
	}
	
	Transform AddSegment(int finger, int joint, Transform previous)
	{
		//create string suffix for naming convention
		float length = fingerLengths[finger*3+joint];
		
		GameObject geometry = GameObject.CreatePrimitive(PrimitiveType.Cube);
		geometry.name = "finger" + finger.ToString() + "_" + joint.ToString();
		geometry.transform.localScale = new Vector3(palmWidth/4f, palmHeight, length);
		geometry.transform.SetParent(previous, false);
		geometry.transform.localPosition = new Vector3(0f, -palmHeight/2f, length/2f);
		
		if (joint == 2) return null;
		
		return AddJoint(previous, new Vector3(0f, 0f, length), finger, joint+1);
	}
	
	void OnApplicationQuit()
	{
		GloveDisconnect();
	}
}
