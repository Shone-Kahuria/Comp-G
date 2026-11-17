#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

// Global Variables 
float rotationAngle = 0.0f;      // Tracks current rotation
float rotationSpeed = 1.0f;      // How fast the cube spins
bool isPaused = false;           // Pause state
int currentAxis = 1;             // 1 = X, 2 = Y, 3 = Z (Default to Y)

// Function to draw a single face of the cube
void drawFace(float v1[3], float v2[3], float v3[3], float v4[3], float r, float g, float b) {
    glBegin(GL_QUADS);
    glColor3f(r, g, b); // Set color for the face
    glVertex3fv(v1);    // Bottom-left
    glVertex3fv(v2);    // Bottom-right
    glVertex3fv(v3);    // Top-right
    glVertex3fv(v4);    // Top-left
    glEnd();
}

// Display callback function (called every frame)
void display() {
    // Clear color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Projection Matrix (Perspective)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 1.0, 100.0); // FOV, Aspect Ratio, Near Plane, Far Plane

    // Model-View Matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Move the camera backward so we can see the cube (which is at origin)
    glTranslatef(0.0f, 0.0f, -6.0f);

    // Apply 3D Rotations 
    if (currentAxis == 1) glRotatef(rotationAngle, 1.0f, 0.0f, 0.0f); // Rotate X
    if (currentAxis == 2) glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f); // Rotate Y
    if (currentAxis == 3) glRotatef(rotationAngle, 0.0f, 0.0f, 1.0f); // Rotate Z

    // Draw the Cube
    // Vertices for a cube from -1 to 1 on all axes
    float p[8][3] = {
        {-1.0f, -1.0f,  1.0f}, // 0: Front-Bottom-Left
        { 1.0f, -1.0f,  1.0f}, // 1: Front-Bottom-Right
        { 1.0f,  1.0f,  1.0f}, // 2: Front-Top-Right
        {-1.0f,  1.0f,  1.0f}, // 3: Front-Top-Left
        {-1.0f, -1.0f, -1.0f}, // 4: Back-Bottom-Left
        { 1.0f, -1.0f, -1.0f}, // 5: Back-Bottom-Right
        { 1.0f,  1.0f, -1.0f}, // 6: Back-Top-Right
        {-1.0f,  1.0f, -1.0f}  // 7: Back-Top-Left
    };

    // Draw each face with a distinct color 
    drawFace(p[0], p[1], p[2], p[3], 1.0f, 0.0f, 0.0f); // Front (Red)
    drawFace(p[1], p[5], p[6], p[2], 0.0f, 1.0f, 0.0f); // Right (Green)
    drawFace(p[5], p[4], p[7], p[6], 0.0f, 0.0f, 1.0f); // Back  (Blue)
    drawFace(p[4], p[0], p[3], p[7], 1.0f, 1.0f, 0.0f); // Left  (Yellow)
    drawFace(p[3], p[2], p[6], p[7], 1.0f, 0.0f, 1.0f); // Top   (Magenta)
    drawFace(p[4], p[5], p[1], p[0], 0.0f, 1.0f, 1.0f); // Bottom(Cyan)

    // Swap buffers to display the rendered frame
    glutSwapBuffers();
}

// Timer callback (drives animation)
void timer(int value) {
    if (!isPaused) {
        rotationAngle += rotationSpeed; // Update angle
        if (rotationAngle > 360.0f) rotationAngle -= 360.0f; // Keep it within 0-360
    }
    glutPostRedisplay(); // Request redraw
    glutTimerFunc(16, timer, 0); // Call timer again in 16ms (~60 FPS)
}

// Keyboard callback for controls
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: // ESC key to exit program
        exit(0);
        break;
    case 'p': // Pause
        isPaused = true;
        break;
    case 'r': // Resume
        isPaused = false;
        break;
    case 'a': // Speed up
        rotationSpeed += 0.5f;
        break;
    case '-': // Slow down
        rotationSpeed -= 0.5f;
        break;
    case 'x': // Rotate on X axis
        currentAxis = 1;
        break;
    case 'y': // Rotate on Y axis
        currentAxis = 2;
        break;
    case 'z': // Rotate on Z axis
        currentAxis = 3;
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    // Initialize GLUT
    glutInit(&argc, argv);
    // Double buffering, RGB color, Depth buffer enabled
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(600, 600);
    glutCreateWindow("OpenGL 3D Rotating Cube");

    // Enable depth testing (Critical for 3D!)
    glEnable(GL_DEPTH_TEST);

    // Register callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0); // Start timer

    // Enter the GLUT event loop
    glutMainLoop();
    return 0;
}