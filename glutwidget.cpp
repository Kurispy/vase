#include <GL/glew.h>
#include <GL/gl.h> 
#include <GL/glu.h>
#include <GL/glut.h>

#include "bitmap.h"
#include "glutwidget.hpp"
#include "shader_utils.hpp"
#include <iostream>
#include <cmath>
#include <ctime>
#include <algorithm>

#define PI 3.1415926

int glutWidget::m_pos_attribute_location;
unsigned int glutWidget::m_vertexbuffer;
unsigned int glutWidget::m_frame;
unsigned int glutWidget::m_program;
unsigned int glutWidget::m_vertexsh;
unsigned int glutWidget::m_fragmentsh;
int glutWidget::x0 = 0;
int glutWidget::y0 = 0;
float glutWidget::rotx = 0;
float glutWidget::roty = 0;
float glutWidget::rotz = 0;
float glutWidget::cposx = 0;
float glutWidget::cposy = 0;
float glutWidget::cposz = 5;
float glutWidget::zoom = 10;

static int degree = 20;
static float t;

static unsigned int m_texture;

GLfloat M[16];

GLfloat cpoints[][3] = {
        {2.5f, 3.5f, 0.0f},
        {2.0f, 3.0f, 0.0f},
        {1.8f, 2.7f, 0.0f},
        {2.8f, 2.2f, 0.0f},
        {3.0f, 1.0f, 0.0f},
        {1.0f, -1.0f, 0.0f}
};
size_t npts = 24;
size_t sec = 40;

struct vec4f {
    float x, y, z, w;
    vec4f() : x(0.0f),y(0.0f),z(0.0f),w(1.0f) {
    }
    vec4f(float x, float y, float z) : x(x),y(y),z(z),w(1.0f) {
    }
    vec4f(float x, float y, float z, float w) : x(x),y(y),z(z),w(w) {
    }
    //getter
    float operator[] (size_t i) const {
        switch(i) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            case 3:
                return w;
        }
    }
    //setter
    float & operator[] (size_t i) {
        switch(i) {
            case 0:
                return x;
            case 1:
                return y;
            case 2:
                return z;
            case 3:
                return w;
        }
    }
    vec4f operator- (const vec4f & rhs) const {
        vec4f res(*this);
        res.x -= rhs.x;
        res.y -= rhs.y;
        res.z -= rhs.z;
        return res;
    }
    float norm() const {
        float len = 0.0;
        len = sqrt(x*x+y*y+z*z);
        return len;
    }
    void normalize() {
        float len = norm();
        x /= len;
        y /= len;
        z /= len;
    }
    vec4f cross(const vec4f & rhs) const {
        vec4f out;
        out.x = y*rhs.z - z*rhs.y;
        out.y = z*rhs.x - x*rhs.z;
        out.z = x*rhs.y - y*rhs.x;
        return out;
    }
};

vec4f multiply(GLfloat * m, vec4f & v) {
    // v' = m*v
    vec4f v_new;
    for(size_t i=0; i<4; ++i) {
        GLfloat vi = v[i];
        for(size_t j=0; j<4; ++j) {
            v_new[j] += m[4*i+j]*vi;
        }
    }
    return v_new;
}

void bezier(float t, float & x, float & y, float & z) {
    // nice to pre-compute 1-t because we will need it frequently
    float it = 1.0f -t;

    // calculate blending functions
    float b0 = 1*t*t*t*t*t;
    float b1 = 5*t*t*t*t*it;
    float b2 = 10*t*t*t*it*it;
    float b3 = 10*t*t*it*it*it;
    float b4 = 5*t*it*it*it*it;
    float b5 = 1*it*it*it*it*it;

    // calculate the x,y and z of the curve point by summing
    // the Control vertices weighted by their respective blending
    // functions
    x = b0*cpoints[0][0] +
        b1*cpoints[1][0] +
        b2*cpoints[2][0] +
        b3*cpoints[3][0] +
        b4*cpoints[4][0] +
        b5*cpoints[5][0];

    y = b0*cpoints[0][1] +
        b1*cpoints[1][1] +
        b2*cpoints[2][1] +
        b3*cpoints[3][1] +
        b4*cpoints[4][1] +
        b5*cpoints[5][1];

    z = b0*cpoints[0][2] +
        b1*cpoints[1][2] +
        b2*cpoints[2][2] +
        b3*cpoints[3][2] +
        b4*cpoints[4][2] +
        b5*cpoints[5][2];
}

