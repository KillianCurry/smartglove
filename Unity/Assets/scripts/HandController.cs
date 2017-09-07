using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using UnityEngine;


public class HandController:MonoBehaviour
{
    [Tooltip("Maximum degrees the finger can rotate per second.")]
    public float rotationSpeed = 180f;
	//list of finger rotation ranges
    [HideInInspector]
	public List<double> rotationMinimum;
    [HideInInspector]
    public List<double> rotationMaximum;
	//list of each joint
	private List<Transform> joints;
	
    //the 'forward' rotation
	private Quaternion zeroRotation;

	[HideInInspector]
	//-1 is left-handed, 1 is right-handed
	public int handedness = 1;

    //list of each joint's rotation
    [HideInInspector]
    public List<double> fingerRotations;
	//vector of the x, y, and z rotation
    [HideInInspector]
	public Vector3 palmOrientation;
	
	//is the BLE connected?
	[HideInInspector]
	public bool connected = false;
	public int ID;

    //these values need to be initialized before MainInterface accesses them in its Update loop
    private void Awake()
    {
        fingerRotations = new List<double>()
        {
            0f,0f,0f,//thumb
			0f,0f,0f,//index
			0f,0f,0f,//middle
			0f,0f,0f,//ring
			0f,0f,0f //pinky
        };
        //set rotation limits
        rotationMinimum = new List<double>()
        {
            0f,0f,0f,	 //thumb
			-10f,-15f,0f,//index
			-10f,-15f,0f,//middle
			-10f,-15f,0f,//ring
			-10f,-15f,0f //pinky
		};
        rotationMaximum = new List<double>()
        {
            90f,90f,90f, //thumb
			75f,110f,80f,//index
			75f,110f,80f,//middle
			75f,110f,80f,//ring
			75f,110f,80f,//pinky
		};
    }
	
	void Start()
	{
		joints = new List<Transform>();
		//instantiate the hand prefab and transfer its contents to this
		GameObject gloveObject = (GameObject)Instantiate(Resources.Load("models/glove", typeof(GameObject)));
		gloveObject.transform.GetChild(0).SetParent(transform, false);
		gloveObject.transform.GetChild(0).SetParent(transform, false);
		//mark all the joints for easier reference later
		for (int i = 0; i < 5; i++)
		{
			joints.Add(transform.GetChild(0).GetChild(0).GetChild(i));
			joints.Add(joints[i*3].GetChild(0));
			joints.Add(joints[i*3+1].GetChild(0));
		}
		//destroy the now empty prefab
		Destroy(gloveObject);
		UpdateTexture();
	}
	
	void Update()
	{
		transform.localScale = new Vector3(-handedness, 1, 1);
        //update joint angles and palm orientation to match input
		joints[0].localRotation = Quaternion.Euler(joints[0].localRotation.eulerAngles.x, -(float)fingerRotations[0], joints[0].localRotation.eulerAngles.z);
		for (int r = 1; r < fingerRotations.Count; r++)
		{
			float spread = 0f;
			if (r % 3 == 0) spread = ((((r/3)-2)*5)*((90f-(float)fingerRotations[r])/90f));
            joints[r].localRotation = Quaternion.RotateTowards(joints[r].localRotation, Quaternion.Euler((float)fingerRotations[r], 0f, spread), rotationSpeed * Time.deltaTime);
		}
		if (connected) this.transform.localRotation = zeroRotation * Quaternion.Euler(palmOrientation.x, palmOrientation.y, palmOrientation.z);
	}

    public void SetZeroRotation()
    {
        zeroRotation = Quaternion.Inverse(Quaternion.Euler(palmOrientation.x, palmOrientation.y, palmOrientation.z));
    }
	
	public void UpdateTexture()
	{
		string texName = "textures/logo";
		if (handedness == -1) texName += "R";
		else texName += "L";
		transform.GetChild(1).GetComponent<SkinnedMeshRenderer>().material.mainTexture = (Texture)Resources.Load(texName, typeof(Texture));
	}
}
