// Lab 4 - OpenGL Area Filling Algorithms
// Build (GLUT/OpenGL window):  cl /EHsc AreaFill.cpp freeglut.lib opengl32.lib glu32.lib
// Build (headless, no GL):     cl /EHsc /DHEADLESS AreaFill.cpp   -> writes BMP screenshots + benchmark table
//
// All pixel algorithms work on a CPU framebuffer (fb) so GetPixel/SetPixel are exact and
// portable; the buffer is shown with glDrawPixels. Fills use an explicit stack instead of
// recursion (the recursive form from the handout overflows the call stack on a 600x600 region).
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <chrono>
#include <string>

#ifndef HEADLESS
#include <GL/glut.h>
#endif

typedef uint32_t Color; // 0x00RRGGBB
static const int W = 600, H = 600;
static std::vector<Color> fb(W * H);

static Color rgb(int r, int g, int b) { return (Color)((r << 16) | (g << 8) | b); }
static const Color WHITE = rgb(255, 255, 255), BLACK = rgb(0, 0, 0);

struct Pt { int x, y; };

// ---------- pixel access ----------
static inline bool inside(int x, int y) { return x >= 0 && x < W && y >= 0 && y < H; }
static inline Color getPixel(int x, int y) { return inside(x, y) ? fb[y * W + x] : BLACK; }
static inline void setPixel(int x, int y, Color c) { if (inside(x, y)) fb[y * W + x] = c; }
static void clearBuffer(Color c = WHITE) { std::fill(fb.begin(), fb.end(), c); }

// ---------- shapes ----------
static std::vector<Pt> triangle() { return { {300, 100}, {500, 450}, {100, 450} }; }
static std::vector<Pt> pentagon() { return { {200, 100}, {400, 100}, {470, 300}, {300, 480}, {130, 300} }; }

// Bresenham line (all octants)
static void line(Pt a, Pt b, Color c)
{
	int dx = abs(b.x - a.x), dy = -abs(b.y - a.y);
	int sx = a.x < b.x ? 1 : -1, sy = a.y < b.y ? 1 : -1, err = dx + dy;
	for (;;) {
		setPixel(a.x, a.y, c);
		if (a.x == b.x && a.y == b.y) break;
		int e2 = 2 * err;
		bool stepX = e2 >= dy, stepY = e2 <= dx;
		if (stepX) { err += dy; a.x += sx; }
		if (stepX && stepY) setPixel(a.x, a.y, c); // fill diagonal corner (a.x moved) ...
		if (stepY) { err += dx; a.y += sy; }
		if (stepX && stepY) setPixel(a.x - sx, a.y, c); // ... and the other corner: boundary becomes 4-connected, so 8-connected fill cannot leak
	}
}
static void outline(const std::vector<Pt>& p, Color c)
{
	for (size_t i = 0; i < p.size(); i++) line(p[i], p[(i + 1) % p.size()], c);
}
static Pt centroid(const std::vector<Pt>& p)
{
	long sx = 0, sy = 0;
	for (auto& q : p) { sx += q.x; sy += q.y; }
	return { (int)(sx / p.size()), (int)(sy / p.size()) };
}

// ---------- Exercise 2 / assignment 14, 15: Boundary Fill (4- and 8-connected) ----------
static long boundaryFill(int x, int y, Color fill, Color boundary, bool eight)
{
	static const int dx[8] = { 1, -1, 0, 0, 1, 1, -1, -1 };
	static const int dy[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };
	int n = eight ? 8 : 4;
	long count = 0;
	std::vector<Pt> st;
	st.push_back({ x, y });
	while (!st.empty()) {
		Pt p = st.back(); st.pop_back();
		if (!inside(p.x, p.y)) continue;
		Color cur = getPixel(p.x, p.y);
		if (cur == boundary || cur == fill) continue;
		setPixel(p.x, p.y, fill);
		count++;
		for (int i = 0; i < n; i++) st.push_back({ p.x + dx[i], p.y + dy[i] });
	}
	return count;
}

// ---------- Exercise 3 / assignment 16: Flood Fill (4-connected) ----------
static long floodFill(int x, int y, Color oldC, Color newC)
{
	if (oldC == newC) return 0;
	long count = 0;
	std::vector<Pt> st;
	st.push_back({ x, y });
	while (!st.empty()) {
		Pt p = st.back(); st.pop_back();
		if (!inside(p.x, p.y) || getPixel(p.x, p.y) != oldC) continue;
		setPixel(p.x, p.y, newC);
		count++;
		st.push_back({ p.x + 1, p.y }); st.push_back({ p.x - 1, p.y });
		st.push_back({ p.x, p.y + 1 }); st.push_back({ p.x, p.y - 1 });
	}
	return count;
}

