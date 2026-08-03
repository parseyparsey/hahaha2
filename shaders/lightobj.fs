#version 460 core
out vec4 FragColor;

struct Material{
    sampler2D texture_diffuse;
    sampler2D texture_specular;
    float shininess;
};

struct dirLight{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec3 direction;
};

struct pointLight{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct spotLight{
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutoff;
    float outercutoff;

    float constant;
    float linear;
    float quadratic;
};

#define PLIGHTNUM 6
uniform Material material;
uniform dirLight dirlight;
uniform pointLight pLight[PLIGHTNUM];
uniform spotLight spotlight;
uniform bool blinn;

in VS_OUT {
    vec3 fragPos;
    vec3 normal;
    vec2 TexCoords;
    vec4 fragPosLightSpace;
    mat3 TBN;
} fs_in;

//in vec2 TexCoords;
//in vec3 normal;
//in vec3 fragPos;

uniform vec3 viewPos;
uniform sampler2D shadowMap;

float near = 0.1;
float far = 100.0;

vec3 calcDirLight(dirLight light, vec3 normal, vec3 viewDir, vec2 TexCoords);
vec3 calcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords);
vec3 calcSpotLight(spotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords);
float lineariseDepth(float depth);
float shadowCalc(vec4 fragPosLightSpace, dirLight light);
float PointShadowCalc(vec3 fragPos, vec3 plightPos);

uniform float far_plane;

uniform samplerCube pDepthMap;

vec4 sFragColor;

//nmap
uniform sampler2D normalMap;
uniform bool useNormalMap;

//pmap
uniform float height_scale;
uniform sampler2D parallaxDepthMap;
uniform sampler2D debug_parallaxDepthMap;
uniform bool useParallaxMap;

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

    vec3 viewDir = normalize(viewPos - fs_in.fragPos);

    vec3 result = calcDirLight(dirlight, norm, viewDir, TexCoords);

    for (int i = 0; i < PLIGHTNUM; i++){
        result += calcPointLight(pLight[i], norm, fs_in.fragPos, viewDir, TexCoords);
    }

    result += calcSpotLight(spotlight, norm, fs_in.fragPos, viewDir, TexCoords);

    FragColor = vec4(result, 1.0);
    //FragColor = vec4(vec3(texture(debug_parallaxDepthMap, fs_in.TexCoords).r), 1.0);
    
    //FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
    //float depth = lineariseDepth(gl_FragCoord.z) / far;
    //FragColor = vec4(vec3(depth), 1.0);

    /*vec3 fragToLight = fs_in.fragPos - pLight[0].position;
    float depth = texture(pDepthMap[0], normalize(fragToLight)).r;
    FragColor = vec4(vec3(depth * 5.0), 1.0);*/
}

vec3 calcDirLight(dirLight light, vec3 normal, vec3 viewDir, vec2 TexCoords){
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;

    if (blinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    }
    else {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular, TexCoords));

    float shadow = shadowCalc(fs_in.fragPosLightSpace, light);

    return (ambient + (1.0 - shadow) * (diffuse + specular));
    
    //return (ambient + diffuse + specular);
    //vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;  
}

vec3 calcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords){
    vec3 lightDir = normalize(light.position - fragPos);
    //diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;

    if (blinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    }
    else {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }

    //attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular, TexCoords));

    //ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    float shadow;

    if (light.position == pLight[0].position)
        shadow = PointShadowCalc(fragPos, light.position);

    if (light.position == pLight[0].position)
        return ambient + (1.0 - shadow) * (diffuse + specular);
    else
        return (ambient + diffuse + specular);
}

vec3 calcSpotLight(spotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords){
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;

    if (blinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    }
    else {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular, TexCoords));

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    //float theta = dot(lightDir, normalize(-light.direction));
    float theta = dot(normalize(-lightDir), normalize(light.direction));
    float epsilon = light.cutoff - light.outercutoff;
    float intensity = clamp((theta - light.outercutoff) / epsilon, 0.0, 1.0);

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}

float lineariseDepth(float depth){
    float z = depth * 2.0 - 1.0; // turn z [0,1] into ndc [-1, 1]
    return (2.0 * near * far) / (far + near - z * (far - near));
}

float shadowCalc(vec4 fragPosLightSpace, dirLight light){

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    vec3 normal = normalize(fs_in.normal);
    vec3 lightDir = normalize(-light.direction);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

vec3 sampleOffsetDirections[20] = vec3[]
(
    vec3( 1, 1, 1), vec3( 1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3( 1, 1, 0), vec3( 1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0, -1), vec3(-1, 0, -1),
    vec3( 0, 1, 1), vec3( 0, -1, 1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float PointShadowCalc(vec3 fragPos, vec3 plightPos)
{

    /*vec3 fragToLight = fragPos - plightPos;

    float closestDepth = texture(pDepthMap, fragToLight).r;
    closestDepth *= far_plane;

    float currentDepth = length(fragToLight);
    
    vec3 norm = normalize(fs_in.normal);
    vec3 lightDir = normalize(fragToLight);
    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);
    float bias = 0.05;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

    sFragColor = vec4(vec3(closestDepth / far_plane), 1.0);

    return shadow;*/

    vec3 fragToLight = fragPos - plightPos;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;

    vec3 norm = normalize(fs_in.normal);
    vec3 lightDir = normalize(fragToLight);
    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);
    //float bias = 0.15;
    
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;

    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(pDepthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

    shadow /= float(samples);

    return shadow;
}

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
    float height =  texture(parallaxDepthMap, texCoords).r;    
    vec2 p = viewDir.xy / viewDir.z * (height * height_scale);
    return texCoords - p; 

    /*
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
    
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(parallaxDepthMap, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;*/
}