#version 400 core

// tessellation control shader
layout (vertices = 3) out;

in vec2 UV[];
in vec3 normal_wcs[];
in vec3 lightDir_tcs[];
in vec3 viewDir_tcs[];
in float varyingHeight[];

out vec2 tc_UV[];
out vec3 tc_normal_wcs[];
out vec3 tc_lightDir_tcs[];
out vec3 tc_viewDir_tcs[];
out float tc_varyingHeight[];




void main()
{
	//transfer the data
	tc_UV[gl_InvocationID] = UV[gl_InvocationID];
	tc_normal_wcs[gl_InvocationID] = normal_wcs[gl_InvocationID];
	tc_lightDir_tcs[gl_InvocationID] = lightDir_tcs[gl_InvocationID];
	tc_viewDir_tcs[gl_InvocationID] = viewDir_tcs[gl_InvocationID];
	tc_varyingHeight[gl_InvocationID] = varyingHeight[gl_InvocationID]; 

	if(gl_InvocationID == 0){
		gl_TessLevelInner[0] = 1.0;
		gl_TessLevelOuter[0] = 1.0f;
		gl_TessLevelOuter[1] = 1.0f;
		gl_TessLevelOuter[2] = 1.0f;

	}


}