// ---------- Exercise 4 / assignment 17: Scan-Line Polygon Fill ----------
// Even-odd rule. Edges use half-open interval [ymin, ymax) so shared vertices are
// counted once and local extrema are counted zero or two times; horizontals are skipped.
static long scanLineFill(const std::vector<Pt>& poly, Color fill)
{
	int ymin = H, ymax = 0;
	for (auto& p : poly) { ymin = std::min(ymin, p.y); ymax = std::max(ymax, p.y); }
	long count = 0;
	size_t n = poly.size();
	std::vector<int> xs;
	for (int y = ymin; y <= ymax; y++) {
		xs.clear();
		for (size_t i = 0; i < n; i++) {
			Pt a = poly[i], b = poly[(i + 1) % n];
			if (a.y == b.y) continue;
			if (a.y > b.y) std::swap(a, b);
			if (y >= a.y && y < b.y) {
				// x of intersection, rounded to nearest integer
				double x = a.x + (double)(y - a.y) * (b.x - a.x) / (b.y - a.y);
				xs.push_back((int)(x + 0.5));
			}
		}
		std::sort(xs.begin(), xs.end());
		for (size_t i = 0; i + 1 < xs.size(); i += 2)
			for (int x = xs[i]; x <= xs[i + 1]; x++) { setPixel(x, y, fill); count++; }
	}
	return count;
}

// ---------- Algorithm dispatcher ----------
enum Algo { BOUNDARY4, BOUNDARY8, FLOOD, SCANLINE };
static const char* algoName[] = { "Boundary Fill (4)", "Boundary Fill (8)", "Flood Fill (4)", "Scan-Line Fill" };

// Draws the outline in BLACK on white, then fills the interior with 'fill'.
static long runFill(Algo a, const std::vector<Pt>& poly, Color fill)
{
	clearBuffer();
	outline(poly, BLACK);
	Pt s = centroid(poly);
	switch (a) {
	case BOUNDARY4: return boundaryFill(s.x, s.y, fill, BLACK, false);
	case BOUNDARY8: return boundaryFill(s.x, s.y, fill, BLACK, true);
	case FLOOD:     return floodFill(s.x, s.y, WHITE, fill);
	default:        return scanLineFill(poly, fill);
	}
}

// ---------- BMP writer (for screenshots) ----------
static void saveBMP(const std::string& name)
{
	FILE* f = fopen(name.c_str(), "wb");
	if (!f) return;
	uint32_t size = 54 + W * H * 3;
	uint8_t hdr[54] = { 'B', 'M' };
	auto put32 = [&](int o, uint32_t v) { for (int i = 0; i < 4; i++) hdr[o + i] = (v >> (8 * i)) & 255; };
	put32(2, size); put32(10, 54); put32(14, 40); put32(18, W); put32(22, H);
	hdr[26] = 1; hdr[28] = 24;
	fwrite(hdr, 1, 54, f);
	std::vector<uint8_t> row(W * 3);
	for (int y = H - 1; y >= 0; y--) { // fb row 0 = top
		for (int x = 0; x < W; x++) {
			Color c = fb[y * W + x];
			row[x * 3] = c & 255; row[x * 3 + 1] = (c >> 8) & 255; row[x * 3 + 2] = (c >> 16) & 255;
		}
		fwrite(row.data(), 1, row.size(), f);
	}
	fclose(f);
}

// ---------- Benchmark + screenshots (assignment 19, 18) ----------
static const Color PALETTE[] = { rgb(0, 0, 255), rgb(255, 0, 0), rgb(0, 160, 0), rgb(255, 140, 0), rgb(160, 32, 240) };
static const int NPAL = 5;

static void benchmark()
{
	struct S { const char* name; std::vector<Pt> poly; } shapes[] = { { "triangle", triangle() }, { "pentagon", pentagon() } };
	printf("%-10s %-20s %10s %12s\n", "Shape", "Algorithm", "Pixels", "Avg ms (20x)");
	int pi = 0;
	for (auto& s : shapes)
		for (int a = 0; a < 4; a++, pi++) {
			Color fill = PALETTE[pi % NPAL];
			long px = 0;
			auto t0 = std::chrono::high_resolution_clock::now();
			for (int r = 0; r < 20; r++) px = runFill((Algo)a, s.poly, fill);
			auto t1 = std::chrono::high_resolution_clock::now();
			double ms = std::chrono::duration<double, std::milli>(t1 - t0).count() / 20.0;
			printf("%-10s %-20s %10ld %12.3f\n", s.name, algoName[a], px, ms);
			std::string fn = std::string(s.name) + "_" + std::to_string(a) + ".bmp";
			saveBMP(fn);
		}

	// Question 23: flood fill over a multi-colour region only fills the connected same-colour part.
	clearBuffer();
	outline(pentagon(), BLACK);
	line({ 130, 300 }, { 470, 300 }, rgb(255, 0, 0)); // red divider splits the interior
	Pt s = { 300, 200 };
	floodFill(s.x, s.y, WHITE, rgb(0, 0, 255));
	saveBMP("flood_multicolour.bmp");
	printf("Saved flood_multicolour.bmp (flood fill stops at the red divider)\n");
}

