#version 460 core
out vec4 FragColor;

#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16  

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

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 fragPosLightSpace;

uniform vec3 viewPos;

uniform dirLight dirlight;
uniform pointLight pLights[MAX_POINT_LIGHTS];
uniform int pl_num;
uniform spotLight spotlight[MAX_SPOT_LIGHTS];
uniform int sl_num;

uniform bool blinn;
uniform Material material;

uniform sampler2D dirShadowMap;

#define MAX_SPOT_SHADOWS 4
uniform sampler2D spotShadowMap[MAX_SPOT_SHADOWS];
uniform mat4 spotLightSpace[MAX_SPOT_SHADOWS];

vec3 calcDirLight(dirLight light, vec3 normal, vec3 viewDir, vec2 TexCoords);
vec3 calcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords);
vec3 calcSpotLight(spotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords, float shadow);
float calcDirShadow(vec4 fragPosLightSpace, dirLight light);
float calcSpotShadow(int i, vec3 fragPos, vec3 normal,vec3 lightDir);

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result;

    result = calcDirLight(dirlight, norm, viewDir, TexCoords);

    for(int i = 0; i < pl_num; i++)
        result += calcPointLight(pLights[i], norm, FragPos, viewDir, TexCoords);

    for(int i = 0; i < sl_num; i++){
        float shadow = calcSpotShadow(i, FragPos, norm, normalize(spotlight[i].position - FragPos));
        result += calcSpotLight(spotlight[i], norm, FragPos, viewDir, TexCoords, shadow);
    }

    FragColor = vec4(result, 1.0);
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

    float shadow = calcDirShadow(fragPosLightSpace, light);

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

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    //float shadow;

    /*if (light.position == pLight[0].position)
        shadow = PointShadowCalc(fragPos, light.position);

    if (light.position == pLight[0].position)
        return ambient + (1.0 - shadow) * (diffuse + specular);
    else
        return (ambient + diffuse + specular);*/
    
    return (ambient + diffuse + specular);
}

vec3 calcSpotLight(spotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords, float shadow){
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

    return (ambient + (1.0 - shadow) * (diffuse + specular));
    //return (ambient + diffuse + specular);
}

float calcDirShadow(vec4 fragPosLightSpace, dirLight light){

    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(dirShadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(-light.direction);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(dirShadowMap, 0);

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(dirShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    if (projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

float calcSpotShadow(int i, vec3 fragPos, vec3 normal,vec3 lightDir){
    vec4 lp = spotLightSpace[i] * vec4(fragPos, 1.0);
    vec3 proj = lp.xyz / lp.w;
    proj = proj * 0.5 + 0.5;

    if (proj.z > 1.0 || proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0)
        return 0.0; // outside the shadow frustum: treat as lit

    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.0005);
    vec2 texel = 1.0 / textureSize(spotShadowMap[i], 0);
    float shadow = 0.0;
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y) {
            float d = texture(spotShadowMap[i], proj.xy + vec2(x, y) * texel).r;
            shadow += (proj.z - bias > d) ? 1.0 : 0.0;
        }
    return shadow / 9.0;
}