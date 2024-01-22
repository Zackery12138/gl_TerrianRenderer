#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition_ocs;
layout(location = 1) in vec2 vertexUV;
// Output data
out vec2 UV;
out mat3 TBN;
out vec3 normal_wcs;
out vec3 lightDir_tcs;
out vec3 viewDir_tcs;
out float varyingHeight;
// Uniforms
uniform mat4 MVP;
uniform mat4 Model;
uniform sampler2D heightMapSampler;
uniform float heightMapScale;
uniform int numOfVertices;
uniform vec3 lightDir_wcs;
uniform vec3 viewPos_wcs;

// Function to get the height value at specific texture coordinates uv from a height map
float getHeightFromHeightMap(vec2 uv)
{
    vec3 heightRGB = texture(heightMapSampler, uv).rgb;

    int dataByR = int(heightRGB.x * 255.0) << 16;
    int dataByG = int(heightRGB.y * 255.0) << 8;
    int dataByB = int(heightRGB.z * 255.0);

    // Height value scaling: Convert the normalized value read from the height map to the actual height value
    return (dataByR + dataByG + dataByB) * heightMapScale;
}

void main()
{
    // Task 1
    float height = getHeightFromHeightMap(vertexUV);

    // Copy and modify the vertex position
    vec3 position_ocs = vertexPosition_ocs;
    position_ocs.y += height;

    // Calculate and store the vertex position in clip space
    gl_Position = MVP * vec4(position_ocs, 1);

    // Calculate and store the vertex position in world coordinate system
    vec3 position_wcs = (Model * vec4(position_ocs, 1)).xyz;
    UV = vertexUV;

    // Task 2
    // Calculate the pixel size for each vertex in the height map texture
    vec2 pixelSize = 1.0 / textureSize(heightMapSampler, 0) * numOfVertices;

    // Estimate the surface normal at texture coordinate UV based on the height map
    float l  = getHeightFromHeightMap(UV - vec2(pixelSize.x, 0.0));
    float r  = getHeightFromHeightMap(UV + vec2(pixelSize.x, 0.0));
    float b  = getHeightFromHeightMap(UV - vec2(0.0, pixelSize.y));
    float t  = getHeightFromHeightMap(UV + vec2(0.0, pixelSize.y));

    // Calculate the horizontal and vertical height gradients at texture coordinate UV, a step in computing the normal vector
    float dx = (l - r) / (2.0 * pixelSize.x);
    float dz = (t - b) / (2.0 * pixelSize.y);

    // Generate and transform the surface normal
    vec3 normal = normalize(vec3(dx, 1, dz));
    normal_wcs = (Model * vec4(normal, 0)).xyz;

    // TASK 4
    vec3 N = normalize(normal);
    vec3 B = vec3(1, 0, 0);
    vec3 T = vec3(0, 0, 1);

    // Graham-Schmidt
    T = normalize(T - dot(T, N) * N);
    B = normalize(B - dot(B, N) * N - dot(B, T) * T);

    TBN = mat3(T, B, N);

    lightDir_tcs = TBN * (-lightDir_wcs);
    viewDir_tcs = TBN * (viewPos_wcs - position_wcs);
    
    // Task 5
    varyingHeight = height;
}
