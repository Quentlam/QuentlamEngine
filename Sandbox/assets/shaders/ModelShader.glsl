#type vertex
#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_FragPos;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	v_TexCoord = a_TexCoord;
	v_Normal = mat3(transpose(inverse(u_Transform))) * a_Normal;
	v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0));
	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 330 core

layout(location = 0) out vec4 color;		

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_FragPos;

uniform vec4 u_Color;

void main()
{
	// Simple directional lighting
	vec3 norm = normalize(v_Normal);
	vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0));
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1.0);
	
	vec3 ambient = vec3(0.2);
	
	vec3 result = (ambient + diffuse) * u_Color.rgb;
	color = vec4(result, u_Color.a);
}
