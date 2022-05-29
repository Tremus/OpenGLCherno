#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

// https://www.youtube.com/watch?v=MXNMC1YAxVQ&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2&index=9


static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall (const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << "): " << function <<
            " " << file << ": " << line << std::endl;
        return false;
    }
    return true;
} 

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
    GLCall(unsigned int id = glCreateShader(type));
    const char* src = source.c_str();
    // set shader code
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    // returns a param from the shader
    // in this case the compile status
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
    if (result == GL_FALSE)
    {
        int length;
        // returns a param from the shader
        // in this case the length of the log message
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        char* message = (char*)alloca(length * sizeof(char));
        // retrieve log message
        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        // cleanup before exiting function
        GLCall(glDeleteShader(id));
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
    GLCall(unsigned int program = glCreateProgram());
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    // must be called after setting attachments.
    // after 'linking', the attached shader becomes
    // an executable used by the program.
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    // cleanup, as the executables have already been created
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    // TODO: handle errors

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    // Limits frame rate to monitors max frame rate
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK)
        std::cout << "Error!" << std::endl;

    // Now that glew and glfw have initialised, we can use GLCall

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

    unsigned vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    // init our buffer on the GPU
    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    // set our new buffer as the current buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    // fill our active buffer on the GPU with some data
    GLCall(glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW));

    // tell our GPU about the structure of our data (cols & rows)
    GLCall(glEnableVertexAttribArray(0));
    // links the vertex array to the currently bound vertex buffer
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));


    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    // set our new buffer as the current buffer
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    // fill our active buffer on the GPU with some data
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned), indices, GL_STATIC_DRAW));

    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    // remember: shader = program
    // use the program as our current rendering state
    GLCall(glUseProgram(shader));

    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != -1);
    GLCall(glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f));

    GLCall(glBindVertexArray(0));
    GLCall(glUseProgram(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));


    float r = 0.0f;
    float increment = 0.05f;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        GLCall(glUseProgram(shader));
        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
        
        GLCall(glBindVertexArray(vao));
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));

        // When using an index buffer (GL_ELEMENT_ARRAY) you must use
        // glDrawElements instead of glDrawArrays
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r > 1 || r < 0)
            increment = -increment;
        r += increment;

        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());
    }

    // cleanup
    GLCall(glDeleteProgram(shader));

    GLCall(glfwTerminate());
    return 0;
}
