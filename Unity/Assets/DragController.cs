using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class DragController:MonoBehaviour
{
	private bool dragging = false;
	private Vector3 lastPos = Vector3.zero;
	public Vector3 target = Vector3.zero;
	public float sensitivity = 0.2f;

	void Start()
	{
		EventTrigger trigger = (EventTrigger)gameObject.AddComponent(typeof(EventTrigger));
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[0].eventID = EventTriggerType.PointerDown;
		trigger.triggers[0].callback.AddListener(delegate { dragging = true; lastPos = Input.mousePosition; });
		trigger.triggers.Add(new EventTrigger.Entry());
		trigger.triggers[1].eventID = EventTriggerType.PointerUp;
		trigger.triggers[1].callback.AddListener(delegate { dragging = false; });
	}
	
	void Update()
	{
		if (dragging)
		{
			Vector3 drag = (Input.mousePosition - lastPos)*sensitivity;
			Transform cam = Camera.main.transform;
			cam.RotateAround(target, Vector3.up, drag.x);
			cam.RotateAround(target, cam.right, -drag.y);
			cam.LookAt(target);
			lastPos = Input.mousePosition;
		}
	}
}
