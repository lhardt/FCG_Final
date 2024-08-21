#include "collisions.h"
#include "logger.h"

#include <vector>

std::vector< glm::vec4 > cornerPointsOf(SolidObject a){
    return {
        a.m.pos,
        
        a.m.pos + glm::vec4(a.size[0],         0,         0,         0),
        a.m.pos + glm::vec4(        0, a.size[1],         0,         0),
        a.m.pos + glm::vec4(        0,         0, a.size[2],         0),

        a.m.pos + glm::vec4(a.size[0], a.size[1],         0,         0),
        a.m.pos + glm::vec4(a.size[0],         0, a.size[2],         0),
        a.m.pos + glm::vec4(        0, a.size[1], a.size[2],         0),

        a.m.pos + glm::vec4(a.size[0], a.size[1], a.size[2],         0),
    };
}

bool pointInSolidObject(glm::vec4 pt, SolidObject b){
    return pt[0] >= b.m.pos[0] && pt[0]  <= b.m.pos[0] + b.size[0] 
        && pt[1] >= b.m.pos[1] && pt[1]  <= b.m.pos[1] + b.size[1]
        && pt[2] >= b.m.pos[2] && pt[2]  <= b.m.pos[2] + b.size[2] ;
}

bool checkCollision(SolidObject a, SolidObject b){
    auto corner_points = cornerPointsOf(a);
    for( glm::vec4 pt : corner_points){
        if( pointInSolidObject(pt, b))        
            return true;
    }
    
    return false;    
}


