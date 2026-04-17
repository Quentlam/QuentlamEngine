#type vertex
#version 410 core

out vec2 v_TexCoord;

void main()
{
    // Generate a fullscreen triangle using gl_VertexID
    vec2 pos = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    v_TexCoord = pos;
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
}

#type fragment
#version 410 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform isampler2D u_EntityIDTexture;
uniform int u_SelectedEntity;
uniform vec4 u_OutlineColor;
uniform int u_OutlineWidth;
uniform vec2 u_TexSize; // 1.0 / vec2(width, height)
uniform float u_OutlineIntensity; // added parameter

int getMask(vec2 offset)
{
    int id = texture(u_EntityIDTexture, v_TexCoord + offset * u_TexSize).r;
    return id == u_SelectedEntity ? 1 : 0;
}

void main()
{
    int center = getMask(vec2(0.0));
    if (center == 1)
    {
        discard; // Don't draw over the object itself
    }

    // Use Sobel operator for edge detection
    // To support adjustable width, we sample at distance = u_OutlineWidth
    float w = float(u_OutlineWidth);
    
    // 3x3 Sobel kernels
    float Gx = 0.0;
    float Gy = 0.0;
    
    // We can also sample densely within the radius to ensure we don't miss thin geometry
    // But a simple variable-distance Sobel gives a nice soft edge
    
    Gx += -1.0 * float(getMask(vec2(-w, -w)));
    Gx += -2.0 * float(getMask(vec2(-w,  0.0)));
    Gx += -1.0 * float(getMask(vec2(-w,  w)));
    Gx +=  1.0 * float(getMask(vec2( w, -w)));
    Gx +=  2.0 * float(getMask(vec2( w,  0.0)));
    Gx +=  1.0 * float(getMask(vec2( w,  w)));

    Gy += -1.0 * float(getMask(vec2(-w, -w)));
    Gy += -2.0 * float(getMask(vec2( 0.0, -w)));
    Gy += -1.0 * float(getMask(vec2( w, -w)));
    Gy +=  1.0 * float(getMask(vec2(-w,  w)));
    Gy +=  2.0 * float(getMask(vec2( 0.0,  w)));
    Gy +=  1.0 * float(getMask(vec2( w,  w)));

    float edge = sqrt(Gx * Gx + Gy * Gy);
    
    if (edge > 0.0)
    {
        // Smooth alpha based on edge strength and intensity
        float alpha = clamp(edge * u_OutlineIntensity, 0.0, 1.0) * u_OutlineColor.a;
        o_Color = vec4(u_OutlineColor.rgb, alpha);
    }
    else
    {
        discard;
    }
}