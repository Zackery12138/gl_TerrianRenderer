#version 400 core

in vec2 tc_UV[];
in vec3 tc_normal_wcs[];
in vec3 tc_lightDir_tcs[];
in vec3 tc_viewDir_tcs[];
in float tc_varyingHeight[];

out vec2 te_UV;
out vec3 te_normal_wcs;
out vec3 te_lightDir_tcs;
out vec3 te_viewDir_tcs;
out float te_varyingHeight;


void main(){

// ��ֵ���������
    te_UV = tc_UV[0]; 
    te_normal_wcs = tc_normal_wcs[0]; 
    te_lightDir_tcs = tc_lightDir_tcs[0]; 
    te_viewDir_tcs = tc_viewDir_tcs[0]; 
    te_varyingHeight = tc_varyingHeight[0];  

    // �������ն���λ�ã�ͨ������ԭʼ��������
    gl_Position = gl_in[0].gl_Position;
}