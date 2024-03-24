#include "scene.h"
#include "binary/animation.h"
#include "binary/skeleton.h"
#include "binary/player.h"
#include <cmath>

Shader *Scene::vertexShader = nullptr;
Shader *Scene::fragmentShader = nullptr;
Program *Scene::program = nullptr;
Camera *Scene::camera = nullptr;
Object *Scene::player = nullptr;
Texture *Scene::diffuse = nullptr;
Material *Scene::material = nullptr;
Object *Scene::lineDraw = nullptr;
Texture *Scene::lineColor = nullptr;
Material *Scene::lineMaterial = nullptr;

bool Scene::upperFlag = true;
bool Scene::lowerFlag = true;
float upper_cur_time = 0.0f;
float lower_cur_time = 0.0f;

void Scene::setup(AAssetManager *aAssetManager)
{
    Asset::setManager(aAssetManager);

    Scene::vertexShader = new Shader(GL_VERTEX_SHADER, "vertex.glsl");
    Scene::fragmentShader = new Shader(GL_FRAGMENT_SHADER, "fragment.glsl");

    Scene::program = new Program(Scene::vertexShader, Scene::fragmentShader);

    Scene::camera = new Camera(Scene::program);
    Scene::camera->eye = vec3(0.0f, 0.0f, 80.0f);

    Scene::diffuse = new Texture(Scene::program, 0, "textureDiff", playerTexels, playerSize);
    Scene::material = new Material(Scene::program, diffuse);
    Scene::player = new Object(program, material, playerVertices, playerIndices);
    player->worldMat = scale(vec3(1.0f / 3.0f));

    Scene::lineColor = new Texture(Scene::program, 0, "textureDiff", {{0xFF, 0x00, 0x00}}, 1);
    Scene::lineMaterial = new Material(Scene::program, lineColor);
    Scene::lineDraw = new Object(program, lineMaterial, {{}}, {{}}, GL_LINES);
    lineDraw->worldMat = scale(vec3(1.0f / 3.0f));
}

void Scene::screen(int width, int height)
{
    Scene::camera->aspect = (float)width / height;
}

