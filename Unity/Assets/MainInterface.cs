using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

public class MainInterface:MonoBehaviour
{
	[HideInInspector]
	public Dictionary<int,GameObject> gloves;
	
	private DragInterface dragInterface;
	private ConnectInterface connectInterface;
	private PairInterface pairInterface;
	
	[DllImport("smartglove", EntryPoint="findGloves", CallingConvention = CallingConvention.Cdecl)]
	public static extern IntPtr findGloves();
	[DllImport("smartglove", EntryPoint="freePointer")]
	public static extern void freePointer(IntPtr ptr);
	[DllImport("smartglove", EntryPoint="closeLibrary")]
	public static extern void closeLibrary();
	[DllImport("smartglove", EntryPoint="addUUID")]
	public static extern void addUUID(StringBuilder buffer, ref int bufferSize);
	
	void Start()
	{
		MainInterface thisScript = GetComponent<MainInterface>();
		dragInterface = transform.GetChild(0).gameObject.GetComponent<DragInterface>();
		dragInterface.mainInterface = thisScript;
		connectInterface = transform.GetChild(1).gameObject.GetComponent<ConnectInterface>();
		connectInterface.mainInterface = thisScript;
		pairInterface = transform.GetChild(2).gameObject.GetComponent<PairInterface>();
		pairInterface.mainInterface = thisScript;
		
		//use a dictionary to refer to gloves
		gloves = new Dictionary<int,GameObject>();
		
		string UUID1 = "{00601001-7374-7265-7563-6873656e7365}";
		int bufferSize1 = UUID1.Length;
		StringBuilder buffer1 = new StringBuilder(UUID1, bufferSize1);
		addUUID(buffer1, ref bufferSize1);
		string UUID2 = "{00602001-7374-7265-7563-6873656e7365}";
		int bufferSize2 = UUID2.Length;
		StringBuilder buffer2 = new StringBuilder(UUID2, bufferSize2);
		addUUID(buffer2, ref bufferSize2);
		
		Populate();
	}
	
	void Populate()
	{
		//retrieve the UUID list from the library
		IntPtr ptr = findGloves();
		string source = Marshal.PtrToStringAnsi(ptr);
		//freePointer(ptr);
		//iterate through UUIDs, using space as a delimiter
		string[] delimiters = {" "};
		string[] UUIDs = source.Split(delimiters, StringSplitOptions.RemoveEmptyEntries);
		//add panels corresponding to the library's internal glove objects
		for (int i = 0; i < UUIDs.Length; i++)
		{
			connectInterface.AddPanel(i, UUIDs[i]);
		}
	}
	
	//update the orbit center to reflect changes in glove transforms
	public void UpdateOrbit()
	{
		Vector3 center = Vector3.zero;
		foreach (GameObject g in gloves.Values)
		{
			center += g.transform.localPosition + g.transform.forward * 2.3f * g.transform.localScale.z;
		}
		center /= gloves.Count;
		dragInterface.target = center;
	}
	
	public void AddGlove(int ID)
	{
		//create a new hand object with the ID
		GameObject newGlove = new GameObject();
		newGlove.name = "Glove " + ID.ToString();
		HandController gloveScript = (HandController)newGlove.AddComponent(typeof(HandController));
		gloveScript.ID = ID;
		gloves.Add(ID, newGlove);
		
		//change 'add' button to 'connect' button
		connectInterface.transform.GetChild(1+ID).GetChild(2).GetChild(0).GetComponent<Text>().text = "Connect";
		connectInterface.transform.GetChild(1+ID).GetChild(2).GetComponent<Button>().onClick.RemoveAllListeners();
		connectInterface.transform.GetChild(1+ID).GetChild(2).GetComponent<Button>().onClick.AddListener(delegate { ConnectGlove(ID); });
		
		//add pairing block
		pairInterface.AddPairBlock(ID);
		
		//highlight new glove
		Camera.main.GetComponent<HighlightEffect>().highlightObject = gloves[ID];
		
		//update the orbit center
		UpdateOrbit();
	}
	
	void ConnectGlove(int ID)
	{
		gloves[ID].GetComponent<HandController>().GloveConnect();
	}
	
	public void ClearGlove(int ID)
	{
		gloves[ID].GetComponent<HandController>().GloveClear();
	}
	
	void OnApplicationQuit()
	{
		closeLibrary();
	}
}