#ifdef HEADLESS
int main() { benchmark(); return 0; }
#else
// ---------- GLUT front end ----------
static int mode = 0;         // 0 = GL_POLYGON demo, otherwise Algo+1
static bool usePentagon = true;
static int palIdx = 0;
static double lastMs = 0;
static long lastPx = 0;

static void drawGLPolygon() // Exercise 1: GL_POLYGON in normalised coordinates
{
	glClear(GL_COLOR_BUFFER_BIT);
	Color c = PALETTE[palIdx];
	glColor3ub((c >> 16) & 255, (c >> 8) & 255, c & 255);
	glBegin(GL_POLYGON);
	if (usePentagon) {
		glVertex2f(-0.5f, -0.5f); glVertex2f(0.5f, -0.5f); glVertex2f(0.7f, 0.2f);
		glVertex2f(0.0f, 0.7f);   glVertex2f(-0.7f, 0.2f);
	} else {
		glVertex2f(-0.65f, -0.55f); glVertex2f(0.65f, -0.55f); glVertex2f(0.0f, 0.65f);
	}
	glEnd();
}

static void title()
{
	char t[160];
	if (mode == 0) snprintf(t, sizeof t, "GL_POLYGON | %s | keys: 0-4 mode, t/p shape, c colour",
						  usePentagon ? "pentagon" : "triangle");
	else snprintf(t, sizeof t, "%s | %s | %ld px | %.3f ms | keys: 0-4 mode, t/p shape, c colour",
				  algoName[mode - 1], usePentagon ? "pentagon" : "triangle", lastPx, lastMs);
	glutSetWindowTitle(t);
}

static void display()
{
	if (mode == 0) {
		glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(-1, 1, -1, 1);
		drawGLPolygon();
	} else {
		glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(0, W, 0, H);
		auto poly = usePentagon ? pentagon() : triangle();
		auto t0 = std::chrono::high_resolution_clock::now();
		lastPx = runFill((Algo)(mode - 1), poly, PALETTE[palIdx]);
		auto t1 = std::chrono::high_resolution_clock::now();
		lastMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
		glClear(GL_COLOR_BUFFER_BIT);
		// fb row 0 is the top; flip by drawing from the top-left and using negative zoom
		glRasterPos2i(0, H);
		glPixelZoom(1.0f, -1.0f);
		std::vector<uint8_t> rgbBuf(W * H * 3);
		for (int i = 0; i < W * H; i++) {
			rgbBuf[i * 3] = (fb[i] >> 16) & 255; rgbBuf[i * 3 + 1] = (fb[i] >> 8) & 255; rgbBuf[i * 3 + 2] = fb[i] & 255;
		}
		glDrawPixels(W, H, GL_RGB, GL_UNSIGNED_BYTE, rgbBuf.data());
	}
	glFlush();
	title();
}

static void keyboard(unsigned char k, int, int)
{
	if (k >= '0' && k <= '4') mode = k - '0';
	else if (k == 't') usePentagon = false;
	else if (k == 'p') usePentagon = true;
	else if (k == 'c') palIdx = (palIdx + 1) % NPAL;
	else if (k == 'b') { benchmark(); return; }
	else if (k == 27) exit(0);
	glutPostRedisplay();
}

static void saveWindowBMP(const std::string& name)
{
	std::vector<uint8_t> px(W * H * 3);
	glReadBuffer(GL_FRONT);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, W, H, GL_RGB, GL_UNSIGNED_BYTE, px.data());
	for (int i = 0; i < W * H; i++) std::swap(px[i * 3], px[i * 3 + 2]); // RGB -> BGR

	FILE* f = fopen(name.c_str(), "wb");
	if (!f) return;
	uint8_t hdr[54] = { 'B', 'M' };
	auto put32 = [&](int o, uint32_t v) { for (int i = 0; i < 4; i++) hdr[o + i] = (v >> (8 * i)) & 255; };
	put32(2, 54 + W * H * 3); put32(10, 54); put32(14, 40); put32(18, W); put32(22, H);
	hdr[26] = 1; hdr[28] = 24;
	fwrite(hdr, 1, 54, f);
	fwrite(px.data(), 1, px.size(), f);
	fclose(f);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(W, H);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Polygon Filling");
	glClearColor(1, 1, 1, 1);
	if (argc > 1 && std::string(argv[1]) == "--shots") { // save real window screenshots, then exit
		for (int m = 0; m <= 4; m++) {
			mode = m; usePentagon = true; palIdx = m % NPAL;
			display();
			saveWindowBMP("window_mode" + std::to_string(m) + ".bmp");
		}
		mode = 0; usePentagon = false; palIdx = 0;
		display();
		saveWindowBMP("window_triangle.bmp");
		return 0;
	}
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	printf("0 GL_POLYGON | 1 Boundary4 | 2 Boundary8 | 3 Flood | 4 Scan-line | t/p shape | c colour | b benchmark | Esc quit\n");
	glutMainLoop();
	return 0;
}
#endif
