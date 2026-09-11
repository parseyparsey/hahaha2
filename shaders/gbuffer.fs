#version 460 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

struct Material{
    sampler2D texture_diffuse;
    sampler2D texture_specular;
};

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 TexCoords;
    mat3 TBN;
} fs_in;

uniform Material material;

uniform sampler2D normalMap;
uniform bool useNormalMap;

uniform float height_scale;
uniform sampler2D parallaxDepthMap;
//uniform sampler2D debug_parallaxDepthMap;
uniform bool useParallaxMap;

uniform vec3 viewPos;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir);

void main()
{
    vec3 norm;
    vec2 TexCoords = fs_in.TexCoords;
    vec3 tangentViewDir;

    if(useParallaxMap){
        tangentViewDir = normalize(transpose(fs_in.TBN) * (viewPos - fs_in.fragPos));
        TexCoords = ParallaxMapping(TexCoords, tangentViewDir);

        if(TexCoords.x > 1.0 || TexCoords.y > 1.0 || TexCoords.x < 0.0 || TexCoords.y < 0.0)
            discard;
    }

    if(useNormalMap){
        vec3 normalMapSample = texture(normalMap, TexCoords).rgb;
        normalMapSample = normalMapSample * 2.0 - 1.0;
        norm = normalize(fs_in.TBN * normalMapSample);
    } else {
        norm = normalize(fs_in.normal);
    }


    gPosition = fs_in.fragPos;

    gNormal = norm;

    gAlbedoSpec.rgb = texture(material.texture_diffuse, TexCoords).rgb;

    gAlbedoSpec.a = texture(material.texture_specular, TexCoords).r;

}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
    /*float height =  texture(parallaxDepthMap, texCoords).r;    
    vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    return texCoords - p; */
    
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * height_scale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(parallaxDepthMap, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(parallaxDepthMap, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }

    //return currentTexCoords;
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(parallaxDepthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}