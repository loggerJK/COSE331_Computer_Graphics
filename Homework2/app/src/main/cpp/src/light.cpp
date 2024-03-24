#include "light.h"

Light::Light(Program* program)
        : position(0.0f),
          diffuse(0.6f),
          specular(0.6f),
          ambient(0.3f) {

    create(program);
}

void Light::create(Program* program) {
    LOG_PRINT_DEBUG("Create light");

    this->program = program;


    if (glGetUniformLocation(program->get(), "srcDiff") < 0)
        LOG_PRINT_WARN("Fail to get uniform location: %s", "srcDiff");
    if (glGetUniformLocation(program->get(), "srcSpec") < 0)
        LOG_PRINT_WARN("Fail to get uniform location: %s", "srcSpec");
    if (glGetUniformLocation(program->get(), "srcAmbi") < 0)
        LOG_PRINT_WARN("Fail to get uniform location: %s", "srcAmbi");
    if (glGetUniformLocation(program->get(), "lightPos") < 0)
        LOG_PRINT_WARN("Fail to get uniform location: %s", "lightPos");
}

void Light::update() const {
    // get uniform locations
    GLint srcDiffLoc = glGetUniformLocation(program->get(), "srcDiff");
    GLint srcSpecLoc = glGetUniformLocation(program->get(), "srcSpec");
    GLint srcAmbiLoc = glGetUniformLocation(program->get(), "srcAmbi");
    GLint lightPosLoc = glGetUniformLocation(program->get(), "lightPos");

    // set uniform data
    if (srcDiffLoc >= 0) glUniform3fv(srcDiffLoc, 1, value_ptr(diffuse));
    else LOG_PRINT_ERROR("Fail to find uniform location: %s", "srcDiff");
    if (srcSpecLoc >= 0) glUniform3fv(srcSpecLoc, 1, value_ptr(specular));
    else LOG_PRINT_ERROR("Fail to find uniform location: %s", "srcSpec");
    if (srcAmbiLoc >= 0) glUniform3fv(srcAmbiLoc, 1, value_ptr(ambient));
    else LOG_PRINT_ERROR("Fail to find uniform location: %s", "srcAmbi");
    if (lightPosLoc >= 0) glUniform3fv(lightPosLoc, 1, value_ptr(position));
    else LOG_PRINT_ERROR("Fail to find uniform location: %s", "lightPos");
}


