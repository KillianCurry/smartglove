using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

public class InterfaceController:MonoBehaviour
{
	public GameObject glovePrefab;
	
	private List<GameObject> glovePanels;
	private Dictionary<int,GameObject> gloves;
	private List<GameObject> leftPairSlots;
	private List<GameObject> rightPairSlots;
	private List<Vector3> glovePositions;
	private int highlightedGlove;
	private Vector3 dragStart;
	private Transform originalParent;
	
	[DllImport("smartglove", EntryPoint="findGloves", CallingConvention = CallingConvention.Cdecl)]
	public static extern IntPtr findGloves();
	[DllImport("smartglove", EntryPoint="freePointer")]
	public static extern void freePointer(IntPtr ptr);
	[DllImport("smartglove", EntryPoint="closeLibrary")]
	public static extern void closeLibrary();
	
	void Start()
	{
		highlightedGlove = -1;
		//initialize a list of panels to reference
		glovePanels = new List<GameObject>();
		//use a dictionary to refer to gloves
		gloves = new Dictionary<int,GameObject>();
		//use two lists to match glove pairs
		leftPairSlots = new List<GameObject>();
		rightPairSlots = new List<GameObject>();
		glovePositions = new List<Vector3>();
		glovePositions.Add(Vector3.zero);
		glovePositions.Add(Vector3.up * 1f);
		glovePositions.Add(Vector3.up * 2f);
		Populate();
		AddPair();
		AddPair();
		AddPair();
	}
	
	//add a new glove 
	void AddPanel(int ID, string UUID)
	{
		GameObject panel = Instantiate(glovePrefab, transform.GetChild(0));
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
		panel.transform.GetChild(2).GetComponent<Button>().onClick.AddListener(delegate { AddGlove(ID); });
		panel.transform.GetChild(3).GetComponent<Button>().onClick.AddListener(delegate { ClearGlove(ID); });
		glovePanels.Add(panel);
	}
	
	void EnterPanel(int ID)
	{
		if (gloves.ContainsKey(ID)) Camera.main.GetComponent<HighlightEffect>().highlightObject = gloves[ID];
		highlightedGlove = ID;
	}
	
	void ExitPanel()
	{
		Camera.main.GetComponent<HighlightEffect>().highlightObject = null;
		highlightedGlove = -1;
	}
	
	void Populate()
	{
		//retrieve the UUID list from the library
		IntPtr ptr = findGloves();
		string source = Marshal.PtrToStringAnsi(ptr);
		freePointer(ptr);
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
		
		//add pairing block
		AddPairBlock(ID);
		
		//highlight new glove
		EnterPanel(ID);
	}
	
	void AddPairBlock(int ID)
	{
		GameObject pairSlot = FindSlot();
		GameObject pairBlock = new GameObject();
		RectTransform pairBlockRect = (RectTransform)pairBlock.AddComponent(typeof(RectTransform));
		pairBlockRect.sizeDelta = new Vector2(18f,18f);
		pairBlock.AddComponent(typeof(CanvasRenderer));
		pairBlock.AddComponent(typeof(Image));
		pairBlock.transform.SetParent(pairSlot.transform);
		pairBlockRect.anchoredPosition = Vector3.zero;
		pairBlock.name = ID.ToString();
		
		GameObject pairText = new GameObject();
		RectTransform pairTextRect = (RectTransform)pairText.AddComponent(typeof(RectTransform));
		pairTextRect.sizeDelta = new Vector2(18f,18f);
		pairText.AddComponent(typeof(CanvasRenderer));
		Text pairTextText = (Text)pairText.AddComponent(typeof(Text));
		pairTextText.text = ID.ToString();
		pairTextText.alignment = TextAnchor.MiddleCenter;
		pairTextText.font = (Font)Resources.GetBuiltinResource(typeof(Font), "Arial.ttf");
		pairText.transform.SetParent(pairBlock.transform);
		pairText.transform.localPosition = Vector3.zero;
		pairText.name = ID.ToString() + " Text"; 
		
		//update glove handedness
		int pairHand = (pairSlot.name[0] == 'R') ? 1 : -1;
		if (pairHand == 1) pairBlock.GetComponent<Image>().color = Color.red;
		else pairBlock.GetComponent<Image>().color = Color.blue;
		int pairID = Int32.Parse(pairBlock.name);
		if (gloves.ContainsKey(pairID)) 
		{
			gloves[pairID].GetComponent<HandController>().handedness = pairHand;
			int pair = (int)Char.GetNumericValue(pairSlot.name[1]);
			gloves[pairID].transform.localPosition = glovePositions[pair] + pairHand * Vector3.right * 1.3f;
		}
		
		//make block draggable
		EventTrigger trigger = (EventTrigger)pairBlock.AddComponent(typeof(EventTrigger));
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[0].eventID = EventTriggerType.Drag;
		trigger.triggers[0].callback.AddListener(delegate { pairBlock.transform.position = Input.mousePosition - dragStart; });
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[1].eventID = EventTriggerType.PointerDown;
		trigger.triggers[1].callback.AddListener(delegate { originalParent = pairBlock.transform.parent; pairBlock.transform.SetParent(transform.GetChild(1)); dragStart = Input.mousePosition - pairBlock.transform.position; });
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[2].eventID = EventTriggerType.PointerUp;
		trigger.triggers[2].callback.AddListener(delegate { DropPairBlock(pairBlock); });
	}
	
	GameObject FindSlot()
	{
		for (int i = 0; i < leftPairSlots.Count; i++)
		{
			if (leftPairSlots[i].transform.childCount == 0)
			{
				return leftPairSlots[i];
			}
			if (rightPairSlots[i].transform.childCount == 0)
			{
				return rightPairSlots[i];
			}
		}
		return AddPair();
	}
	
