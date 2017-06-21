using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

[CustomEditor(typeof(HandController))]
public class HandEditor:Editor
{
	bool connected = false;
	bool minCalibrated = false;
	
	//handedness selection
	string[] handednessNames = {"Left", "Right"};
	int[] handednessSigns = {-1, 1};
	
	public override void OnInspectorGUI()
	{
		DrawDefaultInspector();
		
		HandController hand = (HandController)target;
		
		hand.handedness = EditorGUILayout.IntPopup("Handedness: ", hand.handedness, handednessNames, handednessSigns);
		
		GUI.enabled = Application.isPlaying;
		
		EditorGUILayout.Space();
		
		if (GUILayout.Button("Connect Serial"))
		{
			bool opened = hand.SerialConnect();
			if (opened)
			{
				Debug.Log("Serial port opened.");
				connected = true;
			}
			connected = true;
		}
		
		GUI.enabled = Application.isPlaying && connected;
		
		if (GUILayout.Button("Disconnect Serial"))
		{
			bool closed = hand.SerialDisconnect();
			if (closed) Debug.Log("Serial port closed.");
		}
		
		EditorGUILayout.Space();
		
		if (GUILayout.Button("Calibrate Minimum"))
		{
			hand.CalibrateMinimum();
			minCalibrated = true;
		}
		
		GUI.enabled = Application.isPlaying && connected && minCalibrated;
		
		if (GUILayout.Button("Calibrate Maximum")) hand.CalibrateMaximum(-1);
		GUILayout.BeginHorizontal();
		if (GUILayout.Button("1")) hand.CalibrateMaximum(0);
		if (GUILayout.Button("2")) hand.CalibrateMaximum(1);
		if (GUILayout.Button("3")) hand.CalibrateMaximum(2);
		if (GUILayout.Button("4")) hand.CalibrateMaximum(3);
		if (GUILayout.Button("5")) hand.CalibrateMaximum(4);
		if (GUILayout.Button("6")) hand.CalibrateMaximum(5);
		if (GUILayout.Button("7")) hand.CalibrateMaximum(6);
		if (GUILayout.Button("8")) hand.CalibrateMaximum(7);
		if (GUILayout.Button("9")) hand.CalibrateMaximum(8);
		if (GUILayout.Button("10")) hand.CalibrateMaximum(9);
		GUILayout.EndHorizontal();
	}
	
	void OnInspectorUpdate()
	{
		this.Repaint();
	}
}
