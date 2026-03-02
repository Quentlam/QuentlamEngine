#type vertex
#version 330 core
layout(location = 0)in vec3 u_Position;
layout(location = 1)in vec2 TexCoord;

out vec2 v_TexCoord;
out vec2 v_ScreenPos;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;



void main()
{
	v_TexCoord = TexCoord;
	gl_Position = u_ViewProjection * u_Transform * vec4(u_Position,1.0);
	v_ScreenPos = gl_Position.xy;
}


#type fragment
#version 330 core
out vec4 color;

in vec2 v_TexCoord;
in vec2 v_ScreenPos;


uniform vec4 u_Color;
uniform sampler2D u_Texture;

uniform float m_TilingFactor;



void main()
{
	float dist = 1.0f - distance(v_ScreenPos * 0.8f,vec2(0.0f));
	dist = clamp(dist,0.0f,1.0f);
	dist = sqrt(dist);
	color = texture(u_Texture,v_TexCoord * m_TilingFactor) * u_Color * dist;
}
