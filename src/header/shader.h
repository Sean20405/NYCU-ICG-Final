#include <bits/stdc++.h>

class shader_program_t{
public:
    shader_program_t();
    ~shader_program_t();
    void add_shader(std::string& filepath, unsigned int type);
    void link_shader();
    void create();
    void use();
    void release();
    void set_uniform_value(const char* name, const glm::mat4& mat);
    void set_uniform_value(const char* name, const glm::mat3& mat);
    void set_uniform_value(const char* name, const glm::vec3& vec);
    void set_uniform_value(const char* name, const float value);
    void set_uniform_value(const char* name, const int value);
    
private:
    unsigned int program_handle;
    std::vector<unsigned int> shader_handles;
};