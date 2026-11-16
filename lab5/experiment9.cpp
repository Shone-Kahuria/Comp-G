// LAB5.cpp : Defines the entry point for the application.
//

#include "LAB5.h"

using namespace std;

struct Point2D {
    float x, y;
};

// Stored separately so original geometry is never overwritten
Point2D origTri[3] = {
    { 30.0f,  30.0f },
    { 150.0f,  30.0f },
    { 90.0f, 130.0f }
};
Point2D scaledTri[3];

float sx = 1.5f;
float sy = 1.5f;
int axisMode = 0;         // 0 = Uniform XY (s, s), 1 = X-only (sx, 1), 2 = Y-only (1, sy)
int refMode = 0;          // 0 = Origin (0,0), 1 = Custom Fixed Point (a,b), 2 = Centroid
Point2D customPivot = { 80.0f, 60.0f };

// Helper for safe numeric console input
float readValidatedFloat(const std::string& prompt, bool requirePositive = false) {
    float value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            if (requirePositive && value <= 0.0f) {
                std::cout << "  [Invalid] Scale factor must be strictly positive (> 0). Try again.\n";
                continue;
            }
            return value;
        }
        std::cout << "  [Invalid] Please enter a valid number.\n";
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
    }
}

void getUserInput() {
    std::cout << "=== Experiment 9: Manual 2D Triangle Scaling ===\n";
    std::cout << "Enter 1 to input custom coordinates & scale factors, or 0 for default preset: ";
    int choice = 0;
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
        choice = 0;
    }

    if (choice == 1) {
        std::cout << "\nEnter the 3 vertices of the triangle (range -300 to 300):\n";
        for (int i = 0; i < 3; ++i) {
            origTri[i].x = readValidatedFloat("  Vertex " + std::to_string(i + 1) + " x: ");
            origTri[i].y = readValidatedFloat("  Vertex " + std::to_string(i + 1) + " y: ");
        }

        // Validate positive scale factors
        std::cout << "\nEnter positive scale factors (>1 = zoom-in, 0<s<1 = zoom-out):\n";
        sx = readValidatedFloat("  Scale factor sx (> 0): ", true);
        sy = readValidatedFloat("  Scale factor sy (> 0): ", true);

        // Determine axis mode from input
        if (std::fabs(sx - sy) < 1e-5f) {
            axisMode = 0; // Uniform XY
        }
        else if (std::fabs(sy - 1.0f) < 1e-5f) {
            axisMode = 1; // X-only
        }
        else if (std::fabs(sx - 1.0f) < 1e-5f) {
            axisMode = 2; // Y-only
        }
        else {
            axisMode = 3; // Custom non-uniform (sx, sy)
        }

        std::cout << "\nSelect reference point:\n"
            << "  0: Origin (0, 0)\n"
            << "  1: Custom Fixed Point (a, b)\n"
            << "  2: Triangle Centroid\n"
            << "Choice: ";
        int rChoice = 0;
        if (std::cin >> rChoice && rChoice >= 0 && rChoice <= 2) {
            refMode = rChoice;
        }
        if (refMode == 1) {
            customPivot.x = readValidatedFloat("  Fixed point a (x): ");
            customPivot.y = readValidatedFloat("  Fixed point b (y): ");
        }
    }
    std::cout << "\nOpening OpenGL window... (Use keyboard or right-click menu to test other modes)\n";
}

Point2D getCentroid() {
    return {
        (origTri[0].x + origTri[1].x + origTri[2].x) / 3.0f,
        (origTri[0].y + origTri[1].y + origTri[2].y) / 3.0f
    };
}

Point2D getActiveReferencePoint() {
    if (refMode == 1) return customPivot;
    if (refMode == 2) return getCentroid();
    return { 0.0f, 0.0f };
}

// Manual coordinate scaling without built-in glScalef/glScaled
void computeManualScaling() {
    float activeSx = (axisMode == 2) ? 1.0f : sx;
    float activeSy = (axisMode == 1) ? 1.0f : sy;
    Point2D ref = getActiveReferencePoint();

    for (int i = 0; i < 3; ++i) {
        // Step 1: Translate reference point to origin
        float tx = origTri[i].x - ref.x;
        float ty = origTri[i].y - ref.y;

        // Step 2: Scale coordinates manually
        float scx = activeSx * tx;
        float scy = activeSy * ty;

        // Step 3: Translate back to reference point
        scaledTri[i].x = scx + ref.x;
        scaledTri[i].y = scy + ref.y;
    }
}

