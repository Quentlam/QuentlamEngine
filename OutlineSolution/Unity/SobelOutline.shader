Shader "Hidden/Custom/SobelOutline"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
        _OutlineColor ("Outline Color", Color) = (1, 0.5, 0, 1)
        _OutlineWidth ("Outline Width", Range(0.1, 10.0)) = 1.0
        _OutlineIntensity ("Outline Intensity", Range(0.1, 5.0)) = 1.0
        _DepthThreshold ("Depth Threshold", Range(0.001, 1.0)) = 0.05
        _NormalThreshold ("Normal Threshold", Range(0.01, 1.0)) = 0.2
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" "RenderPipeline" = "UniversalPipeline" }
        Cull Off ZWrite Off ZTest Always

        Pass
        {
            Name "SobelOutline"

            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/DeclareDepthTexture.hlsl"
            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/DeclareNormalsTexture.hlsl"

            struct Attributes
            {
                float4 positionOS : POSITION;
                float2 uv : TEXCOORD0;
            };

            struct Varyings
            {
                float4 positionCS : SV_POSITION;
                float2 uv : TEXCOORD0;
            };

            TEXTURE2D(_MainTex);
            SAMPLER(sampler_MainTex);

            float4 _OutlineColor;
            float _OutlineWidth;
            float _OutlineIntensity;
            float _DepthThreshold;
            float _NormalThreshold;

            float2 _MainTex_TexelSize;

            Varyings vert (Attributes input)
            {
                Varyings output;
                output.positionCS = TransformObjectToHClip(input.positionOS.xyz);
                output.uv = input.uv;
                return output;
            }

            float SampleDepth(float2 uv)
            {
                return SampleSceneDepth(uv);
            }

            float3 SampleNormal(float2 uv)
            {
                return SampleSceneNormals(uv);
            }

            float4 frag (Varyings input) : SV_Target
            {
                float2 uv = input.uv;
                float w = _OutlineWidth * _MainTex_TexelSize.xy;

                // Sobel offsets
                float2 offsets[9] = {
                    float2(-w.x, -w.y), float2(0, -w.y), float2(w.x, -w.y),
                    float2(-w.x, 0),    float2(0, 0),    float2(w.x, 0),
                    float2(-w.x, w.y),  float2(0, w.y),  float2(w.x, w.y)
                };

                // Sobel kernels
                float Gx[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 };
                float Gy[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 };

                float depthDx = 0, depthDy = 0;
                float3 normalDx = 0, normalDy = 0;

                for (int i = 0; i < 9; i++)
                {
                    float d = SampleDepth(uv + offsets[i]);
                    float3 n = SampleNormal(uv + offsets[i]);

                    depthDx += d * Gx[i];
                    depthDy += d * Gy[i];

                    normalDx += n * Gx[i];
                    normalDy += n * Gy[i];
                }

                float depthEdge = sqrt(depthDx * depthDx + depthDy * depthDy);
                float normalEdge = sqrt(dot(normalDx, normalDx) + dot(normalDy, normalDy));

                depthEdge = depthEdge > _DepthThreshold ? 1.0 : 0.0;
                normalEdge = normalEdge > _NormalThreshold ? 1.0 : 0.0;

                float edge = max(depthEdge, normalEdge) * _OutlineIntensity;
                
                float4 col = SAMPLE_TEXTURE2D(_MainTex, sampler_MainTex, uv);
                return lerp(col, _OutlineColor, saturate(edge));
            }
            ENDHLSL
        }
    }
}