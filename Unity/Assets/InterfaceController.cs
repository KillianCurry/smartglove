using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using UnityEngine;
using UnityEngine.UI;

public class InterfaceController:MonoBehaviour
{
	public GameObject glovePrefab;
	
	private List<GameObject> glovePanels;
	private Dictionary<int,GameObject> gloves;
	
	[DllImport("smartglove", EntryPoint="findGloves", CallingConvention = CallingConvention.Cdecl)]
	public static extern IntPtr findGloves();
	
	void Start()
	{
		//initialize a list of panels to reference
		glovePanels = new List<GameObject>();
		//use a dictionary to refer to gloves
		gloves = new Dictionary<int,GameObject>();
		Populate();
	}
	
	//add a new glove 
	void AddPanel(int ID, string UUID)
	{
		GameObject panel = Instantiate(glovePrefab, transform.GetChild(0));
		panel.transform.localPosition += Vector3.up * (45f * glovePanels.Count);
		panel.name = panel.transform.GetChild(0).GetComponent<Text>().text = "Glove " + glovePanels.Count.ToString();
		//TODO retrieve UUID from library's glove struct
		panel.transform.GetChild(1).GetComponent<Text>().text = UUID;
		panel.transform.GetChild(2).GetComponent<Button>().onClick.AddListener(delegate { AddGlove(ID); });
		panel.transform.GetChild(3).GetComponent<Button>().onClick.AddListener(delegate { ClearGlove(ID); });
		glovePanels.Add(panel);
	}
	
	void Populate()
	{
		//retrieve the UUID list from the library
		IntPtr ptr = findGloves();
		string source = Marshal.PtrToStringAnsi(ptr);
		Debug.Log(source);
		//iterate through UUIDs, using space as a delimiter
		string[] delimiters = {" "};
		string[] UUIDs = source.Split(delimiters, StringSplitOptions.RemoveEmptyEntries);
		//add panels corresponding to the library's internal glove objects
		for (int i = 0; i < UUIDs.Length; i++)
		{
			AddPanel(i, UUIDs[i]);
		}
	}
	
	void AddGlove(int ID)
	{
		//create a new hand object with the ID
		GameObject newGlove = new GameObject();
		newGlove.name = "Glove " + ID.ToString();
		HandController gloveScript = (HandController)newGlove.AddComponent(typeof(HandController));
		gloveScript.ID = ID;
		gloves.Add(ID, newGlove);
		
		//change 'add' button to 'connect' button
		transform.GetChild(0).GetChild(1+ID).GetChild(2).GetChild(0).GetComponent<Text>().text = "Connect";
		transform.GetChild(0).GetChild(1+ID).GetChild(2).GetComponent<Button>().onClick.RemoveAllListeners();
		transform.GetChild(0).GetChild(1+ID).GetChild(2).GetComponent<Button>().onClick.AddListener(delegate { ConnectGlove(ID); });
	}
	
	void ConnectGlove(int ID)
	{
		gloves[ID].GetComponent<HandController>().GloveConnect();
	}
	
	
	void ClearGlove(int ID)
	{
		gloves[ID].GetComponent<HandController>().GloveClear();
	}
}
