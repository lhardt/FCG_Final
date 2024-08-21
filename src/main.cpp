//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   LABORATÓRIO 4
//

// Arquivos "headers" padrões de C podem ser incluídos em um
// programa C++, sendo necessário somente adicionar o caractere
// "c" antes de seu nome, e remover o sufixo ".h". Exemplo:
//    #include <stdio.h> // Em C
//  vira
//    #include <cstdio> // Em C++
//
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tiny_obj_loader.h>
#include <stb_image.h>

#include "utils.h"
#include "matrices.h"
#include "textrendering.h"
#include "logger.h"

// Sleeping 
#include <chrono>
#include <thread>

#include "model.h"
#include "logger.h"
#include "registered_variables.h"
#include "objects.h"
#include "collisions.h"

#define WINDOW_NAME ("Dire Dire FCG")

#define SPHERE      0
#define BUNNY       1
#define PLANE       2
#define BOMB_BODY   3
#define BOMB_OTHER  4
#define WALL        5
#define PAINTING    6
#define FOOTSLAB    7
#define SHREK1      8
#define SHREK2      9
#define BLOCK       10
#define PAINTING2   11


// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void errorCallbackForGLFW(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

struct Model {
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
};

// enum CollisionType {
//     NONE = 0,
//     CUBE,
//     SPHERE
// }; 

// struct GameObject {
//     CollisionType collision;
//     std::string model_name;
//     glm::vec4 pos;
//     glm::vec4 rot;
//     float scale;
// };

enum CameraType {
    C_LOOKAT = 1,
    C_FREE = 2
};

struct FreeCamera {
    glm::vec4 pos;
    //will not always look 'forward'
    float phi, theta;
};
struct LookatCamera {
    float dist;
    std::string target;
    float phi, theta;
};

struct Camera {
    CameraType type;
    FreeCamera free;
    LookatCamera lookat;  
};

glm::vec4 makeBezier(glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 p4, float &input){
    // 0...1 => move forward
    // 1...2 => move backward
    while( input > 2 ) input -= 2;
    if( input < 0 ) input = 0;
    
    float in = input;
    if( in > 1 ) in = 2 - in;
    
    glm::vec4 p5 = p1 * (1-in) + p2 * in ;
    glm::vec4 p6 = p2 * (1-in) + p3 * in ;
    glm::vec4 p7 = p3 * (1-in) + p4 * in ;
    
    glm::vec4 p8 = p5 * (1-in) + p6 * in ;
    glm::vec4 p9 = p6 * (1-in) + p7 * in ;
    
    glm::vec4 p10= p8 * (1-in) + p9 * in ;
    
    return p10;
}

std::vector<SolidObject> g_objects = {
    {
        .m = {
            .pos = glm::vec4(-50.0f, -5.0f, 0.0f, 1.0f),
            .vel = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
            .front = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
            .y_angle = 0.0f,
            .scale = 1
        },
        .size=glm::vec4(5,5,5,1),
        .model_name = "bomb",
        .object_id_uniform = -1,
        .show = true,
        .show_box = false
    }, 
    {
         .m = {
            .pos = glm::vec4(-25,10, -110,1),
            .vel = glm::vec4(0,0,0,0),
            .front = glm::vec4(0,0,1,0),
            .y_angle = 0,
            .scale = 15
        },
        .size = glm::vec4(10,10,10,1),
        .model_name = "block",
        .object_id_uniform = BLOCK,
        .show = true,
        .show_box = false,   
        .stretch_to_size = true,
    }, 
    {
         .m = {
            .pos = glm::vec4(15,10, -110,1),
            .vel = glm::vec4(0,0,0,0),
            .front = glm::vec4(0,0,1,0),
            .y_angle = 0,
            .scale = 15
        },
        .size = glm::vec4(10,10,10,1),
        .model_name = "block",
        .object_id_uniform = BLOCK,
        .show = true,
        .show_box = false,   
        .stretch_to_size = true,
    }, 
    {
         .m = {
            .pos = glm::vec4(-13,78,-60,1),
            .vel = glm::vec4(0,0,0,0),
            .front = glm::vec4(0,0,1,0),
            .y_angle = 0,
            .scale = 15
        },
        .size = glm::vec4(30,30,30,1),
        .model_name = "block",
        .object_id_uniform = BLOCK,
        .show = true,
        .show_box = false,   
        .stretch_to_size = true,
    },
    {
        .m = {
            .pos = glm::vec4(0,110,-60,1), // glm::vec4(-20.0f, 2.0f, -20.0f, 1.0f),
            .vel = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
            .front = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
            .y_angle = 0.0f,
            .scale = 0.2
        },
        .size=glm::vec4(5,20,5,1),
        .model_name = "shrek",
        .object_id_uniform = -1,
        .show = true,
        .show_box = false
    },
    {
        .m = {
            .pos = glm::vec4(-50.0f, 0.0f, 120.71f, 1.0f),
            .vel = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
            .front = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
            .y_angle = 0.0f,
            .scale = 0.2
        },
        .size=glm::vec4(100,38,5,1),
        // .model_name = "wall",
        .object_id_uniform = -1,
        .show = false,
        .show_box = false
    },
    {
        .m = {
            .pos = glm::vec4(-50.0f, 0.0f, -125.71f, 1.0f),
            .vel = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f),
            .front = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
            .y_angle = 0.0f,
            .scale = 0.2
        },
        .size=glm::vec4(100,38,5,1),
        // .model_name = "wall",
        .object_id_uniform = -1,
        .show = false,
        .show_box = false
    },
    { // bezier block
         .m = {
            .pos = glm::vec4(-30,10, 0,1),
            .vel = glm::vec4(0,0,0,0),
            .front = glm::vec4(0,0,1,0),
            .y_angle = 0,
            .scale = 15
        },
        .size = glm::vec4(5,5, 5,1),
        .model_name = "block",
        .object_id_uniform = BLOCK,
        .show = true,
        .show_box = false,   
        .stretch_to_size = true,
    },
    { // high platform
         .m = {
            .pos = glm::vec4(-10,70,-10,1),
            .vel = glm::vec4(0,0,0,0),
            .front = glm::vec4(0,0,1,0),
            .y_angle = 0,
            .scale = 15
        },
        .size = glm::vec4(20, 15, 80,1),
        .model_name = "block",
        .object_id_uniform = BLOCK,
        .show = true,
        .show_box = false,   
        .stretch_to_size = true,
    },

};

