//
//  triangle.hpp
//  SDL-OpenGL-Tests-2
//
//  Created by Tobias Pflüger on 08.03.19.
//  Copyright © 2019 Tobias Pflüger. All rights reserved.
//

#ifndef triangle_hpp
#define triangle_hpp

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

#include "arrayBuffers/arrayBuffer.hpp"
#include "arrayBuffers/dynamicArrayBuffer.hpp"
#include "shader.hpp"
#include "texture.hpp"
#include "utils.hpp"
#include "object.hpp"

class Triangle: public Object {
public:
    Triangle(Shader *shader, const RenderData *data, bool *wireframe = nullptr);
    ~Triangle();
    
    void render();
    void calculateModelMat();
    
    void setPosition(glm::vec3 position);
    void setAngle(float angle);
    void setRotationAxis(glm::vec3 rotationAxis);
    void setSize(glm::vec3 size);
    void setModelMat(glm::mat4 modelMat);
    
    glm::vec3 getPosition();
    float getAngle();
    glm::vec3 getRotationAxis();
    glm::vec3 getSize();
    glm::vec3 getRealPosition();
    
    Shader *getShaderPointer();
    
private:
    GLuint VAO;
    DynamicArrayBuffer vertex;
    ArrayBuffer texCoord;
    Shader *shader;
    const RenderData *data;
    
    bool *wireframe;
    
    
    glm::vec3 position;
    float angle;
    glm::vec3 rotationAxis;
    glm::vec3 size;
    
    glm::mat4 model;
};

#endif /* triangle_hpp */
