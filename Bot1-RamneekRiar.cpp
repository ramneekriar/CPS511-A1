/*******************************************************************
           Hierarchical Multi-Part Model Example
********************************************************************/
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <glut/glut.h>
#else
#include <windows.h>
#include <gl/glut.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <utility>
#include <vector>
#include "VECTOR3D.h"
//#include "cube.h"
#include "QuadMesh.h"

const int vWidth  = 650;    // Viewport width in pixels
const int vHeight = 500;    // Viewport height in pixels

// Note how everything depends on robot body dimensions so that can scale entire robot proportionately
// just by changing robot body scale
float robotBodyWidth = 10.0;
float robotBodyLength = 2.0;
float robotBodyDepth = 6.0;

float headWidth = robotBodyWidth;
float headLength = robotBodyLength*2;
float headDepth = robotBodyDepth;

float cannonHeight = robotBodyWidth*0.7;
float cannonRadius = robotBodyLength / 4.0;

float upperLegLength = robotBodyLength*2.0;
float upperLegWidth = 0.15*robotBodyWidth;

float lowerLegLength = upperLegLength*1.2;
float lowerLegWidth = upperLegWidth*0.9;

float clawWidth = 0.5*lowerLegWidth;
float clawLength = 0.3*lowerLegLength;

char curJoint = 'b';

// Control angles
float robotAngle = 0.0;
float hipAngle = 0.0;
float leftKneeAngle = 0.0;

float upperLegAngle = 30.0;
float lowerLegAngle = -60.0;
float cannonAngle = 10.0;

// Lighting/shading and material properties for robot - upcoming lecture - just copy for now
// Robot RGBA material properties (NOTE: we will learn about this later in the semester)
GLfloat robotBody_mat_ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat robotBody_mat_specular[] = { 0.45f,0.55f,0.45f,1.0f };
GLfloat robotBody_mat_diffuse[] = { 0.1f,0.35f,0.1f,1.0f };
GLfloat robotBody_mat_shininess[] = { 32.0F };

GLfloat robotLeg_mat_ambient[] = { 0.0215f, 0.1745f, 0.0215f, 0.55f };
GLfloat robotLeg_mat_diffuse[] = { 0.5f,0.0f,0.0f,1.0f };
GLfloat robotLeg_mat_specular[] = { 0.7f, 0.6f, 0.6f, 1.0f };
GLfloat robotLeg_mat_shininess[] = { 32.0F };

GLfloat gun_mat_ambient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat gun_mat_diffuse[] = { 0.01f,0.0f,0.01f,0.01f };
GLfloat gun_mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat gun_mat_shininess[] = { 100.0F };

GLfloat robotLowerBody_mat_ambient[] = { 0.25f, 0.25f, 0.25f, 1.0f };
GLfloat robotLowerBody_mat_diffuse[] = { 0.4f, 0.4f, 0.4f, 1.0f };
GLfloat robotLowerBody_mat_specular[] = { 0.774597f, 0.774597f, 0.774597f, 1.0f };
GLfloat robotLowerBody_mat_shininess[] = { 76.8F };

// Light properties
GLfloat light_position0[] = { -4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_position1[] = { 4.0F, 8.0F, 8.0F, 1.0F };
GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat light_ambient[] = { 0.2F, 0.2F, 0.2F, 1.0F };


// Mouse button
int currentButton;

// A flat open mesh
QuadMesh *groundMesh = NULL;

// Structure defining a bounding box, currently unused
typedef struct BoundingBox {
    VECTOR3D min;
    VECTOR3D max;
} BBox;

// Default Mesh Size
int meshSize = 16;

// Prototypes for functions in this module
void initOpenGL(int w, int h);
void display(void);
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void mouseMotionHandler(int xMouse, int yMouse);
void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
void cannonAnimationHandler(int param);
void drawRobot();
void drawBody();
void drawHead();
void drawCannon();
void drawLowerBody();
//void drawLeftLeg();
//void drawRightLeg();
void drawLeftUpperLeg();
void drawRightUpperLeg();
void drawLeftLowerLeg();
void drawRightLowerLeg();
void drawLeftFoot();
void drawRightFoot();

int main(int argc, char **argv)
{
    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(vWidth, vHeight);
    glutInitWindowPosition(200, 30);
    glutCreateWindow("Bot 1 - Ramneek Riar");

    // Initialize GL
    initOpenGL(vWidth, vHeight);

    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotionHandler);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(functionKeys);

    // Start event loop, never returns
    glutMainLoop();

    return 0;
}


