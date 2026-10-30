#version 460 core
out vec4 FragColor;

#define MAX_POINT_LIGHTS 16
#define MAX_SPOT_LIGHTS 16  

#define BLINNPHONG 0
#define PBR 1

struct Material{
    sampler2D texture_diffuse;
    sampler2D texture_specular;
    float shininess;
};

struct PBRMaterial{
    sampler2D albedoMap;
    sampler2D metallicMap;
    sampler2D roughnessMap;
    sampler2D aoMap;

    float metallicFactor;
    float roughnessFactor;
};

struct Surface{
    vec3 pos, N, Ng;
    vec3 albedo;

    //BLINN-PHONG
    float specMask, shininess;

    //PBR
    float metallic, roughness, ao;
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
uniform PBRMaterial pbrMat;

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

uniform int shadingModel;

float calcDirShadow(vec4 fragPosLightSpace, dirLight light);
float calcSpotShadow(int i, vec3 fragPos, vec3 normal,vec3 lightDir);
float CalcPointShadow(int i, vec3 fragPos, vec3 lightPos, vec3 normal);
vec2 ParallaxMapping(vec2 uv, vec3 v);

const float PI = 3.14159265359;

//PBR FUNCTIONS
vec3 brdfPBR(Surface s, vec3 L, vec3 V, vec3 radiance);
vec3 brdfBlinnPhong(Surface s, vec3 L, vec3 V, vec3 radiance);
vec3 shade(Surface s, vec3 V);
vec3 evalBRDF(Surface s, vec3 L, vec3 V, vec3 radiance);

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

    Surface s;
    s.pos = FragPos;
    s.N = norm;
    s.Ng = normalize(Normal);
    s.ao = 1.0;

    s.albedo = texture(material.texture_diffuse, uv).rgb;

    if (shadingModel == BLINNPHONG) {
        s.specMask = texture(material.texture_specular, uv).r;
        s.shininess = material.shininess;
    }
    else if (shadingModel == PBR) {
        s.metallic = texture(pbrMat.metallicMap, uv).r * pbrMat.metallicFactor;
        s.roughness = clamp(texture(pbrMat.roughnessMap, uv).r * pbrMat.roughnessFactor, 0.04, 1.0);
        s.ao = texture(pbrMat.aoMap, uv).r;
    }
    FragColor = vec4(shade(s, viewDir), 1.0);
}

vec3 brdfPBR(Surface s, vec3 L, vec3 V, vec3 radiance) {
    vec3 H = normalize(V + L);

    float NDotL = max(dot(s.N, L), 0.0);
    float NDotV = max(dot(s.N, V), 1e-4);
    float NdotH = max(dot(s.N, H), 0.0);

    float a = s.roughness * s.roughness;
    float a2 = a * a;
    float dd = NdotH * NdotH * (a2 - 1.0) + 1.0;
    float D = a2 / (PI * dd * dd);

    float k = (s.roughness + 1.0) * (s.roughness + 1.0) / 8.0;
    float G = (NDotV / (NDotV * (1.0 - k) + k)) * 
              (NDotL / (NDotL * (1.0 - k) + k));
    
    vec3 F0 = mix(vec3(0.04), s.albedo, s.metallic);
    vec3 F = F0 + (1.0 - F0) * pow(clamp(1.0 - max(dot(H, V), 0.0), 0.0, 1.0), 5.0);

    vec3 spec = (D * G * F) / (4.0 * NDotV * NDotL + 1e-4);
    vec3 kD = (1.0 - F) * (1.0 - s.metallic);

    return (kD * s.albedo / PI + spec) * radiance * NDotL;
}

vec3 brdfBlinnPhong(Surface s, vec3 L, vec3 V, vec3 radiance) {
    float diff = max(dot(s.N, L), 0.0);
    float spec = blinn ? pow(max(dot(s.N, normalize(L + V)), 0.0), s.shininess)
                       : pow(max(dot(V, reflect(-L, s.N)), 0.0), s.shininess);

    return (s.albedo * diff + vec3(s.specMask * spec)) * radiance;
}

vec3 evalBRDF(Surface s, vec3 L, vec3 V, vec3 radiance) {
    return (shadingModel == BLINNPHONG) ? brdfBlinnPhong(s, L, V, radiance)
                                        : brdfPBR(s, L, V, radiance);
}

vec3 shade(Surface s, vec3 V) {
    vec3 Lo = vec3(0.0), ambient = vec3(0.0);

    vec3 Ld = normalize(-dirlight.direction);
    Lo += evalBRDF(s, Ld, V, dirlight.diffuse * (1.0 - calcDirShadow(fragPosLightSpace, dirlight)));
    ambient += dirlight.ambient * s.albedo;

    for (int i = 0; i < pl_num; i++) {
        pointLight l = pLights[i];
        vec3 L = normalize(l.position - s.pos);
        float d = length(l.position - s.pos);
        float att = 1.0 / (l.constant + l.linear * d + l.quadratic * d * d);
        float sh = (i < pointShadowCount) ? CalcPointShadow(i, s.pos, l.position, s.Ng) : 0.0;
        Lo += evalBRDF(s, L, V, l.diffuse * att * (1.0 - sh));
        ambient += l.ambient * att;
    }

    for (int i = 0; i < sl_num; i++) {
        spotLight l = spotlight[i];
        vec3 L = normalize(l.position - s.pos);
        float d = length(l.position - s.pos);
        float att = 1.0 / (l.constant + l.linear * d + l.quadratic * d * d);
        float theta = dot(-L, normalize(l.direction));
        float cone = clamp((theta - l.outercutoff) / (l.cutoff - l.outercutoff), 0.0, 1.0);
        float sh = (i < sl_num) ? calcSpotShadow(i, s.pos, s.Ng, L) : 0.0;
        Lo += evalBRDF(s, L, V, l.diffuse * att * cone * (1.0 - sh));
        ambient += l.ambient * att * cone;
    }

    return ambient * s.albedo * s.ao + Lo;
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
    float numLayers = mix(64.0, 8.0, abs(v.z));
    float layerDepth = 1.0 / numLayers;
    vec2 delta = (v.xy / max(v.z, 0.05) * height_scale) / numLayers;

    vec2 cur = uv;
    float curDepth = 0.0;
    float h = textureLod(parallaxDepthMap, cur, 0.0).r;
    for (int i = 0; i < 64 && curDepth < h; i++) {
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