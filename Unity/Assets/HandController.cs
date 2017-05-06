using System;
using System.Collections;
using System.Collections.Generic;
//DEV to use System.IO.Ports, Unity needs to be forced to use all of NET 2.0
//DEV go to Edit -> Project Settings -> Player
//DEV change API Compatibility Level at the bottom
using System.IO.Ports;
using System.Threading;
using System.Runtime.InteropServices;
using UnityEngine;


public class HandController:MonoBehaviour
{
	//serialport object to get data from the arduino
	SerialPort serial;
	
	//the height of the hand
	public float palmHeight;
	//the width of the hand, excluding the thumb
	public float palmWidth;
	//list of each segment's length, for proportions
	private List<float> fingerLengths;
	//list of each joint
	private List<GameObject> joints;
	//how many segments in a finger?
	private int fingerSegments = 3;
	
	//list of each joint's rotation
	private List<double> fingerRotations;
	
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
	public static extern void calibrateMaximum();
	
	
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
	
	public void CalibrateMaximum()
	{
		calibrateMaximum();
	}
	
	void Start()
	{
		//finger proportions
		fingerLengths = new List<float>();
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
		//update joint angles to match input
		for (int r = 0; r < fingerRotations.Count; r++)
		{
			joints[r].transform.localRotation = Quaternion.Euler((float)fingerRotations[r], 0f, 0f);
		}
	}
	
	//coroutine to read data from arduino into fingerRotations, avoiding lag from waiting to read serial
	IEnumerator ReadSerial()
	{
		while(true)
		{
			//copy data from the DLL's unmanaged memory into a managed array
			double[] data = new double[10];
			IntPtr ptr = readPort();
			Marshal.Copy(ptr, data, 0, 10);
			
			//loop through the calibrated data
			int i = 0;
			foreach (double v in data)
			{
				//currently support is only available for joints 0, 1, 3, 4, 6, 7, 9, and 10.
				if (i >= 12) continue;
				//for the first joint, take the 0-1 value as 0-90 degrees
				if (i % 3 == 0)
				{
					fingerRotations[i] = v*90f;
					i += 1;
				}
				//for the second joint, split the 0-1 value across the second and third joint
				else
				{
					fingerRotations[i] = fingerRotations[i+1] = v*45f;
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
		obj.transform.localScale = new Vector3(palmWidth, palmHeight, 1f);
		//child to the heirarchy root
		obj.transform.parent = this.transform;
		
		//gameobject to store the joint
		GameObject empty;
		//previous joint transform to use as the parent of the next joint in a finger
		Transform prev;
		//how long a finger segment should be
		float length;
		
		//number of fingers on the hand based on how many segments are given and how many segments are in a finger
		int fingers = fingerLengths.Count / fingerSegments;
		//the width of a single finger
		float divWidth = palmWidth / (float)fingers;
		
		joints = new List<GameObject>();
		
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
				length = fingerLengths[i*3+j];
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
