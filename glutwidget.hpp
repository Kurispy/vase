#ifndef _GLUTWIDGET_HPP_
#define _GLUTWIDGET_HPP_

class Cube;

/*
 GLUT widget singleton
 */
class glutWidget
{
private:
    static unsigned int m_program;                 //shader program
    static unsigned int m_vertexsh;                //vertex shader
    static unsigned int m_fragmentsh;              //fragment shader
    static int m_pos_attribute_location;           //points to attribute location in vertex shader
    static unsigned int m_vertexbuffer;
    static unsigned int m_frame;
    static const unsigned int m_width = 500;
    static const unsigned int m_height = 500;
    static void render();
    static void update();
    static void mouseHandler(int button, int state, int x, int y);
    static void keyDown(unsigned char key, int, int);
    static void keyUp(unsigned char key, int, int);
    static void specialKeyDown(int key, int, int);
    static void specialKeyUp(int key, int, int);
    static void stop();
    static void initOpenGL();
    static void drawCubes();
    static void makeShaders();
    glutWidget(int argc, char **argv);
    static void checkExtensions();
public:
    static glutWidget& init(int argc, char **argv)
    {
        static glutWidget gw(argc, argv);
        return gw;
    }
    static void run();
};





#endif

