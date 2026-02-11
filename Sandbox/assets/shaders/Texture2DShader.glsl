#type vertex
#version 330 core
layout(location = 0)in vec3 u_Position;
layout(location = 1)in vec2 TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
		 
void main()
{
	v_TexCoord = TexCoord;
	gl_Position = u_ViewProjection * u_Transform * vec4(u_Position,1.0);
}


#type fragment
#version 330 core
out vec4 color;		
uniform vec4 u_Color;

in vec2 v_TexCoord;
uniform float m_TilingFactor;
uniform sampler2D u_Texture;




void main()
{
	color = texture(u_Texture,v_TexCoord * m_TilingFactor) * u_Color;
}