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
	//use a dictionary to match glove IDs to corresponding 3D objects
	[HideInInspector]
	public Dictionary<int,GameObject> gloves;
	
	//manages mouse-based camera control
	private DragInterface dragInterface;
	//manages glove connection panels
	private ConnectInterface connectInterface;
	//manages glove pairing blocks
	private PairInterface pairInterface;
	
	//find available gloves in the library
	[DllImport("smartglove", EntryPoint="findGloves", CallingConvention = CallingConvention.Cdecl)]
	public static extern IntPtr findGloves();
	//free a pointer made earlier to avoid memory leaks
	[DllImport("smartglove", EntryPoint="freePointer")]
	public static extern void freePointer(IntPtr ptr);
	//close the library, cleaning up all data structures
	[DllImport("smartglove", EntryPoint="closeLibrary")]
	public static extern void closeLibrary();
	//add a new glove to the library based on a UUID string
	[DllImport("smartglove", EntryPoint="addUUID")]
	public static extern void addUUID(StringBuilder buffer, ref int bufferSize);
	
	void Start()
	{
		//keep track of children interfaces
		MainInterface thisScript = GetComponent<MainInterface>();
		dragInterface = transform.GetChild(0).gameObject.GetComponent<DragInterface>();
		dragInterface.mainInterface = thisScript;
		connectInterface = transform.GetChild(1).gameObject.GetComponent<ConnectInterface>();
		connectInterface.mainInterface = thisScript;
		pairInterface = transform.GetChild(2).gameObject.GetComponent<PairInterface>();
		pairInterface.mainInterface = thisScript;
		
		gloves = new Dictionary<int,GameObject>();
		
		//add new gloves to the library
		string UUID1 = "{00601001-7374-7265-7563-6873656e7365}";
		int bufferSize1 = UUID1.Length;
		StringBuilder buffer1 = new StringBuilder(UUID1, bufferSize1);
		addUUID(buffer1, ref bufferSize1);
		string UUID2 = "{00602001-7374-7265-7563-6873656e7365}";
		int bufferSize2 = UUID2.Length;
		StringBuilder buffer2 = new StringBuilder(UUID2, bufferSize2);
		addUUID(buffer2, ref bufferSize2);
		
		//add corresponding connection panels for the library's gloves
		Populate();
	}
	
	private void Populate()
	{
		//retrieve the UUID list from the library
		IntPtr ptr = findGloves();
		string source = Marshal.PtrToStringAnsi(ptr);
		//TODO figure out why this causes a crash
		//freePointer(ptr);
		//iterate through UUIDs, using space as a delimiter
		string[] delimiters = {" "};
		string[] UUIDs = source.Split(delimiters, StringSplitOptions.RemoveEmptyEntries);
		for (int i = 0; i < UUIDs.Length; i++)
		{
			//add panel corresponding to the library's internal glove object
			connectInterface.AddPanel(i, UUIDs[i]);
		}
	}
	
	//update the orbit center to reflect changes in glove transforms
	public void UpdateOrbit()
	{
		Vector3 center = Vector3.zero;
		foreach (GameObject g in gloves.Values)
		{
			//add the center of the glove (origin is at the wrist, so some geometry is required)
			center += g.transform.localPosition + g.transform.forward * 2.3f * g.transform.localScale.z;
		}
		//average all positions
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
	
	//connect a given glove object
	private void ConnectGlove(int ID)
	{
		gloves[ID].GetComponent<HandController>().GloveConnect();
	}
	
	//clear a given glove object's calibration data
	public void ClearGlove(int ID)
	{
		gloves[ID].GetComponent<HandController>().GloveClear();
	}
	
	//called when quitting the application
	void OnApplicationQuit()
	{
		//disconnect all gloves so reading coroutines end
		foreach (int ID in gloves.Keys)
		{
			gloves[ID].GetComponent<HandController>().GloveDisconnect();
		}
		//clean up the library
		closeLibrary();
	}
}
