#define STB_IMAGE_IMPLEMENTATION

// 需要更小的编译体积时,可以只保留用到的格式(默认放开全部):
// #define STBI_ONLY_PNG
// #define STBI_ONLY_JPEG
// #define STBI_NO_STDIO      // 不要从文件路径读取时定义(用 stbi_load_from_memory)

#include "stb_image.h"
