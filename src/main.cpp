#include <bits/stdc++.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "header/cube.h"
#include "header/object.h"
#include "header/shader.h"
#include "header/stb_image.h"

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
unsigned int loadCubemap(std::vector<string> &mFileName);

struct material_t{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float gloss;
};

struct light_t{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct model_t{
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec3 rotation;
    Object* object;
};

struct camera_t{
    glm::vec3 position;
    glm::vec3 up;
    float rotationY;
    float rotationX;
    glm::vec3 look;
};

struct bomb_t {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    Object* object;
    float explodeTime;  // time to explode
};
Object* bombObject;
std::vector<bomb_t> bombs;

struct bullet_t{
    glm::vec3 position;
    float explodeTime;
};

// settings
int SCR_WIDTH = 800;
int SCR_HEIGHT = 600;

// cube map 
unsigned int cubemapTexture;
unsigned int cubemapVAO, cubemapVBO;

// shader programs 
int shaderProgramIndex = 0;
std::vector<shader_program_t*> shaderPrograms;
shader_program_t* cubemapShader;

// additional dependencies
light_t light;
material_t material;
camera_t camera;

vector<model_t> jinx;
vector<model_t> jinxLeftHand;
vector<model_t> jinxRightHand;

model_t cube;

vector<model_t> bullet;

// model matrix
int moveDir = 1;
glm::mat4 cameraModel;

glm::mat4 jinxModel;
glm::mat4 jinxLeftHandModel;
glm::mat4 jinxRightHandModel;

// camera movement
float cameraSpeed = 2.0f;

// default
char skillIdx = 'Q';
bool shootTime = false;
bool turn = false;
bool stop = false;
float handRoate = 0.0f;
bool fire = false;
vector<vector<bullet_t>> bulletsInfo;  // (time, position)
glm::vec3 currentLook;
int inverse = 1;
int bulletIdx = 1;  // 0: silver, 1: yellow, 2: R
float ult_cnt = 0;
bool shootW = false;
float w_cnt = 0;

// jinx
float jinxMoveSpeed = 0.5;
float jinxMove = jinxMoveSpeed;

// time
float timeCoef= 1.0f;
float time_threshold = 0.1f;
float deltaTime = 0.03f;
int handRoateCoef = 30;

// W, R 要在什麼時候開始發射、停止
int ult_cnt_start = 20;
int w_cnt_start1 = 13;
int w_cnt_start2 = 30;

// E (Bomb)
// 丟出去時 velocity & 移動的重力加速度
glm::vec3 acclerateE = glm::vec3(0.0f, -50.0f, 0.0f);
float velX = 8.0f;
float velY = 20.0f;
float velZ = 20.0f;
std::vector<glm::vec3> velocitiesE = {
    glm::vec3(0.0f, velY, velZ),
    glm::vec3(velX, velY, velZ),
    glm::vec3(-velX, velY, velZ)
};
// 掉到地板後
float e_explosion_countdown = 2;  // After this, E will start to explode
float e_explosion_time = 2.5;     // After this, E will stop exploding and disappear
float num_turns = 1.5;            // Number of turns of the bomb before exploding
float jump_height = 2.0;          // Height of the bomb when it jumps
float num_jumps = 1.5;            // Number of jumps of the bomb before exploding

//////////////////////////////////////////////////////////////////////////
// Parameter setup, 
// You can change any of the settings if you want

void camera_setup(){
    camera.position = glm::vec3(0.0, 20.0, 100.0);
    camera.up = glm::vec3(0.0, 1.0, 0.0);
    camera.rotationX = 0;
    camera.rotationY = 0;
    camera.look = glm::vec3(0.0f, 10.0f, 0.0f);
}

void light_setup(){
    light.position = glm::vec3(0.0, 1000.0, 0.0);
    light.ambient = glm::vec3(1.0);
    light.diffuse = glm::vec3(1.0);
    light.specular = glm::vec3(1.0);
}

void material_setup(){
    material.ambient = glm::vec3(1.0);
    material.diffuse = glm::vec3(1.0);
    material.specular = glm::vec3(0.7);
    material.gloss = 10.5;
}
//////////////////////////////////////////////////////////////////////////


void model_setup(){
// Load the object and texture for each model here 

#if defined(__linux__) || defined(__APPLE__)
    std::string objDir = "../../src/asset/obj/";
    std::string textureDir = "../../src/asset/texture/";
#else
    std::string objDir = "..\\..\\src\\asset\\obj\\";
    std::string textureDir = "..\\..\\src\\asset\\texture\\";
#endif
 
    bombObject = new Object(objDir + "bomb.obj");
    bombObject->load_to_buffer();
    bombObject->load_texture(textureDir + "Material_baseColor.png");

    std::vector<std::string> jinxName = {
        "takemachinegun",  // default, auto
        "takesmallgun",  // W
        "takeshark",  // R
    };
    
    // 設定每個 Jinx 的模型
    jinx.resize(jinxName.size());
    jinxLeftHand.resize(jinxName.size());
    jinxRightHand.resize(jinxName.size());

    jinx[0].position = glm::vec3(8.0f, 0.0f, 0.0f);
    jinx[0].scale = glm::vec3(10.0f, 10.0f, 10.0f);
    jinx[0].rotation = glm::vec3(0.0f, 0.0f, 0.0f);

    jinxLeftHand[0].position = jinx[0].position;
    jinxLeftHand[0].scale = jinx[0].scale;
    jinxLeftHand[0].rotation = jinx[0].rotation;

    jinxRightHand[0].position = jinx[0].position;
    jinxRightHand[0].scale = jinx[0].scale;
    jinxRightHand[0].rotation = jinx[0].rotation;
        
    for (int i = 0; i < jinxName.size(); i++) {
        std::string modelName = jinxName[i];

        jinx[i].object = new Object(objDir + modelName + "\\body.obj");
        jinx[i].object->load_to_buffer();   
        jinx[i].object->load_texture(textureDir + "initialShadingG_baseColor.jpeg");
        
        jinxLeftHand[i].object = new Object(objDir + modelName + "\\lefthand.obj");
        jinxLeftHand[i].object->load_to_buffer();
        jinxLeftHand[i].object->load_texture(textureDir + "initialShadingG_baseColor.jpeg");
        
        jinxRightHand[i].object = new Object(objDir + modelName + "\\righthand.obj");
        jinxRightHand[i].object->load_to_buffer();
        jinxRightHand[i].object->load_texture(textureDir + "initialShadingG_baseColor.jpeg");
    }

    // Load the object and texture for the cube
    cube.position = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.scale = glm::vec3(1.0f, 1.0f, 1.0f);
    cube.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.object = new Object(objDir + "cube.obj");
    cube.object->load_to_buffer();
    cube.object->load_texture(textureDir + "cube_texture.jpg");

    // Load the bullet object
    std::vector<std::string> bulletName = {
        "bullet",  // silver
        "bulletv2",  // yellow
        "rocket",  // R
    };
    bullet.resize(bulletName.size());
    bulletsInfo.resize(bulletName.size());
    bullet[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    bullet[0].scale = glm::vec3(1.0f, 1.0f, 1.0f);
    bullet[0].rotation = glm::vec3(0.0f, 0.0f, 90.0f);
    for (int i = 0; i < bulletName.size(); i++) {
        std::string modelName = bulletName[i];
        bullet[i].object = new Object(objDir + modelName + ".obj");
        bullet[i].object->load_to_buffer();
        if(i == 2){
            bullet[i].object->load_texture(textureDir + modelName + "_texture" + ".jpg");
        }else{
            bullet[i].object->load_texture(textureDir + modelName + "_texture" + ".png");
        }
    }
}


void shader_setup(){
// Setup the shader program for each shading method

#if defined(__linux__) || defined(__APPLE__)
    std::string shaderDir = "../../src/shaders/";
#else
    std::string shaderDir = "..\\..\\src\\shaders\\";
#endif

    std::vector<std::string> shadingMethod = {
        "default",                              // default shading
        // "bling-phong", "gouraud", "metallic",   // addional shading effects (basic)
        // "glass_schlick", "glass_empricial",     // addional shading effects (advanced)
    };

    for(int i=0; i<shadingMethod.size(); i++){
        std::string vpath = shaderDir + shadingMethod[i] + ".vert";
        std::string fpath = shaderDir + shadingMethod[i] + ".frag";
        std::string gpath = shaderDir + shadingMethod[i] + ".geom";

        shader_program_t* shaderProgram = new shader_program_t();
        shaderProgram->create();
        shaderProgram->add_shader(vpath, GL_VERTEX_SHADER);
        shaderProgram->add_shader(fpath, GL_FRAGMENT_SHADER);
        if (i == 0) shaderProgram->add_shader(gpath, GL_GEOMETRY_SHADER);  // add geometry shader for default shading
        shaderProgram->link_shader();
        shaderPrograms.push_back(shaderProgram);
    } 
}


void cubemap_setup(){
// Setup all the necessary things for cubemap rendering
// Including: cubemap texture, shader program, VAO, VBO

#if defined(__linux__) || defined(__APPLE__)
    std::string cubemapDir = "../../src/asset/texture/skybox/";
    std::string shaderDir = "../../src/shaders/";
#else
    std::string cubemapDir = "..\\..\\src\\asset\\texture\\skybox\\";
    std::string shaderDir = "..\\..\\src\\shaders\\";
#endif

    // setup texture for cubemap
    std::vector<std::string> faces
    {
        cubemapDir + "right.jpg",
        cubemapDir + "left.jpg",
        cubemapDir + "top.jpg",
        cubemapDir + "bottom.jpg",
        cubemapDir + "front.jpg",
        cubemapDir + "back.jpg"
    };
    cubemapTexture = loadCubemap(faces);   

    // setup shader for cubemap
    std::string vpath = shaderDir + "cubemap.vert";
    std::string fpath = shaderDir + "cubemap.frag";
    
    cubemapShader = new shader_program_t();
    cubemapShader->create();
    cubemapShader->add_shader(vpath, GL_VERTEX_SHADER);
    cubemapShader->add_shader(fpath, GL_FRAGMENT_SHADER);
    cubemapShader->link_shader();

    glGenVertexArrays(1, &cubemapVAO);
    glGenBuffers(1, &cubemapVBO);
    glBindVertexArray(cubemapVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), &cubemapVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
}


void setup(){
    // Initialize shader model camera light material
    light_setup();
    model_setup();
    shader_setup();
    camera_setup();
    currentLook = camera.look;
    cubemap_setup();
    material_setup();

    // Enable depth test, face culling ...
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // Debug: enable for debugging
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback([](  GLenum source, GLenum type, GLuint id, GLenum severity, 
                                GLsizei length, const GLchar* message, const void* userParam) {

    std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") 
              << "type = " << type 
              << ", severity = " << severity 
              << ", message = " << message << std::endl;
    }, nullptr);
}


void update(){
// Update the heicopter position, camera position, rotation, etc.

    // Bomb
    for (int i = 0; i < bombs.size(); i++) {
        bombs[i].velocity += bombs[i].acceleration * deltaTime * (timeCoef * !stop);
        bombs[i].position += bombs[i].velocity * deltaTime * (timeCoef * !stop);

        if (bombs[i].position.y <= 0.0f) {  // Hit the ground, start to explode
            bombs[i].velocity = glm::vec3(0.0f);
            bombs[i].acceleration = glm::vec3(0.0f);
            bombs[i].position.y = 0.0f;
            bombs[i].explodeTime += deltaTime * timeCoef * !stop;
            
            if (bombs[i].explodeTime > e_explosion_time) {  // Delete the bomb after this time
                bombs.erase(bombs.begin() + i);
                i--;
            }
        }
    }

    // shoot time
    if(shootTime && timeCoef > time_threshold){
        timeCoef -= deltaTime;
    }
    else if(timeCoef < 1.0){
        timeCoef += deltaTime;
    }

    // Jinx 上下移動
    // float range = 0.1;
    // if(jinx[0].position.y > range){
    //     jinxMove = -jinxMoveSpeed;
    // }else if(jinx[0].position.y < -range){
    //     jinxMove = jinxMoveSpeed;
    // }
    // jinx[0].position.y += jinxMove * timeCoef * !stop;

    int rotateSpeed = handRoateCoef * timeCoef * !stop * inverse;
    if(turn){
        handRoate += rotateSpeed;
        if(handRoate > 360.0 || handRoate < -360.0){
            handRoate = 0.0;
            turn = false;
        }
        jinxLeftHand[0].rotation.x = handRoate;
        jinxRightHand[0].rotation.x = handRoate;
    }

    jinxModel = glm::mat4(1.0f);
    jinxModel = glm::translate(jinxModel, jinx[0].position);
    jinxModel = glm::scale(jinxModel, jinx[0].scale);

    float delta_y = 1.68;
    jinxLeftHandModel = glm::translate(jinxModel, glm::vec3(0.0, delta_y, 0.0));
    jinxLeftHandModel = glm::rotate(jinxLeftHandModel, glm::radians(jinxLeftHand[0].rotation.x), glm::vec3(1.0, 0.0, 0.0));
    jinxLeftHandModel = glm::translate(jinxLeftHandModel, glm::vec3(0.0, -delta_y, 0.0));

    jinxRightHandModel = glm::translate(jinxModel, glm::vec3(0.0, delta_y, 0.0));
    jinxRightHandModel = glm::rotate(jinxRightHandModel, glm::radians(jinxRightHand[0].rotation.x), glm::vec3(1.0, 0.0, 0.0));
    jinxRightHandModel = glm::translate(jinxRightHandModel, glm::vec3(0.0, -delta_y, 0.0));

    camera.rotationY = (camera.rotationY > 360.0) ? 0.0 : camera.rotationY;
    camera.rotationX = (camera.rotationX > 360.0) ? 0.0 : camera.rotationX;
    cameraModel = glm::mat4(1.0f);
    cameraModel = glm::translate(cameraModel, currentLook);
    cameraModel = glm::rotate(cameraModel, glm::radians(camera.rotationY), camera.up);
    glm::vec3 left = glm::normalize(glm::cross(camera.up, currentLook - camera.position));
    cameraModel = glm::rotate(cameraModel, glm::radians(camera.rotationX), left);
    cameraModel = glm::translate(cameraModel, -currentLook);
    cameraModel = glm::translate(cameraModel, camera.position);

    // bullets
    for(int j=0; j<3; j++){  // 0: silver, 1: yellow
        for(int i = 0; i < bulletsInfo[j].size(); i++){
            // Stop moving, explode
            if(bulletsInfo[j][i].position.z > 50.0){
                bulletsInfo[j][i].explodeTime += deltaTime * timeCoef * !stop;
                // cout << bulletsInfo[j][i].explodeTime << endl;
                if (bulletsInfo[j][i].explodeTime > 0.5) {  // Delete the bullet after exploding for 0.5s
                    bulletsInfo[j].erase(bulletsInfo[j].begin() + i);
                    i--;
                }
            }
            else {
                if(!stop){
                    bulletsInfo[j][i].position += glm::vec3(0.0, 0.0, 0.5) * (timeCoef * !stop);
                }
            }
        }
    }
    if(fire){
        fire = false;
        if(skillIdx == 'Q' && bulletIdx == 1 && turn == false){
            bulletsInfo[bulletIdx].push_back(bullet_t{jinx[0].position + glm::vec3(-8.0, 11.0, 10.0), 0.0});
        }
        else if(skillIdx == 'R' && bulletIdx == 0 && turn == false){
            bulletsInfo[bulletIdx].push_back(bullet_t{jinx[0].position + glm::vec3(-5.5, 18.0, 10.0), 0.0});
        }
    }

    // R
    if(skillIdx == 'R' && bulletIdx == 2 && ult_cnt > 0){
        ult_cnt -= (timeCoef + 1e-5) * !stop;
        if(ult_cnt < 0){
            // shoot R
            ult_cnt = 0;
            bulletsInfo[bulletIdx].push_back(bullet_t{jinx[0].position + glm::vec3(-6.0, 18.0, 12.0), 0.0});
        }
    }

    // W
    if(skillIdx == 'W' && ult_cnt > 0){
        ult_cnt -= (timeCoef + 1e-5) * !stop;
        if(ult_cnt < 0){
            // shoot W
            shootW = true;
            w_cnt = w_cnt_start2;
            ult_cnt = 0;
        }
    }
}


void render(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Calculate view, projection matrix
    glm::mat4 view = glm::lookAt(glm::vec3(cameraModel[3]), currentLook, camera.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);

    // Set matrix for view, projection, model transformation
    shaderPrograms[shaderProgramIndex]->use();
    shaderPrograms[shaderProgramIndex]->set_uniform_value("view", view);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("projection", projection);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("skybox", 1);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("noTexture", false);

    // Jinx
    int idx = 0;
    if(skillIdx == 'Q'){
        idx = 0;
    }else if(skillIdx == 'W'){
        idx = 1;
    }else if(skillIdx == 'R'){
        idx = 2;
    }
    shaderPrograms[shaderProgramIndex]->set_uniform_value("model", jinxModel);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("time", 0.0f);
    jinx[idx].object->render();
    shaderPrograms[shaderProgramIndex]->set_uniform_value("model", jinxLeftHandModel);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("time", 0.0f);
    jinxLeftHand[idx].object->render();
    shaderPrograms[shaderProgramIndex]->set_uniform_value("model", jinxRightHandModel);
    shaderPrograms[shaderProgramIndex]->set_uniform_value("time", 0.0f);
    jinxRightHand[idx].object->render();

    // bullets
    for(int j=0; j<3; j++){  // 0: silver, 1: yellow, 2: rocket(R)
        for(int i = 0; i < bulletsInfo[j].size(); i++){
            glm::mat4 bulletModel = glm::mat4(1.0f);
            bulletModel = glm::translate(bulletModel, bulletsInfo[j][i].position);
            bulletModel = glm::rotate(bulletModel, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
            if(j == 1){
                bulletModel = glm::scale(bulletModel, glm::vec3(50.0f));
            }
            if(j == 2){
                bulletModel = glm::rotate(bulletModel, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
                bulletModel = glm::scale(bulletModel, glm::vec3(0.5f));
            }
                
            shaderPrograms[shaderProgramIndex]->set_uniform_value("model", bulletModel);

            if (bulletsInfo[j][i].explodeTime > 0.0) {
                shaderPrograms[shaderProgramIndex]->set_uniform_value("time", bulletsInfo[j][i].explodeTime);
                shaderPrograms[shaderProgramIndex]->set_uniform_value("aExplosionColor", glm::vec3(0.7f, 0.7f, 0.7f));
                bullet[j].object->render();
            }
            else {
                shaderPrograms[shaderProgramIndex]->set_uniform_value("time", 0.0f);
                bullet[j].object->render();
            }
        }
    }

    // Render bombs
    for (const auto& bomb : bombs) {
        glm::mat4 bombModel = glm::mat4(1.0f);
        
        if (bomb.explodeTime > e_explosion_countdown) {
            bombModel = glm::translate(bombModel, bomb.position);
            bombModel = glm::scale(bombModel, glm::vec3(0.5f));
            
            shaderPrograms[shaderProgramIndex]->set_uniform_value("time", bomb.explodeTime - e_explosion_countdown);
            shaderPrograms[shaderProgramIndex]->set_uniform_value("aExplosionColor", glm::vec3(0.8f, 0.4f, 0.0f));
        }
        else if (bomb.explodeTime > 0.0) {
            // Rotate and jump according to the time
            float angle = 360.0 * num_turns * bomb.explodeTime / e_explosion_countdown;
            float height = max(0.0, jump_height * sin(num_jumps * 2 * M_PI * bomb.explodeTime));
            bombModel = glm::translate(bombModel, bomb.position + glm::vec3(0.0f, height, 0.0f));
            bombModel = glm::rotate(bombModel, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
            bombModel = glm::scale(bombModel, glm::vec3(0.5f));

            shaderPrograms[shaderProgramIndex]->set_uniform_value("time", 0.0f);
        }
        else {
            bombModel = glm::translate(bombModel, bomb.position);
            bombModel = glm::scale(bombModel, glm::vec3(0.5f));   // 縮小到 50%

            shaderPrograms[shaderProgramIndex]->set_uniform_value("time", 0.0f);
        }
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", bombModel);
        bomb.object->render();
        
    }
    
    if(shootW){
        // shoot laser
        // float cur_len = w_cnt;
        // if(w_cnt > 10){
        //     cur_len = 10;
        // }
        // shaderPrograms[shaderProgramIndex]->set_uniform_value("noTexture", true);
        // shaderPrograms[shaderProgramIndex]->set_uniform_value("Color", glm::vec3(1.0f, 0.71f, 0.76f));  // // (255, 182, 193)
        // glm::mat4 wCubeModel = glm::mat4(1.0f);
        // wCubeModel = glm::translate(wCubeModel, jinx[0].position + glm::vec3(-7.5, 14.5, 10.0) + glm::vec3(0.0, 0.0, 40 - cur_len*2));
        // wCubeModel = glm::scale(wCubeModel, glm::vec3(3.0f, 0.5f, cur_len*4));
        // shaderPrograms[shaderProgramIndex]->set_uniform_value("time", 0.0f);
        // shaderPrograms[shaderProgramIndex]->set_uniform_value("model", wCubeModel);
        // cube.object->render();
        // w_cnt -= timeCoef * 1 * !stop;
        // if(w_cnt < 0){
        //     shootW = false;
        // }
        
        // laser explosion
        float cur_len = 10;
        shaderPrograms[shaderProgramIndex]->set_uniform_value("noTexture", true);
        shaderPrograms[shaderProgramIndex]->set_uniform_value("Color", glm::vec3(1.0f, 0.71f, 0.76f));
        glm::mat4 wCubeModel = glm::mat4(1.0f);
        wCubeModel = glm::translate(wCubeModel, jinx[0].position + glm::vec3(-7.5, 14.5, 10.0) + glm::vec3(0.0, 0.0, 40 - cur_len*2));
        wCubeModel = glm::scale(wCubeModel, glm::vec3(3.0f, 0.5f, cur_len*4));
        if(w_cnt < 5){
            float explodeW = (5 - w_cnt) / 12;
            shaderPrograms[shaderProgramIndex]->set_uniform_value("time", explodeW);
            shaderPrograms[shaderProgramIndex]->set_uniform_value("aExplosionColor", glm::vec3(1.0f));
        }
        else{
            shaderPrograms[shaderProgramIndex]->set_uniform_value("time", 0.0f);
        }
        shaderPrograms[shaderProgramIndex]->set_uniform_value("model", wCubeModel);
        cube.object->render();
        w_cnt -= timeCoef * 1 * !stop;
        if(w_cnt < 0){
            shootW = false;
        }
    }

    // Render a cube at currentLook position
    // shaderPrograms[shaderProgramIndex]->set_uniform_value("noTexture", true);
    // shaderPrograms[shaderProgramIndex]->set_uniform_value("Color", glm::vec3(1.0f, 0.0f, 0.0f));
    // glm::mat4 redCubeModel = glm::mat4(1.0f);
    // redCubeModel = glm::translate(redCubeModel, currentLook);
    // redCubeModel = glm::scale(redCubeModel, glm::vec3(1.0f, 1.0f, 1.0f));
    // shaderPrograms[shaderProgramIndex]->set_uniform_value("model", redCubeModel);
    // shaderPrograms[shaderProgramIndex]->set_uniform_value("time", 0.0f);
    // cube.object->render();

    shaderPrograms[shaderProgramIndex]->release();

    // TODO 4-2 
    // Rendering cubemap environment
    cubemapShader->use();
    cubemapShader->set_uniform_value("view", glm::mat4(glm::mat3(view)));  // remove translation
    cubemapShader->set_uniform_value("projection", projection);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(cubemapVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
    glBindVertexArray(0);
    cubemapShader->release();    
}


int main() {
    
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Champion Spotlight of Jinx", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSwapInterval(1);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // set viewport
    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    // Setup texture, model, shader ...e.t.c
    setup();
    
    // Render loop, main logic can be found in update, render function
    while (!glfwWindowShouldClose(window)) {
        update(); 
        render(); 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// Add key callback
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // The action is one of GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE.
    // Events with GLFW_PRESS and GLFW_RELEASE actions are emitted for every key press.
    // Most keys will also emit events with GLFW_REPEAT actions while a key is held down.
    // https://www.glfw.org/docs/3.3/input_guide.html

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if(!stop){
        if (key == GLFW_KEY_Q && (action == GLFW_REPEAT || action == GLFW_PRESS)){
            if(skillIdx == 'Q'){
                skillIdx = 'R';
                bulletIdx = 0;
            }
            else{
                skillIdx = 'Q';
                bulletIdx = 1;
            }
            turn = true;
            inverse = 1;
            handRoate = 0.0f;
        }
        if (key == GLFW_KEY_W && (action == GLFW_REPEAT || action == GLFW_PRESS)){
            skillIdx = 'W';
            turn = true;
            inverse = 1;
            handRoate = 0.0f;
            ult_cnt = w_cnt_start1;
        }
        if (key == GLFW_KEY_E && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
            turn = true;
            inverse = -1;
            handRoate = 0.0f;
            ult_cnt = 0;
            
            for (const auto& velocity : velocitiesE) {
                bomb_t newBomb;
                newBomb.position = jinx[0].position + glm::vec3(-8.0f, 10.0f, 0.0f); // 初始位置從 Jinx 出發
                newBomb.velocity = velocity;        // 不同的初速度
                newBomb.acceleration = acclerateE; // 重力加速度
                newBomb.object = bombObject;       // 使用同一個炸彈模型
                newBomb.explodeTime = 0.0f;
                bombs.push_back(newBomb);
            }
        }
        if (key == GLFW_KEY_R && (action == GLFW_REPEAT || action == GLFW_PRESS)){
            skillIdx = 'R';
            bulletIdx = 2;
            turn = true;
            inverse = 1;
            handRoate = 0.0f;
            ult_cnt = ult_cnt_start;
        }

        if (key == GLFW_KEY_A && (action == GLFW_REPEAT || action == GLFW_PRESS))
            fire = true;
        if (key == GLFW_KEY_T && (action == GLFW_REPEAT || action == GLFW_PRESS))
            shootTime = !shootTime;
    }
    if (key == GLFW_KEY_SPACE && (action == GLFW_REPEAT || action == GLFW_PRESS))
        stop = !stop;
    
    // camera movement
    // Rotate
    if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.rotationX += 10.0f; // 向上旋轉
    if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.rotationX -= 10.0f; // 向下旋轉
    if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.rotationY -= 10.0f; // 向左旋轉
    if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.rotationY += 10.0f; // 向右旋轉
    
    // 中心點 左右平移
    glm::vec3 left = glm::normalize(glm::cross(camera.up, currentLook - glm::vec3(cameraModel[3])));
    if (key == GLFW_KEY_COMMA && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        camera.position = glm::vec3(cameraModel[3]) + cameraSpeed * left;
        camera.rotationY = 0;
        camera.rotationX = 0;
        currentLook += cameraSpeed * left;
    }
    if (key == GLFW_KEY_SLASH && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        camera.position = glm::vec3(cameraModel[3]) - cameraSpeed * left;
        camera.rotationY = 0;
        camera.rotationX = 0;
        currentLook -= cameraSpeed * left;
    }
    // 放大縮小
    glm::vec3 front = glm::normalize(currentLook - camera.position);
    if (key == GLFW_KEY_L && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.position += cameraSpeed * front; // 向前移動
    if (key == GLFW_KEY_PERIOD && (action == GLFW_REPEAT || action == GLFW_PRESS))
        camera.position -= cameraSpeed * front; // 向後移動
    // 回歸初始位置
    if (key == GLFW_KEY_P && (action == GLFW_REPEAT || action == GLFW_PRESS)){
        currentLook = camera.look;
        camera.position = glm::vec3(0.0, 20.0, 100.0);
        camera.rotationY = 0;
        camera.rotationX = 0;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

// Loading cubemap texture
unsigned int loadCubemap(vector<std::string>& faces){

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        stbi_set_flip_vertically_on_load(false);
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texture;
}  
