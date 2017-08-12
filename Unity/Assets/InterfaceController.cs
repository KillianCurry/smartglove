using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class InterfaceController:MonoBehaviour
{
	public GameObject glovePrefab;
	
	private List<GameObject> glovePanels;
	private Dictionary<int,GameObject> gloves;
	
	void Start()
	{
		//initialize a list of panels to reference
		glovePanels = new List<GameObject>();
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
		//TODO call a function to return a string of ordered UUIDs from paired gloves
		//basically just copying the internal list from the library
		string UUIDs = "00601001-7374-7265-7563-6873656e7365 00601002-7374-7265-7563-6873656e7365 00601003-7374-7265-7563-6873656e7365";
		
		//iterate through substrings, using space as a delimiter
		
		AddPanel(0, "00601001-7374-7265-7563-6873656e7365");
		AddPanel(1, "00601001-7374-7265-7563-6873656e7365");
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
