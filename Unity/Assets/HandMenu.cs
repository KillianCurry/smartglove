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
	}
}
