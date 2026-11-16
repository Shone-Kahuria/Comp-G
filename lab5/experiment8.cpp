// LAB5.cpp : Defines the entry point for the application.
//

#include "LAB5.h"

using namespace std;

struct Point2D {
    float x, y;
};

struct ColorRGB {
    float r, g, b;
    std::string name;
};

std::vector<Point2D> polygon;
bool isClosed = false;
bool isFilled = false;
bool inputMode = false;

ColorRGB fillColor = { 0.2f, 0.75f, 0.95f, "Cyan" };

// Load predefined test polygons required by the manual
void loadPreset(int type) {
    polygon.clear();
    isFilled = false;
    inputMode = false;

    if (type == 1) {
        // Convex Pentagon
        polygon = {
            { -120.0f, -80.0f },
            { 120.0f, -80.0f },
            { 160.0f,  50.0f },
            { 0.0f, 160.0f },
            { -160.0f,  50.0f }
        };
        isClosed = true;
    }
    else if (type == 2) {
        // Concave Polygon (Non-convex notch at top)
        polygon = {
            { -150.0f, -120.0f },
            { 150.0f, -120.0f },
            { 150.0f,  130.0f },
            { 0.0f,   10.0f },
            { -150.0f,  130.0f }
        };
        isClosed = true;
    }
    else if (type == 3) {
        // Polygon with Horizontal Edges
        polygon = {
            { -160.0f, -100.0f },
            { 160.0f, -100.0f },
            { 160.0f,   20.0f },
            { 40.0f,   20.0f },
            { 40.0f,  140.0f },
            { -160.0f,  140.0f }
        };
        isClosed = true;
    }
}

