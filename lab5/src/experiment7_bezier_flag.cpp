#include <GL/freeglut.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <string>

namespace {

constexpr float kPi = 3.14159265358979323846F;
constexpr int kSamples = 80;

struct Point2 {
    float x;
    float y;
};

bool animating = true;
float phase = 0.0F;
float windSpeed = 1.0F;
int colourScheme = 0;

enum MenuAction {
    Start = 1,
    Stop,
    Reset,
    SpeedSlow,
    SpeedNormal,
    SpeedFast,
    ColourBlue,
    ColourRed,
    ColourGreen,
    Exit,
};

Point2 bezier(const std::array<Point2, 4>& control, float t) {
    const float u = 1.0F - t;
    const float b0 = u * u * u;
    const float b1 = 3.0F * u * u * t;
    const float b2 = 3.0F * u * t * t;
    const float b3 = t * t * t;
    return {
        b0 * control[0].x + b1 * control[1].x + b2 * control[2].x + b3 * control[3].x,
        b0 * control[0].y + b1 * control[1].y + b2 * control[2].y + b3 * control[3].y,
    };
}

std::array<Point2, 4> makeCurve(float baseY) {
    // P0 never moves, which keeps the cloth attached to the flagpole.
    return {{{-2.45F, baseY},
             {-1.25F, baseY + 0.28F * std::sin(phase)},
             {0.15F, baseY + 0.38F * std::sin(phase + 0.85F * kPi)},
             {1.55F, baseY + 0.30F * std::sin(phase + 1.55F * kPi)}}};
}

std::array<float, 3> currentColour() {
    constexpr std::array<std::array<float, 3>, 3> colours{{
        {{0.10F, 0.45F, 0.95F}},
        {{0.90F, 0.18F, 0.18F}},
        {{0.10F, 0.70F, 0.32F}},
    }};
    return colours[static_cast<std::size_t>(colourScheme)];
}

void drawPole() {
    glColor3f(0.82F, 0.84F, 0.88F);
    glLineWidth(8.0F);
    glBegin(GL_LINES);
    glVertex2f(-2.55F, -2.1F);
    glVertex2f(-2.55F, 2.15F);
    glEnd();

    glColor3f(0.95F, 0.72F, 0.20F);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(-2.55F, 2.25F);
    for (int i = 0; i <= 24; ++i) {
        const float angle = 2.0F * kPi * static_cast<float>(i) / 24.0F;
        glVertex2f(-2.55F + 0.11F * std::cos(angle), 2.25F + 0.11F * std::sin(angle));
    }
    glEnd();
}

void drawFlag() {
    const auto top = makeCurve(1.55F);
    const auto bottom = makeCurve(0.05F);
    const auto colour = currentColour();

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= kSamples; ++i) {
        const float t = static_cast<float>(i) / kSamples;
        const Point2 upper = bezier(top, t);
        const Point2 lower = bezier(bottom, t);
        const float lighting = 0.78F + 0.22F * std::sin(t * 4.0F * kPi + phase);
        glColor3f(colour[0] * lighting, colour[1] * lighting, colour[2] * lighting);
        glVertex2f(upper.x, upper.y);
        glVertex2f(lower.x, lower.y);
    }
    glEnd();

    glColor3f(1.0F, 1.0F, 1.0F);
    glLineWidth(2.0F);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= kSamples; ++i) {
        const Point2 point = bezier(top, static_cast<float>(i) / kSamples);
        glVertex2f(point.x, point.y);
    }
    glEnd();
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= kSamples; ++i) {
        const Point2 point = bezier(bottom, static_cast<float>(i) / kSamples);
        glVertex2f(point.x, point.y);
    }
    glEnd();
}

void drawText(float x, float y, const std::string& text) {
    glRasterPos2f(x, y);
    for (const unsigned char character : text) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, character);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawPole();
    drawFlag();

    glColor3f(1.0F, 1.0F, 1.0F);
    std::ostringstream status;
    status << (animating ? "RUNNING" : "STOPPED") << "   Wind speed: " << windSpeed << "x";
    drawText(-2.85F, -2.45F, status.str());
    drawText(-2.85F, -2.68F, "Right-click for menu | Space start/stop | R reset | Esc exit");
    glutSwapBuffers();
}

void reshape(int width, int height) {
    const int safeHeight = std::max(height, 1);
    glViewport(0, 0, width, safeHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    const double aspect = static_cast<double>(width) / safeHeight;
    if (aspect >= 1.0) {
        gluOrtho2D(-3.0 * aspect, 3.0 * aspect, -3.0, 3.0);
    } else {
        gluOrtho2D(-3.0, 3.0, -3.0 / aspect, 3.0 / aspect);
    }
    glMatrixMode(GL_MODELVIEW);
}

void resetFlag() {
    animating = true;
    phase = 0.0F;
    windSpeed = 1.0F;
    colourScheme = 0;
}

void handleMenu(int action) {
    switch (action) {
    case Start:       animating = true; break;
    case Stop:        animating = false; break;
    case Reset:       resetFlag(); break;
    case SpeedSlow:   windSpeed = 0.5F; break;
    case SpeedNormal: windSpeed = 1.0F; break;
    case SpeedFast:   windSpeed = 2.0F; break;
    case ColourBlue:  colourScheme = 0; break;
    case ColourRed:   colourScheme = 1; break;
    case ColourGreen: colourScheme = 2; break;
    case Exit:        glutLeaveMainLoop(); return;
    default: return;
    }
    glutPostRedisplay();
}

void createMenu() {
    const int speedMenu = glutCreateMenu(handleMenu);
    glutAddMenuEntry("Slow (0.5x)", SpeedSlow);
    glutAddMenuEntry("Normal (1.0x)", SpeedNormal);
    glutAddMenuEntry("Fast (2.0x)", SpeedFast);

    const int colourMenu = glutCreateMenu(handleMenu);
    glutAddMenuEntry("Blue", ColourBlue);
    glutAddMenuEntry("Red", ColourRed);
    glutAddMenuEntry("Green", ColourGreen);

    glutCreateMenu(handleMenu);
    glutAddMenuEntry("Start", Start);
    glutAddMenuEntry("Stop", Stop);
    glutAddMenuEntry("Reset", Reset);
    glutAddSubMenu("Change Wind Speed", speedMenu);
    glutAddSubMenu("Change Colour", colourMenu);
    glutAddMenuEntry("Exit", Exit);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void timer(int) {
    if (animating) {
        phase += 0.045F * windSpeed;
        if (phase > 2.0F * kPi) {
            phase -= 2.0F * kPi;
        }
        glutPostRedisplay();
    }
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
    case ' ':
        animating = !animating;
        break;
    case 'r':
    case 'R':
        resetFlag();
        break;
    case 27:
        glutLeaveMainLoop();
        return;
    default:
        return;
    }
    glutPostRedisplay();
}

} // namespace

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(900, 600);
    glutCreateWindow("Experiment 7 - Animated Bezier Flag");
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

    glClearColor(0.035F, 0.055F, 0.11F, 1.0F);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    createMenu();
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return EXIT_SUCCESS;
}

