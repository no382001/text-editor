#include "bufferpool.h"
#include "cursor.h"
#include "document.h"
#include "linenode.h"
#include "node.h"
#include "utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include <math.h>

int scroll_offset = 0;
int font_size = 20;
int line_padding = 2;

extern Cursor cursor;

void draw(Document *d, Font *font) {
    LineNode *ln = d->first_line;
    int line_number = 0;

    while (ln) {
        Node *node = ln->head;
        int column = 0;
        int x = 10;
        bool cursor_drawn = false;

        if (node->size == 0) {  // Handle empty line
            if (line_number == cursor.line) {
                Vector2 position = {x, 200 + line_number * (font_size + line_padding) -
                                           scroll_offset};
                Rectangle text_rect = {position.x, position.y, font_size / 2, font_size};
                DrawRectangleRec(text_rect, Fade(WHITE, 0.3f)); // Semi-transparent rectangle
                cursor_drawn = true;
            }
        }

        while (node) {
            for (int i = 0; i < node->size; i++) {
                char character[2] = {node->chunk[i], '\0'};
                Vector2 position = {x, 200 + line_number * (font_size + line_padding) -
                                           scroll_offset};

                Vector2 text_size = MeasureTextEx(*font, character, font_size, 0);
                Rectangle text_rect = {position.x, position.y, text_size.x, text_size.y};

                // Draw the text
                DrawTextEx(*font, character, position, font_size, 0, LIGHTGRAY);

                // Check if this is where the cursor should be drawn
                if (line_number == cursor.line && column == cursor.column) {
                    // Draw the rectangle around the text at the cursor position
                    DrawRectangleRec(text_rect, Fade(WHITE, 0.3f)); // Semi-transparent rectangle
                    cursor_drawn = true;
                }

                x += text_size.x;
                column++;
            }
            node = node->next;
        }

        // If the cursor is at the end of the line, draw it after the last character
        if (line_number == cursor.line && !cursor_drawn) {
            Vector2 position = {x, 200 + line_number * (font_size + line_padding) -
                                       scroll_offset};
            Rectangle text_rect = {position.x, position.y, font_size / 2, font_size};
            DrawRectangleRec(text_rect, Fade(WHITE, 0.3f)); // Semi-transparent rectangle
        }

        line_number++;
        ln = ln->next;
    }
}
