﻿using System;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

public class ConnectInterface:MonoBehaviour
{
	[HideInInspector]
	public MainInterface mainInterface;
	
	public void AddPanel(int ID, string UUID)
	{
		//instantiate a new panel from the prefab
		GameObject panel = Instantiate((GameObject)Resources.Load("prefabs/Glove Panel", typeof(GameObject)), transform);
		//position the panel in the list (stack from bottom to top)
		panel.transform.localPosition += Vector3.up * (45f * (transform.childCount - 2));
		panel.name = panel.transform.GetChild(0).GetComponent<Text>().text = "Glove " + ID.ToString();
		
		//set up event system
		EventTrigger trigger = (EventTrigger)panel.AddComponent(typeof(EventTrigger));
		//when hovering mouse over panel, highlight corresponding glove
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[0].eventID = EventTriggerType.PointerEnter;
		trigger.triggers[0].callback.AddListener(state => EnterPanel(ID));
		//when exiting panel with mouse, remove highlight
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[1].eventID = EventTriggerType.PointerExit;
		trigger.triggers[1].callback.AddListener(state => ExitPanel());
		panel.transform.GetChild(1).GetComponent<Text>().text = UUID;
        //set up buttons
        Button addButton = panel.transform.GetChild(2).GetComponent<Button>();
        addButton.onClick.AddListener(() => AddGlove(ID, addButton));
        Button connectButton = panel.transform.GetChild(3).GetComponent<Button>();
        connectButton.onClick.AddListener(() => ConnectGlove(ID, connectButton));
        Button clearButton = panel.transform.GetChild(4).GetComponent<Button>();
        clearButton.onClick.AddListener(() => mainInterface.ClearGlove(ID));
	}

    private void AddGlove(int ID, Button thisButton)
    {
        //treat the button as a toggle based on its current text
        Text thisText = thisButton.transform.GetChild(0).GetComponent<Text>();
        if (thisText.text == "Add")
        {
            thisText.text = "Remove";
            mainInterface.AddGlove(ID);
            //enable the connect button
            thisButton.transform.parent.GetChild(3).gameObject.GetComponent<Button>().interactable = true;
        }
        else
        {
            thisText.text = "Add";
            mainInterface.RemoveGlove(ID);
            //disable other buttons and disconnect the glove if it's connected
            thisButton.transform.parent.GetChild(3).GetChild(0).gameObject.GetComponent<Text>().text = "Connect";
            thisButton.transform.parent.GetChild(3).gameObject.GetComponent<Button>().interactable = false;
            thisButton.transform.parent.GetChild(4).gameObject.GetComponent<Button>().interactable = false;
        }
    }

    private void ConnectGlove(int ID, Button thisButton)
    {
        Text thisText = thisButton.transform.GetChild(0).GetComponent<Text>();
        if (thisText.text == "Connect")
        {
            thisText.text = "Disconnect";
            mainInterface.ConnectGlove(ID);
            thisButton.transform.parent.GetChild(4).gameObject.GetComponent<Button>().interactable = true;
        }
        else
        {
            thisText.text = "Connect";
            mainInterface.DisconnectGlove(ID);
            thisButton.transform.parent.GetChild(4).gameObject.GetComponent<Button>().interactable = false;
        }
    }
	
	private void EnterPanel(int ID)
	{
		//if this panel's glove has been added, highlight it in the 3D view
		if (mainInterface.gloves.ContainsKey(ID)) Camera.main.GetComponent<HighlightEffect>().highlightObject = mainInterface.gloves[ID];
	}
	
	private void ExitPanel()
	{
		//remove the highlight effect from the glove
		Camera.main.GetComponent<HighlightEffect>().highlightObject = null;
	}
}