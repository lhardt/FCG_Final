#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <tiny_obj_loader.h>

// See https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel {
public:
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // See: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true) ;
};

#endif /* MODEL_H */