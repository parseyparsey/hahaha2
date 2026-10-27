#version 460 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D screenTex;

#define REDUCE_MIN (1.0 / 128.0)
#define REDUCE_MUL (1.0 / 8.0)
#define SPAN_MAX   8.0

float luma(vec3 c) { return dot(c, vec3(0.299, 0.587, 0.114)); }

void main() {
    vec2 texel = 1.0 / vec2(textureSize(screenTex, 0));

    vec3 rgbM  = texture(screenTex, TexCoords).rgb;
    vec3 rgbNW = texture(screenTex, TexCoords + vec2(-1.0, -1.0) * texel).rgb;
    vec3 rgbNE = texture(screenTex, TexCoords + vec2( 1.0, -1.0) * texel).rgb;
    vec3 rgbSW = texture(screenTex, TexCoords + vec2(-1.0,  1.0) * texel).rgb;
    vec3 rgbSE = texture(screenTex, TexCoords + vec2( 1.0,  1.0) * texel).rgb;

    float lM = luma(rgbM), lNW = luma(rgbNW), lNE = luma(rgbNE);
    float lSW = luma(rgbSW), lSE = luma(rgbSE);
    float lMin = min(lM, min(min(lNW, lNE), min(lSW, lSE)));
    float lMax = max(lM, max(max(lNW, lNE), max(lSW, lSE)));

    vec2 dir;
    dir.x = -((lNW + lNE) - (lSW + lSE));
    dir.y =  ((lNW + lSW) - (lNE + lSE));

    float dirReduce = max((lNW + lNE + lSW + lSE) * (0.25 * REDUCE_MUL), REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, vec2(-SPAN_MAX), vec2(SPAN_MAX)) * texel;

    vec3 rgbA = 0.5 * (texture(screenTex, TexCoords + dir * (1.0 / 3.0 - 0.5)).rgb +
                       texture(screenTex, TexCoords + dir * (2.0 / 3.0 - 0.5)).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (texture(screenTex, TexCoords + dir * -0.5).rgb +
                                      texture(screenTex, TexCoords + dir *  0.5).rgb);

    float lB = luma(rgbB);
    FragColor = vec4((lB < lMin || lB > lMax) ? rgbA : rgbB, 1.0);
}