/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
#include <stdio.h>
#include <string.h>

#include "freetype-gl.h"
#include "mat4.h"
#include "shader.h"
#include "vertex-buffer.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

typedef struct {
    float x, y, z;    // position
    float s, t;       // texture
    float r, g, b, a; // color
} vertex_t;

GLuint shader;
texture_atlas_t *atlas;
vertex_buffer_t *buffer;
mat4   model, view, projection;

struct editor_t {
    std::ifstream file;
    std::vector<std::string> lines;
    texture_font_t* font = nullptr;
    vec2 cursor_pos = {0,0};
    float scroll_pos = 0;

    editor_t(){
        file = std::ifstream("tinyscm/stl/core.scm");
        std::string line;

        if (file.is_open()) {
            while (getline(file, line)) {
                lines.push_back(line);
            }
            file.close();
        } else {
            std::cout << "Unable to open file" << std::endl;
            return;
        }
    }
};

editor_t editor{};

void add_text( vertex_buffer_t * buffer, texture_font_t * font, char * text, vec4 * color, vec2 * pen ) {
    size_t i;
    float r = color->red, g = color->green, b = color->blue, a = color->alpha;
    for( i = 0; i < strlen(text); ++i )
    {
        texture_glyph_t *glyph = texture_font_get_glyph( font, text + i );
        if( glyph != NULL )
        {
            float kerning =  0.0f;
            if( i > 0)
            {
                kerning = texture_glyph_get_kerning( glyph, text + i - 1 );
            }
            pen->x += kerning;
            int x0  = (int)( pen->x + glyph->offset_x );
            int y0  = (int)( pen->y + glyph->offset_y );
            int x1  = (int)( x0 + glyph->width );
            int y1  = (int)( y0 - glyph->height );
            float s0 = glyph->s0;
            float t0 = glyph->t0;
            float s1 = glyph->s1;
            float t1 = glyph->t1;
            GLuint indices[6] = {0,1,2, 0,2,3};
            vertex_t vertices[4] = { { x0,y0,0,  s0,t0,  r,g,b,a },
                                     { x0,y1,0,  s0,t1,  r,g,b,a },
                                     { x1,y1,0,  s1,t1,  r,g,b,a },
                                     { x1,y0,0,  s1,t0,  r,g,b,a } };
            vertex_buffer_push_back( buffer, vertices, 4, indices, 6 );
            pen->x += glyph->advance_x;
        }
    }
}

void init(vec2 pen) {
    size_t i;
    atlas = texture_atlas_new( 512, 512, 1 );
    buffer = vertex_buffer_new( "vertex:3f,tex_coord:2f,color:4f" );
    vec4 black = {{0,0,0,1}};

    for(auto l : editor.lines) {
        editor.font = texture_font_new_from_file( atlas, 10, "fonts/Vera.ttf" );
        pen.x = 5;
        pen.y -= editor.font->height;
        texture_font_load_glyphs( editor.font, l.c_str() );
        add_text( buffer, editor.font, const_cast<char*>(l.c_str()), &black, &pen );
        texture_font_delete( editor.font );
    }

    glGenTextures( 1, &atlas->id );
    glBindTexture( GL_TEXTURE_2D, atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height,
                  0, GL_RED, GL_UNSIGNED_BYTE, atlas->data );

    shader = shader_load("shaders/v3f-t2f-c4f.vert",
                         "shaders/v3f-t2f-c4f.frag");
    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
}

void display( GLFWwindow* window ) {
    
    glClearColor( 1, 1, 1, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glUseProgram( shader );
    {
        glUniform1i( glGetUniformLocation( shader, "texture" ), 0 );
        glUniformMatrix4fv( glGetUniformLocation( shader, "model" ), 1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "view" ), 1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( shader, "projection" ), 1, 0, projection.data);
        glUniform1f(glGetUniformLocation(shader, "scroll_offset"), editor.scroll_pos);
        vertex_buffer_render( buffer, GL_TRIANGLES );
    }
    // go back to imm. mode
    glUseProgram(0);

    float x = editor.cursor_pos.x;
    float y = editor.cursor_pos.y;

    glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        glVertex3f(x, y + 0.1f, 0.0f);

        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        glVertex3f(x - 0.1f, y - 0.1f, 0.0f);

        glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
        glVertex3f(x + 0.1f, y - 0.1f, 0.0f);
    glEnd();

    glfwSwapBuffers( window );
}

void reshape( GLFWwindow* window, int width, int height ) {
    glViewport(0, 0, width, height);
    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods ) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch(key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GL_TRUE);
                break;
            case GLFW_KEY_UP:
                editor.cursor_pos.y += 0.01;
                break;
            case GLFW_KEY_DOWN:
                editor.cursor_pos.y -= 0.01;
                break;
            case GLFW_KEY_LEFT:
                editor.cursor_pos.x -= 0.01;
                break;
            case GLFW_KEY_RIGHT:
                editor.cursor_pos.x += 0.01;
                break;
            case GLFW_KEY_PAGE_UP:
                editor.scroll_pos += 1;
                break;
            case GLFW_KEY_PAGE_DOWN:
                editor.scroll_pos -= 1;
                break;
        }
    }
}

void error_callback( int error, const char* description ) {
    fputs( description, stderr );
}

int main( int argc, char **argv ) {
    GLFWwindow* window;
    char* screenshot_path = NULL;

    glfwSetErrorCallback( error_callback );

    if (!glfwInit( ))
    {
        exit( EXIT_FAILURE );
    }

    glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );

    window = glfwCreateWindow( 800, 500, argv[0], NULL, NULL );

    if (!window)
    {
        glfwTerminate( );
        exit( EXIT_FAILURE );
    }

    glfwMakeContextCurrent( window );
    glfwSwapInterval( 1 );

    glfwSetFramebufferSizeCallback( window, reshape );
    glfwSetWindowRefreshCallback( window, display );
    glfwSetKeyCallback( window, keyboard );

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf( stderr, "Error: %s\n", glewGetErrorString(err) );
        exit( EXIT_FAILURE );
    }
    fprintf( stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION) );

    init({{5,500}});

    glfwShowWindow( window );
    reshape( window, 800, 500 );

    while (!glfwWindowShouldClose( window ))
    {
        display( window );
        glfwPollEvents( );
    }

    glDeleteTextures( 1, &atlas->id );
    atlas->id = 0;
    texture_atlas_delete( atlas );

    glfwDestroyWindow( window );
    glfwTerminate( );

    return EXIT_SUCCESS;
}
