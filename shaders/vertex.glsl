#version 410 core
layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform int  uShadingMode;      // 0 = Gouraud, 1 = Phong
uniform vec3 uLightDir;         // already in eye space
uniform vec3 uKa, uKd, uKs;
uniform float uShininess;
uniform bool uUseAmb, uUseDiff, uUseSpec;

out vec3 fNormal;
out vec2 fTex;
flat out vec4 fColor;           // ‘flat’ so the colour for each vertex is
                                // *not* re-interpolated in Gouraud

void main()
{
    // 1. Position in eye space
    vec4 posEye = view * model * vec4(vPosition, 1.0);
    gl_Position = projection * posEye;

    // 2. Normal -> eye space (use 3×3 part; assumes no non-uniform scale)
    fNormal = normalize(mat3(view * model) * vNormal);

    // 3. Pass texture coordinate straight through
    fTex = vTex;

    // 4. Optional per-vertex lighting
    if (uShadingMode == 0)                    // Gouraud
    {
        vec3 N = fNormal;
        vec3 L = normalize(uLightDir);
        vec3 V = normalize(-posEye.xyz);
        vec3 R = reflect(-L, N);

        vec3 ambient  = uKa;
        vec3 diffuse  = uKd * max(dot(N, L), 0.0);
        vec3 specular = uKs * pow(max(dot(R, V), 0.0), uShininess);

        vec3 col = (uUseAmb  ? ambient  : vec3(0.0)) +
                   (uUseDiff ? diffuse  : vec3(0.0)) +
                   (uUseSpec ? specular : vec3(0.0));

        fColor = vec4(col, 1.0);
    }
    else
    {
        // Avoid ‘uninitialised varying’ warnings when we are in Phong mode.
        fColor = vec4(0.0);
    }
}