// Scan-line area-filling algorithm using the even-odd rule
void scanLineFill() {
    if (!isClosed || polygon.size() < 3) return;

    // 1. Find y_min and y_max of the polygon
    float minY = polygon[0].y;
    float maxY = polygon[0].y;
    for (const auto& pt : polygon) {
        if (pt.y < minY) minY = pt.y;
        if (pt.y > maxY) maxY = pt.y;
    }

    int startY = static_cast<int>(std::ceil(minY));
    int endY = static_cast<int>(std::floor(maxY));

    glColor3f(fillColor.r, fillColor.g, fillColor.b);
    glLineWidth(1.0f);

    // 2. Process each horizontal scan line y
    for (int y = startY; y <= endY; ++y) {
        std::vector<float> intersections;
        size_t n = polygon.size();

        for (size_t i = 0; i < n; ++i) {
            Point2D p1 = polygon[i];
            Point2D p2 = polygon[(i + 1) % n];

            // Ignore horizontal edges (handled naturally by adjacent non-horizontal edges)
            if (std::fabs(p1.y - p2.y) < 1e-5f) {
                continue;
            }

            float edgeMinY = std::min(p1.y, p2.y);
            float edgeMaxY = std::max(p1.y, p2.y);

            // Include lower endpoint, exclude upper endpoint to avoid double-counting vertices
            if (static_cast<float>(y) >= edgeMinY && static_cast<float>(y) < edgeMaxY) {
                // Linear interpolation for x-intersection
                float xIntersect = p1.x + (static_cast<float>(y) - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                intersections.push_back(xIntersect);
            }
        }

        // 3. Sort x-intersections in ascending order
        std::sort(intersections.begin(), intersections.end());

        // 4. Fill spans between alternating pairs (even-odd rule)
        glBegin(GL_LINES);
        for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
            glVertex2f(intersections[i], static_cast<float>(y));
            glVertex2f(intersections[i + 1], static_cast<float>(y));
        }
        glEnd();
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Faint reference grid axes
    glColor3f(0.25f, 0.27f, 0.32f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    glVertex2f(-350.0f, 0.0f); glVertex2f(350.0f, 0.0f);
    glVertex2f(0.0f, -350.0f); glVertex2f(0.0f, 350.0f);
    glEnd();

    // Fill interior if requested
    if (isFilled && isClosed) {
        scanLineFill();
    }

    // Draw polygon boundary over fill to preserve a crisp outline
    if (!polygon.empty()) {
        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(2.0f);
        glBegin(isClosed ? GL_LINE_LOOP : GL_LINE_STRIP);
        for (const auto& pt : polygon) {
            glVertex2f(pt.x, pt.y);
        }
        glEnd();

        // Highlight vertices and display coordinates
        glPointSize(6.0f);
        glColor3f(1.0f, 0.85f, 0.2f);
        glBegin(GL_POINTS);
        for (const auto& pt : polygon) {
            glVertex2f(pt.x, pt.y);
        }
        glEnd();

        for (size_t i = 0; i < polygon.size(); ++i) {
            char vLabel[48];
            std::snprintf(vLabel, sizeof(vLabel), "V%zu(%.0f,%.0f)", i + 1, polygon[i].x, polygon[i].y);
            drawBitmapText(polygon[i].x + 6.0f, polygon[i].y + 6.0f, vLabel);
        }
    }

    // On-screen status & legend[cite: 8]
    char hud[160];
    std::snprintf(hud, sizeof(hud), "Vertices: %zu | State: %s | Fill Color: %s",
        polygon.size(),
        inputMode ? "CLICK TO ADD VERTICES (Press ENTER to close)" : (isFilled ? "FILLED" : "BOUNDARY ONLY"),
        fillColor.name.c_str());
    glColor3f(0.95f, 0.95f, 0.95f);
    drawBitmapText(-335.0f, 325.0f, hud);
    drawBitmapText(-335.0f, 305.0f, "Right-Click for Menu | Keys: [F] Fill | [C] Clear | [N] New Polygon | [Enter] Close Polygon");

    glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && inputMode) {
        // Map window pixel coordinates (800x800) to world coordinates (-350..350)
        float wx = -350.0f + (static_cast<float>(x) / 800.0f) * 700.0f;
        float wy = 350.0f - (static_cast<float>(y) / 800.0f) * 700.0f;
        polygon.push_back({ std::round(wx), std::round(wy) });
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
    case 13: // ENTER key closes user-drawn polygon
        if (inputMode && polygon.size() >= 3) {
            isClosed = true;
            inputMode = false;
        }
        break;
    case 'f': case 'F':
        if (isClosed && polygon.size() >= 3) isFilled = true;
        break;
    case 'c': case 'C':
        isFilled = false;
        break;
    case 'n': case 'N':
        polygon.clear();
        isClosed = false;
        isFilled = false;
        inputMode = true;
        break;
    case 27:
        exit(0);
    }
    glutPostRedisplay();
}

void menuHandler(int option) {
    switch (option) {
    case 1: // Enter/Reset Polygon (interactive mouse clicks)[cite: 8]
        polygon.clear();
        isClosed = false;
        isFilled = false;
        inputMode = true;
        break;
    case 2: loadPreset(1); break; // Preset: Convex[cite: 8]
    case 3: loadPreset(2); break; // Preset: Concave[cite: 8]
    case 4: loadPreset(3); break; // Preset: Horizontal Edge[cite: 8]
    case 5: fillColor = { 0.2f, 0.75f, 0.95f, "Cyan" }; break;
    case 6: fillColor = { 0.95f, 0.45f, 0.2f, "Orange" }; break;
    case 7: fillColor = { 0.35f, 0.9f, 0.45f, "Emerald Green" }; break;
    case 8: fillColor = { 0.85f, 0.35f, 0.9f, "Magenta" }; break;
    case 9: // Fill[cite: 8]
        if (isClosed && polygon.size() >= 3) isFilled = true;
        break;
    case 10: // Clear fill[cite: 8]
        isFilled = false;
        break;
    case 11: // Exit[cite: 8]
        exit(0);
    }
    glutPostRedisplay();
}

void createMenu() {
    int presetSubMenu = glutCreateMenu(menuHandler);
    glutAddMenuEntry("Convex Polygon", 2);
    glutAddMenuEntry("Concave Polygon", 3);
    glutAddMenuEntry("Polygon with Horizontal Edge", 4);

    int colorSubMenu = glutCreateMenu(menuHandler);
    glutAddMenuEntry("Cyan", 5);
    glutAddMenuEntry("Orange", 6);
    glutAddMenuEntry("Emerald Green", 7);
    glutAddMenuEntry("Magenta", 8);

    glutCreateMenu(menuHandler);
    glutAddMenuEntry("Enter / Reset Custom Polygon (Click Vertices)", 1);
    glutAddSubMenu("Load Test Polygon", presetSubMenu);
    glutAddSubMenu("Choose Fill Colour", colorSubMenu);
    glutAddMenuEntry("Fill Polygon", 9);
    glutAddMenuEntry("Clear Fill", 10);
    glutAddMenuEntry("Exit", 11);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv) {
    loadPreset(2); // Start with Concave polygon boundary ready to inspect & fill[cite: 8]

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Exp 8: Scan-Line Polygon Area-Filling Algorithm");

    glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-350.0, 350.0, -350.0, 350.0);

    createMenu();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}