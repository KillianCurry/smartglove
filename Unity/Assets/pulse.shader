Shader "Hidden/pulse"
{
	Properties
	{
		_MainTex ("Texture", 2D) = "white" {}
	}
	SubShader
	{
		// No culling or depth
		Cull Off ZWrite Off ZTest Always

		Pass
		{
			CGPROGRAM
			#pragma vertex vert_img
			#pragma fragment frag
			
			#include "UnityCG.cginc"
			
			uniform sampler2D _MainTex;
			uniform float _origin[2];
			uniform float _pulseTime;
			uniform float _screen[2];
			static const float PI = 3.14159265f;
			
			float4 frag (v2f_img i) : SV_Target
			{
				float2 ori = float2(_origin[0],_origin[1]*_screen[1]/_screen[0]);
				i.uv.y *= (_screen[1]/_screen[0]);
				float originDist = length(i.uv - ori);
				float pulseDist = _pulseTime*0.1;
				float pulseBand = 0.1;
				float s = -sin(((originDist-pulseDist)*PI/pulseBand))*pulseBand;
				i.uv.y /= (_screen[1]/_screen[0]);
				float4 col;
				if (originDist > pulseDist - pulseBand && originDist < pulseDist)
				{
					i.uv += float2(s,s);
					col = tex2D(_MainTex, i.uv);
					//col -= abs(s);
				}
				else{
					col = tex2D(_MainTex, i.uv);
				}
				//float4 col = tex2D(_MainTex, i.uv + float3(1.0, 0.0, 0.0) * sin(_Time.y + i.uv.y * 10) * 0.1);
				return col;
			}
			ENDCG
		}
	}
}
