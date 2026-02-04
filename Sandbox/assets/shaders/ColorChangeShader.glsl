#type vertex
#version 330 core
layout(location = 0)in vec3 a_Position;		
layout(location = 1)in vec4 a_Color;
		
uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 Color;
void main()
{
	Color = a_Color;
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position,1.0);
}

#type fragment
#version 330 core
in vec4 Color;
out vec4 color;		
					

void main()
{
	color = vec4(Color);
}


