#version 460 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

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
uniform dirLight dirlight;
uniform pointLight pLight[PLIGHTNUM];
uniform spotLight spotlight;
uniform bool blinn;

vec3 calcDirLight(dirLight light, vec3 normal, vec3 viewDir, vec2 TexCoords);
vec3 calcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords);
vec3 calcSpotLight(spotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords);
float lineariseDepth(float depth);
float shadowCalc(vec4 fragPosLightSpace, dirLight light);
float PointShadowCalc(vec3 fragPos, vec3 plightPos);

uniform vec3 viewPos;
uniform sampler2D shadowMap;

float near = 0.1;
float far = 100.0;

uniform float far_plane;

uniform samplerCube pDepthMap;

uniform float shininess;

vec4 fragPosLightSpace;
uniform mat4 LightSpaceMatrix;

void main(){

    vec3 normalRaw = texture(gNormal, TexCoords).rgb;
    if (dot(normalRaw, normalRaw) < 0.0001) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    fragPosLightSpace = LightSpaceMatrix * vec4(fragPos, 1.0);

    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 result = calcDirLight(dirlight, normal, viewDir, TexCoords);

    for(int i = 0; i < PLIGHTNUM; i++)
        result += calcPointLight(pLight[i], normal, fragPos, viewDir, TexCoords);

    result += calcSpotLight(spotlight, normal, fragPos, viewDir, TexCoords);

    FragColor = vec4(result, 1.0);

}

vec3 calcDirLight(dirLight light, vec3 normal, vec3 viewDir, vec2 TexCoords){
    vec3 lightDir = normalize(-light.direction);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;

    if (blinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    }
    else {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    }

    vec4 AlbedoSpec = texture(gAlbedoSpec, TexCoords);

    vec3 ambient = light.ambient * vec3(AlbedoSpec.rgb);
    vec3 diffuse = light.diffuse * diff * vec3(AlbedoSpec.rgb);
    vec3 specular = light.specular * spec * vec3(AlbedoSpec.a);

    float shadow = shadowCalc(fragPosLightSpace, light);

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
        spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    }
    else {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    }

    //attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec4 AlbedoSpec = texture(gAlbedoSpec, TexCoords);

    vec3 ambient = light.ambient * vec3(AlbedoSpec.rgb);
    vec3 diffuse = light.diffuse * diff * vec3(AlbedoSpec.rgb);
    vec3 specular = light.specular * spec * vec3(AlbedoSpec.a);

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
        spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    }
    else {
        spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    }

    vec4 AlbedoSpec = texture(gAlbedoSpec, TexCoords);

    vec3 ambient = light.ambient * vec3(AlbedoSpec.rgb);
    vec3 diffuse = light.diffuse * diff * vec3(AlbedoSpec.rgb);
    vec3 specular = light.specular * spec * vec3(AlbedoSpec.a);

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

    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
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

    vec3 norm = normalize(texture(gNormal, TexCoords).rgb);
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