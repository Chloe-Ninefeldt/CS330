#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>  // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> 



// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "cam.h"

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Chloe Ninefeldt - Milestone 1"; // Macro for window title
    const int STRIDE = 7;
    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint cylinderVao; // Handle for the vertex array object        
        GLuint cylinderVbo[2]; // Handles for the vertex buffer objects           
        GLuint cylinderVertices; // Number of indices of the mesh
        
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    
    //Texture
    GLuint gTextureId;
    
    
    // Shader program
    GLuint gProgramId;
    
    
    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    glm::vec3 gSodaPosition(-0.3f, -0.2f, 0.0f);
    glm::vec3 gSodaScale(2.0f);
    
    

}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateSoda(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* vertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position;
    layout(location = 1) in vec4 color;
    layout(location = 2) in vec2 textureCoordinate;

    out vec4 vertexColor; // variable to transfer color data to the fragment shader
    out vec3 vertexFragmentPos;     // For outgoing color / pixels to fragment shader
    out vec2 vertexTextureCoordinate;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexFragmentPos = vec3(model * vec4(position, 1.0f));

    vertexColor = color; // references incoming color data
    vertexTextureCoordinate = textureCoordinate;
}
);



/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSource = GLSL(440,
    in vec2 vertexTextureCoordinate;

    in vec3 vertexFragmentPos; // For incoming fragment position

    out vec4 fragmentColor; // For outgoing cube color to the GPU


    uniform sampler2D uTexture;
                                          
void main()
{
    fragmentColor = texture(uTexture, vertexTextureCoordinate); // Send lighting results to GPU
         //fragmentColor = texture(uTexture, vertexTextureCoordinate); // Sends texture to the GPU for rendering
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

int main(int argc, char* argv[])
{
     if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSource, fragmentShaderSource, gProgramId))
        return EXIT_FAILURE;


    // Load texture
    const char* texFilename = "galaxy.png";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0)


    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {

        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);
    
    
    // Release texture
    UDestroyTexture(gSodaPattern);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    
   

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);;

     // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    
    // Add stubs for Q/E Upward/Downward movement
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////
//Create the soda can

// source from https://www.youtube.com/watch?v=rGcEZmYIjoo

void UCreateSoda(GLfloat verts[], GLushort indices[], int numSides, float radius, float halfLen) {
    const float TWO_PI = 2.0f * 3.1415926f;
    const float radiansPerSide = TWO_PI / numSides;

    int currentVertex = 0;

    verts[0] = 0.0f;        //0 x
    verts[1] = halfLen;     //0 y
    verts[2] = 0.0f;        //0 z
    verts[3] = 0.0f;        //0 r
    verts[4] = 1.0f;        //0 g    
    verts[5] = 1.0f;        //0 b
    verts[6] = 1.0f;        //0 a
    currentVertex++;
    verts[7] = 0.0f;        //1 x
    verts[8] = -halfLen;    //1 y
    verts[9] = 0.0f;        //1 z
    verts[10] = 1.0f;       //1 r
    verts[11] = 0.0f;       //1 g
    verts[12] = 1.0f;       //1 b
    verts[13] = 1.0f;       //1 a
    currentVertex++;

    int currentTriangle = 0;
    for (int edge = 0; edge < numSides; edge++) {
        float theta = ((float)edge) * radiansPerSide;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);   //x
        verts[currentVertex * STRIDE + 1] = halfLen;                //y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    //z
        verts[currentVertex * STRIDE + 3] = 0.0f;                   //r
        verts[currentVertex * STRIDE + 4] = 1.0f;                   //g
        verts[currentVertex * STRIDE + 5] = 1.0f;                   //b
        verts[currentVertex * STRIDE + 6] = 1.0f;                   //a
        currentVertex++;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    //x
        verts[currentVertex * STRIDE + 1] = -halfLen;               //y
        verts[currentVertex * STRIDE + 2] = radius * sin(theta);    //z
        verts[currentVertex * STRIDE + 3] = 0.0f;                   //r
        verts[currentVertex * STRIDE + 4] = 1.0f;                   //g
        verts[currentVertex * STRIDE + 5] = 1.0f;                   //b
        verts[currentVertex * STRIDE + 6] = 1.0f;                   //a
        currentVertex++;

        
        if (edge > 0) {
            //top triangle
            indices[(3 * currentTriangle) + 0] = 0;
            indices[(3 * currentTriangle) + 1] = currentVertex - 4;
            indices[(3 * currentTriangle) + 2] = currentVertex - 2;
            currentTriangle++;

            //bottom triangle
            indices[(3 * currentTriangle) + 0] = 1;
            indices[(3 * currentTriangle) + 1] = currentVertex - 3;
            indices[(3 * currentTriangle) + 2] = currentVertex - 1;
            currentTriangle++;

            //triangle for 1/2 rectangular side
            indices[(3 * currentTriangle) + 0] = currentVertex - 4;
            indices[(3 * currentTriangle) + 1] = currentVertex - 3;
            indices[(3 * currentTriangle) + 2] = currentVertex - 1;
            currentTriangle++;

            //traingle for second 1/2 rectangular side
            indices[(3 * currentTriangle) + 0] = currentVertex - 1;
            indices[(3 * currentTriangle) + 1] = currentVertex - 2;
            indices[(3 * currentTriangle) + 2] = currentVertex - 4;
            currentTriangle++;
        }
    }

    indices[(3 * currentTriangle) + 0] = 0;
    indices[(3 * currentTriangle) + 1] = currentVertex - 2;
    indices[(3 * currentTriangle) + 2] = 2;
    currentTriangle++;
    indices[(3 * currentTriangle) + 0] = 1;
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;
    indices[(3 * currentTriangle) + 2] = 3;
    currentTriangle++;
    indices[(3 * currentTriangle) + 0] = currentVertex - 2;
    indices[(3 * currentTriangle) + 1] = currentVertex - 1;
    indices[(3 * currentTriangle) + 2] = 3;
    currentTriangle++;
    indices[(3 * currentTriangle) + 0] = 3;
    indices[(3 * currentTriangle) + 1] = 2;
    indices[(3 * currentTriangle) + 2] = currentVertex - 2;
    currentTriangle++;
}


