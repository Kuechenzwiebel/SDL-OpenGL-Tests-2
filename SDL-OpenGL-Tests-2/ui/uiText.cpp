//
//  uiText.cpp
//  SDL-OpenGL-Tests-2
//
//  Created by Tobias Pflüger on 18.04.19.
//  Copyright © 2019 Tobias Pflüger. All rights reserved.
//

#include "uiText.hpp"

static glm::vec2 uiTextUVs[] = {
    glm::vec2(0.0f, 0.234375f),
    glm::vec2(0.01025390625f, 0.234375f),
    glm::vec2(0.01025390625f, 0.765625f),
    glm::vec2(0.0f, 0.234375f),
    glm::vec2(0.0f, 0.765625f),
    glm::vec2(0.01025390625f, 0.765625f)
};

UIText::UIText(std::string s, Shader *shader, const RenderData *data):
s(s), data(data), shader(shader), position(0.0f), size(1.0f), charSet("resources/textures/text.png"), colorMultiplier(1.0f) {
    rects.resize(s.length());
    originalPositions.resize(s.length());
    charSet.setTextureName("tex");
    
    float texOffset;
    int rows = 0, cols = 0;

    for(int i = 0; i < s.length(); i++) {
        if(s[i] == '\n') {
            cols = 0;
            rows++;
            
            rects[i].set(shader, data, uiTextUVs);
            rects[i].setSize(glm::vec2(0.0f, 0.0f));
            continue;
        }
        
        texOffset = (char(s[i]) - 32) * 0.01025390625f;
        rects[i].set(shader, data, uiTextUVs);
        rects[i].setXTexOffset(texOffset);
        rects[i].setTexture(&charSet);
        rects[i].setSize(pixelSizeToUIRect(glm::vec2(charWidth, charHeight)));
        rects[i].setPosition(pixelPosToUIRect(glm::vec2(cols * charWidth, -rows * charHeight), rects[i].getSize()));
        
        originalPositions[i] = rects[i].getPosition();
        
        cols++;
    }
}

UIText::~UIText() {
    
}

void UIText::setText(std::string s) {
    if(this->s != s) {
        this->s = s;
        rects.clear();
        rects.resize(s.length());
        originalPositions.clear();
        originalPositions.resize(s.length());
        
        float texOffset;
        int rows = 0, cols = 0;
        
        for(int i = 0; i < s.length(); i++) {
            if(s[i] == '\n') {
                cols = 0;
                rows++;
                
                rects[i].set(shader, data, uiTextUVs);
                rects[i].setSize(glm::vec2(0.0f, 0.0f));
                continue;
            }
            
            texOffset = (char(s[i]) - 32) * 0.01025390625f;
            rects[i].set(shader, data, uiTextUVs);
            rects[i].setXTexOffset(texOffset);
            rects[i].setTexture(&charSet);
            rects[i].setSize(pixelSizeToUIRect(glm::vec2(charWidth, charHeight)));
            rects[i].setPosition(pixelPosToUIRect(glm::vec2(cols * charWidth, -rows * charHeight), rects[i].getSize()));
            
            originalPositions[i] = rects[i].getPosition();
            
            cols++;
        }
        
        setSize(size);
        setPosition(position);
    }
}

void UIText::render() {
    shader->sendVec4(colorMultiplier, "colorMultiplier");
    for(int i = 0; i < rects.size(); i++) {
        if(rects[i].getSize() != glm::vec2(0.0f, 0.0f)) {
            rects[i].render();
        }
    }
}

void UIText::setPosition(glm::vec2 position) {
    for(int i = 0; i < rects.size(); i++) {
        rects[i].setPosition(position + originalPositions[i]);
    }
    this->position = position;
}

void UIText::setSize(glm::vec2 size) {
    for(int i = 0; i < rects.size(); i++) {
        rects[i].setSize(size * pixelSizeToUIRect(glm::vec2(charWidth, charHeight)));
    }
    this->size = size;
}

void UIText::setPixelPosition(glm::vec2 position) {
    this->position = pixelPosToUIRect(position, size * pixelSizeToUIRect(glm::vec2(charWidth, charHeight)));
    setPosition(this->position);
}

void UIText::setPixelSize(glm::vec2 size) {
    this->size = pixelSizeToUIRect(size);
    setSize(this->size);
}

void UIText::setColorMultiplier(glm::vec4 multiplier) {
    this->colorMultiplier = multiplier;
}


glm::vec2 UIText::getPosition() {
    return position;
}

glm::vec2 UIText::getSize() {
    return size;
}

Shader* UIText::getShaderPointer() {
    return shader;
}

glm::vec4 UIText::getColorMultiplier() {
    return colorMultiplier;
}
