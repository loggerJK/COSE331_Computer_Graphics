#ifndef HW2_MATERIAL_H
#define HW2_MATERIAL_H

#include "global.h"
#include "program.h"
#include "texture.h"

class Material {
public:
    vec3 specular, ambient, emissive;
    float shininess;
    float threshold;

    Material(Program* program,
            Texture* textureDiff=nullptr, Texture* textureDissolve=nullptr,
            vec3 specular=vec3(1.0f), vec3  ambient=vec3(0.3f), vec3 emissive=vec3(0.0f), float shininess=100.0f);

    virtual ~Material();

    virtual void update() const;

private:
    Program* program;
    Texture* textureDiff;
    Texture* textureDissolve;
    virtual void create(Program* program,
            Texture* textureDiff=nullptr, Texture* textureDissolve=nullptr);

};

#endif //HW2_MATERIAL_H