// Functioned called to render a frame
void URender()
{

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gSodaPattern);
    
    // placing the object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, -8.0f));

    // scaling the object
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));

    // rotating the shape on the x axis
    glm::mat4 rotation = glm::rotate(45.0f, glm::vec3(-90.0, 1.0f, 1.0f));

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = translation * rotation * scale;

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Set the shader to be used
    glUseProgram(gProgramId);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    model = glm::translate(gSodaPosition) * glm::scale(gSodaScale);

    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");


    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    const glm::vec3 cameraPosition = gCamera.Position;
    
    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));
    
    glBindVertexArray(gMesh.cylinderVao);
    
     // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    glDrawElements(GL_TRIANGLES, gMesh.cylinderVertices, GL_UNSIGNED_SHORT, NULL);


      // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    
    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}



// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    //the number of sides wanted to create the cylinder
    const int NUM_SIDES = 100;
    //the number of vertices = number of sides * 2
    const int NUM_VERTICES = STRIDE * (2 + (2 * NUM_SIDES));
    // the number of indices is 3 * the number of triangles that will be drawn
    const int NUM_INDICES = 12 * NUM_SIDES;
    //position and color data
    GLfloat verts[NUM_VERTICES];
    //index data and color data
    GLushort indices[NUM_INDICES];

    //fill verts/indices arrays with data 
    UCreateSoda(verts, indices, NUM_SIDES, 0.20f, 0.35f);

    //draw Cylinder - Soda


    const GLuint floatsPerVertex2 = 3;
    const GLuint floatsPerColor2 = 4;
    const GLuint floatsPerUV2 = 2;



    glGenVertexArrays(1, &mesh.cylinderVao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.cylinderVao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.cylinderVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.cylinderVbo[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU


    mesh.cylinderVertices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.cylinderVbo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride2 = sizeof(float) * (floatsPerVertex2 + floatsPerColor2);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex2, GL_FLOAT, GL_FALSE, stride2, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor2, GL_FLOAT, GL_FALSE, stride2, (char*)(sizeof(float) * floatsPerVertex2));
    glEnableVertexAttribArray(1);
    
    glVertexAttribPointer(2, floatsPerUV2, GL_FLOAT, GL_FALSE, stride2, (char*)(sizeof(float) * (floatsPerVertex2 + floatsPerColor2)));
    glEnableVertexAttribArray(2);

    };
   
    


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.cylinderVao);
    glDeleteBuffers(2, mesh.cylinderVbo);

}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
