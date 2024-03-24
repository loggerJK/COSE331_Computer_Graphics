#ifndef HW2_LIGHT_H
#define HW2_LIGHT_H

#include "global.h"
#include "program.h"

class Light {
public:
    vec3 position;
    vec3 diffuse, specular, ambient;

    Light(Program* program);
    virtual void update() const;

protected:
    Program* program;
    virtual void create(Program* program);
};


#endif //HW2_LIGHT_H
