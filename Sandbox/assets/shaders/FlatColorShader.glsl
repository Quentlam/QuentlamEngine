#type vertex
#version 330 core
layout(location = 0)in vec3 u_Position;
layout(location = 1)in vec2 TexCoord;

out vec2 v_TexCoord;
out vec4 v_Color;
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;
uniform vec4 u_Color;
		 
void main()
{
	v_Color = u_Color;
	v_TexCoord = TexCoord;
	gl_Position = u_ViewProjection * u_Transform * vec4(u_Position,1.0);
}


#type fragment
#version 330 core
out vec4 color;		

in vec4 v_Color;		
in vec2 v_TexCoord;
uniform sampler2D u_Texture;

void main()
{
	color = v_Color;//texture(u_Texture,v_TexCoord);
}