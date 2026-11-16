#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <cstdio>
#include <iostream>

const int WIDTH = 800, HEIGHT = 800;

// Triangle vertices
float tx1 = 100.0f, ty1 = 100.0f;
float tx2 = 250.0f, ty2 = 100.0f;
float tx3 = 175.0f, ty3 = 250.0f;

// Rotation settings (Degrees)
float angleDegrees = 30.0f; 
float pivotX = 150.0f, pivotY = -100.0f;

// Helper function to draw a triangle given its vertices
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glVertex2f(x3, y3);
    glEnd();
}

// Draw the coordinate axes
void drawAxes() {
    glColor3f(0.3f, 0.3f, 0.3f); // Dark grey
    glBegin(GL_LINES);
    glVertex2f(-WIDTH, 0.0f); glVertex2f(WIDTH, 0.0f);   // X-axis
    glVertex2f(0.0f, -HEIGHT); glVertex2f(0.0f, HEIGHT); // Y-axis
    glEnd();
}

// Draw the pivot point
void drawPivot(float px, float py) {
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glVertex2f(px, py);
    glEnd();
}

// Render loop function
void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawAxes();

    // Convert angle to radians for math functions
    float rad = angleDegrees * (3.14159265f / 180.0f);
    float cosTheta = cos(rad);
    float sinTheta = sin(rad);

    // 1. Original Triangle (White)
    glColor3f(1.0f, 1.0f, 1.0f);
    drawTriangle(tx1, ty1, tx2, ty2, tx3, ty3);

    // 2. Rotate about Origin (Green)
    float ox1 = tx1 * cosTheta - ty1 * sinTheta;
    float oy1 = tx1 * sinTheta + ty1 * cosTheta;

    float ox2 = tx2 * cosTheta - ty2 * sinTheta;
    float oy2 = tx2 * sinTheta + ty2 * cosTheta;

    float ox3 = tx3 * cosTheta - ty3 * sinTheta;
    float oy3 = tx3 * sinTheta + ty3 * cosTheta;

    glColor3f(0.0f, 1.0f, 0.0f); // Green
    drawTriangle(ox1, oy1, ox2, oy2, ox3, oy3);

    // 3. Rotate about Fixed Pivot (Red)
    float px1 = (tx1 - pivotX) * cosTheta - (ty1 - pivotY) * sinTheta + pivotX;
    float py1 = (tx1 - pivotX) * sinTheta + (ty1 - pivotY) * cosTheta + pivotY;

    float px2 = (tx2 - pivotX) * cosTheta - (ty2 - pivotY) * sinTheta + pivotX;
    float py2 = (tx2 - pivotX) * sinTheta + (ty2 - pivotY) * cosTheta + pivotY;

    float px3 = (tx3 - pivotX) * cosTheta - (ty3 - pivotY) * sinTheta + pivotX;
    float py3 = (tx3 - pivotX) * sinTheta + (ty3 - pivotY) * cosTheta + pivotY;

    glColor3f(1.0f, 0.0f, 0.0f); // Red
    drawTriangle(px1, py1, px2, py2, px3, py3);

    drawPivot(pivotX, pivotY);
}

int main() {
    if (!glfwInit()) return -1;

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "2D Rotation Experiment", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) return -1;

    // Set coordinate system from -400 to 400 so (0,0) is in the exact center
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WIDTH / 2, WIDTH / 2, -HEIGHT / 2, HEIGHT / 2, -1.0, 1.0);

    // Print Legend to Console
    printf("--- 2D ROTATION LEGEND ---\n");
    printf("Angle: %.1f degrees\n", angleDegrees);
    printf("White Triangle: Original\n");
    printf("Green Triangle: Rotated about Origin (0,0)\n");
    printf("Red Triangle  : Rotated about Pivot (%.1f, %.1f)\n", pivotX, pivotY);
    printf("Yellow Point  : Pivot Point\n");
    printf("--------------------------\n");

    while (!glfwWindowShouldClose(window)) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}