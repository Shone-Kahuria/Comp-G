#define _CRT_SECURE_NO_WARNINGS
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <math.h>

#define PI 3.14159265358979323846f

const int WIDTH = 800, HEIGHT = 600;

// Camera Vectors
float eyeX = 2.0f, eyeY = 2.0f, eyeZ = 4.0f;
float targetX = 0.0f, targetY = 0.0f, targetZ = 0.0f;
float upX = 0.0f, upY = 1.0f, upZ = 0.0f;

// Projection Parameters
float fov = 45.0f;
float nearPlane = 0.1f;
float farPlane = 100.0f;
bool isPerspective = true;

// Custom Perspective Matrix (replaces gluPerspective)
void setPerspective(float fovDeg, float aspect, float zNear, float zFar)
{
    float f = 1.0f / tanf((fovDeg * PI / 180.0f) / 2.0f);
    float m[16] = {
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (zFar + zNear) / (zNear - zFar), -1.0f,
        0.0f, 0.0f, (2.0f * zFar * zNear) / (zNear - zFar), 0.0f
    };
    glLoadMatrixf(m);
}

// Custom Look-At View Matrix (replaces gluLookAt)
void setLookAt(float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz)
{
    // Forward vector (f = normalize(target - eye))
    float fx = tx - ex, fy = ty - ey, fz = tz - ez;
    float fLen = sqrtf(fx * fx + fy * fy + fz * fz);
    if (fLen > 0.0001f) { fx /= fLen; fy /= fLen; fz /= fLen; }

    // Side vector (s = normalize(f x up))
    float sx = fy * uz - fz * uy;
    float sy = fz * ux - fx * uz;
    float sz = fx * uy - fy * ux;
    float sLen = sqrtf(sx * sx + sy * sy + sz * sz);
    if (sLen > 0.0001f) { sx /= sLen; sy /= sLen; sz /= sLen; }

    // Recomputed orthogonal up vector (u = s x f)
    float vx = sy * fz - sz * fy;
    float vy = sz * fx - sx * fz;
    float vz = sx * fy - sy * fx;

    float m[16] = {
        sx, vx, -fx, 0.0f,
        sy, vy, -fy, 0.0f,
        sz, vz, -fz, 0.0f,
        -(sx * ex + sy * ey + sz * ez),
        -(vx * ex + vy * ey + vz * ez),
        (fx * ex + fy * ey + fz * ez),
        1.0f
    };
    glLoadMatrixf(m);
}

// Draw a 1x1x1 cube centered at the world origin (0, 0, 0)
void drawColorCube()
{
    glBegin(GL_QUADS);

    // Front face (Red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);

    // Back face (Green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);

    // Top face (Blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-0.5f, 0.5f, -0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);

    // Bottom face (Yellow)
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);

    // Right face (Magenta)
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f(0.5f, -0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, -0.5f);
    glVertex3f(0.5f, 0.5f, 0.5f);
    glVertex3f(0.5f, -0.5f, 0.5f);

    // Left face (Cyan)
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-0.5f, -0.5f, -0.5f);
    glVertex3f(-0.5f, -0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, 0.5f);
    glVertex3f(-0.5f, 0.5f, -0.5f);

    glEnd();
}

// Handle single-press keys (Projection toggle and Reset)
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_P)
            isPerspective = !isPerspective;

        if (key == GLFW_KEY_R)
        {
            eyeX = 2.0f; eyeY = 2.0f; eyeZ = 4.0f;
            fov = 45.0f;
            isPerspective = true;
        }
    }
}

// Handle continuous camera movement and FOV adjustment
void processInput(GLFWwindow* window)
{
    float speed = 0.04f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera Left / Right
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) eyeX -= speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) eyeX += speed;

    // Camera Forward / Backward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) eyeZ -= speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) eyeZ += speed;

    // Camera Up / Down
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) eyeY += speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) eyeY -= speed;

    // Field of View (FOV) adjustment
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && fov > 15.0f) fov -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS && fov < 110.0f) fov += 0.5f;
}

int main()
{
    if (!glfwInit())
    {
        printf("GLFW initialisation failed!\n");
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Experiment 3: Colour Cube", NULL, NULL);
    if (!window)
    {
        printf("GLFW window creation failed!\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        printf("GLEW initialisation failed!\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    glEnable(GL_DEPTH_TEST);

    char titleBuffer[160];

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        processInput(window);

        int bufferWidth, bufferHeight;
        glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);
        glViewport(0, 0, bufferWidth, bufferHeight);
        float aspect = (bufferHeight > 0) ? (float)bufferWidth / (float)bufferHeight : 1.0f;

        glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Set Projection Matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        if (isPerspective)
        {
            setPerspective(fov, aspect, nearPlane, farPlane);
        }
        else
        {
            float orthoSize = 2.0f;
            glOrtho(-orthoSize * aspect, orthoSize * aspect, -orthoSize, orthoSize, nearPlane, farPlane);
        }

        // 2. Set ModelView Matrix (Look-At View)
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        setLookAt(eyeX, eyeY, eyeZ, targetX, targetY, targetZ, upX, upY, upZ);

        // 3. Draw Stationary Cube at World Origin
        drawColorCube();

        // 4. Display camera coordinates and projection parameters in window title
        sprintf(titleBuffer, "Eye: (%.2f, %.2f, %.2f) | Mode: %s | FOV: %.1f | Near: %.1f Far: %.1f",
            eyeX, eyeY, eyeZ, isPerspective ? "Perspective" : "Orthographic", fov, nearPlane, farPlane);
        glfwSetWindowTitle(window, titleBuffer);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