void drawTriangle(const Point2D tri[3], bool dashed) {
    if (dashed) {
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(2, 0xAAAA);
    }
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 3; ++i) {
        glVertex2f(tri[i].x, tri[i].y);
    }
    glEnd();
    if (dashed) {
        glDisable(GL_LINE_STIPPLE);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    computeManualScaling();

    // Coordinate Axes
    glColor3f(0.4f, 0.4f, 0.45f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(-350.0f, 0.0f); glVertex2f(350.0f, 0.0f);
    glVertex2f(0.0f, -350.0f); glVertex2f(0.0f, 350.0f);
    glEnd();

    // Original Triangle (White, Solid)
    glLineWidth(2.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawTriangle(origTri, false);

    // Manually Scaled Triangle (Emerald Green, Dashed)
    glColor3f(0.2f, 0.95f, 0.55f);
    drawTriangle(scaledTri, true);

    // Reference Point Marker
    Point2D ref = getActiveReferencePoint();
    glPointSize(8.0f);
    glColor3f(1.0f, 0.3f, 0.3f);
    glBegin(GL_POINTS);
    glVertex2f(ref.x, ref.y);
    glEnd();

    char refLabel[64];
    std::snprintf(refLabel, sizeof(refLabel), "Ref (%.1f, %.1f)", ref.x, ref.y);
    drawBitmapText(ref.x + 8.0f, ref.y + 8.0f, refLabel);

    // On-Screen HUD
    float effSx = (axisMode == 2) ? 1.0f : sx;
    float effSy = (axisMode == 1) ? 1.0f : sy;
    const char* modeNames[] = { "Uniform XY (s, s)", "X-Only (sx, 1)", "Y-Only (1, sy)", "Custom (sx, sy)" };
    const char* refNames[] = { "Origin (0, 0)", "Fixed Point (a, b)", "Centroid" };

    char hud[160];
    std::snprintf(hud, sizeof(hud), "Mode: %s | Scale: (sx=%.2f, sy=%.2f) | Ref: %s",
        modeNames[axisMode], effSx, effSy, refNames[refMode]);

    glColor3f(0.95f, 0.95f, 0.95f);
    drawBitmapText(-335.0f, 325.0f, hud);
    drawBitmapText(-335.0f, 305.0f, "Keys: [+/-] Zoom | [X] X-Only | [Y] Y-Only | [U] Uniform | [P] Cycle Ref | [R] Reset");

    glColor3f(1.0f, 1.0f, 1.0f);
    drawBitmapText(-335.0f, -310.0f, "White (Solid): Original Triangle");
    glColor3f(0.2f, 0.95f, 0.55f);
    drawBitmapText(-335.0f, -330.0f, "Green (Dashed): Scaled Triangle");

    glutSwapBuffers();
}

void applyZoom(float delta) {
    // Keep scale factors strictly positive (>= 0.1)
    if (axisMode == 0) {
        sx = std::max(0.1f, sx + delta);
        sy = sx;
    }
    else if (axisMode == 1) {
        sx = std::max(0.1f, sx + delta);
    }
    else if (axisMode == 2) {
        sy = std::max(0.1f, sy + delta);
    }
    else {
        sx = std::max(0.1f, sx + delta);
        sy = std::max(0.1f, sy + delta);
    }
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
    case '+': case '=': applyZoom(0.15f); break;
    case '-': case '_': applyZoom(-0.15f); break;
    case 'x': case 'X': axisMode = 1; break;
    case 'y': case 'Y': axisMode = 2; break;
    case 'u': case 'U': axisMode = 0; sy = sx; break;
    case 'p': case 'P': refMode = (refMode + 1) % 3; break;
    case 'r': case 'R': sx = 1.5f; sy = 1.5f; axisMode = 0; refMode = 0; break;
    case 27: exit(0);
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    getUserInput();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Exp 9: Manual 2D Triangle Scaling (User Input)");

    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-350.0, 350.0, -350.0, 350.0);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}