SolidObject* g_hero = & g_objects[0]; 
SolidObject* g_testSolid = & g_objects[1];
SolidObject* g_shrek = & g_objects[2];
SolidObject* g_smallBlock = & g_objects[7];


float g_feetAnimationStep = 0;

glm::vec4 unit_vector_towards(float phi, float theta){
    // PHI in relation to Y axis, starting with the horizontal;
    // THETA in relation ZX plane, counting from Z axis, so Z direction when theta=0;  
    float x = cos(phi) * sin(-theta);    
    float z = cos(phi) * cos(-theta);
    float y = sin(phi);
    return glm::vec4(x,y,z, 0.0f);
}


Camera g_camera = {
    .type = C_LOOKAT,
    .free = {
        .pos = glm::vec4(0.0f, 80.0f, -120.0f, 1.0f),
        .phi = -0.6,
        .theta = 0.
    },
    .lookat = {
        .dist = 41,
        .phi = 0.480,
        .theta = 0.0        
    }
};

glm::mat4 computeViewMatrix( Camera & camera ){ 
    log_info("Camemra initial [p  %.2f  t  %.2f]", g_camera.lookat.phi, g_camera.lookat.theta);

    glm::vec4 vec_up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 vec_front = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    glm::vec4 camera_pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    if( camera.type == C_LOOKAT ){
        /***/
        
        LookatCamera& c = camera.lookat;
        glm::vec4 camera_dir_vec =  Matrix_Rotate_Y(g_hero->m.y_angle) * unit_vector_towards(-g_camera.lookat.phi,- g_camera.lookat.theta); 
        
        log_info("[%.2f %.2f %.2f %.2f]", camera_dir_vec.x, camera_dir_vec.y, camera_dir_vec.z, camera_dir_vec.w);
        
        vec_front = camera_dir_vec / norm(camera_dir_vec); //Always look at him from behind ; unit_vector_towards(c.phi, c.theta);
        camera_pos = g_hero->m.pos - vec_front*c.dist;
    } else {
        FreeCamera& c = camera.free;
        
        vec_front = unit_vector_towards(c.phi, c.theta);
        // glm::vec4 vec_right = crossproduct(vec_front, vec_up); 
        // glm::vec4 vec_left = -vec_right;

        camera_pos = c.pos; // Ponto "c", centro da câmera
    }
    log_info("Camemra final [p  %.2f  t  %.2f]", g_camera.lookat.phi, g_camera.lookat.theta);
    return Matrix_Camera_View(camera_pos, vec_front, vec_up);
}

struct KeyState {
    bool press;
    bool hold;
};

#define KEYS_ARR_SIZE (2000)
/** Use  GLFW_KEY_?  to index this array. */
KeyState g_keyboardState[KEYS_ARR_SIZE];

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, Model> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

float g_bezierTime = 0;

class TextureManager {
public: 
    std::map<std::string, GLuint> texture_ids;
    std::map<std::string, std::string> filenames;
    
    void loadTexture(std::string name, std::string refname = "");
    void reloadAll();
};

TextureManager g_textureManager;

void TextureManager::reloadAll(){
    for( auto & [ refname,  name] : filenames ){
        loadTexture(name, refname);
    }
}

void TextureManager::loadTexture(std::string name, std::string refname){
    if( refname == "" ) refname = name;
    std::string filename = "../data/" + name + ".png";
    log_info("Will load texture %s [%d] from file %s", name.c_str(), texture_ids.size(), filename.c_str());
    stbi_set_flip_vertically_on_load(true);
    
    int width, height, channels;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &channels, 3);

    log_assert(data != NULL, "Could not load image from [%s]", filename.c_str());
    log_info("Loaded image with h %d w %d c %d" , height, width, channels);
    
    GLuint texture_id, sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = (GLuint) texture_ids.size();

    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);
    
    texture_ids[ refname ] = textureunit;
    filenames[ refname ] = name;
    
}

class ShaderManager {
public:
    std::map<std::string, GLuint> vertex_shader_ids;
    std::map<std::string, GLuint> fragment_shader_ids;
    std::map<std::string, GLuint> gpu_program_ids;
    
    /** Register a program under a name of your choosing. */
    GLuint registerProgram(std::string program_name, std::string vertex_shader_filename, std::string fragment_shader_filename);
    
    GLuint getOrLoadVertexShader(std::string filename);
    GLuint getOrLoadFragmentShader(std::string filename);
    bool   loadShader(std::string filename, GLuint shader_id);
    GLuint getProgramId(std::string program_name);
    
    void   reloadAll();
};


void   ShaderManager::reloadAll(){
    for(auto& [filename, shader_id] : vertex_shader_ids){
        std::string filename_copy = filename;
        vertex_shader_ids.erase(filename);
        getOrLoadVertexShader(filename_copy);
    }
    for(auto& [filename, shader_id] : fragment_shader_ids){
        std::string filename_copy = filename;
        fragment_shader_ids.erase(filename);
        getOrLoadFragmentShader(filename_copy);
    }
}

