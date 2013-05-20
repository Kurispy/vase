#include <GL/glew.h>
#include <GL/gl.h> 
#include <GL/glu.h>
#include <GL/glut.h>

#include "glutwidget.hpp"
#include "shader_utils.hpp"
#include <iostream>
#include <cmath>
#include <ctime>

int glutWidget::m_pos_attribute_location;
unsigned int glutWidget::m_vertexbuffer;
unsigned int glutWidget::m_frame;
unsigned int glutWidget::m_program;
unsigned int glutWidget::m_vertexsh;
unsigned int glutWidget::m_fragmentsh;


/*
 Initializes GLUT context
 */
glutWidget::glutWidget(int argc, char** argv)
{
    m_frame = 0;
    glutInitWindowSize(glutWidget::m_width, glutWidget::m_height);
    glutInit(&argc,argv);
    glutInitDisplayString("samples rgb double depth");
    glutCreateWindow("Bezier");
    glutMouseFunc(mouseHandler);     //what to call when user clicks or scrolls
    glutKeyboardFunc(keyDown);       //what to call when user presses a key
    glutKeyboardUpFunc(keyUp);       //what to call when user releases a key
    glutSpecialFunc(specialKeyDown); //what to call when user presses a special key
    glutSpecialFunc(specialKeyUp);   //what to call when user releases a special key
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
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    
    float data[] = {0.0, 0.05, 0.0, 0.0, -0.05, 0.0, 0.1, 0.0, 0.0};
    
    //Initialize vertex buffer
    glGenBuffers(1, &m_vertexbuffer);               //generate a vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexbuffer);  //bind buffer to be active
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW); //set buffer data
    glVertexPointer(3, GL_FLOAT, 0, NULL); //Let OpenGl know that there are 3 coordinates per vertex
    glEnableClientState(GL_VERTEX_ARRAY); //Let OpenGL know that the VBO contains verticies
    glEnableVertexAttribArray(m_pos_attribute_location); //point to position attribute in shader
    glVertexAttribPointer(m_pos_attribute_location, 3, GL_FLOAT, GL_FALSE, 0, 0); //indicates array data of position attribute  
    
    makeShaders();          //load data of fragment and vertex programs/shaders - compile shaders
    
    glEnable(GL_MAP1_VERTEX_3);
    
    
}


/*
    Redraws window contents
 */