	GameObject AddPair()
	{
		int pair = leftPairSlots.Count;
		transform.GetChild(1).GetComponent<RectTransform>().sizeDelta = new Vector2(80f, 35f * pair);
		GameObject leftSlot = CreatePairSlot();
		RectTransform leftRect = leftSlot.GetComponent<RectTransform>();
		leftSlot.name = "L" + pair.ToString();
		leftRect.anchorMin = new Vector2(0.5f, 1f);
		leftRect.anchorMax = new Vector2(0.5f, 1f);
		leftRect.pivot = new Vector2(0.5f, 1f);
		leftSlot.transform.localPosition += Vector3.left*(15f) + Vector3.down*(pair*25f+10f);
		leftPairSlots.Add(leftSlot);
		
		GameObject rightSlot = CreatePairSlot();
		RectTransform rightRect = rightSlot.GetComponent<RectTransform>();
		rightSlot.name = "R" + pair.ToString();
		rightSlot.GetComponent<RectTransform>();
		rightRect.anchorMin = new Vector2(0.5f, 1f);
		rightRect.anchorMax = new Vector2(0.5f, 1f);
		rightRect.pivot = new Vector2(0.5f, 1f);
		rightSlot.transform.localPosition += Vector3.right*(15f) + Vector3.down*(pair*25f+10f);
		rightPairSlots.Add(rightSlot);
		return leftSlot;
	}
	
	GameObject CreatePairSlot()
	{
		GameObject pairSlot = new GameObject();
		pairSlot.transform.SetParent(transform.GetChild(1), false);
		RectTransform pairSlotRect = (RectTransform)pairSlot.AddComponent(typeof(RectTransform));
		pairSlotRect.sizeDelta = new Vector2(20f,20f);
		pairSlot.AddComponent(typeof(CanvasRenderer));
		Image pairSlotImage = (Image)pairSlot.AddComponent(typeof(Image));
		pairSlotImage.color = Color.black;
		return pairSlot;
	}
	
	void DropPairBlock(GameObject pairBlock)
	{
		//if we're above a viable slot, reassign pairblock to slot
		Transform newParent = originalParent;
		//loop through open slots
		for (int i = 0; i < leftPairSlots.Count; i++)
		{
			Rect slotRect = leftPairSlots[i].GetComponent<RectTransform>().rect;
			if (Input.mousePosition.x >= leftPairSlots[i].transform.position.x - slotRect.width/2f &&
				Input.mousePosition.y >= leftPairSlots[i].transform.position.y - slotRect.height/2f &&
				Input.mousePosition.x <= leftPairSlots[i].transform.position.x + slotRect.width/2f &&
				Input.mousePosition.y <= leftPairSlots[i].transform.position.y + slotRect.height/2f)
			{
				newParent = leftPairSlots[i].transform;
				break;
			}
			slotRect = leftPairSlots[i].GetComponent<RectTransform>().rect;
			if (Input.mousePosition.x >= rightPairSlots[i].transform.position.x - slotRect.width/2f &&
				Input.mousePosition.y >= rightPairSlots[i].transform.position.y - slotRect.height/2f &&
				Input.mousePosition.x <= rightPairSlots[i].transform.position.x + slotRect.width/2f &&
				Input.mousePosition.y <= rightPairSlots[i].transform.position.y + slotRect.height/2f)
			{
				newParent = rightPairSlots[i].transform;
				break;
			}
		}
		
		//swap out old block if this slot was occupied
		if (newParent.childCount != 0)
		{
			GameObject swapBlock = newParent.GetChild(0).gameObject;
			swapBlock.transform.SetParent(originalParent);
			swapBlock.GetComponent<RectTransform>().anchoredPosition = Vector3.zero;
			int swapHand = (swapBlock.transform.parent.name[0] == 'R') ? 1 : -1;
			if (swapHand == 1) swapBlock.GetComponent<Image>().color = Color.red;
			else swapBlock.GetComponent<Image>().color = Color.blue;
			swapBlock.GetComponent<RectTransform>().anchoredPosition = Vector3.zero;
			int swapID = Int32.Parse(swapBlock.name);
			if (gloves.ContainsKey(swapID)) 
			{
				gloves[swapID].GetComponent<HandController>().handedness = swapHand;
				int pair = (int)Char.GetNumericValue(swapBlock.transform.parent.name[1]);
				gloves[swapID].transform.localPosition = glovePositions[pair] + swapHand * Vector3.right * 1.3f;
			}
		}
		//slot pairblock into parent (empty space)
		pairBlock.transform.SetParent(newParent);
		pairBlock.GetComponent<RectTransform>().anchoredPosition = Vector3.zero;
		int pairHand = (pairBlock.transform.parent.name[0] == 'R') ? 1 : -1;
		if (pairHand == 1) pairBlock.GetComponent<Image>().color = Color.red;
		else pairBlock.GetComponent<Image>().color = Color.blue;
		int pairID = Int32.Parse(pairBlock.name);
		if (gloves.ContainsKey(pairID)) 
		{
			gloves[pairID].GetComponent<HandController>().handedness = pairHand;
			int pair = (int)Char.GetNumericValue(pairBlock.transform.parent.name[1]);
			gloves[pairID].transform.localPosition = glovePositions[pair] + pairHand * Vector3.right * 1.3f;
		}
	}
	
	void ConnectGlove(int ID)
	{
		gloves[ID].GetComponent<HandController>().GloveConnect();
	}
	
	void ClearGlove(int ID)
	{
		gloves[ID].GetComponent<HandController>().GloveClear();
	}
	
	void OnApplicationQuit()
	{
		closeLibrary();
	}
}