GLuint ShaderManager::getProgramId(std::string program_name){
    log_assert(gpu_program_ids.find(program_name) != gpu_program_ids.end(), "Calling an unregistered program? [%s]", program_name.c_str());
    GLuint program_id = gpu_program_ids[program_name];
    log_info("Using GL Program %u ", program_id);
    
    glUseProgram(0);
    glUseProgram(program_id);

    g_model_uniform      = glGetUniformLocation(program_id, "model"); // Variável da matriz "model"
    g_view_uniform       = glGetUniformLocation(program_id, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(program_id, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(program_id, "object_id"); // Variável "object_id" em shader_fragment.glsl

    for( auto & [ texture_name, texture_id ] : g_textureManager.texture_ids ){  
        // When the shader does not have the uniform,   glGetUniformLocation outputs -1.
        // When glUniform1i receives -1, it will silently ignore (i.e. not put a texture where it shouldn't)
        // So referencing textures by name is actually fine. 
        log_debug("Binding uniform texture [%s] = %d to program.", texture_name.c_str(), texture_id);
        GLint uniform_location = glGetUniformLocation(program_id, texture_name.c_str());
        glUniform1i( uniform_location, texture_id);
        if( uniform_location == -1 ){
            log_debug("Warning: shader uniform unbound!");
        }
    }
    return program_id;
}

bool   ShaderManager::loadShader(std::string filename, GLuint shader_id){
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        log_severe("Cannot load Fragment shader on [%s]. Reason: %s\n", filename.c_str(), e.what());
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compilation_succeeded;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compilation_succeeded);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    std::vector<char> log_buffer(log_length + 1 );
    glGetShaderInfoLog(shader_id, log_length, &log_length, &log_buffer[0]);

    std::string log( &log_buffer[0] );
    if(!compilation_succeeded){
        log_severe("OpenGL compilation of shader [%s] FAILED. See log below: \n%s", filename.c_str(), log.c_str());
        std::exit(EXIT_FAILURE);
    } else {
        log_info("OpenGL compilation of shader [%s] successful! See logs: %s\n", filename.c_str(), log.c_str());
    }
    return compilation_succeeded;
}

GLuint  ShaderManager::getOrLoadFragmentShader(std::string filename){
    if( fragment_shader_ids.find(filename) == fragment_shader_ids.end() ){
        GLuint shader_id = glCreateShader(GL_FRAGMENT_SHADER);
        bool success = loadShader(filename, shader_id);
        log_assert(success, "Could not load shader from file! [%s].", filename);
        log_info("Registered fragment shader as %d", shader_id);
        fragment_shader_ids[ filename ] = shader_id;
    }
    return fragment_shader_ids[ filename ];
}

GLuint  ShaderManager::getOrLoadVertexShader(std::string filename){
    if( vertex_shader_ids.find(filename) == vertex_shader_ids.end() ){
        GLuint shader_id = glCreateShader(GL_VERTEX_SHADER);
        bool success = loadShader(filename, shader_id);
        log_assert(success, "Could not load shader from file! [%s].", filename);
        log_info("Registered vertex shader as %d", shader_id);
        vertex_shader_ids[ filename ] = shader_id;
    }
    return vertex_shader_ids[ filename ];
}

GLuint ShaderManager::registerProgram(std::string program_name, std::string vertex_shader_filename, std::string fragment_shader_filename){
    GLuint vertex_shader_id = getOrLoadVertexShader(vertex_shader_filename);
    GLuint fragment_shader_id = getOrLoadFragmentShader(fragment_shader_filename);
    
    GLuint program_id = CreateGpuProgram(vertex_shader_id, fragment_shader_id);
    log_info("Registered GPU program '%s' (%d, %d) with id %d. ", program_name.c_str(), vertex_shader_id, fragment_shader_id, program_id );
    
    gpu_program_ids[program_name] = program_id;
    return program_id;
}

std::map< std::string, RegisteredVariable > g_globalVariables;
TypingMode g_typingMode = tm_PLAY;
std::string g_stringInput = "";
std::string g_editVariable = "";

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
//float g_CameraTheta = -3.14f; // Ângulo no plano ZX em relação ao eixo Z
//float g_CameraPhi = 0.75f;   // Ângulo em relação ao eixo Y
//float g_CameraDistance = 80; // Distância da câmera para a origem

bool g_shouldLogFrame = true;
bool g_fixedFPS = true;
float  g_targetFPS = 30.0;
long long g_experimentalState = 0;

ShaderManager g_shaderManager;

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

/// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// bool g_showBunny = false;
bool g_showFloor = true;


float g_baseSpeed = 40.0f;
float g_baseSideSpeed = 8.0f;
float g_baseAngleSpeed = 2.0f; // in radians per second?
float g_gravity = -100.0;
float g_jumpForce = 1000;

void moveCameraByKeyboard(float delta_time){
    float frontSpeed = 0.0f;
    float sideSpeed = 0.0f;
    
    float speedMultiplier = 100;

    if(g_keyboardState[ GLFW_KEY_UP ].hold )    frontSpeed += 1;
    if(g_keyboardState[ GLFW_KEY_DOWN ].hold )  frontSpeed -= 1;
    if(g_keyboardState[ GLFW_KEY_LEFT ].hold )  sideSpeed -= 1;
    if(g_keyboardState[ GLFW_KEY_RIGHT ].hold ) sideSpeed += 1;

    glm::vec4 vec_up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 vec_front = unit_vector_towards(g_camera.free.phi, g_camera.free.theta);
    glm::vec4 vec_right = crossproduct(vec_front, vec_up); 
    // glm::vec4 vec_left = -vec_right;
    
    glm::vec4 vec_move_front = vec_front * frontSpeed * delta_time * speedMultiplier;
    glm::vec4 vec_move_hor = vec_right * sideSpeed * delta_time * speedMultiplier;
    log_info("vecinfo %.2f %.2f %.2f %.2f", vec_front[0], vec_front[1], vec_front[2], vec_front[3]);
    log_info("vecinfo %.2f %.2f %.2f %.2f", vec_front[0], frontSpeed, delta_time, speedMultiplier);

    g_camera.free.pos += vec_move_front + vec_move_hor;
}

void setSpeedByKeyboard(Movable& m, float delta_time){
    float LEG_MOVEMENT_SPEED = 1.8f;
    float frontSpeed = 0.0f;
    float sideSpeed = 0.0f;
    float verticalSpeed = 0.0f;
    
    if(g_keyboardState[ GLFW_KEY_W ].hold )  frontSpeed += 1;
    if(g_keyboardState[ GLFW_KEY_S ].hold )  frontSpeed -= 1;
    if(g_keyboardState[ GLFW_KEY_A ].hold )   sideSpeed -= 1;
    if(g_keyboardState[ GLFW_KEY_D ].hold )   sideSpeed += 1;
    if(g_keyboardState[ GLFW_KEY_SPACE ].press )   verticalSpeed += 1;

    m.vel[ 2 ] = std::max(frontSpeed * g_baseSpeed,  std::abs(sideSpeed) * g_baseSideSpeed);
    m.vel[ 1 ] += verticalSpeed * delta_time * g_jumpForce;

    if( m.pos[ 1 ] > 0.01 ) {
        m.vel[1] += g_gravity * delta_time;
        log_info("Gravity. Vel was %.2f and now is %.2f", m.vel[1], m.vel[1] - g_gravity * delta_time);
    } else {
        m.vel[1] = std::max(0.0f, m.vel[1]);
        m.pos[1] = 0.;
    }
    
    
    log_debug("Object speed %.2f %.2f %.2f ", m.vel[0], m.vel[1], m.vel[2]);

    float angleSpeed = -1 *  sideSpeed *  g_baseAngleSpeed * delta_time;
    log_info("Angle speed is %f %f %f = %f ", sideSpeed, g_baseAngleSpeed, delta_time, angleSpeed);

    glm::vec4 newfront = m.front;
    m.y_angle += angleSpeed;

    newfront[0] =  m.front[2] * sin(angleSpeed) + m.front[0] * cos(angleSpeed);
    newfront[1] = 0; // It helps with the camera calculation if 'front' is completely horizontal.
    newfront[2] =  m.front[2] * cos(angleSpeed) - m.front[0] * sin(angleSpeed);
    
    m.front = newfront;
    
    // feet step animation based on delta time
    if( frontSpeed > 0 ) g_feetAnimationStep += delta_time * LEG_MOVEMENT_SPEED;
    while(g_feetAnimationStep > 1) g_feetAnimationStep -= 1;
}

float getFeetAngle(){
    float LEGS_SPREAD_CONST = 0.333f; // only allow angles btwn. -X rad and X rad
    // feet animation -- at 0   the feet are together down -- agle 0
    //                   at 0.5 the feet are spread: one at angle  MAX and other -MAX
    //                   at 1   they are together again
   
    // sin(x) is zero both in 0 and in 2pi, and wobbles btwn positive and negative in the path.
    float angleStep = sin(g_feetAnimationStep * 2 * 3.141598f);  
   
    return angleStep * LEGS_SPREAD_CONST; // allow negative values for feet to switch places
}

void drawSolid(SolidObject & s ){
    if( s.show_box ) {
        glm::vec3 half_size = s.size * 0.5f;
        glm::mat4 model
            = Matrix_Translate(s.m.pos[0] + half_size[0],s.m.pos[1] + half_size[1],s.m.pos[2] + half_size[2])
            * Matrix_Scale(half_size[0], half_size[1], half_size[2])
            * Matrix_Rotate_Y(s.m.y_angle);

        log_info("Will show box of [%s]", s.model_name.c_str());
        
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, 2);
        DrawVirtualObject("block");
    }
    if( s.show ) {
        glm::vec3 half_size = s.size * 0.5f;
        glm::vec3 size_scale = s.size * 0.5f / s.m.scale;
        glm::mat4 model
            = Matrix_Translate(s.m.pos[0] + half_size[0],s.m.pos[1] + half_size[1],s.m.pos[2] + half_size[2])
            * (s.stretch_to_size ?  Matrix_Scale(size_scale[0],size_scale[1],size_scale[2]) : Matrix_Scale(s.m.scale,s.m.scale,s.m.scale))
            * Matrix_Scale(s.m.scale,s.m.scale,s.m.scale)
            * Matrix_Rotate_Y(s.m.y_angle);
            
        log_info("Will show solid [%s]", s.model_name.c_str());
        
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, 2);

        if( s.model_name == "shrek" ) {
            glm::mat4 model
                = Matrix_Translate(s.m.pos[0] + half_size[0],s.m.pos[1] + half_size[1]  - 10,s.m.pos[2] + half_size[2])
                * Matrix_Scale(s.m.scale,s.m.scale,s.m.scale)
                * Matrix_Rotate_Y(s.m.y_angle)
                * Matrix_Rotate_Z( getFeetAngle() * 0.5);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));

            glUniform1i(g_object_id_uniform, SHREK2);
            DrawVirtualObject("shrek2");
            glUniform1i(g_object_id_uniform, SHREK2);
            DrawVirtualObject("shrek3");
            glUniform1i(g_object_id_uniform, SHREK1);
            DrawVirtualObject("shrek4");
            glUniform1i(g_object_id_uniform, SHREK1);
            DrawVirtualObject("shrek5");            
        } else if ( s.model_name == "bomb" ){
            model = Matrix_Translate(s.m.pos[0],s.m.pos[1],s.m.pos[2])
                * Matrix_Scale(s.m.scale, s.m.scale, s.m.scale)
                * Matrix_Rotate_Y(s.m.y_angle)
                * Matrix_Rotate_Z( getFeetAngle() * 0.2);

            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));

            glUniform1i(g_object_id_uniform, BOMB_BODY);
            DrawVirtualObject("bomb_body");

            glUniform1i(g_object_id_uniform, BOMB_OTHER);
            DrawVirtualObject("bomb_other");
            DrawVirtualObject("bomb_needle");
            
            model = Matrix_Translate(s.m.pos[0],s.m.pos[1],s.m.pos[2])
                * Matrix_Scale(s.m.scale, s.m.scale, s.m.scale)
                * Matrix_Rotate_Y(s.m.y_angle)
                * Matrix_Translate(0.0f, 4.0f, 0.0f)
                * Matrix_Rotate_X( getFeetAngle() * 2 )
                * Matrix_Translate(0.0f, -4.0f, 0.0f);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            DrawVirtualObject("bomb_foot1");
            model = Matrix_Translate(s.m.pos[0],s.m.pos[1],s.m.pos[2])
                * Matrix_Scale(s.m.scale, s.m.scale, s.m.scale)
                * Matrix_Rotate_Y(s.m.y_angle)
                * Matrix_Translate(0.0f, 4.0f, 0.0f)
                * Matrix_Rotate_X( -getFeetAngle() * 2 )
                * Matrix_Translate(0.0f, -4.0f, 0.0f);
            glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            DrawVirtualObject("bomb_foot2");
        } else {
            glUniform1i(g_object_id_uniform, s.object_id_uniform);
            DrawVirtualObject( s.model_name.c_str() );
        }


    }
}