GLfloat * myRotatef(GLfloat * m, GLfloat angle, GLfloat x, GLfloat y, GLfloat z) {
    GLfloat len = sqrt(x*x+y*y+z*z);
    x /= len;
    y /= len;
    z /= len;

    GLfloat rangle = angle*PI/180.0f;
    GLfloat vcos = cos(rangle);
    GLfloat ivcos = 1.0f-vcos;
    GLfloat vsin = sin(rangle);

    // 1st col
    m[0] = x*x*ivcos +vcos;
    m[1] = x*y*ivcos +z*vsin;
    m[2] = x*z*ivcos -y*vsin;
    m[3] = 0.0f;
    // 2nd col
    m[4] = y*x*ivcos -z*vsin;
    m[5] = y*y*ivcos +vcos;
    m[6] = y*z*ivcos +x*vsin;
    m[7] = 0.0f;
    // 3rd col
    m[8] = z*x*ivcos +y*vsin;
    m[9] = z*y*ivcos -x*vsin;
    m[10] = z*z*ivcos +vcos;
    m[11] = 0.0f;
    // 4rd col
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;

//    glMultMatrixf(m);
    return m;
}



/*
 Initializes GLUT context
 */
glutWidget::glutWidget(int argc, char** argv)
{
    m_frame = 0;
    glutInitWindowSize(glutWidget::m_width, glutWidget::m_height);
    glutInit(&argc,argv);
    glutInitDisplayString("samples rgb double depth");
    glutCreateWindow("Vase");
    glutMouseFunc(mouseHandler);     //what to call when user clicks or scrolls
    glutKeyboardFunc(keyDown);       //what to call when user presses a key
    glutKeyboardUpFunc(keyUp);       //what to call when user releases a key
    glutSpecialFunc(specialKeyDown); //what to call when user presses a special key
    glutSpecialFunc(specialKeyUp);   //what to call when user releases a special key
    glutMotionFunc(mouseMove);       //what to call when user moves the mouse
    glutDisplayFunc(render);         //what to call when window needs redrawing
    glutIdleFunc(update);            //what to call when no user input given
    initOpenGL();
}


/*
 Checks whether graphics driver supports all required OpenGL features
 */
void glutWidget::checkExtensions()
{
    //query some extensions to make sure they are present
    if(glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE)
    {
        std::cout << "ERROR: Shading language extension not present." << std::endl;
    }
    if(glewGetExtension("GL_ARB_vertex_program") != GL_TRUE)
    {
        std::cout << "ERROR: Vertex program extension not present." << std::endl;
    }
    if(glewGetExtension("GL_ARB_vertex_shader") != GL_TRUE)
    {
        std::cout << "ERROR: Vertex shader extension not present." << std::endl;
    }
    if(glewGetExtension("GL_ARB_fragment_program") != GL_TRUE)
    {
        std::cout << "ERROR: Fragment program extension not present." << std::endl;
    }
    if(glewGetExtension("GL_ARB_fragment_shader") != GL_TRUE)
    {
        std::cout << "ERROR: Fragment shader extension not present." << std::endl;
    }
    if(glewGetExtension("GL_ARB_vertex_buffer_object") != GL_TRUE)
    {
        std::cout << "ERROR: VBO extension not present." << std::endl;
    }    
}




/*
 Initializes opengl states
 */
void glutWidget::initOpenGL()
{
    glewExperimental = GL_TRUE; 
    GLenum err = glewInit();                             //initialize GLEW - this enables us to use extensions
    if(err != GLEW_OK)
    {
        std::cout << "ERROR: Loading GLEW failed." << std::endl;
        exit(-1);
    }
    checkExtensions();
    glClearColor(0, 0, 0, 0);   //default "empty"/background color is set to white
    
    glEnable(GL_DEPTH_TEST);
    
    CBitmap image("texture.bmp");               //read bitmap image
    glGenTextures(1, &m_texture);               //allocate 1 texture
    glUniform1i(m_texture, 0);			//pass texture location to vertex shader
    glBindTexture(GL_TEXTURE_2D, m_texture);    //bind this texture to be active
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.GetWidth(), image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetBits());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   //specify minificaton filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   //specify magnificaton filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);        //specify texture coordinate treatment
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);        //specify texture coordinate treatment


    glBindTexture(GL_TEXTURE_2D, 0);    //bind default texture to be active
    
    makeShaders();          //load data of fragment and vertex programs/shaders - compile shaders
    
    glMatrixMode(GL_PROJECTION);       
    glLoadIdentity();                                             //initializes projection matrix with identity
    gluPerspective(60,(float)m_width/(float)m_height,0.1,100);  //set up projection mode (field of view, aspect ratio, near and far clipping plane)
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();                       //initializes modelview matrix with identity

}