void glutWidget::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     //clears color and depth bits of framebuffer
    
    //Convenience variables
    float t = (float) ((int) (((float) clock()) * 50.0 / CLOCKS_PER_SEC) % 30) / 30.0;
    int t2 = ((int) (((float) clock()) * 50.0 / CLOCKS_PER_SEC) % 360) / 30;
    float t3 = (float) clock() * 5.0 / CLOCKS_PER_SEC;
    float ampl1 = 0.15, ampl2 = 0.1;
    
    //These are the control points for the bezier curves
    GLfloat curves[12][4][3] =
    {
        {
            {-0.752, 0.124, 0.0},
            {-0.572, 0.36, 0.0},
            {0.188, 0.156, 0.0},
            {0.244, 0.336, 0.0}
        },
        {
            {0.244, 0.336, 0.0},
            {0.3, 0.516, 0.0},
            {0.268, 0.744, 0.0},
            {0.416, 0.424, 0.0}
        },
        {
            {0.416, 0.424, 0.0},
            {0.48132, 0.28272, 0.0},
            {0.436, 0.296, 0.0},
            {0.58, 0.22, 0.0}
        },
        {
            {0.58, 0.22, 0.0},
            {0.724, 0.144, 0.0},
            {0.80952, 0.0728, 0.0},
            {0.84, 0.028, 0.0}
        },
        {
            {0.84, 0.028, 0.0},
            {0.908, -0.072, 0.0},
            {0.448, -0.036, 0.0},
            {0.348, -0.032, 0.0}
        },
        {
            {0.348, -0.032, 0.0},
            {0.248, -0.028, 0.0},
            {0.38 + ampl1 * sin(t3), -0.792, 0.0}, //Foot start
            {0.232 + ampl1 * sin(t3), -0.596, 0.0}
        },
        {
            {0.232 + ampl1 * sin(t3), -0.596, 0.0},
            {0.084 + ampl1 * sin(t3), -0.4, 0.0}, //Foot end
            {0.15772, -0.1828, 0.0},
            {0.1, -0.14, 0.0}
        },
        {
            {0.1, -0.14, 0.0},
            {0.04248, -0.09736, 0.0},
            {-0.25096, -0.14956, 0.0},
            {-0.284, -0.22, 0.0}
        },
        {
            {-0.284, -0.22, 0.0},
            {-0.3244, -0.30612, 0.0},
            {-0.2664 + ampl2 * sin(t3 + 2.0), -0.44784, 0.0}, //Foot start
            {-0.324 + ampl2 * sin(t3 + 2.0), -0.56, 0.0}
        },
        {
            {-0.324 + ampl2 * sin(t3 + 2.0), -0.56, 0.0},
            {-0.476 + ampl2 * sin(t3 + 2.0), -0.856, 0.0}, //Foot end
            {-0.5136, -0.33052, 0.0},
            {-0.608, -0.212, 0.0}
        },
        {
            {-0.608, -0.212, 0.0},
            {-0.65616, -0.15152, 0.0},
            {-0.73004, -0.15228, 0.0},
            {-0.776, -0.328, 0.0}
        },
        {
            {-0.776, -0.328, 0.0},
            {-0.912, -0.848, 0.0},
            {-0.932, -0.108, 0.0},
            {-0.752, 0.124, 0.0}
        }
    };
    
    
    //Manually evaluate the bezier curves
    float s = 1 - t;
    float AB[2] = {curves[t2][0][0]*s + curves[t2][1][0]*t, curves[t2][0][1]*s + curves[t2][1][1]*t};
    float BC[2] = {curves[t2][1][0]*s + curves[t2][2][0]*t, curves[t2][1][1]*s + curves[t2][2][1]*t};
    float CD[2] = {curves[t2][2][0]*s + curves[t2][3][0]*t, curves[t2][2][1]*s + curves[t2][3][1]*t};
    float ABC[2] = {AB[0]*s + BC[0]*t, AB[1]*s + BC[1]*t};
    float BCD[2] = {BC[0]*s + CD[0]*t, BC[1]*s + CD[1]*t};
    float pos[2] = {ABC[0]*s + BCD[0]*t, ABC[1]*s + BCD[1]*t};
    float slope = (ABC[1] - BCD[1]) / (ABC[0] - BCD[0]);
    float theta = atan(slope);
    
    //Rotation/translation matrix
    float matrix[4][4] = 
    {
        {cos(theta), -sin(theta), 0, 0},
        {sin(theta), cos(theta), 0, 0},
        {0, 0, 1, 0},
        {pos[0], pos[1], 0, 1}
    };
    
    //Keep the texture pointing in the right direction throughout the loop
    if((ABC[0] - BCD[0]) < 0)
    {
        matrix[0][1] = -(matrix[0][1]);
        matrix[1][0] = -(matrix[1][0]);
    }
    else if((ABC[0] - BCD[0]) > 0)
    {
        matrix[0][0] = -(matrix[0][0]);
        matrix[1][1] = -(matrix[1][1]);
    }
    
    //Since we aren't using it for anything else, we'll use the ModelView matrix
    //to store our transformations. This way we don't have to manually pass the
    //matrix to the vertex shader.
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&matrix[0][0]);
    
    glUseProgram(m_program); //Enables shaders
    glColor3f(0.0, 0.0, 0.0);
    glDrawArrays(GL_TRIANGLES, 0, 3); //draw triangle
    glUseProgram(0); //Disables shaders
    
    glLoadIdentity();
    
    //Draws the bezier curves themselves using EvalCoord()
    glColor3f(0.0, 1.0, 1.0);
    for(int i = 0; i < 12; i++)
    {
        glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &curves[i][0][0]);
        glBegin(GL_LINE_STRIP);
          for (int i = 0; i <= 30; i++) 
             glEvalCoord1f((GLfloat) i/30.0);
        glEnd();
    }
    
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
            
            break;
        case 's':
            
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



    