glm::vec4 tryMoveTo(SolidObject & s, float delta_time, glm::vec4 new_pos, float throwback){
    Movable & m = s.m;
    glm::vec4 old_pos = s.m.pos; 
    s.m.pos = new_pos; 
    for( int i = 1; i < g_objects.size(); ++i){
        SolidObject other = g_objects[i];
        bool this_collides = checkCollision( s, other );
        if( this_collides ){
            // Throwback if collision is detected.
            new_pos = m.pos;
            glm::vec4 dist = old_pos - new_pos;
            new_pos = new_pos +  dist * throwback;
        }
    }
    if( s.m.pos[1] < 0 ) s.m.pos[1] = 0;
    return s.m.pos = new_pos;
}

void moveObject(SolidObject & s, float delta_time){    
    Movable & m = s.m;
    
    glm::vec4 vec_up = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    glm::vec4 vec_right = crossproduct(m.front, vec_up); 
    // glm::vec4 vec_left = -vec_right;

    glm::vec4 new_pos = m.pos;
    new_pos +=  m.front   * m.vel[2] * delta_time;
    new_pos +=  vec_up    * m.vel[1] * delta_time;
    new_pos +=  vec_right * m.vel[0] * delta_time;
    
    { /* If can't move up or down,  */
        float old_y = m.pos[1];
        
        glm::vec4 new_pos_only_y = m.pos;
        new_pos_only_y[1] = new_pos[1];
        tryMoveTo(s, delta_time, new_pos_only_y, 1);

        float new_y = m.pos[1];
        bool y_changed = (std::abs(new_y - old_y) > 1e-6); 
        if( !y_changed ) {
            s.m.vel[1] = 0;
        }
    }
    {
        glm::vec4 new_pos_only_z = m.pos;
        new_pos_only_z[2] = new_pos[2];
        tryMoveTo(s, delta_time, new_pos_only_z, 1.3);
    }
    {
        glm::vec4 new_pos_only_x = m.pos;
        new_pos_only_x[0] = new_pos[0];
        tryMoveTo(s, delta_time, new_pos_only_x, 1.3);
    }
}

