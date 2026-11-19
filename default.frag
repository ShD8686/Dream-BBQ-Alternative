#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec2 UV;
in vec3 WorldPos;

uniform sampler2D tex0;
uniform bool useTexture;
uniform vec3 baseColor;
uniform vec3 lightDir;
uniform vec3 camPos;
uniform float fogDensity;

void main()
{
    vec4 base = useTexture ? texture(tex0, UV) : vec4(baseColor, 1.0);
    if (base.a < 0.1) discard;

    vec3 n = normalize(Normal);
    float diff = max(dot(n, normalize(-lightDir)), 0.0);
    float light = 0.55 + 0.45 * diff;

    vec3 litColor = base.rgb * light;

    vec3 fogColor = vec3(0.86, 0.90, 0.94);
    float dist = length(camPos - WorldPos);
    float fogFactor = exp(-pow(dist * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalColor = mix(fogColor, litColor, fogFactor);
    FragColor = vec4(finalColor, base.a);
}