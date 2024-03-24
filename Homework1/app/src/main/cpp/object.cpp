#include "object.h"

Object::Object(Program *program, Material *material,
               GLfloat *verticesData, GLushort *indicesData,
               GLsizei verticesSize, GLsizei indicesSize) {

    // create object
    create(program, material);

    // set object data
    data(verticesData, indicesData, verticesSize, indicesSize);

    // draw object
    draw();
}

Object::~Object() {

    // delete buffer objects
    destroy();
}

void Object::create(Program* program, Material* material) {
    LOG_PRINT_DEBUG("Create object");

    // set attributes
    this->program = program;
    this->material = material;

    // generate buffer objects
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    // check buffer object ID
    if (!vbo || !ibo) {
        LOG_PRINT_ERROR("Fail to generate buffer");
    }

    // check world matrix uniform location
    if (glGetUniformLocation(program->get(), "worldMat") < 0) {
        LOG_PRINT_WARN("Fail to get uniform location: %s", "worldMat");
    }
}

void Object::data(GLfloat *verticesData, GLushort *indicesData,
                  GLsizei verticesSize, GLsizei indicesSize) {
    LOG_PRINT_DEBUG("Set object data");

    // set attributes
    this->indicesSize = indicesSize;
//    this->vertices = vertices;
//    this->indices = indices;

    // bind buffer objects
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // set buffer data
    glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(GLfloat),
                 verticesData, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize * sizeof(GLushort),
                 indicesData, GL_STATIC_DRAW);
}

void Object::draw(){
    drawTeapot();
    drawAxis();
    drawPlane();
}

void Object::drawTeapot() const {
//    LOG_PRINT_DEBUG("Draw object");

    // get world matrix uniform location
    GLint loc = glGetUniformLocation(program->get(), "worldMat");

    // set world matrix uniform data
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, value_ptr(worldMatrix));

    // bind buffer objects
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    // enable vertex attribute arrays
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // set vertex attribute pointer
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (const void *) (0 * sizeof(GLfloat)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (const void *) (3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat),
                          (const void *) (6 * sizeof(GLfloat)));

    // setup material
    material->setup();

    // draw elements
    glDrawElements(GL_TRIANGLES, indicesSize, GL_UNSIGNED_SHORT, (const void *) 0);
}

void Object::drawPlane(){
    const char *vertexShaderSource =
            "#version 300 es\n"
            "uniform mat4 viewMat, projMat;\n"
            "layout(location = 0) in vec3 position;\n"
            "void main()\n"
            "{\n"
            "gl_Position = projMat * viewMat * vec4(position, 1.0);\n"
            "}";

    const char *fragmentShaderSource =
            "#version 300 es\n"
            "precision mediump float;\n"
            "layout(location = 0) out vec4 FragColor;\n"
            "void main()\n"
            "{\n"
            "   FragColor = vec4(1.0f, 1.0f, 1.0f, 0.5f);\n"
            "}\n";

    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1 ,&vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    float verticesData[] = {
            -30.0f, 0.0f, 30.0f,
            30.0f, 0.0f, 30.0f,
            30.0f, 0.0f, -30.0f,
            -30.0f, 0.0f, -30.0f
    };

    unsigned int indicesData[] = {
            0,1,2,
            2,3,0
    };

    GLsizei verticesSize = 3 * 4;
    GLsizei iSize = 3 * 2;

    unsigned int VBO, IBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);

    GLint loc2 = glGetUniformLocation(shaderProgram, "viewMat");
    GLint loc3 = glGetUniformLocation(shaderProgram, "projMat");

    if (loc2 >= 0) glUniformMatrix4fv(loc2, 1, GL_FALSE, value_ptr(viewMatrix));
    if (loc3 >= 0) glUniformMatrix4fv(loc3, 1, GL_FALSE, value_ptr(projMatrix));

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(GLfloat),verticesData, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize * sizeof(GLuint),indicesData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                          (const void *) (0 * sizeof(GLfloat)));

    glDrawElements(GL_TRIANGLES, iSize, GL_UNSIGNED_INT, (const void *) 0);

    glDeleteProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}

void Object::drawAxis(){
    const char *vertexShaderSource =
            "#version 300 es\n"
            "uniform mat4 worldMat, viewMat, projMat;\n"
            "layout(location = 0) in vec3 position;\n"
            "out vec3 posWS;\n"
            "void main()\n"
            "{\n"
            "   gl_Position = projMat * viewMat * vec4(position, 1.0);\n"
            "   posWS = position;\n"
            "}\n";

    const char *fragmentShaderSource =
            "#version 300 es\n"
            "precision mediump float;\n"
            "layout(location = 0) out vec4 FragColor;\n"
            "in vec3 posWS;\n"
            "void main()\n"
            "{\n"
            "   vec3 color = vec3(0.0, 0.0, 0.0);\n"
            "   if(posWS.x > 0.0)\n"
            "   {\n"
            "       color.x = 1.0;\n"
            "   }\n"
            "   else if(posWS.y > 0.0)\n"
            "   {\n"
            "       color.y = 1.0;\n"
            "   }\n"
            "   else\n"
            "   {\n"
            "       color.z = 1.0;\n"
            "   }\n"
            "   FragColor = vec4(color, 1.0);\n"
            "}\n";

    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1 ,&vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // draw line
    float verticesData[] = {
            5.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 5.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 5.0f,
            0.0f, 0.0f, 0.0f
    };
    unsigned int indicesData[] = {
            0,1,
            2,3,
            4,5
    };

    GLsizei verticesSize = 3*6;
    GLsizei iSize = 2*3;

    unsigned int VBO, IBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &IBO);

    GLint loc1 = glGetUniformLocation(shaderProgram, "worldMat");
    GLint loc2 = glGetUniformLocation(shaderProgram, "viewMat");
    GLint loc3 = glGetUniformLocation(shaderProgram, "projMat");

    if (loc1 >= 0) glUniformMatrix4fv(loc1, 1, GL_FALSE, value_ptr(worldMatrix));
    if (loc2 >= 0) glUniformMatrix4fv(loc2, 1, GL_FALSE, value_ptr(viewMatrix));
    if (loc3 >= 0) glUniformMatrix4fv(loc3, 1, GL_FALSE, value_ptr(projMatrix));

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(GLfloat),
                 verticesData, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize * sizeof(GLuint),
                 indicesData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
                          (const void *) (0 * sizeof(GLfloat)));

    glLineWidth(8);
    glDrawElements(GL_LINES, iSize, GL_UNSIGNED_INT, (const void *) 0);

    glDeleteProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &IBO);
}

void Object::destroy() {
    LOG_PRINT_DEBUG("Delete object");

    // delete buffer objects
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
}
