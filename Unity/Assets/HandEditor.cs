using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

[CustomEditor(typeof(HandController))]
public class HandEditor:Editor
{
	public override void OnInspectorGUI()
	{
		DrawDefaultInspector();
		
		HandController hand = (HandController)target;
		if (GUILayout.Button("Connect Serial"))
		{
			bool opened = hand.SerialConnect();
			if (opened) Debug.Log("Serial port opened.");
		}
		
		if (GUILayout.Button("Disconnect Serial"))
		{
			bool closed = hand.SerialDisconnect();
			if (closed) Debug.Log("Serial port closed.");
		}
		
		if (GUILayout.Button("Calibrate Minimum"))
		{
			hand.CalibrateMinimum();
		}
		
		if (GUILayout.Button("Calibrate Maximum"))
		{
			hand.CalibrateMaximum();
		}
	}
}
