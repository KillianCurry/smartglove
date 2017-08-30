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
		trigger.triggers[0].callback.AddListener(delegate { EnterPanel(ID); });
		//when exiting panel with mouse, remove highlight
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[1].eventID = EventTriggerType.PointerExit;
		trigger.triggers[1].callback.AddListener(delegate { ExitPanel(); });
		panel.transform.GetChild(1).GetComponent<Text>().text = UUID;
		//set up add and clear buttons
		panel.transform.GetChild(2).GetComponent<Button>().onClick.AddListener(delegate { mainInterface.AddGlove(ID); });
		panel.transform.GetChild(3).GetComponent<Button>().onClick.AddListener(delegate { mainInterface.ClearGlove(ID); });
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