void handleStringCommand(std::string command){
    if(command == "freeCamera"){
        *(g_globalVariables["cameraType"].ir_value) = 2;
        return;
    }
    if(command == "lookatCamera"){
        *(g_globalVariables["cameraType"].ir_value) = 1;
        return;
    }
    
    if(command == "exit"){
        log_info("Processing command 'exit', shutting down.");
        std::exit(0);
    }
    if(g_globalVariables.find(command) != g_globalVariables.end()){
        log_info("Captured string command! Will edit variable [%s].", command.c_str());
        RegisteredVariable r = g_globalVariables[command];
        g_typingMode = typingModeOf(r.type);
        g_editVariable = command;
        log_info("%s %s = %s", variableTypeToStr(r.type).c_str(), command.c_str(), variableValueToString(r).c_str());
    } else {
        log_info("Did not recognize string command [%s]. Ignoring", command.c_str());    
    } 
    
}


bool test1 = false;
bool test2 = false;
bool test3 = false;
bool test4 = false;
bool test5 = false;

bool initGlobalVariables(){
    // Camera
    log_assert( sizeof(g_camera.type) == sizeof(int) , "Should not use enum as int-sized!");
    g_globalVariables["cameraType"] = makeIntRef((int*)(& g_camera.type) );
    g_globalVariables["cameraDist"] = makeFloatRef(& g_camera.lookat.dist );
    g_globalVariables["cameraTheta"] = makeFloatRef(& g_camera.lookat.theta );
    g_globalVariables["cameraPhi"] = makeFloatRef(& g_camera.lookat.phi );
    // Input/Output
    g_globalVariables["stringInput"] = makeStringRef(& g_stringInput );
    g_globalVariables["showInfoText"] = makeBoolRef(& g_ShowInfoText );
    g_globalVariables["targetFPS"] = makeFloatRef(& g_targetFPS );
    g_globalVariables["fixedFPS"] = makeBoolRef(& g_fixedFPS );
    // 
    g_globalVariables["bombX"] = makeFloatRef(& g_hero->m.pos[0] );
    g_globalVariables["bombY"] = makeFloatRef(& g_hero->m.pos[1] );
    g_globalVariables["bombZ"] = makeFloatRef(& g_hero->m.pos[2] );
    g_globalVariables["bombScale"] = makeFloatRef(& g_hero->m.scale );
    // g_globalVariables["showBunny"] = makeBoolRef(& g_showBunny );
    g_globalVariables["showBomb"] = makeBoolRef(& g_hero->show );
    g_globalVariables["showBombBox"] = makeBoolRef(& g_hero->show_box );
    g_globalVariables["showShrek"] = makeBoolRef(& g_shrek->show );
    g_globalVariables["showShrekBox"] = makeBoolRef(& g_shrek->show_box );
    g_globalVariables["showTestBox"] = makeBoolRef(& g_testSolid->show_box );
    g_globalVariables["baseSpeed"] = makeFloatRef(& g_baseSpeed);    
    g_globalVariables["baseASpeed"] = makeFloatRef(& g_baseAngleSpeed);
    
    g_globalVariables["t1"] = makeBoolRef(& test1);
    g_globalVariables["t2"] = makeBoolRef(& test2); // f
    g_globalVariables["t3"] = makeBoolRef(& test3); // n 
    g_globalVariables["t4"] = makeBoolRef(& test4); // y /
    g_globalVariables["t5"] = makeBoolRef(& test5);

    g_globalVariables["sx"] = makeFloatRef(& g_testSolid->m.pos[0]);
    g_globalVariables["sy"] = makeFloatRef(& g_testSolid->m.pos[1]);
    g_globalVariables["sz"] = makeFloatRef(& g_testSolid->m.pos[2]);

    g_globalVariables["sw"] = makeFloatRef(& g_testSolid->size[0]);
    g_globalVariables["sh"] = makeFloatRef(& g_testSolid->size[1]);
    g_globalVariables["sd"] = makeFloatRef(& g_testSolid->size[2]);
    
    g_globalVariables["jumpForce"] = makeFloatRef(& g_jumpForce);
    g_globalVariables["gravity"] = makeFloatRef(& g_gravity);
    return true;
}

void ge_gl_log_vendor_info(){
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    log_info("---- GL Version Info ----");
    log_info("  GL Version    %s ", (const char *)glversion);
    log_info("  GLSL Version: %s ", (const char *)renderer);
    log_info("  GL Vendor:    %s ", (const char *)vendor);
    log_info("  GL Renderer:  %s ", (const char *)glslversion);
    log_info("-------------------------");
}

void glfw_error_callback(int code, const char * msg){
    log_info("GLFW Error! [%d] %s", code, msg );
    app_logger.flush_queue();
}

