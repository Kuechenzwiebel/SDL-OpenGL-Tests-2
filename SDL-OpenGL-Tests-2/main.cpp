//
//  main.cpp
//  SDL-OpenGL-Tests-2
//
//  Created by Tobias Pflüger on 06.03.19.
//  Copyright © 2019 Tobias Pflüger. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <cmath>
#include <list>
#include <utility>
#include <chrono>

#define GLEW_STATIC
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#include <SOIL2/SOIL2.h>

#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/normal.hpp>
#include <glm/gtx/norm.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <HG_List/HG_List.h>

using namespace glm;

#include "texture.hpp"
#include "shader.hpp"
#include "object.hpp"
#include "triangle.hpp"
#include "utils.hpp"
#include "camera.hpp"
#include "rectangle.hpp"
#include "ui/uiObject.hpp"
#include "ui/uiRectangle.hpp"
#include "ui/uiText.hpp"
#include "cube.hpp"
#include "cubemap.hpp"
#include "items/items.hpp"
#include "sphere.hpp"
#include "physicsObjects/collisionInfo.h"
#include "physicsObjects/physicsObject.hpp"
#include "physicsObjects/physicsSphere.hpp"
#include "perlinMap.hpp"
#include "physicsObjects/aabb.hpp"
#include "physicsObjects/obb.hpp"
#include "physicsObjects/ray.hpp"
#include "perlin.hpp"
#include "physicsObjects/physicsWorld.hpp"

int windowWidth = 1080, windowHeight = 760;
std::string windowTitle = "SDL-OpenGL-Tests-2";

bool running = true;
bool checkMouse = false;
bool invOpen = false;
bool opticsOn = false;
bool wireframe = false;
bool render = true;

