#ifndef OBJECTS_H
#define OBJECTS_H

#include <string> 
#include <glm/vec4.hpp>
#include <glm/vec3.hpp>


struct Movable {
    glm::vec4 pos;
    glm::vec4 vel; // relative to himself
    glm::vec4 front;
    float   y_angle;
    float   scale;
};

struct SolidObject {
    Movable m;
    glm::vec3 size;
    std::string model_name;
    
    int object_id_uniform;
    bool show;
    bool show_box;
    
    bool stretch_to_size = false;
};



#endif /* OBJECTS_H */