bool ge_gl_init(GLFWwindow** out){
    log_info("Initializing GLFW...");

    bool success = glfwInit();
    if(! success ){
        log_error("Could not initialize GLFW! Aborting.");
        return false;
    }

    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    GLFWwindow* window = glfwCreateWindow(800, 800, WINDOW_NAME, NULL, NULL);

    if(window == NULL){
        log_error("Could not create GLFW Window!");
        return false;
    }

    *out = window;

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    
    ge_gl_log_vendor_info();

    log_info("Initialized GLFW successfully!");
    return true;
}

void readFilesAndAddObjectsToScene(std::vector<std::string> filenames){
    log_info("Started reading %d files and adding to scene.", filenames.size());
    for( std::string filename : filenames){
        ObjModel model(filename.c_str());
        ComputeNormals(&model);
        BuildTrianglesAndAddToVirtualScene(&model);
    }
    log_info("Finished reading %d files and adding to scene.", filenames.size());
}

void CharCallback(GLFWwindow* window, unsigned int codepoint){
    if( g_typingMode == tm_PLAY){
        g_stringInput = "";
        return;
    }
    
    char* arr = static_cast<char*>(static_cast<void*>(&codepoint));
    
    if(codepoint < 256){
        g_stringInput += (unsigned char) codepoint;
    } else {
        log_error("The following Unicode codepoint was not understood. [%d]. Ignoring.", codepoint);
    }
    log_info("Char callback [%d %d %d %d]. String Input now is [%s]", (int)arr[0], (int)arr[1], (int)arr[2], (int)arr[3], g_stringInput.c_str()); 
    g_shouldLogFrame = true;
}

bool clearKeyboardState(bool init = false){
    for(int i = 0; i < KEYS_ARR_SIZE; ++i){
        // Only clear 'hold' state if the program is starting.
        
        if(init) {
            g_keyboardState[i].hold = false;
            g_keyboardState[i].press = false;            
        } else {
            g_keyboardState[i].hold = g_keyboardState[i].press || g_keyboardState[i].hold;
            g_keyboardState[i].press = false;
        }    
    }
    return true;
}

