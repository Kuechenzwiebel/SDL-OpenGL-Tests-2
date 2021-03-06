//
//  map.cpp
//  SDL-OpenGL-Tests-2
//
//  Created by Tobias Pflüger on 26.09.19.
//  Copyright © 2019 Tobias Pflüger. All rights reserved.
//

#include "map.hpp"

Map::Map(PerlinNoise *noise, Shader *shader, const RenderData *data):
noise(noise), shader(shader), data(data), texture("resources/textures/stones.png") {
    update(glm::vec3(0.0f));
}

void Map::update(glm::vec3 cameraPosition) {
    requiredChunks.clear();
    
    for(int x = -viewRange; x <= viewRange; x += CHUNK_SIZE) {
        for(int y = -viewRange; y <= viewRange; y += CHUNK_SIZE) {
            if(glm::fastDistance(glm::vec2(x, y), glm::vec2(0.0f)) < float(viewRange)) {
                requiredChunks.push_back(glm::vec2(x, y) + glm::vec2(float((int(round(cameraPosition.x)) / CHUNK_SIZE) * CHUNK_SIZE), float((int(round(cameraPosition.z)) / CHUNK_SIZE) * CHUNK_SIZE)));
            }
        }
    }
    
    
    bool chunkNotNeeded = true;
    
    for(int i = 0; i < chunks.size(); i++) {
        chunkNotNeeded = true;
        
        glm::vec2 position = chunks[i]->getPosition().xz();
        for(int j = 0; j < requiredChunks.size(); j++) {
            if(position == requiredChunks[j]) {
                requiredChunks.erase(requiredChunks.begin() + j);
                chunkNotNeeded = false;
                break;
            }
        }
        
        if(chunkNotNeeded) {
#ifdef PRINT_MAP_TIMES
            printf("Erasing %d\n", i);
#endif
            chunks.erase(chunks.begin() + i);
            i--;
        }
    }
     
#ifdef PRINT_MAP_TIMES
    printf("\n%lu chunks to generate \n\n", requiredChunks.size());
#endif
    
    for(int i = 0; i < requiredChunks.size(); i++) {
#ifdef PRINT_MAP_TIMES
        std::cout << "Generating chunk at\t\t";
        printVec2(requiredChunks[i], false);
        std::cout << "\t\t\t";
#endif
        chunks.push_back(std::make_unique<MapChunk>(noise, shader, data, requiredChunks[i]));
        chunks[chunks.size() - 1]->setTexture(&texture);
    }
    
#ifdef PRINT_MAP_TIMES
    printf("\n\n");
#endif
}

void Map::render() {
    for(int i = 0; i < chunks.size(); i++) {
        chunks[i]->render();
    }
}
