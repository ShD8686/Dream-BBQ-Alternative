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

// Свет фонарей: точечные тёплые источники (только слабая подсветка, чтобы не выдавать маршрут)
uniform int   lampCount;
uniform vec3  lampPos[24];
uniform float lampStrength;
uniform float lampBoost;   // > 0: лампа еще и добавляет свет (для тёмных комнат)

void main()
{
    vec4 base = useTexture ? texture(tex0, UV) : vec4(baseColor, 1.0);
    if (base.a < 0.1) discard;

    vec3 n = normalize(Normal);
    float diff = max(dot(n, normalize(-lightDir)), 0.0);
    float light = 0.55 + 0.45 * diff;

    vec3 litColor = base.rgb * light;

    // Тёплая подсветка от ближайших фонарей: плавно угасает на расстоянии ~9 м
    if (lampCount > 0 && lampStrength > 0.0)
    {
        float lamp = 0.0;
        for (int i = 0; i < lampCount; ++i)
        {
            vec3 L = lampPos[i] - WorldPos;
            float d = length(L);
            float att = clamp(1.0 - d / 9.0, 0.0, 1.0);
            att *= att;
            float facing = 0.35 + 0.65 * max(dot(n, L / max(d, 0.001)), 0.0);
            lamp += att * facing;
        }
        lamp = clamp(lamp * lampStrength, 0.0, 1.0);
        vec3 warm = vec3(1.00, 0.74, 0.38);
        litColor = mix(litColor, litColor * warm * 1.12, lamp * 0.55);
        litColor += (base.rgb * 0.5 + 0.5) * warm * (lamp * lampBoost * 0.60);  // абсолютный тёплый свет: виден даже на почти чёрном полу
    }

    // Светящиеся объекты (лампы фонарей): цвет с компонентой > 1.0 считается источником света.
    // Он не затеняется и не растворяется в тумане полностью, поэтому огни видны издалека.
    // У обычных цветов и текстур компоненты <= 1.0, так что на остальное это не влияет.
    float glow = clamp(max(base.r, max(base.g, base.b)) - 1.0, 0.0, 1.0);
    litColor = mix(litColor, base.rgb, glow);

    vec3 fogColor = vec3(0.86, 0.90, 0.94);
    float dist = length(camPos - WorldPos);
    float fogFactor = exp(-pow(dist * fogDensity, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    fogFactor = max(fogFactor, glow * 0.30);

    vec3 finalColor = mix(fogColor, litColor, fogFactor);
    FragColor = vec4(finalColor, base.a);
}