int main(int argc, const char * argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Failed to initialize SDL2");
        return EXIT_FAILURE;
    }
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    SDL_Window *window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    SDL_Event windowEvent;
    
    SDL_GL_SetSwapInterval(1);
    
    if (window == NULL) {
        printf("Failed to open Window");
        return EXIT_FAILURE;
    }
    
    glewExperimental = GL_TRUE;
    
    if (glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW");
        return EXIT_FAILURE;
    }
    
    glViewport(0, 0, windowWidth, windowHeight);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    
    mat4 projection = perspective(radians(45.0f), float(windowWidth) / float(windowHeight), 0.1f, 100.0f);
    mat4 uiProjection = ortho(-1.0f * float(windowWidth) / 1000.0f, 1.0f * float(windowWidth) / 1000.0f, -1.0f * float(windowHeight) / 1000.0f, 1.0f * float(windowHeight) / 1000.0f);
    
    int frame = 0;
    long nextMeasure = SDL_GetTicks() + 1e3;
    int fps = 0;
    unsigned long long totalFrames = 0;
    
    float lastFrame = 0.0f;
    float currentFrame = 0.0f;
    float deltaTime = 0.0f;
    
    float zoom = 1.0f;
    
    float runTime = 0.0f;
    
    std::list<std::pair<float, Object*>> objects;
    std::vector<UIObject*> uiObjects;
    PhysicsWorld physicsWorld;
    
    hg::File noTexShaderVertexFile("resources/shaders/noTex.vs"), noTexShaderFragmentFile("resources/shaders/noTex.fs");
    hg::File basicShaderVertexFile("resources/shaders/basic.vs"), basicShaderFragmentFile("resources/shaders/basic.fs");
    hg::File blurShaderVertexFile("resources/shaders/blur.vs"), blurShaderFragmentFile("resources/shaders/blur.fs");
    hg::File uiShaderVertexFile("resources/shaders/ui.vs"), uiShaderFragmentFile("resources/shaders/ui.fs");
    hg::File skyboxShaderVertexFile("resources/shaders/skybox.vs"), skyboxShaderFragmentFile("resources/shaders/skybox.fs");
    
    std::string spaceSkyboxLocation[6] = {
        "resources/textures/skyboxes/skybox2RT.png",
        "resources/textures/skyboxes/skybox2LF.png",
        "resources/textures/skyboxes/skybox2UP.png",
        "resources/textures/skyboxes/skybox2DN.png",
        "resources/textures/skyboxes/skybox2FT.png",
        "resources/textures/skyboxes/skybox2BK.png"
    };
    
    Shader noTexShader(noTexShaderVertexFile.readFile(), noTexShaderFragmentFile.readFile());
    Shader basicShader(basicShaderVertexFile.readFile(), basicShaderFragmentFile.readFile());
    Shader blurShader(blurShaderVertexFile.readFile(), blurShaderFragmentFile.readFile());
    Shader uiShader(uiShaderVertexFile.readFile(), uiShaderFragmentFile.readFile());
    Shader skyboxShader(skyboxShaderVertexFile.readFile(), skyboxShaderFragmentFile.readFile());
    
    Camera cam(vec3(0.0f, 20.0f, 0.0f), &deltaTime, &windowEvent, &checkMouse);
    cam.objects = &physicsWorld;
    
    RenderData renderData;
    RenderData skyboxData;
    RenderData uiData;
    
    renderData.projection = &projection;
    renderData.viewMat = cam.getViewMatrix();
    renderData.windowWidth = &windowWidth;
    renderData.windowHeight = &windowHeight;
    
    skyboxData.projection = &projection;
    skyboxData.viewMat = mat4(mat3(cam.getViewMatrix()));
    skyboxData.windowWidth = &windowWidth;
    skyboxData.windowHeight = &windowHeight;
    
    uiData.projection = &uiProjection;
    uiData.viewMat = lookAt(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    uiData.windowWidth = &windowWidth;
    uiData.windowHeight = &windowHeight;
    
    
    Cubemap spaceSkybox(spaceSkyboxLocation);
    
    Texture gradientTexture("resources/textures/grad.png", TEXTURE_NO_MIP_MAP);
    Texture nebulaTexture("resources/textures/nebel.jpg");
    Texture compassHeadbarTex("resources/textures/compassBar2.png");
    Texture compassSelectorTex("resources/textures/compassSelector.png", TEXTURE_NO_MIP_MAP);
    Texture declinationMeterTex("resources/textures/declinationMeter2.png");
    Texture declinationMeterSelectorTex("resources/textures/declinationMeterSelector.png", TEXTURE_NO_MIP_MAP);
    Texture jupiterTexture("resources/textures/jupiter.jpg");
    Texture stoneTexture("resources/textures/stones.png");
    Texture invTexture("resources/textures/inv.png", TEXTURE_NO_MIP_MAP);
    Texture crosshairTexture("resources/textures/crosshair.png", TEXTURE_NO_MIP_MAP);
    
    
    PerlinMap map(420, 50, &basicShader, &renderData);
    map.setTexture(&stoneTexture);
    map.setPosition(vec3(0.0f, -2.0f, 0.0f));
    cam.setPerlinMapInfo(map.getMapInfo());
    objects.push_back(std::make_pair(0.0f, &map));
    
    
    Cube aabbTest(&basicShader, &renderData);
    aabbTest.setTexture(&nebulaTexture);
    aabbTest.setPosition(vec3(5.0f, -1.0f, 0.0f));
    objects.push_back(std::make_pair(0.0f, &aabbTest));
    
    AABB aabb1(vec3(4.5f, -1.5f, -0.5f), vec3(5.5f, -0.5f, 0.5f));
    physicsWorld.objects.push_back(&aabb1);
    
    Triangle tri(&noTexShader, &renderData);
    tri.setPosition(vec3(0.0f, 0.0f, -1.5f));
    tri.calculateModelMat();
    
    Triangle tri2(&noTexShader, &renderData);
    tri2.setPosition(vec3(-1.0f, 0.0f, 2.0f));
    tri2.calculateModelMat();
    
    objects.push_back(std::make_pair(0.0f, &tri));
    objects.push_back(std::make_pair(0.0f, &tri2));
    
    vec2 compassUVs[] = {
        vec2(0.0832f * 1.5f, 0.0f),
        vec2(0.0832f * 2.5f, 0.0f),
        vec2(0.0832f * 2.5f, 1.0f),
        vec2(0.0832f * 1.5f, 0.0f),
        vec2(0.0832f * 1.5f, 1.0f),
        vec2(0.0832f * 2.5f, 1.0f)
    };
    
    vec2 pitchUVs[] = {
        vec2(0.0f, 0.5f - 0.088975694444444f),
        vec2(1.0f, 0.5f - 0.088975694444444f),
        vec2(1.0f, 0.5f + 0.088975694444444f),
        vec2(0.0f, 0.5f - 0.088975694444444f),
        vec2(0.0f, 0.5f + 0.088975694444444f),
        vec2(1.0f, 0.5f + 0.088975694444444f)
    };
    
    UIRectangle compassBar(&uiShader, &uiData, compassUVs);
    compassBar.setTexture(&compassHeadbarTex);
    compassBar.setXTexOffset(cam.getYawPointer());
    compassBar.setXTexMultiplier(0.00093f * 2.0f);
    compassBar.setPixelSize(vec2(682.0f, 128.0f) / vec2(4.0f));
    uiObjects.push_back(&compassBar);
    
    UIRectangle declinationMeterBar(&uiShader, &uiData, pitchUVs);
    declinationMeterBar.setTexture(&declinationMeterTex);
    declinationMeterBar.setYTexOffset(cam.getPitchPointer());
    declinationMeterBar.setYTexMultiplier(-0.004448784722222f);
    declinationMeterBar.setPixelSize(vec2(128.0f, 410.0f) / vec2(4.0f));
    uiObjects.push_back(&declinationMeterBar);
    
    UIRectangle compassSelector(&uiShader, &uiData);
    compassSelector.setTexture(&compassSelectorTex);
    compassSelector.setPixelSize(vec2(256.0f / 4.0f));
    uiObjects.push_back(&compassSelector);
    
    UIRectangle declinationMeterSelector(&uiShader, &uiData);
    declinationMeterSelector.setTexture(&declinationMeterSelectorTex);
    declinationMeterSelector.setPixelSize(vec2(256.0f / 4.0f));
    uiObjects.push_back(&declinationMeterSelector);
    
    
    UIRectangle inv(&uiShader, &uiData);
    inv.setTexture(&invTexture);
    inv.setPixelSize(vec2(512.0f, 256.0f));
    
    
    UIRectangle crosshair(&uiShader, &uiData);
    crosshair.setTexture(&crosshairTexture);
    crosshair.setPixelSize(vec2(25.0f));
    uiObjects.push_back(&crosshair);
    
    
    Cube skybox(&skyboxShader, &skyboxData);
    skybox.setCubemap(&spaceSkybox);
    
    Cube sharpCube(&basicShader, &renderData);
    sharpCube.setTexture(&nebulaTexture);
    sharpCube.setPosition(vec3(5.0f));
    sharpCube.setSize(vec3(2.0f));
    sharpCube.setRotation(vec4(0.0f, 1.0f, 1.0f, radians(45.0f)));
    objects.push_back(std::make_pair(0.0f, &sharpCube));
    
    
    
    Cube bluredCube(&blurShader, &renderData);
    bluredCube.setTexture(&nebulaTexture);
    bluredCube.setPosition(vec3(2.0f, 1.0f, -3.0f));
    bluredCube.setRotation(vec4(1.0f, 1.0f, 1.0f, radians(45.0f)));
    objects.push_back(std::make_pair(0.0f, &bluredCube));
    
    OBB obb1(sharpCube.getPosition(), sharpCube.getRotation(), sharpCube.getSize());
    physicsWorld.objects.push_back(&obb1);
    
    OBB obb2(bluredCube.getPosition(), bluredCube.getRotation(), bluredCube.getSize());
    physicsWorld.objects.push_back(&obb2);
    
    
    Cube farCube(&basicShader, &renderData);
    farCube.setTexture(&nebulaTexture);
    farCube.setPosition(vec3(0.0f, 0.0f, -110.0f));
    objects.push_back(std::make_pair(0.0f, &farCube));
    
    
    
    Sphere sphere(&basicShader, &renderData);
    sphere.setPosition(vec3(0.0f));
    sphere.setTexture(&gradientTexture);
    objects.push_back(std::make_pair(0.0f, &sphere));
    
    
    
    UIText fpsText("", &uiShader, &uiData);
    fpsText.setSize(vec2(0.25f));
    uiObjects.push_back(&fpsText);
    
    UIText rayText("Ray hit:", &uiShader, &uiData);
    rayText.setSize(vec2(0.25f));
    rayText.setPixelPosition(vec2(100.0f));
    uiObjects.push_back(&rayText);
    
    
    Sphere jupiter(&basicShader, &renderData);
    jupiter.setTexture(&jupiterTexture);
    jupiter.setPosition(vec3(10.0f));
    objects.push_back(std::make_pair(0.0f, &jupiter));
    
    
    for(std::list<std::pair<float, Object*>>::iterator it = objects.begin(); it != objects.end(); it++) {
        it->first = length2(cam.getPosition() - it->second->getPosition());
    }
    
    objects.sort();
    
    vec3 oldCamPos = cam.getPosition();
    
    PhysicsSphere s1(1.0f, vec3(0.0f)), s2(1.0f, vec3(1.0f, 1.0f, 0.0f));
    CollisionInfo in = spherePointCollision(&s1, vec3(0.5f, 0.5f, 0.0f));
    in = spherePointCollision(&s1, vec3(1.0f, 0.5f, 0.0f));
    
    physicsWorld.objects.push_back(&s1);
    
    compassSelector.setPixelPosition(vec2(0.0f, (float(windowHeight) / 2.0f) - 32.0f));
    declinationMeterSelector.setPixelPosition(vec2((-float(windowWidth) / 2.0f) + 32.0f, 0.0f));
    compassBar.setPixelPosition(vec2(0.0f, (float(windowHeight) / 2.0f) - 32.0f));
    declinationMeterBar.setPixelPosition(vec2((-float(windowWidth) / 2.0f) + 32.0f, 0.0f));
    fpsText.setPixelPosition(vec2(-float(windowWidth) / 2.0f + (charWidth / 2.0f) * 0.25f, float(windowHeight) / 2.0f - (charHeight / 2.0f) * 0.25f));
    
    
    Ray crosshairRay(vec3(0.0f), vec3(0.0f), 0.1f);
    bool crosshairRayCollision = false;
    
    while(running) {
        if(SDL_GetTicks() > nextMeasure) {
            fps = frame;
            frame = 0;
            nextMeasure += 1e3;
            
            fpsText.setText(std::to_string(fps) + " FPS" + "\nFrametime: " + std::to_string(1.0f / fps) + " ms");
        }
        
        currentFrame = SDL_GetTicks() / 1000.0f;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        SDL_SetWindowTitle(window, (windowTitle + "     FPS: " + std::to_string(fps) +  "  Frametime: " + std::to_string(1.0f / fps) + "    Camera Pos X: " + std::to_string(cam.getPosition().x) + " Y: " + std::to_string(cam.getPosition().y) + " Z: " + std::to_string(cam.getPosition().z)).c_str());
     
        while(SDL_PollEvent(&windowEvent) != 0) {
            if(windowEvent.type == SDL_QUIT) {
                running = false;
            }
            
            if(windowEvent.type == SDL_MOUSEBUTTONDOWN) {
                if(windowEvent.button.button == SDL_BUTTON_LEFT) {
                    checkMouse = true;
                    render = true;
                }
            }
            
            if(windowEvent.type == SDL_WINDOWEVENT && windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        
                compassSelector.setPixelPosition(vec2(0.0f, (float(windowHeight) / 2.0f) - 32.0f));
                declinationMeterSelector.setPixelPosition(vec2((-float(windowWidth) / 2.0f) + 32.0f, 0.0f));
                compassBar.setPixelPosition(vec2(0.0f, (float(windowHeight) / 2.0f) - 32.0f));
                declinationMeterBar.setPixelPosition(vec2((-float(windowWidth) / 2.0f) + 32.0f, 0.0f));
                fpsText.setPixelPosition(vec2(-float(windowWidth) / 2.0f + (charWidth / 2.0f) * 0.25f, float(windowHeight) / 2.0f - (charHeight / 2.0f) * 0.25f));
            }
            
            if(windowEvent.type == SDL_WINDOWEVENT && windowEvent.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                render = true;
            }
            
            if(windowEvent.type == SDL_WINDOWEVENT && windowEvent.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                render = false;
            }
            
            if(windowEvent.type == SDL_MOUSEWHEEL) {
                zoom += float(windowEvent.wheel.y) * 0.025f;
                
                if(zoom < 1.0f) {
                    zoom = 1.0f;
                }
                if(zoom > 25.0f) {
                    zoom = 25.0f;
                }
            }
            
            if(windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_i) {
                swapBool(&invOpen);
            }
            
            if(windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_o) {
                swapBool(&opticsOn);
            }
            
            if(windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_f) {
                swapBool(&wireframe);
            }
            
            if(windowEvent.type == SDL_KEYDOWN && windowEvent.key.keysym.sym == SDLK_ESCAPE) {
                render = false;
                checkMouse = false;
            }
            
            
            if(!invOpen) {
                cam.processMouseInput();
            }
        }
        
        if(checkMouse) {
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }
        else {
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        
        if(render) {
            if(!invOpen) {
                cam.processInput();
            }
            
            if(wireframe == true) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            
            
            
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glViewport(0, 0, windowWidth, windowHeight);
            if(!opticsOn) {
                projection = infinitePerspective(radians(45.0f), float(windowWidth) / float(windowHeight), 0.005f);
                cam.setMouseSensitivity(0.25f);
            }
            else {
                projection = perspective(radians(zoom), float(windowWidth) / float(windowHeight), 0.5f, 1000.0f);
                cam.setMouseSensitivity(0.0025f);
            }
            
            if(cam.getPosition() != oldCamPos) {
                for(std::list<std::pair<float, Object*>>::iterator it = objects.begin(); it != objects.end(); it++) {
                    if(it->second == &map) {
                        it->first = 1000.0f;
                    }
                    else {
                        it->first = length2(cam.getPosition() - it->second->getPosition());
                    }
                }
                
                objects.sort();
            }
            
            uiProjection = ortho(-1.0f * float(windowWidth) / 1000.0f, 1.0f * float(windowWidth) / 1000.0f, -1.0f * float(windowHeight) / 1000.0f, 1.0f * float(windowHeight) / 1000.0f);
            renderData.viewMat = cam.getViewMatrix();
            skyboxData.viewMat = mat4(mat3(cam.getViewMatrix()));
            
            
            glDepthMask(GL_FALSE);
            skyboxShader.use();
            skybox.render();
            glDepthMask(GL_TRUE);
            
            
            for(std::list<std::pair<float, Object*>>::reverse_iterator it = objects.rbegin(); it != objects.rend(); it++) {
                it->second->getShaderPointer()->use();
                it->second->render();
            }
            
            crosshairRay.setRayStartPosition(cam.getPosition());
            crosshairRay.setRayDirection(cam.getPosition() + cam.getFront());
            crosshairRay.reset();
            
            crosshairRayCollision = false;
            
            for(int i = 0; i < 50; i++) {
                crosshairRayCollision = worldPointCollision(&physicsWorld, crosshairRay.getRayPosition());
                if(crosshairRayCollision) {
                    break;
                }
                crosshairRay.step();
            }
            
            if(crosshairRayCollision) {
                rayText.setText("Ray hit!");
            }
            else {
                rayText.setText("No ray hit!");
            }
            
            glClear(GL_DEPTH_BUFFER_BIT);
            
            uiShader.use();
            for(int i = 0; i < uiObjects.size(); i++) {
                uiObjects[i]->render();
            }
            
            if(invOpen) {
                inv.render();
            }
            
            frame ++;
            totalFrames++;
            runTime += deltaTime;
            oldCamPos = cam.getPosition();
            
            SDL_GL_SwapWindow(window);
            glFlush();
        }
        else {
            SDL_Delay(33);
        }
    }
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}