/*
    Redraws window contents
 */
void glutWidget::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     //clears color and depth bits of framebuffer
    
    t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f * (float)degree;
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(zoom * sin(roty), zoom * sin(rotx), zoom * cos(roty),0,0,0,0,1,0);
    
    float angle = -360.0f/sec;
    myRotatef(M, angle, 0.0f,1.0f,0.0f);
    
    vec4f * curr_curves = new vec4f[npts];
    vec4f * next_curves = new vec4f[npts];
    for(int j=0; j<npts; ++j) {
        float u = (npts-j-1)*1.0/(npts-1);
        bezier(u, curr_curves[j].x, curr_curves[j].y, curr_curves[j].z);
    }
    
    vec4f curr_norm, next_norm;
    
    //Enable shaders and textures
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glUseProgram(m_program);
    
    for(int i=0; i<sec; ++i) {

        glBegin(GL_TRIANGLE_STRIP);
        for(int j=0; j<npts; ++j) {
            next_curves[j] = multiply(M, curr_curves[j]);
            if(j>0) {
                curr_norm = curr_curves[j] - curr_curves[j-1];
                curr_norm.normalize();
                next_norm = next_curves[j] - next_curves[j-1];
                next_norm.normalize();
            }
            else {
                curr_norm.x = cpoints[1][0]-cpoints[0][0];
                curr_norm.y = cpoints[1][1]-cpoints[0][1];
                curr_norm.z = cpoints[1][2]-cpoints[0][2];
                curr_norm.normalize();
                next_norm = multiply(M, curr_norm);
            }
            vec4f invec(-curr_curves[j].x, 0.0f, -curr_curves[j].z);
            vec4f outvec = curr_norm.cross(invec);
            curr_norm = curr_norm.cross(outvec);
            curr_norm.normalize();
            
            vec4f invec2(-next_curves[j].x, 0.0f, -next_curves[j].z);
            vec4f outvec2 = next_norm.cross(invec2);
            next_norm = next_norm.cross(outvec2);
            next_norm.normalize();

            float u = j*1.0f/(npts-1);
            glColor3f(1.0f,1.0f,1.0f); glTexCoord2f((float)(i+0)/sec, -u); glNormal3f(curr_norm.x,curr_norm.y,curr_norm.z); glVertex3f(curr_curves[j].x, curr_curves[j].y, curr_curves[j].z);
            glColor3f(1.0f,1.0f,1.0f); glTexCoord2f((float)(i+1)/sec, -u); glNormal3f(next_norm.x,next_norm.y,next_norm.z); glVertex3f(next_curves[j].x, next_curves[j].y, next_curves[j].z);
        }
        glEnd();
        
        //Draw bottom of vase
        glBegin(GL_TRIANGLES);
            glColor3f(1.0f,1.0f,1.0f); glNormal3f(0.0f,-1.0f,0.0f); glVertex3f(next_curves[npts-1].x, next_curves[npts-1].y, next_curves[npts-1].z);
            glColor3f(1.0f,1.0f,1.0f); glNormal3f(0.0f, -1.0f, 0.0f); glVertex3f(0.0f, cpoints[4][1], 0.0f);
            glColor3f(1.0f,1.0f,1.0f); glNormal3f(0.0f,-1.0f,0.0f); glVertex3f(curr_curves[npts-1].x, curr_curves[npts-1].y, curr_curves[npts-1].z);
        glEnd();

        std::swap(curr_curves, next_curves);
        myRotatef(M, angle, 0.0f,1.0f,0.0f);
    }
        
    glUseProgram(0);
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete [] curr_curves;
    delete [] next_curves;
    
    glutSwapBuffers();  //swaps front and back buffer for double buffering
}