// Set up OpenGL. For viewport and projection setup see reshape().
void initOpenGL(int w, int h)
{
    // Set up and enable lighting
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);   // This second light is currently off

    // Other OpenGL setup
    glEnable(GL_DEPTH_TEST);   // Remove hidded surfaces
    glShadeModel(GL_SMOOTH);   // Use smooth shading, makes boundaries between polygons harder to see
    glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear
    glClearDepth(1.0f);
    glEnable(GL_NORMALIZE);    // Renormalize normal vectors
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);   // Nicer perspective

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    // Other initializatuion
    // Set up ground quad mesh
    VECTOR3D origin = VECTOR3D(-16.0f, 0.0f, 16.0f);
    VECTOR3D dir1v = VECTOR3D(1.0f, 0.0f, 0.0f);
    VECTOR3D dir2v = VECTOR3D(0.0f, 0.0f, -1.0f);
    groundMesh = new QuadMesh(meshSize, 32.0);
    groundMesh->InitMesh(meshSize, origin, 32.0, 32.0, dir1v, dir2v);

    VECTOR3D ambient = VECTOR3D(0.0f, 0.05f, 0.0f);
    VECTOR3D diffuse = VECTOR3D(0.4f, 0.8f, 0.4f);
    VECTOR3D specular = VECTOR3D(0.04f, 0.04f, 0.04f);
    float shininess = 0.2;
    groundMesh->SetMaterial(ambient, diffuse, specular, shininess);

}


// Callback, called whenever GLUT determines that the window should be redisplayed
// or glutPostRedisplay() has been called.
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    // Create Viewing Matrix V
    // Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
    gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // Draw Robot

    // Apply modelling transformations M to move robot
    // Current transformation matrix is set to IV, where I is identity matrix
    // CTM = IV
    drawRobot();

    // Draw ground
    glPushMatrix();
    glTranslatef(0.0, -20.0, 0.0);
    groundMesh->DrawMesh(meshSize);
    glPopMatrix();

    glutSwapBuffers();   // Double buffering, swap buffers
}

void drawRobot()
{
    glPushMatrix();
    glRotatef(robotAngle, 0.0, 1.0, 0.0); // spin robot on base.
        
    glPushMatrix();
    glRotatef(hipAngle, 0.0, 1.0, 0.0);  // spin robot on hip.
    drawBody();
    drawHead();
    drawCannon();
    glPopMatrix();
    
    glPopMatrix();
    
    // don't want to spin fixed base in this example
    glPushMatrix();
     //spin robot
     glRotatef(robotAngle, 0.0, 1.0, 0.0);
    drawLowerBody();
    
    drawLeftUpperLeg();
    glPushMatrix();
    glTranslatef(0.25*robotBodyWidth + -0.25*upperLegWidth, -0.79*robotBodyWidth, -0.055*robotBodyWidth);
    glRotatef(leftKneeAngle, 1.0, 0.0, 0.0);
    glTranslatef(-(0.25*robotBodyWidth + -0.25*upperLegWidth), -(-0.79*robotBodyWidth), -(-0.055*robotBodyWidth));
    drawLeftLowerLeg();
    drawLeftFoot();
    glPopMatrix();
    
    drawRightUpperLeg();
    drawRightLowerLeg();
    drawRightFoot();
    
    glPopMatrix();
    glPopMatrix();
}


void drawBody()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

    glPushMatrix();
    glScalef(robotBodyWidth, robotBodyLength, robotBodyDepth);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawHead()
{
    // Set robot material properties per body part. Can have seperate material properties for each part
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotBody_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotBody_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotBody_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotBody_mat_shininess);

    glPushMatrix();
    // Position head with respect to parent (body)
    glTranslatef(0, 0.5*robotBodyLength+0.5*headLength, 0); // this will be done last
    
    // Build Head
    glPushMatrix();
    glScalef(0.8*robotBodyWidth, 0.6*robotBodyWidth, 0.6*robotBodyWidth);
    glutSolidCube(1.0);
    glPopMatrix();

    glPopMatrix();
}

