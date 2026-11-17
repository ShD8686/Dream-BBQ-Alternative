#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec2 UV;

uniform sampler2D tex0;
uniform bool useTexture;
uniform vec3 baseColor;
uniform vec3 lightDir;

void main()
{
    vec4 base = useTexture ? texture(tex0, UV) : vec4(baseColor, 1.0);
    if (base.a < 0.1) discard;

    vec3 n = normalize(Normal);
    float diff = max(dot(n, normalize(-lightDir)), 0.0);
    float light = 0.4 + 0.6 * diff;

    FragColor = vec4(base.rgb * light, base.a);
}