/*
 Handles user event: a mouse button was pressed
 */
void glutWidget::mouseHandler(int button, int state, int x, int y)
{
    if (state == GLUT_UP)
        return;
    
    switch(button)
    {
        case 0: //Left click
            x0 = x;
            y0 = y;
            break;
        case 2: //Right click
            
            break;
        case 3: //Scroll up
            
            break;
        case 4: //Scroll down
            
            break;
    }
}


/*
 Handles user event: a key was pressed
 */
void glutWidget::keyDown(unsigned char key, int, int) 
{
    switch(key)
    {
        case 'q':
        case 27:
            stop(); //quit the main loop, exit program
            break;
        case 'c':
            
            break;
        case 'h':
            
            break;
        case 'w':
            zoom /= 1.2;
            break;
        case 'a':

            break;
        case 's':
            zoom *= 1.2;
            break;
        case 'd':

            break;
            
    }
}


/*
 Handles user event: a key was released
 */
void glutWidget::keyUp(unsigned char key, int, int) 
{  
    
}


/*
 Handles user event: a special key was pressed
 */
void glutWidget::specialKeyDown(int key, int, int) 
{
    
}


/*
 Handles user event: a special key was released
 */
void glutWidget::specialKeyUp(int key, int, int) 
{  
    switch(key)
    {
        case GLUT_KEY_UP:
            
            break;
        case GLUT_KEY_DOWN:
            
            break;
        case GLUT_KEY_LEFT:
            
            break;
        case GLUT_KEY_RIGHT:
            
            break;
    }
}

void glutWidget::mouseMove(int x, int y)
{
    float dx = ((x - x0)) / 5.0;
    float dy = ((y - y0)) / 5.0;
    
    roty += dx * (3.14159265 / 180.0);
    rotx -= dy * (3.14159265 / 180.0);
    
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
//    gluLookAt(0,0,5,2 * sin(roty), sin(rotx),2 * cos(roty),0,1,0);
    //gluLookAt(2 * sin(roty), sin(rotx),2 * cos(roty),0,0,0,0,1,0);
    
    //glRotatef(dx, 0, 1.0, 0);
    //glRotatef(dy, 1.0, 0.0, 0.0);
    
    x0 = x;
    y0 = y;
}



/*
    Forces program to stop, cleans up, and exits
 */
void glutWidget::stop()
{
    //clean up shaders
    glDetachShader(m_program,m_fragmentsh);
    
    glDeleteProgram(m_program);
	glDeleteShader(m_fragmentsh);
    
    exit(0);
}

/*
    Reads in and compiles shader objects
 */
void glutWidget::makeShaders()
{
    m_program = glCreateProgram();
	
    char *shadercode = readShader("shaders/vertexshader.vert");	//reads shader code (you can edit shader code with a text editor)
    m_vertexsh = glCreateShader(GL_VERTEX_SHADER_ARB);
    glShaderSource(m_vertexsh, 1, (const GLcharARB **)&shadercode,NULL);
    delete[] shadercode;
    glCompileShader(m_vertexsh);    //compiles shader
    printInfoLog(m_vertexsh);       //prints errors during shader compilation
    
    
    shadercode = readShader("shaders/fragmentshader.frag");     //reads shader code (you can edit shader code with a text editor)
    m_fragmentsh = glCreateShader(GL_FRAGMENT_SHADER_ARB);
    glShaderSource(m_fragmentsh, 1, (const GLcharARB **)&shadercode,NULL);
    delete[] shadercode;
    glCompileShader(m_fragmentsh);  //compiles shader
    printInfoLog(m_fragmentsh);     //prints errors during shader compilation

    glAttachShader(m_program,m_vertexsh);
    glAttachShader(m_program,m_fragmentsh);

    glLinkProgram(m_program);   //compiles fragment and vertex shader into a shader program
    printInfoLog(m_program);    //prints errors during program compilation
    
    glUseProgram(m_program);
    m_pos_attribute_location = glGetAttribLocation(m_program,"in_Position");  //get pointer to "in_Position" attribute of vertex shader
    glUseProgram(0);
}



/*
 Starts the main loop
 */
void glutWidget::run()
{
    glutMainLoop();
}



/*
    Called whenever no user input given
 */
void glutWidget::update()
{
    m_frame++;
    glutPostRedisplay(); //marks window for redrawing
}



    
