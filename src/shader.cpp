
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "header/shader.h"

shader_program_t::shader_program_t(){
    program_handle = 0;
}

shader_program_t::~shader_program_t(){
}

void shader_program_t::create(){
    program_handle = glCreateProgram();
}

void shader_program_t::add_shader(std::string& filepath, unsigned int type){
    
    // compile and add shader to program
    
    if(type == GL_VERTEX_SHADER){
        std::cout << "adding vert shader from " << filepath << std::endl;  
    }
    else if(type == GL_FRAGMENT_SHADER){
        std::cout << "adding frag shader from " << filepath << std::endl;
    }
    else if(type == GL_GEOMETRY_SHADER){
        std::cout << "adding geom shader from " << filepath << std::endl;
    }
    else{
        std::cout << "unknown shader type" << std::endl; 
        return;
    }

    std::ifstream fs(filepath);
    std::stringstream ss;
    std::string s;
    while (getline(fs, s)) {
        ss << s << "\n";
    }
    std::string temp = ss.str();
    const char *source = temp.c_str();

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::" << type << "::COMPLIATION_FAILED"
                  << infoLog << std::endl;
        return;
    }
    shader_handles.push_back(shader);
}

void shader_program_t::link_shader(){

    // attach the compiles shader to program 
    for(auto shader_handle: shader_handles){
        glAttachShader(program_handle, shader_handle);
    }
    // link the attached shader to program
    glLinkProgram(program_handle);
    int success = 0;
    glGetProgramiv(program_handle, GL_LINK_STATUS, &success);

    if (!success) {
        int maxLength = 0;
        glGetProgramiv(program_handle, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        char *infoLog = (char *)malloc(sizeof(char) * (maxLength));
        glGetProgramInfoLog(program_handle, maxLength, &maxLength, infoLog);

        // We don't need the program anymore.
        glDeleteProgram(program_handle);
        
        // Don't leak shaders either.
        for(auto shader_handle: shader_handles)
            glDeleteShader(shader_handle);

        puts(infoLog);
        free(infoLog);
    }
    
    // detach the shader once linked
    for(auto shader_handle: shader_handles){
        glDetachShader(program_handle, shader_handle);
    }
}

void shader_program_t::use(){
    glUseProgram(program_handle);
}

void shader_program_t::release(){
    glUseProgram(0);
}

void shader_program_t::set_uniform_value(const char* name, const glm::mat4 &mat){
    unsigned int loc = glGetUniformLocation(program_handle, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat)); 
}

void shader_program_t::set_uniform_value(const char* name, const glm::mat3 &mat){
    unsigned int loc = glGetUniformLocation(program_handle, name);
    glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void shader_program_t::set_uniform_value(const char* name, const glm::vec3& vec){
    unsigned int loc = glGetUniformLocation(program_handle, name);
    glUniform3fv(loc, 1, glm::value_ptr(vec));

}

void shader_program_t::set_uniform_value(const char* name, const float value){
    unsigned int loc = glGetUniformLocation(program_handle, name);
    glUniform1f(loc, value);
}

void shader_program_t::set_uniform_value(const char* name, const int value){
    unsigned int loc = glGetUniformLocation(program_handle, name);
    glUniform1i(loc, value);
}