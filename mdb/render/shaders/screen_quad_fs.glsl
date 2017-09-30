in  vec2 texcoord;
out vec4 colour;

layout(binding=0) uniform sampler2D render_texture;

uniform float exposure;

#ifdef ENABLE_COLORS

#define PI 3.141592653589793
#define PI2 6.283185307179586

vec3 biLerp3(vec3 a, vec3 b, vec3 c, vec3 d, float s, float t)
{
  vec3 x = mix(a, b, t);
  vec3 y = mix(c, d, t);
  return mix(x, y, s);
}

vec3 get_color(float a)
{
    const vec3 c1 = vec3(0.4, 0.1, 0.1);
    const vec3 c2 = vec3(0.5, 0.6, 1.0);
    const vec3 c3 = vec3(0.0, 0.0, 0.0);
    const vec3 c4 = vec3(0.0, 0.0, 1.0);

    if(a == 0.0)
        return vec3(0,0,0);

    float x = 1 * cos(a*PI2);
    float y = 1 * sin(a*PI2);

    vec3 c = biLerp3(c1, c2, c3, c4, x, y);

    return c;
}

#endif

void main(void)
{
    //const float gamma = 2.2;

    float hdrColor = texture(render_texture, texcoord).r;

    // Exposure tone mapping
    float mapped = 1.0 - exp(-hdrColor * exposure);
    // Gamma correction
    //mapped = pow(mapped,1.0 / gamma);

#ifdef ENABLE_COLORS
    colour = vec4(get_color(mapped), 1.0);
#else
    colour = vec4(vec3(mapped), 1.0);
#endif
}