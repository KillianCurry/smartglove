using System.Collections;
using System.Collections.Generic;
using UnityEngine;

//use immediate GUI to draw a hacky menu for debugging in xcode

public class HandMenu:MonoBehaviour
{
	public HandController hand;
	
	private bool connected = false;
	private bool minCalibrated = false;

	void OnGUI()
	{
		
		if (GUILayout.Button("Connect"))
		{
			bool opened = hand.GloveConnect();
			if (opened)
			{
				connected = true;
			}
			connected = true;
		}
		
		GUI.enabled = Application.isPlaying && connected;
		
		if (GUILayout.Button("Disconnect"))
		{
			bool closed = hand.GloveConnect();
		}
		
		
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
}
