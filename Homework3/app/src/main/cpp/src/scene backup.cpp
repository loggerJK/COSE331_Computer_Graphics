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
float cur_time = 0.0f;

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
    // Calculate Time
    cur_time += deltaTime;
    while (cur_time >= 4) // 0 ~ 3.9999
        cur_time -= 4;
    int ceil_cur_time = ((int)ceil(cur_time));
    float ceil_weight = ceil_cur_time - cur_time;
    if (ceil_cur_time >= 4)
        ceil_cur_time -= 4;
    int floor_cur_time = ((int)floor(cur_time));
    float floor_weight = cur_time - floor_cur_time;
    LOG_PRINT_DEBUG("cur_time : %f, ceil_cur_time : %d, ceil_weight : %f, floor_cur_time : %d, floor_weight : %f", cur_time, ceil_cur_time, ceil_weight, floor_cur_time, floor_weight);
    // assert(ceil_weight + floor_weight == 1.0f);

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

    // ====================== Single Keyframe ======================
    vector<float> keyframe = motions[1];

    // construct parentMat
    // make each vec3 into a vec4 with w = 1 which is translation matrix
    // V_parent = parentMat[i] * V_i
    for (int i = 0; i < jOffsets.size(); i++)
        parentMat[i] = (translate(mat4(1.0f), jOffsets[i]));

    // construct matLocal
    // Each frame consists of 6 * 1 + 3 * 27 = 87 numbers.
    // The first 6 numbers are (XYZ translation, XYZ rotation) of the root.(0)
    // Next, every 3 numbers are (XYZ rotation) of each joint. (1,2 … 27)
    // The rotation order is YXZ
    for (int i = 0; i < 28; i++)
    {
        mat4 rotate_y, rotate_x, rotate_z;

        rotate_x = rotate(radians(keyframe[(int)(3 + 3 * i)]), vec3(1.0f, 0.0f, 0.0f));
        rotate_y = rotate(radians(keyframe[(int)(4 + 3 * i)]), vec3(0.0f, 1.0f, 0.0f));
        rotate_z = rotate(radians(keyframe[(int)(5 + 3 * i)]), vec3(0.0f, 0.0f, 1.0f));
        if (i == 0)
        {
            mat4 trans;
            trans = translate(mat4(1.0f), vec3(keyframe[0], keyframe[1], keyframe[2]));
            matLocal[i] = trans * rotate_z * rotate_x * rotate_y;
        }
        else
            matLocal[i] = rotate_z * rotate_x * rotate_y;
    }

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

    // construct matAnimation
    // for (int i = 0; i < 28; i++)
    // {
    //     if (i == 0)
    //         // identity matrix
    //         matAnimation[i] = mat4(1.0f);
    //     else
    //         matAnimation[i] = matAnimation[jParents[i]] * parentMat[i] * matLocal[i];
    // }

    // ====================== Quaternion Base, Single Frame ======================
    // construct matLocalQuat & trans

    // for (int i = 0; i < 28; i++)
    // {
    //     matLocalQuat[i] = quat_cast(mat3(parentMat[i] * matLocal[i])); // q
    //     // construct trans[j] from parentMat[i] * matLocal[i]
    //     for (int j = 0; j < 3; j++)
    //         trans[i][j] = ((parentMat[i] * matLocal[i])[3][j]); //  t
    // }

    // // construct matAnimation from quaternion
    // for (int i = 0; i < 28; i++)
    // {
    //     mat4 rotate_trans = mat4_cast(matLocalQuat[i]);
    //     for (int j = 0; j < 3; j++)
    //         rotate_trans[3][j] = trans[i][j];

    //     if (i == 0)
    //         // identity matrix
    //         matAnimation[i] = mat4(1.0f);
    //     else
    //         matAnimation[i] = matAnimation[jParents[i]] * rotate_trans;
    // }

    // ====================== Multiple Keyframe ======================

    vector<float> ceil_keyframe = motions[ceil_cur_time];
    vector<float> floor_keyframe = motions[floor_cur_time];

    for (int i = 0; i < 28; i++)
    {
        mat4 rotate_y, rotate_x, rotate_z;

        rotate_x = rotate(radians(ceil_keyframe[(int)(3 + 3 * i)]), vec3(1.0f, 0.0f, 0.0f));
        rotate_y = rotate(radians(ceil_keyframe[(int)(4 + 3 * i)]), vec3(0.0f, 1.0f, 0.0f));
        rotate_z = rotate(radians(ceil_keyframe[(int)(5 + 3 * i)]), vec3(0.0f, 0.0f, 1.0f));
        if (i == 0)
        {
            mat4 trans;
            trans = translate(mat4(1.0f), vec3(keyframe[0], keyframe[1], keyframe[2]));
            ceil_matLocal[i] = trans * rotate_z * rotate_x * rotate_y;
        }
        else
            ceil_matLocal[i] = rotate_z * rotate_x * rotate_y;
    }
    for (int i = 0; i < 28; i++)
    {
        mat4 rotate_y, rotate_x, rotate_z;

        rotate_x = rotate(radians(floor_keyframe[(int)(3 + 3 * i)]), vec3(1.0f, 0.0f, 0.0f));
        rotate_y = rotate(radians(floor_keyframe[(int)(4 + 3 * i)]), vec3(0.0f, 1.0f, 0.0f));
        rotate_z = rotate(radians(floor_keyframe[(int)(5 + 3 * i)]), vec3(0.0f, 0.0f, 1.0f));
        if (i == 0)
        {
            mat4 trans;
            trans = translate(mat4(1.0f), vec3(keyframe[0], keyframe[1], keyframe[2]));
            floor_matLocal[i] = trans * rotate_z * rotate_x * rotate_y;
        }
        else
            floor_matLocal[i] = rotate_z * rotate_x * rotate_y;
    }
    // construct matLocalQuat & trans
    for (int i = 0; i < 28; i++)
    {
        ceil_matLocalQuat[i] = quat_cast(mat3(parentMat[i] * ceil_matLocal[i]));   // q
        floor_matLocalQuat[i] = quat_cast(mat3(parentMat[i] * floor_matLocal[i])); // q
        matLocalQuat[i] = slerp(ceil_matLocalQuat[i], floor_matLocalQuat[i], ceil_weight);

        // construct trans[i][j] from parentMat[i] * matLocal[i]
        for (int j = 0; j < 3; j++)
        {
            ceil_trans[i][j] = ((parentMat[i] * ceil_matLocal[i])[3][j]);   //  t
            floor_trans[i][j] = ((parentMat[i] * floor_matLocal[i])[3][j]); //  t
            trans[i][j] = mix(ceil_trans[i][j], floor_trans[i][j], ceil_weight);
        }
    }
    // construct matAnimation from quaternion
    for (int i = 0; i < 28; i++)
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

    // ====================== No Skinning ======================
    // Vertex.bone: the index of skinned skeleton
    // Vertex.weight : the weight of skinning

    // construct playerVertices_new
    // For test purposes, we just use the first bone of Vertex.bone

    // vector<Vertex> playerVertices_new;
    // for (int i = 0; i < playerVertices.size(); i++)
    // {
    //     // Insert original information
    //     playerVertices_new.push_back(playerVertices[i]);

    //     // bone of i-th vertex
    //     ivec4 bone = playerVertices[i].bone;
    //     vec4 new_pos = matAnimation[bone[0]] * matd_inverse[bone[0]] * vec4(playerVertices[i].pos, 1.0f);

    //     // LOG_PRINT_DEBUG("new_pos : %f %f %f %f", new_pos[0], new_pos[1], new_pos[2], new_pos[3]);

    //     playerVertices_new[i].pos = vec3(new_pos[0], new_pos[1], new_pos[2]);
    // }
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

    // ====================== No Skinning + Quaternion ======================
    // Vertex.bone: the index of skinned skeleton
    // Vertex.weight : the weight of skinning

    // construct playerVertices_new
    // For test purposes, we just use the first bone of Vertex.bone

    // vector<Vertex> playerVertices_new;
    // for (int i = 0; i < playerVertices.size(); i++)
    // {
    //     // Insert original information
    //     playerVertices_new.push_back(playerVertices[i]);

    //     // bone of i-th vertex
    //     ivec4 bone = playerVertices[i].bone;
    //     vec4 new_pos = matAnimation[bone[0]] * matd_inverse[bone[0]] * vec4(playerVertices[i].pos, 1.0f);

    //     // LOG_PRINT_DEBUG("new_pos : %f %f %f %f", new_pos[0], new_pos[1], new_pos[2], new_pos[3]);

    //     playerVertices_new[i].pos = vec3(new_pos[0], new_pos[1], new_pos[2]);
    // }
    // ======================================================

    // TODO : matLocal[0] to quaternion
    player->worldMat = scale(vec3(1.0f / 3.0f));

    Scene::player->load(playerVertices_new, playerIndices);
    Scene::player->draw();
    // ====================== Line ======================

    // Line Drawer
    // TEST
    glLineWidth(1.0);

    // construct Skeleton
    // i-th vertex is the position of i-th joint

    vector<vec3> vertices; // original Skeleton Vertices
    vec3 root = vec3{0.0f, 20.0f, 0.0f};
    vertices.push_back(root);
    for (int i = 1; i < 28; i++)
    {
        vec3 parent_pos, cur_pos;
        parent_pos = vertices[jParents[i]];
        cur_pos = parent_pos + jOffsets[i];
        vertices.push_back(cur_pos);

        // Skeleton Visualization
        // Scene::lineDraw->load({{parent_pos}, {cur_pos}}, {0, 1});
        // Scene::lineDraw->draw();
    }

    vector<vec3> vertices_new;    // transformed Skeleton Vertices
    vertices_new.push_back(root); // Assume root is not transformed
    for (int i = 1; i < 28; i++)
    {
        vec3 new_pos = matAnimation[i] * matd_inverse[i] * vec4(vertices[i], 1.0f);
        vertices_new.push_back(new_pos);
    }

    // Skeleton Visualization
    for (int i = 1; i < 28; i++)
    {
        vec3 parent_pos = vertices_new[jParents[i]];
        vec3 cur_pos = vertices_new[i];
        Scene::lineDraw->load({{parent_pos}, {cur_pos}}, {0, 1});
        Scene::lineDraw->draw();
    }
}

void Scene::setUpperFlag(bool flag)
{
    Scene::upperFlag = flag;
}

void Scene::setLowerFlag(bool flag)
{
    Scene::lowerFlag = flag;
}