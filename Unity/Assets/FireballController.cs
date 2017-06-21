using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FireballController : MonoBehaviour {
	
	[Range(0f, 1f)]
	public float intensity;
	
	//hand to appear in
	public GameObject hand;
	
	//CORE FLAME
	
	private ParticleSystem core;
	
	private ParticleSystem.MainModule coreMain;
	
	private ParticleSystem.ColorOverLifetimeModule coreColorLifetime;
	private ParticleSystem.MinMaxGradient coreColorGradient;
	private List<GradientColorKey> coreColorKeys;
	private List<GradientAlphaKey> coreAlphaKeys;
	
	private ParticleSystem.SizeOverLifetimeModule coreSizeLifetime;
	private ParticleSystem.MinMaxCurve coreSizeCurve;

	// Use this for initialization
	void Start ()
	{
		core = transform.GetChild(0).gameObject.GetComponent<ParticleSystem>();
		
		coreMain = core.main;
		
		//COLOR GRADIENT (dark blue -> blue-white)
		coreColorKeys = new List<GradientColorKey>();
		coreAlphaKeys = new List<GradientAlphaKey>();
		coreColorKeys.Add(new GradientColorKey(Color.HSVToRGB(0.59f, 225f/255f, 255f/255f), 0.0f));
		coreColorKeys.Add(new GradientColorKey(Color.HSVToRGB(0.59f, 225f/255f, 255f/255f), 0.1f));
		coreColorKeys.Add(new GradientColorKey(Color.HSVToRGB(0.50f, 135f/255f, 255f/255f), 0.4f));
		coreColorKeys.Add(new GradientColorKey(Color.white, 1.0f));
		//first argument is value, second is time
		coreAlphaKeys.Add(new GradientAlphaKey(1.0f, 0.0f));
		coreAlphaKeys.Add(new GradientAlphaKey(0.3f, 0.8f));
		coreAlphaKeys.Add(new GradientAlphaKey(0.0f, 1.0f));
		Gradient tempGradient = new Gradient();
		tempGradient.SetKeys(coreColorKeys.ToArray(), coreAlphaKeys.ToArray());
		coreColorGradient = new ParticleSystem.MinMaxGradient(tempGradient);
		coreColorLifetime = core.colorOverLifetime;
		//coreColorLifetime.color = coreColorGradient;
		
		//SIZE CURVE (bigger flame)
		AnimationCurve tempCurve = new AnimationCurve();
		//first argument is time, second is value
		tempCurve.AddKey(0f, intensity);
		tempCurve.AddKey(1f, 0f);
		coreSizeCurve = new ParticleSystem.MinMaxCurve(1f, tempCurve);
		coreSizeLifetime = core.sizeOverLifetime;
		coreSizeLifetime.size = coreSizeCurve;
		coreSizeLifetime.enabled = true;
		
		//PARTICLE LIFETIME (higher flame)
	}
	
	// Update is called once per frame
	void Update ()
	{
		//stick to palm of hand
		transform.position = hand.transform.position - hand.transform.up*hand.transform.GetChild(0).transform.localScale.y + hand.transform.forward*hand.transform.GetChild(0).transform.localScale.z*0.5f;
		
		//match intensity to hand curl
		intensity = 1-hand.GetComponent<HandController>().fingerCurl;
		
		//color gradient
		/*coreColorKeys[0] = new GradientColorKey(Color.HSVToRGB(0.6f, intensity*0.7f+0.3f, 0.9f), 0.0f);
		coreColorKeys[1] = new GradientColorKey(Color.HSVToRGB(0.6f, 0f, 0.9f), 0.0f);
		coreColorGradient.gradient.SetKeys(coreColorKeys.ToArray(), coreAlphaKeys.ToArray());
		coreColorLifetime.color = coreColorGradient;*/
		
		//size
		coreSizeCurve.curve.MoveKey(0, new Keyframe(0f, intensity));
		coreSizeLifetime.size = coreSizeCurve;
		
		//lifetime
		coreMain.startLifetime = intensity*1f;
	}
}
