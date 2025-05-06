#include <GL/glut.h>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <iostream>

#define LEFT 0
#define RIGHT 1
#define BOTTOM 2
#define TOP 3
using namespace std;

float xmin, ymin, xmax, ymax;
bool windowSet = false;
bool clippedFlag = false;
int clickCount = 0;

struct Point {
    float x, y;
};

vector<Point> polygon;
vector<Point> clipped;
vector<Point> windowPoints;

void drawPolygon(const vector<Point>& poly) {
    if (poly.size() < 3) return;
    glBegin(GL_LINE_LOOP);
    for (auto p : poly)
        glVertex2f(p.x, p.y);
    glEnd();
}

Point intersect(Point a, Point b, int edge) {
    Point p;
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float m = (dx != 0) ? dy / dx : 0;

    switch (edge) {
    case LEFT:
        p.x = xmin;
        p.y = a.y + m * (xmin - a.x);
        break;
    case RIGHT:
        p.x = xmax;
        p.y = a.y + m * (xmax - a.x);
        break;
    case BOTTOM:
        p.y = ymin;
        p.x = (m != 0) ? a.x + (ymin - a.y) / m : a.x;
        break;
    case TOP:
        p.y = ymax;
        p.x = (m != 0) ? a.x + (ymax - a.y) / m : a.x;
        break;
    }
    return p;
}

bool inside(Point p, int edge) {
    switch (edge) {
    case LEFT: return p.x >= xmin;
    case RIGHT: return p.x <= xmax;
    case BOTTOM: return p.y >= ymin;
    case TOP: return p.y <= ymax;
    }
    return true;
}

void clipEdge(int edge) {
    vector<Point> input = clipped;
    clipped.clear();
    for (size_t i = 0; i < input.size(); i++) {
        Point curr = input[i];
        Point prev = input[(i + input.size() - 1) % input.size()];
        bool currIn = inside(curr, edge);
        bool prevIn = inside(prev, edge);

        if (prevIn && currIn) {
            clipped.push_back(curr);
        } else if (prevIn && !currIn) {
            clipped.push_back(intersect(prev, curr, edge));
        } else if (!prevIn && currIn) {
            clipped.push_back(intersect(prev, curr, edge));
            clipped.push_back(curr);
        }
    }
}

void suthHodgman() {
    clipped = polygon;
    clipEdge(LEFT);
    clipEdge(RIGHT);
    clipEdge(BOTTOM);
    clipEdge(TOP);
    printf("Clipped polygon has %zu vertices\n", clipped.size()); 
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glLineWidth(3.0f);
    if (windowSet) {
        glColor3f(1, 0, 0);
        glBegin(GL_LINE_LOOP);
        glVertex2f(xmin, ymin);
        glVertex2f(xmax, ymin);
        glVertex2f(xmax, ymax);
        glVertex2f(xmin, ymax);
        glEnd();
    }

    if (!clippedFlag) {
        glColor3f(0, 0, 1);
        drawPolygon(polygon);
    } else {
        glColor3f(0, 1, 0);
        drawPolygon(clipped);
    }

    

    glFlush();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        Point p;
        int winHeight = glutGet(GLUT_WINDOW_HEIGHT);
        p.x = x - 320;
        p.y = (winHeight - y) - 240;

        if (clickCount < 2) {
            windowPoints.push_back(p);
            clickCount++;
            if (clickCount == 2) {
                xmin = min(windowPoints[0].x, windowPoints[1].x);
                xmax = max(windowPoints[0].x, windowPoints[1].x);
                ymin = min(windowPoints[0].y, windowPoints[1].y);
                ymax = max(windowPoints[0].y, windowPoints[1].y);
                windowSet = true;
                printf("Window set: xmin=%.1f, ymin=%.1f, xmax=%.1f, ymax=%.1f\n", xmin, ymin, xmax, ymax);
            }
        } else if (!clippedFlag && windowSet) {
            polygon.push_back(p);
        }

        glutPostRedisplay();
    }
}

void menu(int choice) {
    switch (choice) {
    case 1:
        if (polygon.size() >= 3 && windowSet) {
            suthHodgman();
            clippedFlag = true;
            glutPostRedisplay();
        } else {
            printf("Set window and add at least 3 polygon vertices.\n");
        }
        break;
    case 2:
        polygon.clear();
        clipped.clear();
        windowPoints.clear();
        clippedFlag = false;
        windowSet = false;
        clickCount = 0;
        glutPostRedisplay();
        break;
    case 3:
        exit(0);
    }
}

void init() {
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-320, 320, -240, 240);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    cout << "Instructions:\n";
    cout << " - Click 2 points to define clipping window (opposite corners).\n";
    cout << " - Then click to define polygon vertices (min 3).\n";
    cout << " - Right-click to open menu for actions.\n";

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Click-to-Set Clipping Window and Polygon");

    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);

    // Menu setup
    int submenu = glutCreateMenu(menu);
    glutAddMenuEntry("Clip the polygon", 1);
    glutAddMenuEntry("Reset", 2);
    glutAddMenuEntry("Exit", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutMainLoop();
    return 0;
}
