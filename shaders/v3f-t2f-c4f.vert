/* Freetype GL - A C OpenGL Freetype engine
 *
 * Distributed under the OSI-approved BSD 2-Clause License.  See accompanying
 * file `LICENSE` for more details.
 */
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

attribute vec3 vertex;
attribute vec2 tex_coord;
attribute vec4 color;

uniform float scroll_offset;

void main() {
    vec4 tv = model * vec4(vertex, 1.0);
    tv.y -= scroll_offset;

    gl_TexCoord[0].xy = tex_coord;
    gl_FrontColor = color;
    gl_Position = projection * view * tv;
}
