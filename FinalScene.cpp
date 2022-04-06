//cylinder texture wrap


#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>  // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



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
    const char* const WINDOW_TITLE = "Ninefeldt - Milestone"; // Macro for window title
    const int STRIDE = 7;
    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint cylinderVao, cubeVao, planeVao; // Handle for the vertex array object        
        GLuint cylinderVbo[2], cubeVbo, planeVbo; // Handles for the vertex buffer objects           
        GLuint cylinderVertices, cubeVertices, planeVertices; // Number of indices of the mesh

    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    GLMesh gMeshCube;
    GLMesh gMeshPlane;

    //Texture
    GLuint gTextureId;
    GLuint gTextureIdPlane;  //Plane


    // Shader program
    GLuint gProgramId;
    GLuint gProgramIdCube;
    GLuint gProgramIdPlane;
    GLuint gProgramIdLamp;
    GLuint gProgramIdFill;


    glm::vec2 gUVScale(5.0f, 5.0f);
    //GLint gTexWrapMode = GL_REPEAT;
    GLint gTexWrapMode = GL_CLAMP_TO_EDGE;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    glm::vec3 gPositionSoda(-2.0f, -0.07f, 2.0f);
    glm::vec3 gScaleSoda(2.0f);

    glm::vec3 gPositionCube(-2.0f, 0.55f, 2.0f);
    glm::vec3 gScaleCube(0.30f);

    glm::vec3 gPositionPlane(0.0f, 0.0f, 0.0f);
    glm::vec3 gScalePlane(3.0f);

    
    glm::vec3 gObjectColor(1.0f, 0.2f, 0.0f);

    glm::vec3 gLightColor(1.0f, 1.0f, 1.2f);
    glm::vec3 gLightPosition(8.0f,11.5f, 3.0f);
    glm::vec3 gLightScale(1.3f);
    
    glm::vec3 gFillColor(1.0f, 1.0f, 1.2f);
    glm::vec3 gFillPosition(-8.0f, 11.5f, 7.0f);
    glm::vec3 gFillScale(1.3f);

    
     // Lamp animation
    bool gLampIsOrbiting = true;
}
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
void UCreateMeshCube(GLMesh& mesh);
void UCreateMeshPlane(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU
out vec4 fillFragmentColor;
// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 fillColor;
uniform vec3 fillPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    float ambientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.2f; // Set specular light strength
    float highlightSize = 8.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;


    //Calculate key lighting*/
    float fillAmbientStrength = 0.1f; // Set ambient or global lighting strength
    vec3 fillAmbient = fillAmbientStrength * fillColor; // Generate ambient light color

    //Calculate Diffuse lighting*/

    vec3 fillDirection = normalize(fillPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float fillImpact = max(dot(norm, fillDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 fillDiffuse = fillImpact * fillColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float fillSpecularIntensity = 0.5f; // Set specular light strength
    float fillHighlightSize = 8.0f; // Set specular highlight size
    vec3 fillViewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 fillReflectDir = reflect(-fillDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float fillSpecularComponent = pow(max(dot(fillViewDir, fillReflectDir), 0.0), fillHighlightSize);
    vec3 fillSpecular = fillSpecularIntensity * fillSpecularComponent * fillColor;

    // Calculate phong result
    vec3 objectColor = texture(uTexture, vertexTextureCoordinate).xyz;
    vec3 keyResult = (ambient + diffuse + specular);
    vec3 fillResult = (fillAmbient + fillDiffuse + fillSpecular);
    vec3 lightingResult = keyResult + fillResult;
    vec3 phong = (lightingResult)*objectColor;


    fragmentColor = vec4(phong, 1.0f); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);


const GLchar* fillVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* fillFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
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

    // Create the mesh
    UCreateMeshCube(gMeshCube); // Calls the function to create the Vertex Buffer Object

    // Create the mesh
    UCreateMeshPlane(gMeshPlane); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gProgramIdLamp))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(cubeVertexShaderSource, lampFragmentShaderSource, gProgramIdPlane))
        return EXIT_FAILURE;

    //if (!UCreateShaderProgram(cubeVertexShaderSource, lampFragmentShaderSource, gProgramIdPlane2))
       // return EXIT_FAILURE;

    if (!UCreateShaderProgram(cubeVertexShaderSource, lampFragmentShaderSource, gProgramIdCube))
        return EXIT_FAILURE;
    
    if (!UCreateShaderProgram(fillVertexShaderSource, fillFragmentShaderSource, gProgramIdFill))
        return EXIT_FAILURE;


    // Load texture
    const char* texFilename = "aluminum1.jpg";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }


    texFilename = "galaxy.png";
    if (!UCreateTexture(texFilename, gTextureIdPlane))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    //glUniform1i(glGetUniformLocation(gProgramIdPlane, "uTexture"), 0);


    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);


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
    UDestroyMesh(gMeshCube);
    UDestroyMesh(gMeshPlane);


    // Release texture
    UDestroyTexture(gTextureId);
    UDestroyTexture(gTextureIdPlane);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gProgramIdCube);
    UDestroyShaderProgram(gProgramIdPlane);
    UDestroyShaderProgram(gProgramIdFill);
    UDestroyShaderProgram(gProgramIdLamp);



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

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }

    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;
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

    // Set the shader to be used
    glUseProgram(gProgramId);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    glm::mat4 model = glm::translate(gPositionSoda) * glm::scale(gScaleSoda);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);



    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //const glm::vec3 cameraPosition = gCamera.Position;

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glBindVertexArray(gMesh.cylinderVao);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    glDrawElements(GL_TRIANGLES, gMesh.cylinderVertices, GL_UNSIGNED_SHORT, NULL);


    //Draw cube

    glUseProgram(gProgramIdCube);

    model = glm::translate(gPositionCube) * glm::scale(gScaleCube);


    // Retrieves and passes transform matrices to the Shader program
    GLint modelLocCube = glGetUniformLocation(gProgramIdCube, "model");
    GLint viewLocCube = glGetUniformLocation(gProgramIdCube, "view");
    GLint projLocCube = glGetUniformLocation(gProgramIdCube, "projection");

    glUniformMatrix4fv(modelLocCube, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLocCube, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocCube, 1, GL_FALSE, glm::value_ptr(projection));


    // Activate the plane VAO (used by plane and candle)
    glBindVertexArray(gMeshCube.cubeVao);


    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshCube.cubeVertices);



    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // PLANE: draw plane
    //----------------
    // Set the shader to be used
    glUseProgram(gProgramIdPlane);

    model = glm::translate(gPositionPlane) * glm::scale(gScalePlane);


    // Retrieves and passes transform matrices to the Shader program
    GLint modelLocPlane = glGetUniformLocation(gProgramIdPlane, "model");
    GLint viewLocPlane = glGetUniformLocation(gProgramIdPlane, "view");
    GLint projLocPlane = glGetUniformLocation(gProgramIdPlane, "projection");

    glUniformMatrix4fv(modelLocPlane, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLocPlane, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocPlane, 1, GL_FALSE, glm::value_ptr(projection));

    GLint UVScaleLocPlane = glGetUniformLocation(gProgramIdPlane, "uvScale");
    glUniform2fv(UVScaleLocPlane, 1, glm::value_ptr(gUVScale));




    // Activate the plane VAO (used by plane)
    glBindVertexArray(gMeshPlane.planeVao);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureIdPlane);


    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshPlane.planeVertices);


    
    
    
    
    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.cylinderVertices);


    //Draw fill
    //----------------
    glUseProgram(gFillProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gFillPosition) * glm::scale(gFillScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gFillProgramId, "model");
    viewLoc = glGetUniformLocation(gFillProgramId, "view");
    projLoc = glGetUniformLocation(gFillProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.cylinderVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    glUseProgram(0);

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

void UCreateMeshCube(GLMesh& mesh)

{
    // Vertex data
    GLfloat verts[] = {
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,


        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,

         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,

        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,


        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,   0.0f,  0.0f,  0.0f,  1.0f



    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;


    mesh.cubeVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor));

    glGenVertexArrays(1, &mesh.cubeVao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.cubeVao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.cubeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.cubeVbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);


}

