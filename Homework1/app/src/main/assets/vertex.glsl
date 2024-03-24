#version 300 es
uniform mat4 worldMat, viewMat, projMat;
uniform vec3 lightPos;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 v_normal;
out vec2 v_texCoord;
out vec3 v_lightDir;

void main()
{
    float scale = 1.0;

    //////////////////////////////
    /* TODO: Problem 1.
     *  Fill in the lines below.
     *  Scale the part of the teapot below XZ plane.
     */
    // scale teapot

    mat4 scaleM;
    float scaleFactor = 1.5f;
    scaleM = transpose(mat4(scaleFactor, 0.0f, 0.0f, 0.0f,
                            0.0f, scaleFactor, 0.0f, 0.0f,
                            0.0f, 0.0f, scaleFactor, 0.0f,
                            0.0f, 0.0f, 0.0f, 1.0f));
    vec4 tmp = worldMat * vec4(position, 1.0);

    if (tmp[1] < 0.0f)
        tmp = worldMat * scaleM * inverse(worldMat) * tmp;

    gl_Position =  projMat * viewMat * tmp;
    v_normal = normalize(transpose(inverse(mat3(worldMat))) * normal);
    v_texCoord = texCoord;

    //////////////////////////////

    vec3 posWS = (worldMat * vec4(position, 1.0)).xyz;
    v_lightDir = normalize(lightPos - posWS);
}