int main(int argc, char* argv[]) {
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    GLFWwindow * window;

    bool initialize_success 
            =  ge_logger_init();
            
    log_info("Started program.");
    initialize_success &= true
            && ge_gl_init(&window)
            && initGlobalVariables()
            && clearKeyboardState(true);

    if( !initialize_success || window == NULL ){
        log_error("Failure on initialization! Exiting ... ");
        return -1;
    }

    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetCharCallback(window, CharCallback);

    glfwSetWindowSize(window, 800, 800);

    glfwMakeContextCurrent(window);

    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, 800, 800); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    ShaderManager g_shaderManager;
    g_shaderManager.registerProgram("main", "../src/shader_vertex.glsl","../src/shader_fragment.glsl");

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    readFilesAndAddObjectsToScene({
        "../data/sphere.obj",
        "../data/bunny.obj",
        "../data/floor.obj",
        "../data/bobomb.obj",
        "../data/painting.obj",
        "../data/footslab.obj",
        "../data/wall.obj",
        "../data/Shrek.obj",
        "../data/block.obj",
    });
    
    g_textureManager.loadTexture("bomb_other"); // TextureImage1
    g_textureManager.loadTexture("bomb_body");  // TextureImage0
    g_textureManager.loadTexture("floor_tile");
    g_textureManager.loadTexture("painting");
    g_textureManager.loadTexture("painting2");
    g_textureManager.loadTexture("footslab");
    g_textureManager.loadTexture("wall_tile4", "wall_tile");
    g_textureManager.loadTexture("shrek1");
    g_textureManager.loadTexture("shrek2");
    g_textureManager.loadTexture("block");
    
    TextRendering_Init();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    long long i_frame = 0;
    
    double lastFrameClock = 0;

    log_info("Finished initialization.");

    double last_delta_sample = glfwGetTime(); 
    while(!glfwWindowShouldClose(window)) {
        double targetFrameTimeSecs = 1.0 / g_targetFPS;


        ++i_frame;
        
        if(g_shouldLogFrame){
            app_logger.flush_queue();
        } else {
            app_logger.clear_queue();
        }
        g_shouldLogFrame = false;
        log_info("Cleared logging queue and started frame %d.", i_frame);

        log_info("Typing mode is currently %s.", typingModeToStr(g_typingMode).c_str());
                
        
        //////////////////////////////////////////////////////////////////////
        // Physics
        double current_clock = glfwGetTime();
        float delta_time = (float) (current_clock - last_delta_sample);
        
        /* When the timespan is too high (i.e. giant lag), presume restarting from 'average' frame */
        if( delta_time > 0.5 ){
            delta_time = 0.05;  
        }
        log_info("Physics Delta Time is %f ", delta_time); 
        
        if( g_typingMode == tm_PLAY ){
            setSpeedByKeyboard(g_hero->m, delta_time);            
            moveCameraByKeyboard(delta_time);
        }
        moveObject(*g_hero, delta_time);
        
        g_bezierTime += 0.3 *  delta_time;
        g_smallBlock->m.pos = makeBezier( 
                    glm::vec4( -80, 0, -60, 1),
                    glm::vec4( -30, 0, -110, 1), 
                    glm::vec4( 30, 0, -110, 1) ,
                    glm::vec4( 80, 0,  -60, 1), g_bezierTime);
        
        last_delta_sample = current_clock;
        //////////////////////////////////////////////////////////////////////
        // Rendering 


        //           R     G     B     A #91B6D4
        glClearColor( 0x91/256.0f, 0xB6/256.0f, 0xD4/256.0f, 1.0f);

        // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
        // e também resetamos todos os pixels do Z-buffer (depth buffer).
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view_matrix = computeViewMatrix( g_camera );
        glm::mat4 projection;

        // Note que, no sistema de coordenadas da câmera, os planos near e far
        // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
        float nearplane = -0.1f;  // Posição do "near plane"
        float farplane  = -300.0f; // Posição do "far plane"

        if (g_UsePerspectiveProjection) {
            // Projeção Perspectiva.
            // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
            float field_of_view = 3.141592 / 3.0f;
            projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
        } else {
            // Projeção Ortográfica.
            // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
            // PARA PROJEÇÃO ORTOGRÁFICA veja slides 219-224 do documento Aula_09_Projecoes.pdf.
            // Para simular um "zoom" ortográfico, computamos o valor de "t"
            // utilizando a variável g_CameraDistance.
            float t = 1.5f*g_camera.lookat.dist/2.5f;
            float b = -t;
            float r = t*g_ScreenRatio;
            float l = -r;
            projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
        }

        glUseProgram( g_shaderManager.getProgramId("main") );

        glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

        // Enviamos as matrizes "view" e "projection" para a placa de vídeo
        // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
        // efetivamente aplicadas em todos os pontos.
        glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view_matrix));
        glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
    
        for( SolidObject & obj : g_objects ){
            drawSolid(obj);
        }
        // drawSolid(g_testSolid);
        // drawSolid(g_hero);
        // drawSolid(g_shrek);

        model = Matrix_Identity();
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));

        if( g_showFloor ){
            glUniform1i(g_object_id_uniform, PLANE);
            DrawVirtualObject("floor");            
        }

        glUniform1i(g_object_id_uniform, FOOTSLAB);
        DrawVirtualObject("footslab");


        glUniform1i(g_object_id_uniform, WALL);
        DrawVirtualObject("wall");

        // if( g_showBunny ){
        //     // Desenhamos o modelo do coelho
        //     model = Matrix_Translate(1.0f,0.0f,0.0f)
        //         * Matrix_Rotate_Z(g_AngleZ)
        //         * Matrix_Rotate_Y(g_AngleY)
        //         * Matrix_Rotate_X(g_AngleX);
        //     glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        //     glUniform1i(g_object_id_uniform, BUNNY);
        //     DrawVirtualObject("the_bunny");
        // }
        
        // if( g_showBomb ){
            // model = Matrix_Translate(g_hero.m.pos[0],g_hero.m.pos[1],g_hero.m.pos[2])
            //     * Matrix_Scale(g_hero.scale, g_hero.scale, g_hero.scale)
            //     * Matrix_Rotate_Y(g_hero.y_angle);

            // glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));

            // glUniform1i(g_object_id_uniform, BOMB_BODY);
            // DrawVirtualObject("bomb_body");

            // glUniform1i(g_object_id_uniform, BOMB_OTHER);
            // DrawVirtualObject("bomb_other");
            // DrawVirtualObject("bomb_needle");
            
            // model = Matrix_Translate(g_hero.pos[0],g_hero.pos[1],g_hero.pos[2])
            //     * Matrix_Scale(g_hero.scale, g_hero.scale, g_hero.scale)
            //     * Matrix_Rotate_Y(g_hero.y_angle)
            //     * Matrix_Translate(0.0f, 4.0f, 0.0f)
            //     * Matrix_Rotate_X( getFeetAngle() * 2 )
            //     * Matrix_Translate(0.0f, -4.0f, 0.0f);
            // glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            // DrawVirtualObject("bomb_foot1");
            // model = Matrix_Translate(g_hero.pos[0],g_hero.pos[1],g_hero.pos[2])
            //     * Matrix_Scale(g_hero.scale, g_hero.scale, g_hero.scale)
            //     * Matrix_Rotate_Y(g_hero.y_angle)
            //     * Matrix_Translate(0.0f, 4.0f, 0.0f)
            //     * Matrix_Rotate_X( -getFeetAngle() * 2 )
            //     * Matrix_Translate(0.0f, -4.0f, 0.0f);
            // glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
            // DrawVirtualObject("bomb_foot2");
        // }

        model = Matrix_Translate(-20.0f, 0.0f, 120.f)
              * Matrix_Rotate_Y( 3.141598 );
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PAINTING);
        DrawVirtualObject("painting");

        model = Matrix_Translate(+20.0f, 0.0f, 120.f)
              * Matrix_Rotate_Y( 3.141598 );
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PAINTING);
        DrawVirtualObject("painting");

        model = Matrix_Translate( 0.0f, 0.0f, -120.f) ;
        glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
        glUniform1i(g_object_id_uniform, PAINTING2);
        DrawVirtualObject("painting");
        
        char debug_info[500] = {0};
        
        snprintf(debug_info, 499, "?");
        
        if( g_typingMode == tm_PLAY ){
            snprintf(debug_info, 499, "Play");
        }
        if( g_typingMode == tm_WRITE) {
            snprintf(debug_info, 499, "> %s", g_stringInput.c_str());
        }
        if( g_typingMode == EDIT_BOOL || g_typingMode == EDIT_FLOAT || g_typingMode == EDIT_STRING || g_typingMode == EDIT_INT){
            snprintf(debug_info, 499, "%s %s = %s", typingModeToStr(g_typingMode).c_str(), g_editVariable.c_str(), g_stringInput.c_str());
        }

        float pad = TextRendering_LineHeight(window);
        TextRendering_PrintString(window, debug_info, -1.0f, 1.0f-pad, 1.0f);

        
        if ( g_ShowInfoText ) {
            // TODO: TextRendering_ShowCameraAngle(window, camera?);
            // TextRendering_ShowEulerAngles(window, g_AngleX, g_AngleY, g_AngleZ);
            TextRendering_ShowProjection(window, g_UsePerspectiveProjection);
            TextRendering_ShowFramesPerSecond(window);
        }
        // O framebuffer onde OpenGL executa as operações de renderização não
        // é o mesmo que está sendo mostrado para o usuário, caso contrário
        // seria possível ver artefatos conhecidos como "screen tearing". A
        // chamada abaixo faz a troca dos buffers, mostrando para o usuário
        // tudo que foi renderizado pelas funções acima.
        // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics
        glfwSwapBuffers(window);

        clearKeyboardState(); // Valid until next frame's polling.
        glfwPollEvents();

        double currentClock = glfwGetTime();
        double currentFrameDuration = currentClock - lastFrameClock;
        double sleepTime = targetFrameTimeSecs - currentFrameDuration;
        int sleepTimeMillis = std::max( 0, (int)(1000.0*sleepTime));
        
        log_info("Finished frame in %.4lf seconds (Target is %.4lf). SleepTime = %d ms. ", currentFrameDuration, targetFrameTimeSecs, sleepTimeMillis);
        if( g_fixedFPS ){
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTimeMillis));        
        } else {
            log_info("Will not sleep as fixed FPS is disabled.");
        }
        lastFrameClock = glfwGetTime();
    }

    // Finalizamos o uso dos recursos do sistema operacional
    glfwTerminate();

    // Fim do programa
    return 0;
}

