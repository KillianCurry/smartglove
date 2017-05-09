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
	
	//is there one sensor per finger?
	public bool fiveSensor = false;
	//is this a right hand?
	public bool isRightHand = true;
	
	//list of each joint's rotation
	public List<double> fingerRotations;
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
	
	//TODO support for multiple tinyTILES
	
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
		
		GenerateHand(isRightHand);
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
			
			//TODO more robust matching of sensors to finger representation
			//loop through the calibrated finger rotation data
			int i = 3;
			for (int v = 3; v < 13; v++)
			{
				//for the first joint, take the 0-1 value as 0-90 degrees
				if (i % 3 == 0)
				{
					fingerRotations[i-3] = data[v]*90f;
					if (fiveSensor) fingerRotations[i-2] = fingerRotations[i-1] = data[v]*90f;
					i += 1;
				}
				//for the second joint, split the 0-1 value across the second and third joint
				else
				{
					if (!fiveSensor) fingerRotations[i-3] = fingerRotations[i-2] = data[v]*45f;
					i += 2;
				}
			}
			
			//DEV return null means this will be called again next frame
			//DEV return new WaitForSeconds(t) means this will wait t seconds before being called again
			yield return null;//new WaitForSeconds(0.1f);
		}
	}
	
	void GenerateHand(bool right)
	{
		//TODO final pass of hand generation for tidiness
		int sign = right ? 1 : -1;
		fingerRotations = new List<double>();
		
		//create the palm geometry, a cube
		GameObject obj = GameObject.CreatePrimitive(PrimitiveType.Cube);
		//name the geometry for easy identification
		obj.name = "palm";
		//scale to match specifications
		obj.transform.localScale = new Vector3(palmWidth, palmHeight, palmLength);
		//child to the heirarchy root
		obj.transform.SetParent(this.transform, false);
		
		//previous joint transform to use as the parent of the next joint in a finger
		Transform prev;
		
		joints = new List<GameObject>();
		
		//generate first thumb joint
		fingerRotations.Add(0d);
		GameObject empty = new GameObject("joint0_0");
		empty.transform.localPosition = new Vector3(sign*-palmWidth/4f, 0f, -fingerLengths[0]);
		empty.transform.localRotation = Quaternion.Euler(0f, -45f, 0f);
		empty.transform.SetParent(this.transform, false);
		joints.Add(empty);
		prev = empty.transform;
		
		obj = GameObject.CreatePrimitive(PrimitiveType.Cube);
		obj.name = "finger0_0";
		obj.transform.localScale = new Vector3(palmWidth/2f, palmHeight, fingerLengths[0]);
		obj.transform.localPosition = new Vector3(sign*-palmWidth/4f, 0f, fingerLengths[0]/2f);
		obj.transform.SetParent(empty.transform, false);
		
		//generate rest of thumb
		prev = AddJoint(prev, new Vector3(sign*-3f*palmWidth/8f, 0f, fingerLengths[0]), 0, 1);
		prev = AddSegment(0, 1, prev);
		AddSegment(0, 2, prev);
		
		//generate fingers
		for (int i = 1; i < 5; i++)
		{
			prev = AddJoint(this.transform, new Vector3(sign*(((float)(i-1) * (palmWidth/4f)) - ((palmWidth / 2f) - (palmWidth / 8f))), 0f, palmLength / 2f), i, 0);
			for (int j = 0; j < 3; j++)
			{
				prev = AddSegment(i, j, prev);
			}
		}
	}
	
	Transform AddJoint(Transform parent, Vector3 position, int finger, int joint)
	{
		fingerRotations.Add(0d);
		GameObject empty = new GameObject("joint" + finger.ToString() + "_" + joint.ToString());
		empty.transform.SetParent(parent, false);
		empty.transform.localPosition = position;
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
		geometry.transform.localPosition = new Vector3(0f, 0f, length/2f);
		
		if (joint == 2) return null;
		
		return AddJoint(previous, new Vector3(0f, 0f, length), finger, joint+1);
	}
	
	void OnApplicationQuit()
	{
		SerialDisconnect();
	}
}
