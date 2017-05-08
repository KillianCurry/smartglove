using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using System.Runtime.InteropServices;
using UnityEngine;


public class HandController:MonoBehaviour
{	
	//the height of the hand
	public float palmHeight;
	//the width of the hand, excluding the thumb
	public float palmWidth;
	//the length of the palm from wrist to finger base
	public float palmLength;
	//list of each segment's length, for proportions
	private List<float> fingerLengths;
	//list of each joint
	private List<GameObject> joints;
	//how many segments in a finger?
	private int fingerSegments = 3;
	
	//list of each joint's rotation
	private List<double> fingerRotations;
	//vector of the x, y, and z rotation
	private Vector3 palmOrientation;
	
	//what com port is the arduino connected to?
	public int portNumber = 0;
	//what's the baud rate?
	public int rate = 9600;
	
	//import functions from the DLL
	[DllImport("smartglove", EntryPoint="openPort")]
	public static extern bool openPort(int portNum);
	[DllImport("smartglove", EntryPoint="closePort")]
	public static extern bool closePort();
	[DllImport("smartglove", EntryPoint="getData", CallingConvention = CallingConvention.Cdecl)]
	public static extern IntPtr readPort();
	[DllImport("smartglove", EntryPoint="releaseLine", CallingConvention = CallingConvention.Cdecl)]
	public static extern int releaseLine(IntPtr ptr);
	[DllImport("smartglove", EntryPoint="calibrateMinimum")]
	public static extern void calibrateMinimum();
	[DllImport("smartglove", EntryPoint="calibrateMaximum")]
	public static extern void calibrateMaximum(int sensor);
	
	
	public bool SerialConnect()
	{
		bool opened = openPort(portNumber);
		if (opened) StartCoroutine("ReadSerial");
		return opened;
	}
	
	public bool SerialDisconnect()
	{
		bool closed = closePort();
		if (closed) StopCoroutine("ReadSerial");
		return closed;
	}
	
	public void CalibrateMinimum()
	{
		calibrateMinimum();
	}
	
	public void CalibrateMaximum(int sensor)
	{
		calibrateMaximum(sensor);
	}
	
	void Start()
	{
		//finger proportions
		fingerLengths = new List<float>();
		fingerLengths.Add(palmWidth/2f);
		fingerLengths.Add(palmWidth/2f);
		fingerLengths.Add(palmWidth/4f);
		fingerLengths.Add(0.5f);
		fingerLengths.Add(0.3f);
		fingerLengths.Add(0.2f);
		fingerLengths.Add(0.6f);
		fingerLengths.Add(0.35f);
		fingerLengths.Add(0.2f);
		fingerLengths.Add(0.5f);
		fingerLengths.Add(0.3f);
		fingerLengths.Add(0.2f);
		fingerLengths.Add(0.35f);
		fingerLengths.Add(0.2f);
		fingerLengths.Add(0.15f);
		
		
		GenerateGeometry();
	}
	
	void Update()
	{
		//update joint angles and palm orientation to match input
		joints[0].transform.localRotation = Quaternion.Euler(0f, 0f, (float)fingerRotations[0]);
		for (int r = 1; r < fingerRotations.Count; r++)
		{
			joints[r].transform.localRotation = Quaternion.Euler((float)fingerRotations[r], 0f, 0f);
		}
		this.transform.localRotation = Quaternion.Euler(palmOrientation.x, palmOrientation.y, palmOrientation.z);
	}
	
	//coroutine to read data from arduino into fingerRotations, avoiding lag from waiting to read serial
	IEnumerator ReadSerial()
	{
		while(true)
		{
			//copy data from the DLL's unmanaged memory into a managed array
			double[] data = new double[13];
			IntPtr ptr = readPort();
			Marshal.Copy(ptr, data, 0, 13);
			
			//copy the orientation data (y and z negated to remove mirroring)
			palmOrientation = new Vector3((float)data[1], -(float)data[0], -(float)data[2]);
			
			//loop through the calibrated finger rotation data
			int i = 3;
			for (int v = 3; v < 13; v++)
			{
				//for the first joint, take the 0-1 value as 0-90 degrees
				if (i % 3 == 0)
				{
					fingerRotations[i-3] = data[v]*90f;
					i += 1;
				}
				//for the second joint, split the 0-1 value across the second and third joint
				else
				{
					fingerRotations[i-3] = fingerRotations[i-2] = data[v]*45f;
					i += 2;
				}
			}
			
			//DEV return null means this will be called again next frame
			//DEV return new WaitForSeconds(t) means this will wait t seconds before being called again
			yield return null;//new WaitForSeconds(0.1f);
		}
	}
	