void DrawVirtualObject(const char* object_name) {
    
    if(g_VirtualScene.find(object_name) == g_VirtualScene.end()){
        log_error("Could not find object [%s] to draw!", object_name);
        return;
    }
    
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M){
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M) {
    if ( g_MatrixStack.empty() ) {
        M = Matrix_Identity();
    } else {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model) {
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            // PREENCHA AQUI o cálculo da normal de um triângulo cujos vértices
            // estão nos pontos "a", "b", e "c", definidos no sentido anti-horário.
            const glm::vec4  n =  crossproduct(a - b, b - c);
                        
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model) {
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        Model theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;
        
        log_info("Registered object! [%s]", model->shapes[shape].name.c_str());
        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        log_info("Texture coefficients is not empty!\n");
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id) {
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE ) {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        std::vector<char> log_buffer(log_length + 1 );

        glGetProgramInfoLog(program_id, log_length, &log_length, &log_buffer[0]);

        std::string log( &log_buffer[0] );

        log_severe("OpenGL linkage of program [%d] FAILED. See log below: \n%s", program_id, log.c_str());
        std::exit(EXIT_FAILURE);
    }
    // Os "Shader Objects" podem ser marcados para deleção após serem linkados 
    // glDeleteShader(vertex_shader_id);
    // glDeleteShader(fragment_shader_id);
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    // Abaixo executamos o seguinte: caso o botão esquerdo do mouse esteja
    // pressionado, computamos quanto que o mouse se movimento desde o último
    // instante de tempo, e usamos esta movimentação para atualizar os
    // parâmetros que definem a posição da câmera dentro da cena virtual.
    // Assim, temos que o usuário consegue controlar a câmera.

    if (g_LeftMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da câmera com os deslocamentos
        float & theta = g_camera.type == C_LOOKAT ? g_camera.lookat.theta : g_camera.free.theta; 
        float &  phi  = g_camera.type == C_LOOKAT ? g_camera.lookat.phi   : g_camera.free.phi; 
        theta -= 0.01f*dx;
        phi   += 0.01f*dy;
    
        // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
        float phimax = 3.141592f/2;
        float phimin = -phimax;
    
        if (phi > phimax)
            phi = phimax;
    
        if (phi < phimin)
            phi = phimin;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_RightMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_ForearmAngleZ -= 0.01f*dx;
        g_ForearmAngleX += 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }

    if (g_MiddleMouseButtonPressed)
    {
        // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
        float dx = xpos - g_LastCursorPosX;
        float dy = ypos - g_LastCursorPosY;
    
        // Atualizamos parâmetros da antebraço com os deslocamentos
        g_TorsoPositionX += 0.01f*dx;
        g_TorsoPositionY -= 0.01f*dy;
    
        // Atualizamos as variáveis globais para armazenar a posição atual do
        // cursor como sendo a última posição conhecida do cursor.
        g_LastCursorPosX = xpos;
        g_LastCursorPosY = ypos;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_camera.lookat.dist -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_camera.lookat.dist < verysmallnumber)
        g_camera.lookat.dist = verysmallnumber;
}

void defaultKeyCallback(int key, int scancode, int action, int mod){
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        g_UsePerspectiveProjection = true;
    }

    // Se o usuário apertar a tecla O, utilizamos projeção ortográfica.
    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        g_UsePerspectiveProjection = false;
    }

    // Se o usuário apertar a tecla H, fazemos um "toggle" do texto informativo mostrado na tela.
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        g_ShowInfoText = !g_ShowInfoText;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        g_shaderManager.reloadAll();
        log_info("Reloaded all shaders.");
    }

    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        //readFilesAndAddObjectsToScene({"block.obj"});
        // g_textureManager.reloadAll();
        // log_info("Reloaded all shaders.");
    }
    
    if (key == GLFW_KEY_P && action == GLFW_PRESS){
        g_shouldLogFrame = true;
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        g_fixedFPS = !g_fixedFPS;
    }
    if(key == GLFW_KEY_E && action == GLFW_PRESS) {
        ++g_experimentalState;
        log_info("Next experimental state. [%d]",g_experimentalState);
    }    
    if(key == GLFW_KEY_R && action == GLFW_PRESS){
        g_hero->m.pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        g_hero->m.vel = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        g_hero->m.front = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    }
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod) {
    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    g_keyboardState[key] = {
        /** See also GLFW_PRESS and GLFW_REPEAT */
        .press =  (action == GLFW_PRESS),
        .hold  = !(action == GLFW_RELEASE)
    };

    // bool shiftHold = g_keyboardState[GLFW_KEY_RIGHT_SHIFT].hold || g_keyboardState[GLFW_KEY_LEFT_SHIFT].hold;
    bool ctrlHold  = g_keyboardState[GLFW_KEY_RIGHT_CONTROL].hold || g_keyboardState[GLFW_KEY_LEFT_CONTROL].hold;

    if(ctrlHold && g_keyboardState[GLFW_KEY_Q].press )
        glfwSetWindowShouldClose(window, GL_TRUE);

    log_assert(key >= 0 && key < KEYS_ARR_SIZE, "Key array size is too small. %d. ", key);

    /** Note the distinction between PRESS and HOLD. */
    if(key == g_keyboardState[GLFW_KEY_ESCAPE].press || (ctrlHold && g_keyboardState[GLFW_KEY_2].press)){
        g_typingMode = tm_PLAY;
    }
    if(ctrlHold && g_keyboardState[GLFW_KEY_1].press){
        g_typingMode = tm_WRITE;
    }

    if(g_keyboardState[GLFW_KEY_BACKSPACE].press){
        if(! g_stringInput.empty() )
            g_stringInput.pop_back();
    }    
    if(g_keyboardState[GLFW_KEY_ENTER].press){
        if( g_typingMode == tm_WRITE){
            handleStringCommand(g_stringInput);
        }        
        if( g_typingMode == EDIT_STRING || g_typingMode == EDIT_BOOL || g_typingMode == EDIT_FLOAT || g_typingMode == EDIT_INT){
            handleValueInput(g_stringInput, g_globalVariables[g_editVariable]);
        }
        g_shouldLogFrame = true;
        g_stringInput = "";
    }

    if( g_typingMode == tm_PLAY ){
        defaultKeyCallback(key,scancode,action,mod);
    }
}