void Scene::update(float deltaTime)
{
    int upper_ceil_cur_time, upper_floor_cur_time, lower_ceil_cur_time, lower_floor_cur_time;
    float upper_ceil_weight, upper_floor_weight, lower_ceil_weight, lower_floor_weight;
    // Calculate Time
    if (upperFlag)
    {
        upper_cur_time += deltaTime;
        while (upper_cur_time >= 4) // 0 ~ 3.9999
            upper_cur_time -= 4;
        upper_ceil_cur_time = ((int)ceil(upper_cur_time));
        upper_ceil_weight = upper_ceil_cur_time - upper_cur_time;
        if (upper_ceil_cur_time >= 4)
            upper_ceil_cur_time -= 4;
        upper_floor_cur_time = ((int)floor(upper_cur_time));
        upper_floor_weight = upper_cur_time - upper_floor_cur_time;
        // LOG_PRINT_DEBUG("cur_time : %f, ceil_cur_time : %d, ceil_weight : %f, floor_cur_time : %d, floor_weight : %f", cur_time, ceil_cur_time, ceil_weight, floor_cur_time, floor_weight);
    }
    if (lowerFlag)
    {
        lower_cur_time += deltaTime;
        while (lower_cur_time >= 4) // 0 ~ 3.9999
            lower_cur_time -= 4;
        lower_ceil_cur_time = ((int)ceil(lower_cur_time));
        lower_ceil_weight = lower_ceil_cur_time - lower_cur_time;
        if (lower_ceil_cur_time >= 4)
            lower_ceil_cur_time -= 4;
        lower_floor_cur_time = ((int)floor(lower_cur_time));
        lower_floor_weight = lower_cur_time - lower_floor_cur_time;
        // DEBUG ALL LOWER VARIABLES
        // LOG_PRINT_DEBUG("lower_cur_time : %f, lower_ceil_cur_time : %d, lower_ceil_weight : %f, lower_floor_cur_time : %d, lower_floor_weight : %f", lower_cur_time, lower_ceil_cur_time, lower_ceil_weight, lower_floor_cur_time, lower_floor_weight);
    }

    Scene::program->use();
    Scene::camera->update();

    mat4 parentMat[28];
    mat4 matd[28];
    mat4 matd_inverse[28];
    mat4 matLocal[28];
    mat4 ceil_matLocal[28];
    mat4 floor_matLocal[28];

    mat4 matAnimation[28];

    // Quaternion
    quat matLocalQuat[28];       // q
    vec3 trans[28];              // t
    quat ceil_matLocalQuat[28];  // q
    vec3 ceil_trans[28];         // t
    quat floor_matLocalQuat[28]; // q
    vec3 floor_trans[28];        // t

    // construct parentMat
    // make each vec3 into a vec4 with w = 1 which is translation matrix
    // V_parent = parentMat[i] * V_i
    for (int i = 0; i < jOffsets.size(); i++)
        parentMat[i] = (translate(mat4(1.0f), jOffsets[i]));

    // construct matd
    // Originally, bone space -> character object space
    // used to construct matd_inverse
    for (int i = 0; i < 28; i++)
    {
        if (i == 0)
            // identity matrix
            matd[i] = mat4(1.0f);
        else
            matd[i] = matd[jParents[i]] * parentMat[i];
    }

    // construct matd_inverse
    for (int i = 0; i < 28; i++)
        matd_inverse[i] = inverse(matd[i]);

    // ====================== Multiple Keyframe : lower ======================

    vector<float> ceil_keyframe = motions[lower_ceil_cur_time];
    vector<float> floor_keyframe = motions[lower_floor_cur_time];

    for (int i = 0; i < 12; i++)
    {
        mat4 rotate_y, rotate_x, rotate_z;

        rotate_x = rotate(radians(ceil_keyframe[(int)(3 + 3 * i)]), vec3(1.0f, 0.0f, 0.0f));
        rotate_y = rotate(radians(ceil_keyframe[(int)(4 + 3 * i)]), vec3(0.0f, 1.0f, 0.0f));
        rotate_z = rotate(radians(ceil_keyframe[(int)(5 + 3 * i)]), vec3(0.0f, 0.0f, 1.0f));
        if (i == 0)
        {
            mat4 trans;
            trans = translate(mat4(1.0f), vec3(ceil_keyframe[0], ceil_keyframe[1], ceil_keyframe[2]));
            ceil_matLocal[i] = trans * rotate_z * rotate_x * rotate_y;
        }
        else
            ceil_matLocal[i] = rotate_z * rotate_x * rotate_y;
    }
    for (int i = 0; i < 12; i++)
    {
        mat4 rotate_y, rotate_x, rotate_z;

        rotate_x = rotate(radians(floor_keyframe[(int)(3 + 3 * i)]), vec3(1.0f, 0.0f, 0.0f));
        rotate_y = rotate(radians(floor_keyframe[(int)(4 + 3 * i)]), vec3(0.0f, 1.0f, 0.0f));
        rotate_z = rotate(radians(floor_keyframe[(int)(5 + 3 * i)]), vec3(0.0f, 0.0f, 1.0f));
        if (i == 0)
        {
            mat4 trans;
            trans = translate(mat4(1.0f), vec3(floor_keyframe[0], floor_keyframe[1], floor_keyframe[2]));
            floor_matLocal[i] = trans * rotate_z * rotate_x * rotate_y;
        }
        else
            floor_matLocal[i] = rotate_z * rotate_x * rotate_y;
    }
    // construct matLocalQuat & trans
    for (int i = 0; i < 12; i++)
    {
        ceil_matLocalQuat[i] = quat_cast(mat3(parentMat[i] * ceil_matLocal[i]));   // q
        floor_matLocalQuat[i] = quat_cast(mat3(parentMat[i] * floor_matLocal[i])); // q
        matLocalQuat[i] = slerp(ceil_matLocalQuat[i], floor_matLocalQuat[i], lower_ceil_weight);

        // construct trans[i][j] from parentMat[i] * matLocal[i]
        for (int j = 0; j < 3; j++)
        {
            ceil_trans[i][j] = ((parentMat[i] * ceil_matLocal[i])[3][j]);   //  t
            floor_trans[i][j] = ((parentMat[i] * floor_matLocal[i])[3][j]); //  t
            trans[i][j] = mix(ceil_trans[i][j], floor_trans[i][j], lower_ceil_weight);
        }
    }
    // construct matAnimation from quaternion
    for (int i = 0; i < 12; i++)
    {
        mat4 rotate_trans = mat4_cast(matLocalQuat[i]);
        for (int j = 0; j < 3; j++)
            rotate_trans[3][j] = trans[i][j];

        if (i == 0)
            // identity matrix
            matAnimation[i] = mat4(1.0f);
        else
            matAnimation[i] = matAnimation[jParents[i]] * rotate_trans;
    }
    // ======================================================

    // ====================== Multiple Keyframe : upper ======================

    ceil_keyframe = motions[upper_ceil_cur_time];
    floor_keyframe = motions[upper_floor_cur_time];

    for (int i = 12; i < 28; i++)
    {
        mat4 rotate_y, rotate_x, rotate_z;

        rotate_x = rotate(radians(ceil_keyframe[(int)(3 + 3 * i)]), vec3(1.0f, 0.0f, 0.0f));
        rotate_y = rotate(radians(ceil_keyframe[(int)(4 + 3 * i)]), vec3(0.0f, 1.0f, 0.0f));
        rotate_z = rotate(radians(ceil_keyframe[(int)(5 + 3 * i)]), vec3(0.0f, 0.0f, 1.0f));
        if (i == 0)
        {
            mat4 trans;
            trans = translate(mat4(1.0f), vec3(ceil_keyframe[0], ceil_keyframe[1], ceil_keyframe[2]));
            ceil_matLocal[i] = trans * rotate_z * rotate_x * rotate_y;
        }
        else
            ceil_matLocal[i] = rotate_z * rotate_x * rotate_y;
    }
    for (int i = 12; i < 28; i++)
    {
        mat4 rotate_y, rotate_x, rotate_z;

        rotate_x = rotate(radians(floor_keyframe[(int)(3 + 3 * i)]), vec3(1.0f, 0.0f, 0.0f));
        rotate_y = rotate(radians(floor_keyframe[(int)(4 + 3 * i)]), vec3(0.0f, 1.0f, 0.0f));
        rotate_z = rotate(radians(floor_keyframe[(int)(5 + 3 * i)]), vec3(0.0f, 0.0f, 1.0f));
        if (i == 0)
        {
            mat4 trans;
            trans = translate(mat4(1.0f), vec3(floor_keyframe[0], floor_keyframe[1], floor_keyframe[2]));
            floor_matLocal[i] = trans * rotate_z * rotate_x * rotate_y;
        }
        else
            floor_matLocal[i] = rotate_z * rotate_x * rotate_y;
    }
    // construct matLocalQuat & trans
    for (int i = 12; i < 28; i++)
    {
        ceil_matLocalQuat[i] = quat_cast(mat3(parentMat[i] * ceil_matLocal[i]));   // q
        floor_matLocalQuat[i] = quat_cast(mat3(parentMat[i] * floor_matLocal[i])); // q
        matLocalQuat[i] = slerp(ceil_matLocalQuat[i], floor_matLocalQuat[i], upper_ceil_weight);

        // construct trans[i][j] from parentMat[i] * matLocal[i]
        for (int j = 0; j < 3; j++)
        {
            ceil_trans[i][j] = ((parentMat[i] * ceil_matLocal[i])[3][j]);   //  t
            floor_trans[i][j] = ((parentMat[i] * floor_matLocal[i])[3][j]); //  t
            trans[i][j] = mix(ceil_trans[i][j], floor_trans[i][j], upper_ceil_weight);
        }
    }
    // construct matAnimation from quaternion
    for (int i = 12; i < 28; i++)
    {
        mat4 rotate_trans = mat4_cast(matLocalQuat[i]);
        for (int j = 0; j < 3; j++)
            rotate_trans[3][j] = trans[i][j];

        if (i == 0)
            // identity matrix
            matAnimation[i] = mat4(1.0f);
        else
            matAnimation[i] = matAnimation[jParents[i]] * rotate_trans;
    }
    // ======================================================

    // ====================== Skinning ======================
    // construct playerVertices_new
    // Apply skinning to each vertex

    vector<Vertex> playerVertices_new;
    for (int i = 0; i < playerVertices.size(); i++)
    {
        // Insert original information
        playerVertices_new.push_back(playerVertices[i]);

        // bone and weight of i-th vertex
        ivec4 bone = playerVertices[i].bone;
        vec4 weight = playerVertices[i].weight;

        vec4 new_pos = {0.0f, 0.0f, 0.0f, 0.0f}; // initilize new_pos with 0

        vec4 pos[4];
        for (int j = 0; j < 4; j++)
            if (bone[j] != -1)
                pos[j] = matAnimation[bone[j]] * matd_inverse[bone[j]] * vec4(playerVertices[i].pos, 1.0f);
            else
                pos[j] = vec4(0.0f, 0.0f, 0.0f, 0.0f);

        for (int j = 0; j < 4; j++)
            new_pos += pos[j] * weight[j];

        // LOG_PRINT_DEBUG("new_pos : %f %f %f %f", new_pos[0], new_pos[1], new_pos[2], new_pos[3]);
        playerVertices_new[i].pos = vec3(new_pos[0], new_pos[1], new_pos[2]);
    }
    // ======================================================

    player->worldMat = scale(vec3(1.0f / 3.0f));

    Scene::player->load(playerVertices_new, playerIndices);
    Scene::player->draw();
}

void Scene::setUpperFlag(bool flag)
{
    Scene::upperFlag = flag;
}

void Scene::setLowerFlag(bool flag)
{
    Scene::lowerFlag = flag;
}