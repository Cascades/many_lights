#version 460 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;
in vec2 TexCoords;
  
uniform vec3 lightPos0;
uniform vec3 lightColor0;
uniform vec3 lightPos1;
uniform vec3 lightColor1;
uniform vec3 viewPos; 
uniform vec3 objectColor;

struct MaterialData
{
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_diffuse2;
    sampler2D texture_specular2;
};

uniform MaterialData material; 

void main()
{
    // ambient0
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor0;
  	
    // diffuse0
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos0 - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor0 * texture(material.texture_diffuse1, TexCoords).rgb;
    
    // specular0
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor0 * texture(material.texture_specular1, TexCoords).rgb;  
    
    // result0
    vec3 result = (ambient + diffuse + specular) * objectColor;

    // ambient1
    ambientStrength = 0.1;
    ambient = ambientStrength * lightColor1;
  	
    // diffuse2
    norm = normalize(Normal);
    lightDir = normalize(lightPos1 - FragPos);
    diff = max(dot(norm, lightDir), 0.0);
    diffuse = diff * lightColor1 * texture(material.texture_diffuse1, TexCoords).rgb;
    
    // specular1
    specularStrength = 0.5;
    viewDir = normalize(viewPos - FragPos);
    reflectDir = reflect(-lightDir, norm);  
    spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    specular = specularStrength * spec * lightColor1 * texture(material.texture_specular1, TexCoords).rgb;  
    
    // result0 + result1
    result = result + (ambient + diffuse + specular) * objectColor;

    FragColor = vec4(result / 2.0, 1.0);
} 