void drawCannon()
{
    // Set robot material properties per body part. Can have seperate material properties for each part
    glMaterialfv(GL_FRONT, GL_AMBIENT, gun_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, gun_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, gun_mat_shininess);
    
    glPushMatrix();
    glTranslatef(0, 0.05*robotBodyLength, 0.1*robotBodyWidth);
    glRotatef(cannonAngle, 0.0, 0.0, 1.0);
    glTranslatef(0, -(0.05*robotBodyLength), -(0.1*robotBodyWidth));
    
    glPushMatrix();
    //Position cannon with respect to parent (body)
    glTranslatef(0, 0.05*robotBodyLength, 0.1*robotBodyWidth);
    
    glPushMatrix();
    // Creating cylinder object for cannon
    GLUquadricObj *myCannon;
    myCannon = gluNewQuadric();
    gluQuadricDrawStyle(myCannon, GLU_LINE);
    gluCylinder(myCannon, cannonRadius, cannonRadius, cannonHeight, 100, 100);
    
    glPushMatrix();
    glTranslatef(0, 0.2*robotBodyLength, 0.68*robotBodyWidth);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    glTranslatef(0, -(0.2*robotBodyLength), -(0.68*robotBodyWidth));
    
    glPushMatrix();
    //Position cannon with respect to parent (body)
    glTranslatef(0, 0.2*robotBodyLength, 0.68*robotBodyWidth);
    
    glPushMatrix();
    GLUquadricObj *myCannon_subPart;
    myCannon_subPart = gluNewQuadric();
    gluQuadricDrawStyle(myCannon_subPart, GLU_LINE);
    gluCylinder(myCannon_subPart, 0.4*cannonRadius, 0.4*cannonRadius, 0.1*cannonHeight, 100, 100);
    
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void drawLowerBody()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);
    
    glPushMatrix();
    glRotatef(90.0, 0.0, 1.0, 0.0);
    
    glPushMatrix();
    // Position lower cylindrical join in respect to body
    glTranslatef(0.0, -1.5*robotBodyLength, -0.15*robotBodyWidth); // this will be done last
    
    glPushMatrix();
    // Creating cylinder object for lower body
    gluCylinder(gluNewQuadric(), 0.2*robotBodyWidth, 0.2*robotBodyWidth, 0.5*robotBodyDepth, 100, 100);
    
    glPopMatrix();
    glPopMatrix();
    
    glPushMatrix();
    // Position disk object to close lower body cylinder left side
    glTranslatef(0.0, -1.5*robotBodyLength, 0.01*robotBodyWidth); // this will be done last
    
    glPushMatrix();
    // Creating disk object to close lower body cylinder sides
    GLUquadricObj *rightCylinderDisk;
    rightCylinderDisk = gluNewQuadric();
    gluQuadricDrawStyle(rightCylinderDisk, GLU_LINE);
    gluDisk(rightCylinderDisk, 0.0, 0.19*robotBodyWidth, 100, 100);
    
    glPopMatrix();
    glPopMatrix();
    
    glPushMatrix();
    // Position disk object to close lower body cylinder right side
    glTranslatef(0.0, -1.5*robotBodyLength, 0.15*robotBodyWidth); // this will be done last

    glPushMatrix();
    // Creating disk object to close lower body cylinder sides
    GLUquadricObj *leftCylinderDisk;
    leftCylinderDisk = gluNewQuadric();
    gluQuadricDrawStyle(leftCylinderDisk, GLU_LINE);
    gluDisk(leftCylinderDisk, 0.0, 0.19*robotBodyWidth, 100, 100);
    
    glPopMatrix();
    glPopMatrix();
    
    glPopMatrix();
}

