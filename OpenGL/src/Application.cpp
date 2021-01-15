#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <string>
#include <sstream>

// how does the opengl pipeline work?
// supply gpu with data, store memory in gpu (buffers), shaders are a program that run on the gpu that use that data to display shit on the screen
// how do we draw geometry? vertex buffers, a buffer of memory stored on the gpu. we have to tell opengl how the memory is formatted
// format such as, these first few coordinates are my vertices and my next few are for other stuff: this is what vertex attributes do
// vertexes have much more data than just a position.

// think of fragments as pixels, runs once for each pixel that gets rastorized. rastorized means to get drawn on the screen.
// take our triangle, it needs to get rastorized with pixels. the fragment shader decides which color the pixels are.
// fragment/pixel shaders are called every time a pixel is rastorized. this means that it can get called like a billion times dependiong on how big your triangle is


// what is a shader? a program (block of code) that runs on your gpu. the vertex shader runs for every vertex
// static means that it can only be accessed by this file

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos) //npos just meanas that if it hasnt found #shader or whatever we wanted to find
        {
            if (line.find("vertex") != std::string::npos)
                // set mode to vertex
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                // set mode to fragment
                type = ShaderType::FRAGMENT;
        }
        else
        {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(const std::string& source, unsigned int type)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str(); //returns a pointer
    glShaderSource(id, 1, &src, nullptr); //specifies the source of our shader, the count argument is the number of source codes we input in
    glCompileShader(id);

    //TODO: error handling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex": "fragment") << "shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;

    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    // fundamentally, we need to provide this function with the shader source code and it will link the two in this function
    unsigned int program = glCreateProgram(); //returns an unsigned int
    unsigned int vs = CompileShader(vertexShader, GL_VERTEX_SHADER);
    unsigned int fs = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;
    
    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
        std::cout << "error!" << std::endl;

    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] = {
       -0.5f,  -0.5f,
        0.5f,  -0.5f,
        0.5f,  0.5f,
       -0.5f,  0.5f,
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };


    unsigned int buffer; //buffer is just a row of memory that we can put data into
    glGenBuffers(1, &buffer); //generates a buffer of memory. gives back an ID for the buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffer); //"selecting the buffer" means to bind the buffer. GL array buffer just lets opengl know that we want to bind the buffer to an array
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW); // this literally just gives the buffer with data
   
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0); //informs opengl of the layout of our data

    unsigned int ibo; //buffer is just a row of memory that we can put data into
    glGenBuffers(1, &ibo); //generates a buffer of memory. gives back an ID for the buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //"selecting the buffer" means to bind the buffer. GL array buffer just lets opengl know that we want to bind the buffer to an array
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}