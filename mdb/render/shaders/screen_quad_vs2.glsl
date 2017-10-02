#extension GL_ARB_explicit_attrib_location : require
#extension GL_ARB_explicit_uniform_location : require

// Input vertex data, different for all executions of this shader.
in vec3 vertex_position;

// Output data ; will be interpolated for each fragment.
out vec2 texcoord;

void main()
{
    gl_Position =  vec4(vertex_position,1);
    texcoord = (vertex_position.xy+vec2(1,1))/2.0;
}

