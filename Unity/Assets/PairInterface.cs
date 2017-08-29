using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Linq;
using System.Text;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;

public class PairInterface:MonoBehaviour
{
	[HideInInspector]
	public MainInterface mainInterface;
	
	private List<Vector3> glovePositions;
	private float gloveSeparation;
	
	private Vector3 dragStart;
	private Transform originalParent;
	
	private List<GameObject> leftPairSlots;
	private List<GameObject> rightPairSlots;

	void Start()
	{
		glovePositions = new List<Vector3>();
		glovePositions.Add(Vector3.zero);
		glovePositions.Add(Vector3.up * 2f);
		glovePositions.Add(Vector3.up * 4f);
		gloveSeparation = 2.6f;
		
		//use two lists to match glove pairs
		leftPairSlots = new List<GameObject>();
		rightPairSlots = new List<GameObject>();
	}
	
	void Update()
	{
		
	}
	
	public void AddPairBlock(int ID)
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
		if (mainInterface.gloves.ContainsKey(pairID)) 
		{
			mainInterface.gloves[pairID].GetComponent<HandController>().handedness = pairHand;
			int pair = (int)Char.GetNumericValue(pairSlot.name[1]);
			mainInterface.gloves[pairID].transform.localPosition = glovePositions[pair] + pairHand * Vector3.right * gloveSeparation;
		}
		
		//make block draggable
		EventTrigger trigger = (EventTrigger)pairBlock.AddComponent(typeof(EventTrigger));
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[0].eventID = EventTriggerType.Drag;
		trigger.triggers[0].callback.AddListener(delegate { pairBlock.transform.position = Input.mousePosition - dragStart; });
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[1].eventID = EventTriggerType.PointerDown;
		trigger.triggers[1].callback.AddListener(delegate { originalParent = pairBlock.transform.parent; pairBlock.transform.SetParent(transform); dragStart = Input.mousePosition - pairBlock.transform.position; });
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[2].eventID = EventTriggerType.PointerUp;
		trigger.triggers[2].callback.AddListener(delegate { DropPairBlock(pairBlock); });
		
		//add a new row if all pairs are filled
		FindSlot();
	}
	
	private GameObject FindSlot()
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
	
	private GameObject AddPair()
	{
		int pair = leftPairSlots.Count;
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
	
	private GameObject CreatePairSlot()
	{
		GameObject pairSlot = new GameObject();
		pairSlot.transform.SetParent(transform, false);
		RectTransform pairSlotRect = (RectTransform)pairSlot.AddComponent(typeof(RectTransform));
		pairSlotRect.sizeDelta = new Vector2(20f,20f);
		pairSlot.AddComponent(typeof(CanvasRenderer));
		Image pairSlotImage = (Image)pairSlot.AddComponent(typeof(Image));
		pairSlotImage.color = Color.black;
		return pairSlot;
	}
	
	private void DropPairBlock(GameObject pairBlock)
	{
		//if we're above a viable slot, reassign pairblock to slot
		Transform newParent = originalParent;
		//loop through open slots
		//TODO increase droppable zone to fill margins
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
			if (mainInterface.gloves.ContainsKey(swapID)) 
			{
				mainInterface.gloves[swapID].GetComponent<HandController>().handedness = swapHand;
				mainInterface.gloves[swapID].GetComponent<HandController>().UpdateTexture();
				int pair = (int)Char.GetNumericValue(swapBlock.transform.parent.name[1]);
				mainInterface.gloves[swapID].transform.localPosition = glovePositions[pair] + swapHand * Vector3.right * gloveSeparation;
			}
		}
		//slot pairblock into parent (empty space)
		pairBlock.transform.SetParent(newParent);
		pairBlock.GetComponent<RectTransform>().anchoredPosition = Vector3.zero;
		int pairHand = (pairBlock.transform.parent.name[0] == 'R') ? 1 : -1;
		if (pairHand == 1) pairBlock.GetComponent<Image>().color = Color.red;
		else pairBlock.GetComponent<Image>().color = Color.blue;
		int pairID = Int32.Parse(pairBlock.name);
		if (mainInterface.gloves.ContainsKey(pairID)) 
		{
			mainInterface.gloves[pairID].GetComponent<HandController>().handedness = pairHand;
			mainInterface.gloves[pairID].GetComponent<HandController>().UpdateTexture();
			int pair = (int)Char.GetNumericValue(pairBlock.transform.parent.name[1]);
			mainInterface.gloves[pairID].transform.localPosition = glovePositions[pair] + pairHand * Vector3.right * gloveSeparation;
		}
		
		//update the orbit center
		mainInterface.UpdateOrbit();
	}
}
