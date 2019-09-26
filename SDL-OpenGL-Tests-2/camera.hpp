//
//  camera.hpp
//  SDL-OpenGL-Tests-2
//
//  Created by Tobias Pflüger on 16.03.19.
//  Copyright © 2019 Tobias Pflüger. All rights reserved.
//

#ifndef camera_hpp
#define camera_hpp

#include <vector>
#include <iostream>
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL2/SDL.h>

#include "physicsObjects/physicsWorld.hpp"
#include "physicsObjects/physicsObject.hpp"
#include "physicsObjects/physicsSphere.hpp"
#include "physicsObjects/aabb.hpp"
#include "physicsObjects/obb.hpp"

#include "mapChunk.hpp"
#include "perlinMap/perlinNoise.hpp"
enum CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
public:
    Camera(glm::vec3 position, const float *deltaTime, const SDL_Event *windowEvent, bool *checkMouse);
    
    glm::mat4 getViewMatrix();
    float getZoom();
    glm::vec3 getPosition();
    glm::vec3 getFront();
    float getMouseSensitivity();
    
    void setPosition(glm::vec3 position);
    void setMouseSensitivity(float sensitivity);
    
    float *getYawPointer();
    float *getPitchPointer();
    
    void processMouseInput();
    void processInput();
    
    void setMapNoise(PerlinNoise *noise);
    
    PhysicsWorld *objects;
    
    bool inVehicle;
    
private:
    glm::vec3 position, theoreticalPosition;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    
    const float *deltaTime;
    const SDL_Event *windowEvent;
    bool *checkMouse;
    
    float yaw;
    float pitch;
    
    float movementSpeed;
    float mouseSensitivity;
    float zoom;
    
    void updateCameraVectors();
    
    PerlinNoise *noise;
    bool collisionHappend;
    bool gravity;
};

#endif /* camera_hpp */