void drawLeftUpperLeg()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);
    
    glPushMatrix();
    glTranslatef(0.25*robotBodyWidth + -0.25*upperLegWidth, -0.5*robotBodyWidth, -0.075*robotBodyWidth);
    glRotatef(upperLegAngle, 1.0, 0.0, 0.0);
    glTranslatef(-(0.25*robotBodyWidth + -0.25*upperLegWidth), -(-0.5*robotBodyWidth), -(-0.075*robotBodyWidth));
    
    glPushMatrix();
    // Position upper leg with respect to parent body
    glTranslatef(0.25*robotBodyWidth + -0.25*upperLegWidth, -0.5*robotBodyWidth, -0.075*robotBodyWidth);

    // build upper leg
    glPushMatrix();
    glScalef(upperLegWidth, upperLegLength, upperLegWidth);
    glutSolidCube(1.0);
    
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void drawRightUpperLeg()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);
    
    glPushMatrix();
    glTranslatef(-(0.25*robotBodyWidth + -0.25*upperLegWidth), -0.5*robotBodyWidth, -0.075*robotBodyWidth);
    glRotatef(upperLegAngle, 1.0, 0.0, 0.0);
    glTranslatef((0.25*robotBodyWidth + -0.25*upperLegWidth), 0.5*robotBodyWidth, 0.075*robotBodyWidth);
    
    glPushMatrix();
    // Position upper leg with respect to parent body
    glTranslatef(-(0.25*robotBodyWidth + -0.25*upperLegWidth), -0.5*robotBodyWidth, -0.075*robotBodyWidth);
    
    // build upper leg
    glPushMatrix();
    glScalef(upperLegWidth, upperLegLength, upperLegWidth);
    glutSolidCube(1.0);
    
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void drawLeftLowerLeg()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);
    
    glPushMatrix();
    glTranslatef(0.25*robotBodyWidth + -0.25*upperLegWidth, -0.79*robotBodyWidth, -0.055*robotBodyWidth);
    glRotatef(lowerLegAngle, 1.0, 0.0, 0.0);
    glTranslatef(-(0.25*robotBodyWidth + -0.25*upperLegWidth), -(-0.79*robotBodyWidth), -(-0.055*robotBodyWidth));
    
    glPushMatrix();
    // Position lower leg with respect to parent body
    glTranslatef(0.25*robotBodyWidth + -0.25*upperLegWidth, -0.79*robotBodyWidth, -0.055*robotBodyWidth);

    // build lower leg
    glPushMatrix();
    glScalef(lowerLegWidth, lowerLegLength, lowerLegWidth);
    glutSolidCube(1.0);
    
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void drawRightLowerLeg()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotLeg_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotLeg_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLeg_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotLeg_mat_shininess);
    
    glPushMatrix();
    glTranslatef(-(0.25*robotBodyWidth + -0.25*upperLegWidth), -0.79*robotBodyWidth, -0.055*robotBodyWidth);
    glRotatef(lowerLegAngle, 1.0, 0.0, 0.0);
    glTranslatef((0.25*robotBodyWidth + -0.25*upperLegWidth), 0.79*robotBodyWidth, 0.055*robotBodyWidth);
    
    glPushMatrix();
    // Position lower leg with respect to parent body
    glTranslatef(-(0.25*robotBodyWidth + -0.25*upperLegWidth), -0.79*robotBodyWidth, -0.055*robotBodyWidth);

    // build lower leg
    glPushMatrix();
    glScalef(lowerLegWidth, lowerLegLength, lowerLegWidth);
    glutSolidCube(1.0);
    
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void drawLeftFoot()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);
    
    glPushMatrix();
    glRotatef(90.0, 0.0, 1.0, 0.0);
    
    glPushMatrix();
    // Position lower cylindrical join in respect to body
    glTranslatef(-1.5*lowerLegWidth, -4.1*robotBodyLength, lowerLegWidth); // this will be done last
    
    glPushMatrix();
    // Creating cylinder object for lower body
    gluCylinder(gluNewQuadric(), 0.3*lowerLegWidth, 0.3*lowerLegWidth, 1.1*lowerLegWidth, 100, 100);
    
    glPopMatrix();
    glPopMatrix();
    
    glPushMatrix();
    // Position disk object to close lower body cylinder left side
    glTranslatef(-1.5*lowerLegWidth, -4.1*robotBodyLength, -(-2.1*lowerLegWidth)); // this will be done last

    glPushMatrix();
    // Creating disk object to close lower body cylinder sides
    GLUquadricObj *leftFoot_rightDisk;
    leftFoot_rightDisk = gluNewQuadric();
    gluQuadricDrawStyle(leftFoot_rightDisk, GLU_LINE);
    gluDisk(leftFoot_rightDisk, 0.0, 0.29*lowerLegWidth, 100, 100);
    
    glPopMatrix();
    glPopMatrix();

    glPushMatrix();
    // Position disk object to close lower body cylinder right side
    glTranslatef(-1.5*lowerLegWidth, -4.1*robotBodyLength, -(-1.1*lowerLegWidth)); // this will be done last

    glPushMatrix();
    // Creating disk object to close lower body cylinder sides
    GLUquadricObj *leftFoot_leftDisk;
    leftFoot_leftDisk = gluNewQuadric();
    gluQuadricDrawStyle(leftFoot_leftDisk, GLU_LINE);
    gluDisk(leftFoot_leftDisk, 0.0, 0.29*lowerLegWidth, 100, 100);

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    
    // FOOT
    glPushMatrix();
    // Position upper leg with respect to parent body
    glTranslatef(-(-1.55*lowerLegWidth), -5.0*robotBodyLength, 0.6*lowerLegWidth); // this will be done last

    // build upper leg
    glPushMatrix();
    glScalef(lowerLegWidth, 0.3*lowerLegLength, lowerLegWidth);
    glutSolidCube(1.0);

    glPopMatrix();
    glPopMatrix();
    
    // Front Claw
    glPushMatrix();
    glTranslatef(1.55*lowerLegWidth, -5.3*robotBodyLength, 1.3*lowerLegWidth);
    
    glPushMatrix();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    
    glPushMatrix();
    glScalef(clawWidth, clawLength, clawWidth);
    glutSolidCube(1.0);

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    
    // Left Claw
    glPushMatrix();
    glTranslatef(2.2*lowerLegWidth, -5.3*robotBodyLength, 0.6*lowerLegWidth);
    
    glPushMatrix();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glPushMatrix();
    glRotatef(90.0, 0.0, 0.0, 1.0);
    
    glPushMatrix();
    glScalef(clawWidth, clawLength, clawWidth);
    glutSolidCube(1.0);

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    
    // Right Claw
    glPushMatrix();
    glTranslatef(0.9*lowerLegWidth, -5.3*robotBodyLength, 0.6*lowerLegWidth);
    
    glPushMatrix();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glPushMatrix();
    glRotatef(90.0, 0.0, 0.0, 1.0);
    
    glPushMatrix();
    glScalef(clawWidth, clawLength, clawWidth);
    glutSolidCube(1.0);

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

