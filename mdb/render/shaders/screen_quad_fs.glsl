#version 440

in  vec2 texcoord;
out vec4 colour;

layout(binding=0) uniform sampler2D render_texture;

uniform float exposure;

void main(void)
{
    //const float gamma = 2.2;
    const vec3 cm = vec3(0.3, 0.5, 1.0);

    float hdrColor = texture(render_texture, texcoord).r;

    // Exposure tone mapping
    float mapped = 1.0 - exp(-hdrColor * exposure);
    // Gamma correction
    //mapped = pow(mapped,1.0 / gamma);


    colour = vec4(vec3(mapped)*cm, 1.0);

    //colour = vec4(texcoord, 0, 1);
    //float c = texture(render_texture, texcoord).r;
    //colour = vec4(c, c, c, 1);
}