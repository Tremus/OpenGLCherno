#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>


// https://www.youtube.com/watch?v=MXNMC1YAxVQ&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=9

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
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1,
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else if (type != ShaderType::NONE)
        {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    // creates an empty vertex shader
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    // set shader code
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    // returns a param from the shader
    // in this case the compile status
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        // returns a param from the shader
        // in this case the length of the log message
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        // retrieve log message
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        // cleanup before exiting function
        glDeleteShader(id);
        return 0;
    }
    // TODO: handle any errors
    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    // Create a program
    // from the docs:
    // "A program object is an object to which shader objects can be attached."
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    // must be called after setting attachments.
    // after 'linking', the attached shader becomes
    // an executable used by the program.
    glLinkProgram(program);
    glValidateProgram(program);

    // cleanup, as the executables have already been created
    glDeleteShader(vs);
    glDeleteShader(fs);

    // TODO: handle errors

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

    GLenum err = glewInit();

    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,
    };

    unsigned indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    // init our buffer on the GPU
    unsigned int buffer;
    glGenBuffers(1, &buffer);
    // set our new buffer as the current buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // fill our active buffer on the GPU with some data
    glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(float), positions, GL_STATIC_DRAW);
    // tell our GPU about the structure of our data (cols & rows)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    // set our new buffer as the current buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    // fill our active buffer on the GPU with some data
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned), indices, GL_STATIC_DRAW);

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    // remember: shader = program
    // use the program as our current rendering state
    glUseProgram(shader);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        // When using an index buffer (GL_ELEMENT_ARRAY) you must use
        // glDrawElements instead of glDrawArrays
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    // cleanup
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}
