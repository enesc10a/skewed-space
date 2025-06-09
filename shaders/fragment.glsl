#version 410 core
in vec3 fNormal;
in vec2 fTex;
flat in vec4 fColor;          // matches ‘flat’ qualifier in VS

uniform sampler2D uTexture;
uniform int  uShadingMode;    // 0 = Gouraud, 1 = Phong
uniform bool uTexturing;      // true in TEXTURED mode
uniform vec3 uLightDir;       // eye-space
uniform vec3 uKa, uKd, uKs;
uniform float uShininess;
uniform bool uUseAmb, uUseDiff, uUseSpec;

out vec4 FragColor;

void main()
{
    ////////////////////////////////////////////////////////////////////////////
    //  Gouraud: colour already lit in the vertex shader
    ////////////////////////////////////////////////////////////////////////////
    if (uShadingMode == 0)
    {
        vec4 base = uTexturing ? texture(uTexture, fTex)
                               : vec4(1.0);

        FragColor = base * fColor;
        return;
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Phong: compute per-fragment lighting
    ////////////////////////////////////////////////////////////////////////////
    vec3 N = normalize(fNormal);
    vec3 L = normalize(uLightDir);
    vec3 V = vec3(0.0, 0.0, 1.0);      // eye points down –Z in eye space
    vec3 R = reflect(-L, N);

    vec3 ambient  = uKa;
    vec3 diffuse  = uKd * max(dot(N, L), 0.0);
    vec3 specular = uKs * pow(max(dot(R, V), 0.0), uShininess);

    vec3 lit = (uUseAmb ? ambient  : vec3(0.0)) +
               (uUseDiff? diffuse  : vec3(0.0)) +
               (uUseSpec? specular : vec3(0.0));

    vec4 base = uTexturing ? texture(uTexture, fTex)
                           : vec4(1.0);

    FragColor = base * vec4(lit, 1.0);
}
