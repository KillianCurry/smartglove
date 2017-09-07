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
	public Dictionary<int,HandController> gloves;

    public bool testPoseMet;

    //manages mouse-based camera control
    private DragInterface dragInterface;
	//manages glove connection panels
	private ConnectInterface connectInterface;
	//manages glove pairing blocks
	private PairInterface pairInterface;
	
	//find available gloves in the library
	[DllImport("smartglove", EntryPoint = "findGloves", CallingConvention = CallingConvention.Cdecl)]
	public static extern IntPtr findGloves();
	//close the library, cleaning up all data structures
	[DllImport("smartglove", EntryPoint = "closeLibrary")]
	public static extern void closeLibrary();
	//add a new glove to the library based on a UUID string
	[DllImport("smartglove", EntryPoint = "addUUID")]
	public static extern void addUUID(StringBuilder buffer, ref int bufferSize);
    //open a glove's BLE connection
    [DllImport("smartglove", EntryPoint = "establishConnection", SetLastError = true)]
    public static extern bool openConnection(int gloveID);
    //close a glove's BLE connection
    [DllImport("smartglove", EntryPoint = "closeConnection")]
    public static extern bool closeConnection(int gloveID);
    //read a glove's stretch and IMU data
    [DllImport("smartglove", EntryPoint = "getData", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr readGlove(int gloveID);
    //clear a glove's autocalibrated values
    [DllImport("smartglove", EntryPoint = "clearCalibration")]
    public static extern void clearCalibration(int gloveID);
    //get the time since the last notification on the specified glove
    [DllImport("smartglove", EntryPoint = "getLastNotification", CallingConvention = CallingConvention.Cdecl)]
    public static extern double getLastNotification(int gloveID);
    [DllImport("smartglove", EntryPoint = "setAngles")]
    public static extern void setAngles(int gloveID, IntPtr minAngles, IntPtr maxAngles);

    private void Start()
	{
		//keep track of children interfaces
		MainInterface thisScript = GetComponent<MainInterface>();
		dragInterface = transform.GetChild(0).gameObject.GetComponent<DragInterface>();
		dragInterface.mainInterface = thisScript;
		connectInterface = transform.GetChild(1).gameObject.GetComponent<ConnectInterface>();
		connectInterface.mainInterface = thisScript;
		pairInterface = transform.GetChild(2).gameObject.GetComponent<PairInterface>();
		pairInterface.mainInterface = thisScript;
		
		gloves = new Dictionary<int,HandController>();
		
		//add new gloves to the library
		string UUID1 = "{00601001-7374-7265-7563-6873656e7365}";
		int bufferSize1 = UUID1.Length;
		StringBuilder buffer1 = new StringBuilder(UUID1, bufferSize1);
		addUUID(buffer1, ref bufferSize1);
		string UUID2 = "{00000501-7374-7265-7563-6873656e7365}";
		int bufferSize2 = UUID2.Length;
		StringBuilder buffer2 = new StringBuilder(UUID2, bufferSize2);
		addUUID(buffer2, ref bufferSize2);
		
		//add corresponding connection panels for the library's gloves
		Populate();
	}

    private void Update()
    {
        foreach (HandController glove in gloves.Values)
        {
            //copy data from the DLL's unmanaged memory into a managed array
            double[] data = new double[18];
            IntPtr ptr = readGlove(glove.ID);
            Marshal.Copy(ptr, data, 0, 18);
            
            //copy orientation data
            glove.palmOrientation = new Vector3((float)data[0], (float)data[2], (float)data[1]);

            //read finger rotation values
            for (int i = 0; i < 15; i++)
            {
                //change 0-1 to degrees
                glove.fingerRotations[i] = data[i + 3];
            }

            connectInterface.UpdateNotificationIndicator(glove.ID, getLastNotification(glove.ID));
        }
    }
	
	private void Populate()
	{
		//retrieve the UUID list from the library
		IntPtr ptr = findGloves();
		string source = Marshal.PtrToStringAnsi(ptr);
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
		foreach (HandController g in gloves.Values)
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

        //set joint angle ranges in library
        IntPtr minPtr = Marshal.AllocHGlobal(Marshal.SizeOf(gloveScript.rotationMinimum[0]) * 15);
        Marshal.Copy(gloveScript.rotationMinimum.ToArray(), 0, minPtr, 15);
        IntPtr maxPtr = Marshal.AllocHGlobal(Marshal.SizeOf(gloveScript.rotationMaximum[0]) * 15);
        Marshal.Copy(gloveScript.rotationMaximum.ToArray(), 0, maxPtr, 15);
        setAngles(ID, minPtr, maxPtr);
        Marshal.FreeHGlobal(minPtr);
        Marshal.FreeHGlobal(maxPtr);

        gloveScript.ID = ID;
		gloves.Add(ID, gloveScript);

		//add pairing block
		pairInterface.AddPairBlock(ID);
		
		//highlight new glove
		Camera.main.GetComponent<HighlightEffect>().highlightObject = newGlove;
		
		//update the orbit center
		UpdateOrbit();
	}

    public void RemoveGlove(int ID)
    {
        //remove highlight effect
        Camera.main.GetComponent<HighlightEffect>().highlightObject = null;
        //remove the glove from the dictionary and the scene
        DisconnectGlove(ID);
        GameObject glove = gloves[ID].gameObject;
        gloves.Remove(ID);
        GameObject.Destroy(glove);
        //remove corresponding pairblock
        pairInterface.RemovePairBlock(ID);
    }
	
	//connect a given glove object
	public bool ConnectGlove(int ID)
	{
        if (gloves[ID].connected) return true;
        gloves[ID].connected = openConnection(ID);
        if (!gloves[ID].connected)
        {
            //write the exception from the DLL if the connection doesn't work
            Debug.Log("CONNECTION ERROR: 0x" + Marshal.GetLastWin32Error().ToString("X"));
        }
        return gloves[ID].GetComponent<HandController>().connected;
	}

    //disconnect a given glove object
    public bool DisconnectGlove(int ID)
    {
        if (!gloves[ID].connected) return true;
        gloves[ID].connected = !closeConnection(ID);
        return !gloves[ID].connected;
    }
	
	//clear a given glove object's calibration data
	public void ClearGlove(int ID)
	{
        gloves[ID].SetZeroRotation();
        clearCalibration(ID);
    }
	
	//called when quitting the application
	void OnApplicationQuit()
	{
		//disconnect all gloves so reading coroutines end
		foreach (int ID in gloves.Keys)
		{
            DisconnectGlove(ID);
		}
		//clean up the library
		closeLibrary();
	}
}
