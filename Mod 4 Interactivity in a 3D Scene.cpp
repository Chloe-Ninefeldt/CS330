#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>  // GLFW library




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
        GLuint planeVao, cylinderVao; // Handle for the vertex array object        
        GLuint planeVbo, cylinderVbo[2]; // Handles for the vertex buffer objects           
        GLuint planeVertices, cylinderVertices; // Number of indices of the mesh

    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    GLMesh gMeshPlane;

    // Shader program
    GLuint gProgramId;
    GLuint gProgramIdPlane;


    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 10.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // globals
   // bool keys[1024];
    bool ortho = false;


    glm::vec3 gPositionSoda(-2.0f, -0.25f, 0.0f);
    glm::vec3 gScaleSoda(2.0f);


    glm::vec3 gPositionPlane(0.0f, 0.0f, 0.0f);
    glm::vec3 gScalePlane(3.0f);
 


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
void UCreateMeshPlane(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Plane Vertex Shader Source Code*/

const GLchar* vertexShaderSourcePlane = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1

out vec4 vertexColor; // variable to transfer color data to the fragment shader

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSourcePlane = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor);
}
);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//* Vertex Shader Source Code*/
const GLchar* vertexShaderSourceSoda = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
layout(location = 1) in vec4 color;  // Color data from Vertex Attrib Pointer 1

out vec4 vertexColor; // variable to transfer color data to the fragment shader

//Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates
    vertexColor = color; // references incoming color data
}
);


/* Fragment Shader Source Code*/
const GLchar* fragmentShaderSourceSoda = GLSL(440,
    in vec4 vertexColor; // Variable to hold incoming color data from vertex shader

out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor);
}
);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(vertexShaderSourceSoda, fragmentShaderSourceSoda, gProgramId))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMeshPlane(gMeshPlane); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs
    if (!UCreateShaderProgram(vertexShaderSourcePlane, fragmentShaderSourcePlane, gProgramIdPlane))
        return EXIT_FAILURE;




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
    UDestroyMesh(gMeshPlane);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gProgramIdPlane);

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

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) //creates the ortho projection
    {

        gCamera.Position = glm::vec3(-2.5f, 5.2f, -1.5f);
        gCamera.Pitch = -100.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) { //resets the ortho projection


        gCamera.Position = glm::vec3(0.0f, 0.0f, 5.0f);
        gCamera.Pitch = 0.0f;
        glfwSwapBuffers(window);
        glfwPollEvents();

    }


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
    verts[4] = 0.0f;        //0 g    
    verts[5] = 1.0f;        //0 b
    verts[6] = 1.0f;        //0 a
    currentVertex++;
    verts[7] = 0.0f;        //1 x
    verts[8] = -halfLen;    //1 y
    verts[9] = 0.0f;        //1 z
    verts[10] = 0.0f;       //1 r
    verts[11] = 0.0f;       //1 g
    verts[12] = 1.0f;       //1 b
    verts[13] = 1.0f;       //1 a
    currentVertex++;

    int currentTriangle = 0;
    for (int edge = 0; edge < numSides; edge++) {
        float theta = ((float)edge) * radiansPerSide;

        verts[currentVertex * STRIDE + 0] = radius * cos(theta);    //x
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


    glUseProgram(gProgramId);

    glm::mat4 model = glm::translate(gPositionSoda) * glm::scale(gScaleSoda);

    // camera/view transformation
    glm::mat4 projection;
    glm::mat4 view;

    // Setup views and projections
    if (ortho) {
        GLfloat oWidth = (GLfloat)WINDOW_WIDTH * 0.01f; // 10% of width
        GLfloat oHeight = (GLfloat)WINDOW_HEIGHT * 0.01f; // 10% of height

        view = gCamera.GetViewMatrix();
        projection = glm::ortho(-oWidth, oWidth, oHeight, -oHeight, 0.1f, 100.0f);
    }
    else {
        view = gCamera.GetViewMatrix();
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }


    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

  

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
  

    glBindVertexArray(gMesh.cylinderVao);

    glDrawElements(GL_TRIANGLES, gMesh.cylinderVertices, GL_UNSIGNED_SHORT, NULL);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   // PLANE: draw plane
    //----------------
    
    // Set the shader to be used
    glUseProgram(gProgramIdPlane);

    model = glm::translate(gPositionPlane) * glm::scale(gScalePlane);

    
    // Setup views and projections
    if (ortho) {
        GLfloat oWidth = (GLfloat)WINDOW_WIDTH * 0.01f; // 10% of width
        GLfloat oHeight = (GLfloat)WINDOW_HEIGHT * 0.01f; // 10% of height

        view = gCamera.GetViewMatrix();
        projection = glm::ortho(-oWidth, oWidth, oHeight, -oHeight, 0.1f, 100.0f);
    }
    else {
        view = gCamera.GetViewMatrix();
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }


    // Retrieves and passes transform matrices to the Shader program
    GLint modelLocPlane = glGetUniformLocation(gProgramIdPlane, "model");
    GLint viewLocPlane = glGetUniformLocation(gProgramIdPlane, "view");
    GLint projLocPlane = glGetUniformLocation(gProgramIdPlane, "projection");

    glUniformMatrix4fv(modelLocPlane, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLocPlane, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLocPlane, 1, GL_FALSE, glm::value_ptr(projection));



    // Activate the plane VAO (used by plane and candle)
    glBindVertexArray(gMeshPlane.planeVao);


    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMeshPlane.planeVertices);


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////




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
    UCreateSoda(verts, indices, NUM_SIDES, 0.15f, 0.25f);

    //draw Cylinder - Soda


    const GLuint floatsPerVertex2 = 3;
    const GLuint floatsPerColor2 = 4;


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

};


void UCreateMeshPlane(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
       2.0f, -0.26f,  1.0f,   1.0f,  0.0f,  1.0f,  1.0f,
      -2.0f, -0.26f, -1.0f,   0.0f,  1.0f,  1.0f,  1.0f,
       2.0f, -0.26f, -1.0f,   1.0f,  0.0f,  1.0f,  1.0f,    // Triangle 1

       2.0f, -0.26f,  1.0f,   1.0f,  0.0f,  1.0f,  1.0f,
      -2.0f, -0.26f,  1.0f,   0.0f,  1.0f,  1.0f,  1.0f,
      -2.0f, -0.26f, -1.0f,   0.0f,  0.0f,  1.0f,  1.0f   // Triangle 2
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerColor = 4;
    

    mesh.planeVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerColor));

    glGenVertexArrays(1, &mesh.planeVao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.planeVao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(1, &mesh.planeVbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.planeVbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerColor);// The number of floats before each

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.cylinderVao);
    glDeleteBuffers(2, mesh.cylinderVbo); 
    glDeleteVertexArrays(1, &mesh.planeVao);
    glDeleteBuffers(1, &mesh.planeVbo);

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