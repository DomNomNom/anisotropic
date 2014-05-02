#define GL_GLEXT_PROTOTYPES

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "shader.h"




// http://stackoverflow.com/questions/1798112/removing-leading-and-trailing-spaces-from-a-string
static std::string trim(const std::string& str) {
    const std::string whitespace = " \t\n\r";

    const unsigned int strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const unsigned int strEnd = str.find_last_not_of(whitespace);
    const unsigned int strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

static const std::string preprocess(const std::string &fileName) {
    std::string filePath = std::string("shaders/") + trim(fileName);
    std::stringstream output;
    std::ifstream file(filePath.c_str());
    if (file.fail()) {
        std::cout << " ====== Shader file not found: '" << filePath << "' ====== " << std::endl;
    }

    std::string line;
    const std::string prefix = "#include ";
    unsigned int line_number = 1;
    while(std::getline(file,line)) {
        if (line.find(prefix) == 0) {
            output << preprocess(line.substr(prefix.length())) << std::endl;
        }
        else {
            output << "#line "<< line_number << " \"" << fileName << "\""  << std::endl;
            output <<  line << std::endl;
        }
        ++line_number;
    }

    return output.str();
}



static void validateShader(GLuint shader, const char* file = 0) {
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;

    glGetShaderInfoLog(shader, BUFFER_SIZE, &length, buffer);
    if (length > 0) {
        std::cerr << "Shader " << shader << " (" << (file?file:"") << ") compile error: " << buffer << std::endl;
    }
}

static void validateProgram(GLuint program) {
    const unsigned int BUFFER_SIZE = 512;
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    GLsizei length = 0;

    memset(buffer, 0, BUFFER_SIZE);
    glGetProgramInfoLog(program, BUFFER_SIZE, &length, buffer);
    if (length > 0)
        std::cerr << "Program " << program << " link error: " << buffer << std::endl;

    glValidateProgram(program);
    GLint status;
    glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
    if (status == GL_FALSE)
        std::cerr << "Error validating shader " << program << std::endl;
}

Shader::Shader() {

}

Shader::Shader(const char *vsFile, const char *fsFile) {
    init(vsFile, fsFile);
}

void Shader::init(const char *vsFile, const char *fsFile) {
    shader_vp = glCreateShader(GL_VERTEX_SHADER);
    shader_fp = glCreateShader(GL_FRAGMENT_SHADER);

    const std::string vsText = preprocess(std::string(vsFile));
    const std::string fsText = preprocess(std::string(fsFile));

    const char *vsp = vsText.c_str();
    const char *fsp = fsText.c_str();

    glShaderSource(shader_vp, 1, &vsp, 0);
    glShaderSource(shader_fp, 1, &fsp, 0);

    glCompileShader(shader_vp);
    validateShader(shader_vp, vsFile);
    glCompileShader(shader_fp);
    validateShader(shader_fp, fsFile);

    shader_id = glCreateProgram();
    glAttachShader(shader_id, shader_fp);
    glAttachShader(shader_id, shader_vp);
    glLinkProgram(shader_id);
    validateProgram(shader_id);

}

Shader::~Shader() {
    glDetachShader(shader_id, shader_fp);
    glDetachShader(shader_id, shader_vp);

    glDeleteShader(shader_fp);
    glDeleteShader(shader_vp);
    glDeleteProgram(shader_id);
}

unsigned int Shader::id() {
    return shader_id;
}

void Shader::bind() {
    glUseProgram(shader_id);
}

void Shader::unbind() {
    glUseProgram(0);
}