	void GenerateGeometry()
	{
		fingerRotations = new List<double>();
		
		//create the palm geometry, a cube
		GameObject obj = GameObject.CreatePrimitive(PrimitiveType.Cube);
		//name the geometry for easy identification
		obj.name = "palm";
		//scale to match specifications
		obj.transform.localScale = new Vector3(palmWidth, palmHeight, palmLength);
		//child to the heirarchy root
		obj.transform.parent = this.transform;
		
		//gameobject to store the joint
		GameObject empty;
		//previous joint transform to use as the parent of the next joint in a finger
		Transform prev;
		//how long a finger segment should be
		float length;
		
		//number of fingers on the hand based on how many segments are given and how many segments are in a finger
		int fingers = fingerLengths.Count / fingerSegments - 1;
		//the width of a single finger
		float divWidth = palmWidth / (float)fingers;
		
		joints = new List<GameObject>();
		
		//TODO roll thumb generation into main loop with exception for 0,0
		//generate thumb geometry
		//first joint
		fingerRotations.Add(0d);
		empty = new GameObject("joint0_0");
		empty.transform.parent = this.transform;
		empty.transform.localPosition = new Vector3(0f, 0f, -palmLength/2f);
		joints.Add(empty);
		prev = empty.transform;
		//first geometry
		obj = GameObject.CreatePrimitive(PrimitiveType.Cube);
		obj.name = "finger0_0";
		obj.transform.localScale = new Vector3(palmWidth/2f, palmHeight, fingerLengths[0]);
		obj.transform.parent = empty.transform;
		obj.transform.localPosition = new Vector3(-palmWidth/2f, 0f, fingerLengths[0]/2f);
		//second joint
		fingerRotations.Add(0d);
		empty = new GameObject("joint0_1");
		empty.transform.parent = prev;
		empty.transform.localPosition = new Vector3(-5f*palmWidth/8f, 0f, fingerLengths[0]/2f);
		joints.Add(empty);
		prev = empty.transform;
		//second geometry
		obj = GameObject.CreatePrimitive(PrimitiveType.Cube);
		obj.name = "finger0_1";
		obj.transform.localScale = new Vector3(palmWidth/4f, palmHeight, fingerLengths[1]);
		obj.transform.parent = empty.transform;
		obj.transform.localPosition = new Vector3(0f, 0f, fingerLengths[1]/2f);
		//third joint
		fingerRotations.Add(0d);
		empty = new GameObject("joint0_2");
		empty.transform.parent = prev;
		empty.transform.localPosition = new Vector3(0f, 0f, fingerLengths[1]);
		joints.Add(empty);
		//third geometry
		obj = GameObject.CreatePrimitive(PrimitiveType.Cube);
		obj.name = "finger0_2";
		obj.transform.localScale = new Vector3(palmWidth/4f, palmHeight, fingerLengths[2]);
		obj.transform.parent = empty.transform;
		obj.transform.localPosition = new Vector3(0f, 0f, fingerLengths[2]/2f);
		
		//loop for fingers
		for (int i = 0; i < fingers; i++)
		{
			prev = this.transform;
			//create empty finger base joint for rotation
			fingerRotations.Add(0d);
			empty = new GameObject("joint" + (i+1).ToString() + "_" + "0");
			empty.transform.parent = this.transform;
			//position finger base
			empty.transform.localPosition = new Vector3(((float)i * divWidth) - ((palmWidth / 2f) - (divWidth / 2f)), 0f, 0.5f);
			//add to joints list for future manipulation
			joints.Add(empty);
			//parent the next joint to this
			prev = empty.transform;
			//loop for segments
			for (int j = 0; j < fingerSegments; j++)
			{
				//finger_segment to distinguish joints in the named hierarchy
				string suffix = (i+1).ToString() + "_" + j.ToString();
				//create finger geometry to child to the joint
				length = fingerLengths[(i+1)*3+j];
				obj = GameObject.CreatePrimitive(PrimitiveType.Cube);
				//name the geometry so it's easily identifiable in the inspector
				obj.name = "finger" + suffix;
				//size the geometry based on given parameters
				obj.transform.localScale = new Vector3(divWidth, palmHeight, length);
				//child to the joint
				obj.transform.parent = empty.transform;
				//position end at joint so it rotates around the joint
				obj.transform.localPosition = new Vector3(0f, 0f, length/2f);
				
				//create next joint
				if (j < fingerSegments-1)
				{
					empty = new GameObject("joint" + suffix);
					empty.transform.parent = prev;
					empty.transform.localPosition = new Vector3(0f, 0f, length);
					joints.Add(empty);
					prev = empty.transform;
					fingerRotations.Add(0d);
				}
			}
		}
		
	}
	
	void OnApplicationQuit()
	{
		SerialDisconnect();
	}
}
