//
//  perlinMap.cpp
//  SDL-OpenGL-Tests-2
//
//  Created by Tobias Pflüger on 17.04.19.
//  Copyright © 2019 Tobias Pflüger. All rights reserved.
//

#include "perlinMap.hpp"

PerlinMap::PerlinMap(unsigned int seed, unsigned int width, float triangleWidth, Shader *shader, const RenderData *data):
tex(nullptr), model(1), translate(1), vertex(), texCoord(), position(glm::vec3(0.0f)), data(data), shader(shader), width(width), noise(seed), triangleWidth(triangleWidth), vertices(pow((width * (1.0f / triangleWidth)), 2.0f) * 6), texCoords(pow((width * (1.0f / triangleWidth)), 2.0f) * 6) {
    glGenVertexArrays(1, &this->VAO);
    glBindVertexArray(this->VAO);
    
    float x = -(width / 2.0f), y = -(width / 2.0f);
    
    freq = 5.0f; multiplier = 2.0f;
    octaves = 2;
    
    for(long i = 0; i < pow((width * (1.0f / triangleWidth)), 2.0f) * 6 ; i += 6) {
        if(x >= width / 2.0f) {
            x = -(width / 2.0f);
            y += 1.0f;
        }
        
        vertices[i + 0] = glm::vec3((x + 0.0f) * triangleWidth, noise.perl((x + 0.0f) * triangleWidth, (y + 0.0f) * triangleWidth, freq, octaves) * multiplier, (y + 0.0f) * triangleWidth);
        vertices[i + 1] = glm::vec3((x + 1.0f) * triangleWidth, noise.perl((x + 1.0f) * triangleWidth, (y + 0.0f) * triangleWidth, freq, octaves) * multiplier, (y + 0.0f) * triangleWidth);
        vertices[i + 2] = glm::vec3((x + 0.0f) * triangleWidth, noise.perl((x + 0.0f) * triangleWidth, (y + 1.0f) * triangleWidth, freq, octaves) * multiplier, (y + 1.0f) * triangleWidth);
        vertices[i + 3] = glm::vec3((x + 1.0f) * triangleWidth, noise.perl((x + 1.0f) * triangleWidth, (y + 1.0f) * triangleWidth, freq, octaves) * multiplier, (y + 1.0f) * triangleWidth);
        vertices[i + 4] = glm::vec3((x + 1.0f) * triangleWidth, noise.perl((x + 1.0f) * triangleWidth, (y + 0.0f) * triangleWidth, freq, octaves) * multiplier, (y + 0.0f) * triangleWidth);
        vertices[i + 5] = glm::vec3((x + 0.0f) * triangleWidth, noise.perl((x + 0.0f) * triangleWidth, (y + 1.0f) * triangleWidth, freq, octaves) * multiplier, (y + 1.0f) * triangleWidth);
        
        texCoords[i + 0] = glm::vec2(0.0f +     (fmod(x, 8.0f) / 8.0f), 0.0f +  (fmod(y, 4.0f) / 4.0f));
        texCoords[i + 1] = glm::vec2(0.125f +   (fmod(x, 8.0f) / 8.0f), 0.0f +  (fmod(y, 4.0f) / 4.0f));
        texCoords[i + 2] = glm::vec2(0.0f +     (fmod(x, 8.0f) / 8.0f), 0.25f + (fmod(y, 4.0f) / 4.0f));
        texCoords[i + 3] = glm::vec2(0.125f +   (fmod(x, 8.0f) / 8.0f), 0.25f + (fmod(y, 4.0f) / 4.0f));
        texCoords[i + 4] = glm::vec2(0.125f +   (fmod(x, 8.0f) / 8.0f), 0.0f +  (fmod(y, 4.0f) / 4.0f));
        texCoords[i + 5] = glm::vec2(0.0f +     (fmod(x, 8.0f) / 8.0f), 0.25f + (fmod(y, 4.0f) / 4.0f));
           
        x += 1.0f;
    }
    
    vertices.shrink_to_fit();
    texCoords.shrink_to_fit();
    
    vertex.setData(vertices.data(), sizeof(glm::vec3) * pow((width * (1.0f / triangleWidth)), 2.0f) * 6, 0);
    texCoord.setData(texCoords.data(), sizeof(glm::vec2) * pow((width * (1.0f / triangleWidth)), 2.0f) * 6, 1);
    
    vertex.activate();
    texCoord.activate();
    
    glBindVertexArray(0);
}

PerlinMapInformation PerlinMap::getMapInfo() {
    PerlinMapInformation info;
    
    info.noise = &noise;
    info.freq = freq;
    info.multiplier = multiplier;
    info.octaves = octaves;
    info.width = width;
    
    return info;
}

PerlinMap::~PerlinMap() {
    glDeleteVertexArrays(1, &this->VAO);
}

void PerlinMap::render() {
    vertex.activate();
    texCoord.activate();
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->getData());
    shader->sendInt(0, tex->getTextureName());
    
    glBindVertexArray(VAO);
    shader->sendMat4(*data->projection, "projection");
    shader->sendMat4(data->viewMat, "view");
    shader->sendMat4(model, "model");
    
    glDrawArrays(GL_TRIANGLES, 0, pow((width * (1.0f / triangleWidth)), 2.0f) * 6);
    glBindVertexArray(0);
}

void PerlinMap::setTexture(Texture *tex) {
    this->tex = tex;
}

void PerlinMap::setPosition(glm::vec3 position) {
    model = glm::translate(glm::mat4(1), position);
    this->position = position;
}

glm::vec3 PerlinMap::getPosition() {
    return position;
}

Shader* PerlinMap::getShaderPointer() {
    return shader;
}
