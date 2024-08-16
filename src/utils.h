#ifndef _UTILS_H
#define _UTILS_H

#include <glad/glad.h>

#include "model.h"

GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void PrintObjModelInfo(ObjModel*); // Useful for debugging.

#endif // _UTILS_H
