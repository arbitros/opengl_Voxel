#include <cmath>
#include <iostream>
#include "glad.h"
#include <GLFW/glfw3.h>

#include "shader.h"
#include "camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const unsigned int SCRWIDTH = 800;
const unsigned int SCRHEIGHT = 600;
const unsigned int NUMCHUNK = 1;

const unsigned int CHUNKSIZE = 70;

bool firstMov = true;
float lastX = SCRWIDTH/2.0f;
float lastY = SCRHEIGHT/2.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
float raycast(glm::vec3 viewDir, glm::vec3 viewPos, glm::vec3 planeNormal, glm::vec3 planePos);
Camera cam(glm::vec3(-2.6f,+1.7f,2.6f));

glm::mat4 view;
glm::mat4 projection;

glm::vec3 lightPos = glm::vec3(-2.0f, -2.0f, -2.0f);

int main()
{
    cam.Front = glm::vec3(0.8,-0.43,-0.49);
    cam.Position = glm::vec3(-6.1f, 5.8f, -3.6f);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
    GLFWwindow* window = glfwCreateWindow(SCRWIDTH, SCRHEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, scroll_callback);

    Shader shaderProgram("../vshader.glsl", "../fshader.glsl");
    Shader lightingShader("../lvshader.glsl", "../lfshader.glsl");

    float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, //ZN
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f, //ZP
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, //XN
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, //XP
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, //YN
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, //YP
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

        unsigned int VAO[6];
        unsigned int VBO;

    {
        float verticesFaces[6][36];

        int jk = 0;
        int ik = 0;
        for (int i = 0; i<(36*6); i++) {
            if (i % 36 == 0 && i != 0) {
                jk++;
                ik = 0;
            }
            verticesFaces[jk][ik] = vertices[i];
            ik++;
        }

        const float stride = 6*sizeof(float);
        glGenVertexArrays(6, VAO);

        for (int i = 0; i<6; i++) {
            glBindVertexArray(VAO[i]);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glBufferData(GL_ARRAY_BUFFER, sizeof(verticesFaces[i]), verticesFaces[i], GL_STATIC_DRAW);
            glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, stride, (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
            glEnableVertexAttribArray(1);
        }
    }

    view = cam.GetViewMatrix();
    projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);

    shaderProgram.use();
    shaderProgram.setVec3("objectColor", glm::vec3(0.34f, 0.55f, 0.6f));
    shaderProgram.setVec3("lightColor", glm::vec3(1.0f));
    shaderProgram.setVec3("lightPos", cam.Position);

    shaderProgram.matrix.model = glm::mat4(1.0f);

    glEnable(GL_DEPTH_TEST);

    struct voxel {
        bool isSolid;
        bool isOccluded;
        bool face[6]; //ZN,ZP,XN,XP,YN,YP
        bool isTargeted;

        glm::vec3 pos;

        void update_occlusion() {
            unsigned int cnt = 0;
            for (int i = 0; i < 6; i++) {
                cnt += face[i];
            }
            if (cnt == 0) {
                isOccluded = true;
            }
        }
    };

    struct Chunk {
        voxel voxels[CHUNKSIZE][CHUNKSIZE][CHUNKSIZE];
        bool isRendered;
        bool isTargeted;

        glm::vec3 pos;

        void Chunk_constructor() { // av någon anledning fungerar inte default constructorn när
            // barnet ska kalla på den, föräldern måste kalla på konstruktorn
            isRendered = true;
            isTargeted = false;

            glm::mat4 identMat = glm::mat4(1.0f);
            for (int i = 0; i < CHUNKSIZE; i++) {
                for (int j = 0; j < CHUNKSIZE; j++) {
                    for (int k = 0; k < CHUNKSIZE; k++) {
                        voxels[i][j][k].isSolid = false;
                        voxels[i][j][k].isTargeted = false;
                        for (int l = 0; l < 6; l++) {
                            voxels[i][j][k].face[l] = true;
                        }
                        voxels[i][j][k].pos = glm::vec3(float(i), float(j), float(k)) + pos;
                    }
                }
            }
        }

        void draw(Shader shader, unsigned int* VAO) {
            glm::mat4 identMat = glm::mat4(1.0f);
            for (int i = 0; i < CHUNKSIZE; i++) {
                for (int j = 0; j < CHUNKSIZE; j++) {
                    for (int k = 0; k < CHUNKSIZE; k++) { //ZN,ZP,XN,XP,YN,YP
                        if (voxels[i][j][k].isSolid) {
                            shader.matrix.model = glm::translate(identMat, voxels[i][j][k].pos);
                            shader.setMat4("model", shader.matrix.model);
                            for (int l = 0; l < 6; l++) {
                                if (voxels[i][j][k].face[l]) {
                                    if (voxels[i][j][k].isTargeted) {
                                        shader.setVec3("objectColor", glm::vec3(1.0f,0.0f,0.0f));
                                    }
                                    glBindVertexArray(VAO[l]);
                                    glDrawArrays(GL_TRIANGLES, 0, 6);
                                }
                            }
                        }
                    }
                }
            }
        }
    };

    struct chunkArr {
        Chunk chunk[NUMCHUNK][NUMCHUNK];

        chunkArr() {
            for(int i = 0; i < NUMCHUNK; i++) {
                for (int j = 0; j < NUMCHUNK; j++) {
                    chunk[i][j].pos = glm::vec3(i*(CHUNKSIZE), 0, j*(CHUNKSIZE));
                    chunk[i][j].Chunk_constructor();
                    determine_faces();
                }
            }
        }

        void determine_faces() {
            for (int g = 0; g < NUMCHUNK; g++) {
                for (int h = 0; h < NUMCHUNK; h++) {
                    for (int i = 0; i < CHUNKSIZE; i++) {
                        for (int j = 0; j < CHUNKSIZE; j++) {
                            for (int k = 0; k < CHUNKSIZE; k++) {
                                if (chunk[g][h].voxels[i][j][k].isSolid) {
                                    if (i != 0 && i != (CHUNKSIZE-1)) { //ZN,ZP,XN,XP,YN,YP
                                        if (chunk[g][h].voxels[i+1][j][k].isSolid) {
                                            chunk[g][h].voxels[i][j][k].face[3] = false;
                                        } else {
                                            chunk[g][h].voxels[i][j][k].face[3] = true;
                                        }
                                        if (chunk[g][h].voxels[i-1][j][k].isSolid) {
                                            chunk[g][h].voxels[i][j][k].face[2] = false;
                                        } else {
                                            chunk[g][h].voxels[i][j][k].face[2] = true;
                                        }
                                    } else {
                                        if (i == 0) {
                                            if (chunk[g][h].voxels[i+1][j][k].isSolid) {
                                                chunk[g][h].voxels[i][j][k].face[3] = false;
                                            } else {
                                                chunk[g][h].voxels[i][j][k].face[3] = true;
                                            }
                                            if (g != 0) {
                                                if (chunk[g-1][h].voxels[CHUNKSIZE-1][j][k].isSolid) {
                                                    chunk[g][h].voxels[i][j][k].face[2] = false;
                                                } else {
                                                chunk[g][h].voxels[i][j][k].face[2] = true;
                                                }
                                            }
                                        } else {
                                            if (chunk[g][h].voxels[i-1][j][k].isSolid) {
                                                chunk[g][h].voxels[i][j][k].face[2] = false;
                                            } else {
                                                chunk[g][h].voxels[i][j][k].face[2] = true;
                                            }
                                            if (g != NUMCHUNK-1) {
                                                if (chunk[g+1][h].voxels[0][j][k].isSolid) {
                                                    chunk[g][h].voxels[i][j][k].face[3] = false;
                                                } else {
                                                    chunk[g][h].voxels[i][j][k].face[3] = true;
                                                }
                                            }
                                        }
                                    }

                                    if (j != 0 && j != (CHUNKSIZE-1)) { //ZN,ZP,XN,XP,YN,YP
                                        if (chunk[g][h].voxels[i][j+1][k].isSolid) {
                                            chunk[g][h].voxels[i][j][k].face[5] = false;
                                        } else {
                                            chunk[g][h].voxels[i][j][k].face[5] = true;
                                        }
                                        if (chunk[g][h].voxels[i][j-1][k].isSolid) {
                                            chunk[g][h].voxels[i][j][k].face[4] = false;
                                        } else {
                                            chunk[g][h].voxels[i][j][k].face[4] = true;
                                        }
                                    } else {
                                        if (j == 0) {
                                            if (chunk[g][h].voxels[i][j+1][k].isSolid) {
                                                chunk[g][h].voxels[i][j][k].face[5] = false;
                                            } else {
                                                chunk[g][h].voxels[i][j][k].face[5] = true;
                                            }
                                        } else {
                                            if (chunk[g][h].voxels[i][j-1][k].isSolid) {
                                                chunk[g][h].voxels[i][j][k].face[4] = false;
                                            } else {
                                                chunk[g][h].voxels[i][j][k].face[4] = true;
                                            }
                                        }
                                    }

                                    if (k != 0 && k != (CHUNKSIZE-1)) { //ZN,ZP,XN,XP,YN,YP
                                        if (chunk[g][h].voxels[i][j][k+1].isSolid) {
                                            chunk[g][h].voxels[i][j][k].face[1] = false;
                                        } else {
                                            chunk[g][h].voxels[i][j][k].face[1] = true;
                                        }
                                        if (chunk[g][h].voxels[i][j][k-1].isSolid) {
                                            chunk[g][h].voxels[i][j][k].face[0] = false;
                                        } else {
                                            chunk[g][h].voxels[i][j][k].face[0] = true;
                                        }
                                    } else {
                                        if (k == 0) {
                                            if (chunk[g][h].voxels[i][j][k+1].isSolid) {
                                                chunk[g][h].voxels[i][j][k].face[1] = false;
                                            } else {
                                                chunk[g][h].voxels[i][j][k].face[1] = true;
                                            }
                                            if (h != 0) {
                                                if (chunk[g][h-1].voxels[i][j][CHUNKSIZE-1].isSolid) {
                                                    chunk[g][h].voxels[i][j][k].face[0] = false;
                                                } else {
                                                    chunk[g][h].voxels[i][j][k].face[0] = true;
                                                }
                                            }
                                        } else {
                                            if (chunk[g][h].voxels[i][j][k-1].isSolid) {
                                                chunk[g][h].voxels[i][j][k].face[0] = false;
                                            } else {
                                                chunk[g][h].voxels[i][j][k].face[0] = true;
                                            }
                                            if (h != NUMCHUNK - 1) {
                                                if (chunk[g][h+1].voxels[i][j][0].isSolid) {
                                                    chunk[g][h].voxels[i][j][k].face[1] = false;
                                                } else {
                                                    chunk[g][h].voxels[i][j][k].face[1] = true;
                                                }
                                            }
                                        }
                                    } 
                                }
                                chunk[g][h].voxels[i][j][k].update_occlusion();
                            }
                        }
                    }
                }
            }
        }
    };

    chunkArr chunks{};
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bool isTapped = false;
    chunks.chunk[0][0].voxels[0][0][0].isSolid = true;

    while(!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        view = cam.GetViewMatrix();
        projection = glm::perspective(glm::radians(cam.Zoom), 800.0f/600.0f, 0.1f, 100.0f);

        shaderProgram.use();
        shaderProgram.setMat4("view", view);
        shaderProgram.setMat4("projection", projection);

        shaderProgram.setVec3("viewPos", cam.Position);
        shaderProgram.setVec3("lightPos", cam.Position);

        for (int i = 0; i < NUMCHUNK; i++) {
            for (int j = 0; j < NUMCHUNK; j++) {
                if (chunks.chunk[i][j].isRendered) {
                    if(glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
                    }
                    chunks.chunk[i][j].draw(shaderProgram, VAO);
                }
            }
        }
        chunks.determine_faces();

        isTapped = true;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (int i = 0; i<6; i++) {
        glDeleteVertexArrays(1, &VAO[i]);
    }
    glDeleteBuffers(1, &VBO);
    //glDeleteVertexArrays(1, &lightVAO);
    shaderProgram.terminate();
    lightingShader.terminate();
    glfwTerminate();
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.ProcessKeyboard(FORWARD, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.ProcessKeyboard(LEFT, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.ProcessKeyboard(RIGHT, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cam.ProcessKeyboard(UP, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        cam.ProcessKeyboard(DOWN, deltaTime);
    }
    if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        std::cout << "Pos: " << cam.Position.x << " " << cam.Position.y << " " <<cam.Position.z << std::endl;
        std::cout << cam.Front.x << " " << cam.Front.y << " " <<cam.Front.z << std::endl;
        std::cout << cam.Yaw << " " << cam.Pitch << std::endl;
    }
}

float raycast(glm::vec3 viewDir, glm::vec3 viewPos, glm::vec3 planeNormal, glm::vec3 planePos) {
    float t = glm::dot(planeNormal, planePos-viewPos)/glm::dot(planeNormal,viewDir);
    return t;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if(firstMov) {
        lastX = xpos;
        lastY = ypos;
        firstMov = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    cam.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    cam.ProcessMouseScroll(xoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