void drawRightFoot()
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, robotLowerBody_mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, robotLowerBody_mat_specular);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, robotLowerBody_mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SHININESS, robotLowerBody_mat_shininess);
    
    glPushMatrix();
    glRotatef(90.0, 0.0, 1.0, 0.0);
    
    glPushMatrix();
    // Position lower cylindrical join in respect to body
    glTranslatef(-1.5*lowerLegWidth, -4.1*robotBodyLength, -2.1*lowerLegWidth); // this will be done last
    
    glPushMatrix();
    // Creating cylinder object for lower body
    gluCylinder(gluNewQuadric(), 0.3*lowerLegWidth, 0.3*lowerLegWidth, 1.1*lowerLegWidth, 100, 100);
    
    glPopMatrix();
    glPopMatrix();
    
    glPushMatrix();
    // Position disk object to close lower body cylinder left side
    glTranslatef(-1.5*lowerLegWidth, -4.1*robotBodyLength, -2.1*lowerLegWidth); // this will be done last

    glPushMatrix();
    // Creating disk object to close lower body cylinder sides
    GLUquadricObj *rightFoot_rightDisk;
    rightFoot_rightDisk = gluNewQuadric();
    gluQuadricDrawStyle(rightFoot_rightDisk, GLU_LINE);
    gluDisk(rightFoot_rightDisk, 0.0, 0.29*lowerLegWidth, 100, 100);
    
    glPopMatrix();
    glPopMatrix();

    glPushMatrix();
    // Position disk object to close lower body cylinder right side
    glTranslatef(-1.5*lowerLegWidth, -4.1*robotBodyLength, -1.1*lowerLegWidth); // this will be done last

    glPushMatrix();
    // Creating disk object to close lower body cylinder sides
    GLUquadricObj *rightFoot_leftDisk;
    rightFoot_leftDisk = gluNewQuadric();
    gluQuadricDrawStyle(rightFoot_leftDisk, GLU_LINE);
    gluDisk(rightFoot_leftDisk, 0.0, 0.29*lowerLegWidth, 100, 100);

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    
    // FOOT
    glPushMatrix();
    // Position upper leg with respect to parent body
    glTranslatef(-1.55*lowerLegWidth, -5.0*robotBodyLength, 0.6*lowerLegWidth); // this will be done last

    // build upper leg
    glPushMatrix();
    glScalef(lowerLegWidth, 0.3*lowerLegLength, lowerLegWidth);
    glutSolidCube(1.0);
    
    glPopMatrix();
    glPopMatrix();
    
    // Front Claw
    glPushMatrix();
    glTranslatef(-1.55*lowerLegWidth, -5.3*robotBodyLength, 1.3*lowerLegWidth);
    
    glPushMatrix();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    
    glPushMatrix();
    glScalef(clawWidth, clawLength, clawWidth);
    glutSolidCube(1.0);

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    
    // Right Claw
    glPushMatrix();
    glTranslatef(-2.2*lowerLegWidth, -5.3*robotBodyLength, 0.6*lowerLegWidth);
    
    glPushMatrix();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glPushMatrix();
    glRotatef(90.0, 0.0, 0.0, 1.0);
    
    glPushMatrix();
    glScalef(clawWidth, clawLength, clawWidth);
    glutSolidCube(1.0);

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    
    // Left Claw
    glPushMatrix();
    glTranslatef(-0.9*lowerLegWidth, -5.3*robotBodyLength, 0.6*lowerLegWidth);
    
    glPushMatrix();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    glPushMatrix();
    glRotatef(90.0, 0.0, 0.0, 1.0);
    
    glPushMatrix();
    glScalef(clawWidth, clawLength, clawWidth);
    glutSolidCube(1.0);

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
    glPopMatrix();
}

