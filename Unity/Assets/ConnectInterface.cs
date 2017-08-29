using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

public class ConnectInterface:MonoBehaviour
{
	[HideInInspector]
	public MainInterface mainInterface;
	public GameObject glovePrefab;
	
	private List<GameObject> glovePanels;

	void Start()
	{
		//initialize a list of panels to reference
		glovePanels = new List<GameObject>();
	}
	
	//add a new glove 
	public void AddPanel(int ID, string UUID)
	{
		GameObject panel = Instantiate(glovePrefab, transform);
		panel.transform.localPosition += Vector3.up * (45f * glovePanels.Count);
		panel.name = panel.transform.GetChild(0).GetComponent<Text>().text = "Glove " + glovePanels.Count.ToString();
		EventTrigger trigger = (EventTrigger)panel.AddComponent(typeof(EventTrigger));
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[0].eventID = EventTriggerType.PointerEnter;
		trigger.triggers[0].callback.AddListener(delegate { EnterPanel(ID); });
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[1].eventID = EventTriggerType.PointerExit;
		trigger.triggers[1].callback.AddListener(delegate { ExitPanel(); });
		panel.transform.GetChild(1).GetComponent<Text>().text = UUID;
		panel.transform.GetChild(2).GetComponent<Button>().onClick.AddListener(delegate { mainInterface.AddGlove(ID); });
		panel.transform.GetChild(3).GetComponent<Button>().onClick.AddListener(delegate { mainInterface.ClearGlove(ID); });
		glovePanels.Add(panel);
	}
	
	private void EnterPanel(int ID)
	{
		if (mainInterface.gloves.ContainsKey(ID)) Camera.main.GetComponent<HighlightEffect>().highlightObject = mainInterface.gloves[ID];
	}
	
	private void ExitPanel()
	{
		Camera.main.GetComponent<HighlightEffect>().highlightObject = null;
	}
}