void UCreateMeshPlane(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {

        //positions             //colors (r, g, b, a)           //textures
       2.0f, -0.26f,  1.0f,   1.0f,  0.0f,  1.0f,  1.0f,        1.0f, 1.0f,
      -2.0f, -0.26f, -1.0f,   0.0f,  1.0f,  1.0f,  1.0f,        1.0f, 0.0f,
       2.0f, -0.26f, -1.0f,   1.0f,  0.0f,  1.0f,  1.0f,        0.0f, 1.0f,

       2.0f, -0.26f,  1.0f,   1.0f,  0.0f,  1.0f,  1.0f,        0.0f, 1.0f,
      -2.0f, -0.26f,  1.0f,   0.0f,  1.0f,  1.0f,  1.0f,        0.0f, 0.0f,
      -2.0f, -0.26f, -1.0f,   0.0f,  0.0f,  1.0f,  1.0f,        1.0f, 0.0f
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    const GLuint floatsPerUV = 2;


    mesh.planeVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor + floatsPerUV));

    glGenVertexArrays(1, &mesh.planeVao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.planeVao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.planeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.planeVbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor + floatsPerUV);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerColor)));
    glEnableVertexAttribArray(2);

}
void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.cylinderVao);
    glDeleteBuffers(2, mesh.cylinderVbo);
    glDeleteVertexArrays(1, &mesh.planeVao);
    glDeleteBuffers(1, &mesh.planeVbo);

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