// Callback, called at initialization and whenever user resizes the window.
void reshape(int w, int h)
{
    // Set up viewport, projection, then change to modelview matrix mode -
    // display function will then set up camera and do modeling transforms.
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLdouble)w / h, 0.2, 40.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set up the camera at position (0, 6, 22) looking at the origin, up along positive y axis
    gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

bool stop = false;

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'b':
            curJoint = 'b';
        break;
    case 'h':
            curJoint = 'h';
        break;
    case 'k':
            curJoint = 'k';
        break;
    case 'c':
        glutTimerFunc(10, cannonAnimationHandler, 0);
        break;
    case 'C':
        stop = true;
        break;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}


void cannonAnimationHandler(int param)
{
    if (!stop)
    {
        cannonAngle += 5.0;
        glutPostRedisplay();
        glutTimerFunc(10, cannonAnimationHandler, 0);
    }
}



// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_LEFT:
            if (curJoint == 'b')
            {
                robotAngle -= 2.0;
            }
            else if (curJoint == 'h')
            {
                hipAngle -= 2.0;
            }
            else if (curJoint == 'k')
            {
                leftKneeAngle -= 2.0;
            }
            break;
        case GLUT_KEY_RIGHT:
            if (curJoint == 'b')
            {
                robotAngle += 2.0;
            }
            else if (curJoint == 'h')
            {
                hipAngle += 2.0;
            }
            else if (curJoint == 'k')
            {
                leftKneeAngle += 2.0;
            }
            break;
    }
//    // Help key
//    if (key == GLUT_KEY_LEFT)
//    {
//
//    }
//    // Do transformations with arrow keys
//    //else if (...)   // GLUT_KEY_DOWN, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_LEFT
//    //{
//    //}

    glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse button callback - use only if you want to
void mouse(int button, int state, int x, int y)
{
    currentButton = button;

    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        {
            ;

        }
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
        {
            ;
        }
        break;
    default:
        break;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}


// Mouse motion callback - use only if you want to
void mouseMotionHandler(int xMouse, int yMouse)
{
    if (currentButton == GLUT_LEFT_BUTTON)
    {
        ;
    }

    glutPostRedisplay();   // Trigger a window redisplay
}

