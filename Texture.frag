#version 400 core
// Input
in vec2 te_UV;
in vec3 te_normal_wcs;
in vec3 te_lightDir_tcs;
in vec3 te_viewDir_tcs;
in float te_varyingHeight;




// Output
out vec3 color;
//Uniforms
uniform sampler2D rockDiffSampler;
uniform sampler2D rockSpecSampler;
uniform sampler2D rockNormSampler;

uniform sampler2D grassDiffSampler;
uniform sampler2D grassSpecSampler;
uniform sampler2D grassNormSampler;

uniform sampler2D snowDiffSampler;
uniform sampler2D snowSpecSampler;
uniform sampler2D snowNormSampler;

vec3 getPhong(vec3 diffuseColor, vec3 specularDetail, vec3 normalDetail)
{
    // get normal (TCS)
    vec3 normal = normalize(normalDetail * 2.0 - 1.0);
    vec3 normal_tcs = normalize(normal);

    // get half vector (TCS)
    vec3 halfVector_tcs = normalize(te_lightDir_tcs + te_viewDir_tcs);

    // ambient
    vec3 ambient = diffuseColor * 0.1;

    // diffuse
    float diff = max(dot(normal_tcs, te_lightDir_tcs), 0.0);
    vec3 diffuse = diff * diffuseColor;

    // specular
    vec3 specularColour = vec3(0.3, 0.3, 0.3);
    float roughness = specularDetail.x;
    float shininess = clamp((2/(pow(roughness,4)+1e-2))-2,0,500.0f);
    float specularIntensity = pow(max(dot(normal_tcs, halfVector_tcs), 0.0), shininess);
    vec3 specular = specularIntensity * specularColour;

    // phong color
    //return ambient + diffuse + specular;

    // just diffuse
     return diffuse;

    // just specular
     return specular;
}

void main()
{
	// Task2
	// color = vec3(abs(normal_wcs.x), abs(normal_wcs.y), abs(normal_wcs.z));

    // from normal mapping
//    vec3 rockNormDetail = texture(rockNormSampler, UV * 10).rgb;
//    vec3 normal = normalize(rockNormDetail * 2.0 - 1.0);
//    color = vec3(abs(normal.x), abs(normal.y), abs(normal.z));
	
    vec3 rockDiff = texture(rockDiffSampler, te_UV * 10).rgb;
    vec3 rockSpecDetail = texture(rockSpecSampler, te_UV * 10).rgb;
    vec3 rockNormDetail = texture(rockNormSampler, te_UV * 10).rgb;
    
    vec3 grassDiff = texture(grassDiffSampler, te_UV * 20).rgb;
    vec3 grassSpecDetail = texture(grassSpecSampler, te_UV * 20).rgb;
    vec3 grassNormDetail = texture(grassNormSampler, te_UV * 20).rgb;

    vec3 snowDiff = texture(snowDiffSampler, te_UV * 10).rgb;
    vec3 snowSpecDetail = texture(snowSpecSampler, te_UV * 10).rgb;
    vec3 snowNormDetail = texture(snowNormSampler, te_UV * 10).rgb;

    vec3 rockPhongColor = getPhong(rockDiff, rockSpecDetail, rockNormDetail);
    vec3 grassPhongColor = getPhong(grassDiff, grassSpecDetail, grassNormDetail);
    vec3 snowPhongColor = getPhong(snowDiff, snowSpecDetail, snowNormDetail);

    float currHeight = te_varyingHeight;
    float rockHeight = 1;
    float snowHeight = 2;
    
    if(currHeight < 0.0f + 0.001f){
        color = grassPhongColor;
    }
    else if (currHeight > 0 && currHeight <= rockHeight + 0.25)
    {
        float blendRock = smoothstep(rockHeight - 0.25, rockHeight + 0.25, currHeight);
        float blendGrass = 1 - blendRock;
        color = blendGrass * grassPhongColor + blendRock * rockPhongColor;
    } 
    else if (currHeight <= snowHeight + 0.25)
    {
        float blendSnow = smoothstep(snowHeight - 0.25, snowHeight + 0.25, currHeight);
        float blendRock = 1 - blendSnow;
        color = blendSnow * snowPhongColor + blendRock * rockPhongColor;
    }
    else
    {
        color = snowPhongColor;
    }
    






}
