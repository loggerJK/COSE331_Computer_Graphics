#include "scene.h"
#include "binary/teapot.h"
#include "binary/rgb.h"
#include "binary/cloud.h"
#include "binary/tex_flower.h"
#include "checker.h"

Shader* Scene::vertexShader = nullptr;
Shader* Scene::fragmentShader = nullptr;
Program* Scene::program = nullptr;
Camera* Scene::camera = nullptr;
Object* Scene::teapot = nullptr;
Texture* Scene::diffuse = nullptr;
Texture* Scene::dissolve = nullptr;
Material* Scene::material = nullptr;
Light* Scene::light = nullptr;

int Scene::width = 0;
int Scene::height = 0;

// Arcball variables
float lastMouseX = 0, lastMouseY = 0;
float currentMouseX = 0, currentMouseY = 0;

void Scene::setup(AAssetManager* aAssetManager) {
    Asset::setManager(aAssetManager);

    Scene::vertexShader = new Shader(GL_VERTEX_SHADER, "vertex.glsl");
    Scene::fragmentShader = new Shader(GL_FRAGMENT_SHADER, "fragment.glsl");

    Scene::program = new Program(Scene::vertexShader, Scene::fragmentShader);

    Scene::camera = new Camera(Scene::program);
    Scene::camera->eye = vec3(20.0f, 30.0f, 20.0f);

    Scene::light = new Light(program);
    Scene::light->position = vec3(15.0f, 15.0f, 0.0f);

    //////////////////////////////
    /* TODO: Problem 2 : Change the texture of the teapot
     *  Modify and fill in the lines below.
     */
    for (int i = 0; i < rgbTexels.size(); i++)
    {
        GLubyte tmp_r = rgbTexels[i].red;
        GLubyte tmp_g = rgbTexels[i].green;
        GLubyte tmp_b = rgbTexels[i].blue;
        rgbTexels[i].red = tmp_g;
        rgbTexels[i].green = tmp_b;
        rgbTexels[i].blue = tmp_r;
    }
    Scene::diffuse  = new Texture(Scene::program, 0, "textureDiff", rgbTexels , rgbSize);
    //////////////////////////////

    Scene::material = new Material(Scene::program, diffuse, dissolve);
    Scene::teapot = new Object(program, material, teapotVertices, teapotIndices);
}

void Scene::screen(int width, int height) {
    Scene::camera->aspect = (float) width/height;
    Scene::width = width;
    Scene::height = height;
}

void Scene::update(float deltaTime) {
    static float time = 0.0f;

    Scene::program->use();

    Scene::camera->update();
    Scene::light->update();

    Scene::teapot->draw();

    time += deltaTime;
}

void Scene::mouseDownEvents(float x, float y) {
    lastMouseX = currentMouseX = x;
    lastMouseY = currentMouseY = y;
}

void Scene::mouseMoveEvents(float x, float y) {
    //////////////////////////////
    /* TODO: Problem 3 : Implement Phong lighting
     *  Fill in the lines below.
     */
    currentMouseX = x;
    currentMouseY = y;

    float curX = 2.0 * currentMouseX / width - 1.0;
    float curY = 2.0 * currentMouseY / height - 1.0;
    float lastX = 2.0 * lastMouseX / width - 1.0;
    float lastY = 2.0 * lastMouseY / height - 1.0;

    float lastZ = 0;
    float curZ = 0;
//    lastZ = sqrt(1-(pow(lastX, 2.0) - pow(lastY, 2.0)));
//    curZ = sqrt(1-(pow(curX, 2.0) - pow(curY, 2.0)));
    if ((pow(curX, 2.0) + pow(curY, 2.0)) <= 1.0)
        curZ = sqrt(1.0-(pow(curX, 2.0) - pow(curY, 2.0)));
    if ((pow(lastX, 2.0) + pow(lastY, 2.0)) <= 1.0)
        lastZ = sqrt(1.0-(pow(lastX, 2.0) - pow(lastY, 2.0)));

    vec3 last = normalize(vec3(lastX, lastY, lastZ));
    vec3 current = normalize(vec3(curX, curY, curZ));

    mat4 viewMat = lookAt(Scene::camera->eye, Scene::camera->at, Scene::camera->up);
    vec3 N = glm::normalize(camera->eye - camera->at);
    vec3 U = glm::normalize(glm::cross(camera->up, N));
    vec3 V = glm::normalize(glm::cross(N, U));

    mat4 inverseView = transpose(mat4(
            U.x, V.x, N.x, Scene::camera->eye.x,
            U.y, V.y, N.y, Scene::camera->eye.y,
            U.z, V.z, N.z, Scene::camera->eye.z,
            0.0, 0.0, 0.0, 1.0
    ));


    mat4 worldMat = Scene::teapot->worldMat;
    mat4 R = transpose(mat4(
            worldMat[0][0], worldMat[1][0], worldMat[2][0], 0.0,
            worldMat[0][1], worldMat[1][1], worldMat[2][1], 0.0,
            worldMat[0][2], worldMat[1][2], worldMat[2][2], 0.0,
            0.0, 0.0, 0.0, 1.0
            ));
    mat4 T = transpose(mat4(
            1.0, 0.0, 0.0, worldMat[3][0],
            0.0, 1.0, 0.0, worldMat[3][1],
            0.0, 0.0, 1.0, worldMat[3][2],
            0.0, 0.0, 0.0, 1.0
    ));
    float radians = glm::acos(dot(last,current));
    vec3 axis = normalize(glm::cross(last, current));
    vec3 axis_world = glm::normalize(inverseView * vec4(axis, 1.0));
    vec3 position_world = Scene::light->position;
    vec3 position_object = vec3( inverse(R) * inverse(T)* vec4(position_world,1.0));


//    Scene::light->position = vec3( worldMat * rotateMat * vec4(position_object,1.0));
    mat4 rotateMat = glm::rotate(radians, axis);
//    teapot->worldMat *= rotateMat;
    Scene::light->position = vec3(worldMat* rotateMat * vec4(position_object,1.0));
//
    lastMouseX = currentMouseX;
    lastMouseY = currentMouseY;

    //////////////////////////////
}

