#include <GL/freeglut.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace {

constexpr int kMinimumDepth = 0;
constexpr int kMaximumDepth = 6;

struct Point3 {
    float x;
    float y;
    float z;
};

int recursionDepth = 3;
float rotationX = 20.0F;
float rotationY = -30.0F;
bool wireframe = false;
bool dragging = false;
int previousMouseX = 0;
int previousMouseY = 0;

Point3 midpoint(const Point3& first, const Point3& second) {
    return {
        (first.x + second.x) * 0.5F,
        (first.y + second.y) * 0.5F,
        (first.z + second.z) * 0.5F,
    };
}

void drawTriangle(const Point3& a, const Point3& b, const Point3& c, int colour) {
    constexpr std::array<std::array<float, 3>, 4> colours{{
        {{0.95F, 0.25F, 0.20F}},
        {{0.20F, 0.75F, 0.35F}},
        {{0.20F, 0.45F, 0.95F}},
        {{0.95F, 0.75F, 0.15F}},
    }};

    glColor3fv(colours[static_cast<std::size_t>(colour)].data());
    glBegin(GL_TRIANGLES);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glVertex3f(c.x, c.y, c.z);
    glEnd();
}

void drawTetrahedron(const Point3& a, const Point3& b, const Point3& c, const Point3& d) {
    drawTriangle(a, c, b, 0);
    drawTriangle(a, b, d, 1);
    drawTriangle(a, d, c, 2);
    drawTriangle(b, c, d, 3);
}

void subdivide(const Point3& a, const Point3& b, const Point3& c, const Point3& d, int depth) {
    if (depth == 0) {
        drawTetrahedron(a, b, c, d);
        return;
    }

    const Point3 ab = midpoint(a, b);
    const Point3 ac = midpoint(a, c);
    const Point3 ad = midpoint(a, d);
    const Point3 bc = midpoint(b, c);
    const Point3 bd = midpoint(b, d);
    const Point3 cd = midpoint(c, d);

    const int nextDepth = depth - 1;
    subdivide(a, ab, ac, ad, nextDepth);
    subdivide(ab, b, bc, bd, nextDepth);
    subdivide(ac, bc, c, cd, nextDepth);
    subdivide(ad, bd, cd, d, nextDepth);
}

int tetrahedronCount(int depth) {
    int count = 1;
    for (int level = 0; level < depth; ++level) {
        count *= 4;
    }
    return count;
}

void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (const unsigned char character : text) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, character);
    }
}

void drawOverlay() {
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0F, 1.0F, 1.0F);
    std::ostringstream status;
    status << "Depth: " << recursionDepth << "   Tetrahedra: " << tetrahedronCount(recursionDepth)
           << "   Mode: " << (wireframe ? "wireframe" : "solid");
    drawText(15.0F, 575.0F, status.str());
    drawText(15.0F, 552.0F, "+/- depth | W wireframe | R reset | arrows/mouse rotate | Esc exit");

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0F, 0.0F, -5.0F);
    glRotatef(rotationX, 1.0F, 0.0F, 0.0F);
    glRotatef(rotationY, 0.0F, 1.0F, 0.0F);
    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);

    // A regular tetrahedron centred close to the origin.
    constexpr Point3 a{0.0F, 1.35F, 0.0F};
    constexpr Point3 b{-1.25F, -0.75F, 0.85F};
    constexpr Point3 c{1.25F, -0.75F, 0.85F};
    constexpr Point3 d{0.0F, -0.75F, -1.35F};
    subdivide(a, b, c, d, recursionDepth);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawOverlay();
    glutSwapBuffers();
}

void reshape(int width, int height) {
    const int safeHeight = std::max(height, 1);
    glViewport(0, 0, width, safeHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, static_cast<double>(width) / safeHeight, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
    case '+':
    case '=':
        recursionDepth = std::min(recursionDepth + 1, kMaximumDepth);
        break;
    case '-':
    case '_':
        recursionDepth = std::max(recursionDepth - 1, kMinimumDepth);
        break;
    case 'w':
    case 'W':
        wireframe = !wireframe;
        break;
    case 'r':
    case 'R':
        recursionDepth = 3;
        rotationX = 20.0F;
        rotationY = -30.0F;
        wireframe = false;
        break;
    case 27:
        glutLeaveMainLoop();
        return;
    default:
        return;
    }
    glutPostRedisplay();
}

void specialKeyboard(int key, int, int) {
    switch (key) {
    case GLUT_KEY_LEFT:  rotationY -= 5.0F; break;
    case GLUT_KEY_RIGHT: rotationY += 5.0F; break;
    case GLUT_KEY_UP:    rotationX -= 5.0F; break;
    case GLUT_KEY_DOWN:  rotationX += 5.0F; break;
    default: return;
    }
    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        dragging = (state == GLUT_DOWN);
        previousMouseX = x;
        previousMouseY = y;
    }
}

void mouseMotion(int x, int y) {
    if (!dragging) {
        return;
    }
    rotationY += static_cast<float>(x - previousMouseX) * 0.5F;
    rotationX += static_cast<float>(y - previousMouseY) * 0.5F;
    previousMouseX = x;
    previousMouseY = y;
    glutPostRedisplay();
}

int readDepth() {
    std::cout << "Experiment 6 - 3D Sierpinski gasket\n";
    std::cout << "Enter recursion depth (" << kMinimumDepth << '-' << kMaximumDepth << "): ";
    int depth = 0;
    while (!(std::cin >> depth) || depth < kMinimumDepth || depth > kMaximumDepth) {
        std::cout << "Invalid input. Enter an integer from " << kMinimumDepth << " to " << kMaximumDepth << ": ";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return depth;
}

} // namespace

int main(int argc, char** argv) {
    recursionDepth = readDepth();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Experiment 6 - 3D Sierpinski Gasket");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glClearColor(0.035F, 0.045F, 0.08F, 1.0F);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeyboard);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutMainLoop();
    return 0;
}

