#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D ourTexture;
uniform float width;
uniform int fbmode;

uniform bool hdr;
uniform float exposure;
uniform bool bloom;
uniform sampler2D bloomTex;

const float offset = 1.0 / 300.0;

#define KERNEL 1
#define BLUR 2
#define EDGE 3

vec4 Grayscale(vec4 fragColor);
vec4 Negative(vec4 fragColor);
vec4 Normal();
vec4 Kernel(int mode);
vec4 BlurredGreyscale();
vec4 Halfcolor();

vec3 toneMapping(vec3 hdrColor){
    const float gamma = 2.2;

    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);

    mapped = pow(mapped, vec3(1.0 / gamma));

    return mapped;
}

vec4 mappedfin;

void main()
{

    if (hdr && fbmode < 3){
        /*const float gamma = 2.2;
        vec3 hdrColor = texture(ourTexture, TexCoords).rgb;
        vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
        mapped = pow(mapped, vec3(1.0 / gamma));*/

        vec3 hdrColor = texture(ourTexture, TexCoords).rgb;
        if(bloom)
            hdrColor += texture(bloomTex, TexCoords).rgb;
        mappedfin = vec4(toneMapping(hdrColor), 1.0);
        //mappedfin = vec4(toneMapping(texture(ourTexture, TexCoords).rgb), 1.0);
    } else if (fbmode < 3){
        mappedfin = texture(ourTexture, TexCoords);
        if(bloom)
            mappedfin += texture(bloomTex, TexCoords);
    }

    switch(fbmode){
        case 0: FragColor = mappedfin;/*Normal();*/ break;
        case 1: FragColor = Grayscale(mappedfin); break;
        case 2: FragColor = Negative(mappedfin); break;
        case 3: FragColor = Kernel(1); break;
        case 4: FragColor = Kernel(2); break;
        case 5: FragColor = Kernel(3); break;
        case 6: FragColor = BlurredGreyscale(); break;
        case 7: FragColor = Halfcolor(); break;
    }

    float gamma = 2.2;
    //FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}

vec4 Grayscale(vec4 fragColor){
    //vec4 fragColor = texture(ourTexture, TexCoords);
    float average = 0.2126 * fragColor.r + 0.7152 * fragColor.g +
                    0.0722 * fragColor.b;
    return vec4(average, average, average, 1.0);
}

vec4 Negative(vec4 fragColor){
    return vec4(vec3(1.0 - fragColor.rgb), 1.0);
}

vec4 Normal(){
    return texture(ourTexture, TexCoords);
}

vec4 Kernel(int mode){
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f), // center-left
        vec2( 0.0f,    0.0f), // center-center
        vec2( offset,  0.0f), // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset) // bottom-right
    );

    float kernel[9];

    switch(mode){
        case KERNEL: kernel = float[](
                    -1, -1, -1,
                    -1, 9, -1,
                    -1, -1, -1
                );
                break;
        case BLUR: kernel = float[](
                    1.0 / 16, 2.0 / 16, 1.0 / 16,
                    2.0 / 16, 4.0 / 16, 2.0 / 16,
                    1.0 / 16, 2.0 / 16, 1.0 / 16
                );
                break;
        case EDGE: kernel = float[](
                    1,  1, 1,
                    1, -8, 1,
                    1,  1, 1
                );
    }

    /*vec3 sampleTex[9];
    for (int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(ourTexture, TexCoords.st + 
                                    offsets[i]));
    }*/

    vec3 col = vec3(0.0);
    for (int i = 0; i < 9; i++){
        vec3 fragsample = texture(ourTexture, TexCoords + offsets[i]).rgb;

        if (bloom)
            fragsample += texture(bloomTex, TexCoords + offsets[i]).rgb;

        if (hdr)
            fragsample = toneMapping(fragsample);
        col += fragsample * kernel[i];
    }
        
        //col += sampleTex[i] * kernel[i];

    return vec4(col, 1.0);
}

vec4 BlurredGreyscale(){
    vec4 blur = Kernel(2);
    float average = 0.2126 * blur.r + 0.7152 * blur.g +
                    0.0722 * blur.b;
    return vec4(average, average, average, 1.0);
}

vec4 Halfcolor(){
    vec3 color = texture(ourTexture, TexCoords).rgb;
    if (bloom)
        color += texture(bloomTex, TexCoords).rgb;
    vec4 fragColor;
    if (gl_FragCoord.x < width / 2.0){
        fragColor = Grayscale(vec4((hdr?toneMapping(color):color), 1.0));//BlurredGreyscale();
    } else {
        fragColor = vec4((hdr?toneMapping(color):color), 1.0);//Kernel(2);
    }
    return fragColor;
}
