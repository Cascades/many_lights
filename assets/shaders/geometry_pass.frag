#version 460 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gDiffSpec;
layout (location = 3) out vec3 gAmbient;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_ambient1;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_dissolve1;
uniform sampler2D texture_bump1;
uniform vec2 sampler_size;

void main()
{
    if(texture(texture_dissolve1, TexCoords).r < 0.1)
    {
        discard;
    }

    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;

    vec2 offset = vec2(1.0) / sampler_size * 25;

    float left = texture(texture_bump1, vec2((TexCoords.x - offset.x), TexCoords.y)).r;
    float right = texture(texture_bump1, vec2((TexCoords.x + offset.x), TexCoords.y)).r;
    float down = texture(texture_bump1, vec2(TexCoords.x, (TexCoords.y + offset.y))).r;
    float up = texture(texture_bump1, vec2(TexCoords.x, (TexCoords.y - offset.y))).r;

    vec3 bump_norm;
    bump_norm.x = left - right;
    bump_norm.y = 2.0;
    bump_norm.z = down - up;
    bump_norm = normalize(bump_norm);

    vec4 q;
    q.xyz = cross(bump_norm, vec3(0.0, 1.0, 0.0));
    q.w = dot(bump_norm, vec3(0.0, 1.0, 0.0));
    q = normalize(q);

    vec3 norm = normalize(Normal);

    // Extract the vector part of the quaternion
    vec3 u = vec3(q.x, q.y, q.z);

    // Extract the scalar part of the quaternion
    float s = q.w;

    // Do the math
    norm = 2.0 * dot(u, norm) * u + (s*s - dot(u, u)) * norm + 2.0 * s * cross(u, norm);
    // also store the per-fragment normals into the gbuffer
    gNormal = normalize(norm);

    // and the diffuse per-fragment color
    gDiffSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gDiffSpec.a = texture(texture_specular1, TexCoords).r;

    gAmbient.rgb = texture(texture_ambient1, TexCoords).rgb;
}