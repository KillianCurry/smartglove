using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraController:MonoBehaviour
{
	public int screenWidth;
	public int screenHeight;
	
	private Vector2 pulseOrigin = Vector2.one * 0.5f;
	private float pulseTime = 1f;
	
	private Material effectMat;
	
	void Start()
	{
		effectMat = new Material(Shader.Find("Hidden/pulse"));
	}
	
	void Update()
	{
		pulseTime += Time.deltaTime;
		if (Input.GetMouseButtonDown(0))
		{
			pulseOrigin = Input.mousePosition;
			pulseOrigin.x/=Screen.width;
			pulseOrigin.y/=Screen.height;
			pulseTime = 0f;
		}
	}
	
	void LateUpdate()
	{
		if (Input.GetKeyDown(KeyCode.F1)) GenerateVector(screenWidth, screenHeight);
	}
	//issue to pdflatex
	//issue to pdf2svg
	void GenerateVector(int width, int height)
	{
		//render camera to texture
		Camera camera = GetComponent<Camera>();
		RenderTexture screen = new RenderTexture(width, height, 24);
		camera.targetTexture = screen;
		camera.Render();
		RenderTexture.active = screen;
		
		//initialise LaTeX string
		string latexString = "";
		
		
		//convert vertices to points on render texture
		GameObject[] objects = UnityEngine.Object.FindObjectsOfType<GameObject>();
		for (int g = 0; g < objects.Length; g++)
		{
			if (!objects[g].activeInHierarchy) continue;
			MeshFilter filter = objects[g].GetComponent<MeshFilter>();
			if (filter == null) continue;
			Mesh mesh = filter.sharedMesh;
			Debug.Log(objects[g].name);
			Debug.Log(mesh.vertices.Length);
			Debug.Log(mesh.triangles.Length/3);
			Vector2[] meshNodes = new Vector2[mesh.vertices.Length];
			for (int i = 0; i < mesh.vertices.Length; i++)
			{
				meshNodes[i] = camera.WorldToScreenPoint(objects[g].transform.position + (objects[g].transform.rotation * mesh.vertices[i]));
			}
			
			//connect points by tris
			for (int i = 0; i < mesh.triangles.Length; i += 3)
			{
				latexString += "\\draw ";
				Vector2 n;
				for (int o = 0; o < 3; o++)
				{
					n = meshNodes[mesh.triangles[i+o]];
					latexString += "(" + n.x.ToString() + "," + n.y.ToString() + ") -- ";
				}
				n = meshNodes[mesh.triangles[i]];
				latexString += "(" + n.x.ToString() + "," + n.y.ToString() + ");\n";
			}
		}
		
		//detach rendertexture from camerra
		camera.targetTexture = null;
		RenderTexture.active = null;
		Destroy(screen);
		
		Debug.Log(latexString);
	}

	void OnRenderImage(RenderTexture source, RenderTexture destination)
	{
		float[] originArray = new float[2];
		originArray[0] = pulseOrigin.x; originArray[1] = pulseOrigin.y;
		float[] screen = new float[2];
		screen[0] = Screen.width; screen[1] = Screen.height;
		effectMat.SetFloatArray("_origin", originArray);
		effectMat.SetFloatArray("_screen", screen);
		effectMat.SetFloat("_pulseTime", pulseTime);
		Graphics.Blit(source, destination, effectMat);
	}
}
