#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 cameraPos;

struct Material
{
    vec3 color;
    float ambientStrength;
    float shininess;
    float specularStrength;
}; 
uniform Material material;

struct PointLight
{
    vec3 color;
    vec3 position;
    float strength;
    float linear;
    float quadratic;
};
uniform PointLight lights[MAX_POINT_LIGHTS_LENGTH];
uniform int lightsCount;

struct DirectionalLight 
{
    vec3 color;
    vec3 direction;
    float strength;
};
uniform DirectionalLight directionalLight;

vec3 CalculatePointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(cameraPos - FragPos);

    //directional light
    vec3 dirLightColor = directionalLight.color * directionalLight.strength;
    vec3 directionalAmbient = material.ambientStrength * dirLightColor;
    vec3 dirLightDir = normalize(-directionalLight.direction);
    float directDiff = max(dot(norm, dirLightDir), 0.f);
    vec3 directionalDiffuse = directDiff * dirLightColor;
    vec3 directionalReflectDir = reflect(-dirLightDir, norm);
    float directionalSpec = pow(max(dot(viewDir, directionalReflectDir), 0.f), material.shininess);
    vec3 directionalSpecular = dirLightColor * directionalSpec * material.specularStrength;

    vec3 result = directionalAmbient + directionalDiffuse + directionalSpecular;
    //point lights
    for(int i = 0; i < lightsCount; i++)
        result += CalculatePointLight(lights[i], norm, FragPos, viewDir);
    
    result *= material.color;
    FragColor = vec4(result, 1.0f);
}

vec3 CalculatePointLight(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    //ambient
    vec3 ambient = material.ambientStrength * pointLight.color;
    //diffuse
    vec3 lightDir = normalize(pointLight.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.f);
    vec3 diffuse = diff * pointLight.color;
    //specular
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.f), material.shininess);
    vec3 specular = material.specularStrength * spec * pointLight.color;

    float distance = length(pointLight.position - fragPos);
    float attenuation = 1.0 / (pointLight.strength + pointLight.linear * distance + pointLight.quadratic * (distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}