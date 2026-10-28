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
in mat3 TBN;

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

#define MAX_POINT_SHADOWS 2
uniform samplerCube pointShadowMap[MAX_POINT_SHADOWS];
uniform int pointShadowCount;
uniform float pointFarPlane;

uniform sampler2D normalMap;
uniform bool useNormalMap;

uniform float height_scale;
uniform sampler2D parallaxDepthMap;
uniform bool useParallaxMap;
uniform bool parallaxClip;

vec3 calcDirLight(dirLight light, vec3 normal, vec3 viewDir, vec2 TexCoords);
vec3 calcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords, float shadow);
vec3 calcSpotLight(spotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords, float shadow);
float calcDirShadow(vec4 fragPosLightSpace, dirLight light);
float calcSpotShadow(int i, vec3 fragPos, vec3 normal,vec3 lightDir);
float CalcPointShadow(int i, vec3 fragPos, vec3 lightPos, vec3 normal);
vec2 ParallaxMapping(vec2 uv, vec3 v);

void main() {
    vec2 uv = TexCoords;

    if (useParallaxMap) {
        uv = ParallaxMapping(uv, normalize(transpose(TBN) * (viewPos - FragPos)));
        if (parallaxClip && (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)) discard;
    }

    vec3 norm = normalize(Normal);

    if (useNormalMap)
        norm = normalize(TBN * (texture(normalMap, uv).rgb * 2.0 - 1.0));

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result;

    result = calcDirLight(dirlight, norm, viewDir, uv);

    for(int i = 0; i < pl_num; i++) {
        float shadow = (i < pointShadowCount)
            ? CalcPointShadow(i, FragPos, pLights[i].position, Normal)
            : 0.0;
        result += calcPointLight(pLights[i], norm, FragPos, viewDir, uv, shadow);
    }

    for(int i = 0; i < sl_num; i++){
        //float shadow = calcSpotShadow(i, FragPos, norm, normalize(spotlight[i].position - FragPos));
        float shadow = (i < sl_num)
            ? calcSpotShadow(i, FragPos, norm, normalize(spotlight[i].position - FragPos))
            : 0.0;
        result += calcSpotLight(spotlight[i], norm, FragPos, viewDir, uv, shadow);
    }

    FragColor = vec4(result, 1.0);
    
    //vec3 pc = fragPosLightSpace.xyz / fragPosLightSpace.w * 0.5 + 0.5;
    //FragColor = vec4(pc.xy, 0.0, 1.0);
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

vec3 calcPointLight(pointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec2 TexCoords, float shadow){
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
    
    return ambient + (1.0 - shadow) * (diffuse + specular);
    //return (ambient + diffuse + specular);
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

vec3 sampleOffsetDirections[20] = vec3[]
(
    vec3( 1, 1, 1), vec3( 1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
    vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3( 1, 1, 0), vec3( 1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3( 1, 0, 1), vec3(-1, 0, 1), vec3( 1, 0, -1), vec3(-1, 0, -1),
    vec3( 0, 1, 1), vec3( 0, -1, 1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float CalcPointShadow(int i, vec3 fragPos, vec3 lightPos, vec3 normal) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    if (currentDepth > pointFarPlane) return 0.0;

    float shadow = 0.0;

    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - fragPos);//normalize(fragToLight);
    float bias = max(0.05 * (1.0 - dot(norm, lightDir)), 0.005);
    //float bias = 0.15;
    
    int samples = 20;
    float viewDistance = length(viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / pointFarPlane)) / 25.0;

    for(int j = 0; j < samples; ++j)
    {
        float closestDepth = texture(pointShadowMap[i], fragToLight + sampleOffsetDirections[j] * diskRadius).r;
        closestDepth *= pointFarPlane;
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }

    shadow /= float(samples);

    return shadow;
}

vec2 ParallaxMapping(vec2 uv, vec3 v){
    float numLayers = mix(32.0, 8.0, abs(v.z));
    float layerDepth = 1.0 / numLayers;
    vec2 delta = (v.xy / max(v.z, 0.05) * height_scale) / numLayers;

    vec2 cur = uv;
    float curDepth = 0.0;
    float h = textureLod(parallaxDepthMap, cur, 0.0).r;
    for (int i = 0; i < 32 && curDepth < h; i++) {
        cur -= delta;
        h = textureLod(parallaxDepthMap, cur, 0.0).r;
        curDepth += layerDepth;
    }
    vec2 prev = cur + delta;
    float after  = h - curDepth;
    float before = textureLod(parallaxDepthMap, prev, 0.0).r - curDepth + layerDepth;
    float w = after / (after - before);
    return prev * w + cur * (1.0 - w);
}