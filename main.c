#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <raylib.h>

#include "./editor.h"

#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define FONT_SCALE 5

typedef struct {
    Texture2D spritesheet;
    Rectangle glyph_table[ASCII_DISPLAY_HIGH - ASCII_DISPLAY_LOW + 1];
} Font;

Font font_load_from_file(const char *file_path)
{
    Font font = {0};

    Image font_image = LoadImage(file_path);
    ImageFormat(&font_image, UNCOMPRESSED_R8G8B8A8);
    font.spritesheet = LoadTextureFromImage(font_image);
    UnloadImage(font_image);

    for (size_t ascii = ASCII_DISPLAY_LOW; ascii <= ASCII_DISPLAY_HIGH; ++ascii) {
        const size_t index = ascii - ASCII_DISPLAY_LOW;
        const size_t col = index % FONT_COLS;
        const size_t row = index / FONT_COLS;
        font.glyph_table[index] = (Rectangle) {
            .x = (float) col * FONT_CHAR_WIDTH,
            .y = (float) row * FONT_CHAR_HEIGHT,
            .width = FONT_CHAR_WIDTH,
            .height = FONT_CHAR_HEIGHT,
        };
    }

    return font;
}

void render_char(Font *font, char c, Vector2 pos, float scale)
{
    size_t index = '?' - ASCII_DISPLAY_LOW;
    if (ASCII_DISPLAY_LOW <= c && c <= ASCII_DISPLAY_HIGH) {
        index = c - ASCII_DISPLAY_LOW;
    }

    Rectangle src = font->glyph_table[index];
    Rectangle dst = {
        .x = pos.x,
        .y = pos.y,
        .width = FONT_CHAR_WIDTH * scale,
        .height = FONT_CHAR_HEIGHT * scale,
    };

    DrawTextureRec(font->spritesheet, src, (Vector2){dst.x, dst.y}, WHITE);
}

void render_text_sized(Font *font, const char *text, size_t text_size, Vector2 pos, Color color, float scale)
{
    for (size_t i = 0; i < text_size; ++i) {
        Vector2 char_pos = (Vector2){pos.x + i * FONT_CHAR_WIDTH * scale, pos.y};
        render_char(font, text[i], char_pos, scale);
    }
}

void render_cursor(Font *font)
{
    Vector2 pos = (Vector2){
        .x = (float) editor.cursor_col * FONT_CHAR_WIDTH * FONT_SCALE,
        .y = (float) editor.cursor_row * FONT_CHAR_HEIGHT * FONT_SCALE
    };

    Rectangle cursor_rect = {
        .x = pos.x,
        .y = pos.y,
        .width = FONT_CHAR_WIDTH * FONT_SCALE,
        .height = FONT_CHAR_HEIGHT * FONT_SCALE,
    };

    DrawRectangleRec(cursor_rect, WHITE);

    const char *c = editor_char_under_cursor(&editor);
    if (c) {
        DrawTextureRec(font->spritesheet, font->glyph_table[*c - ASCII_DISPLAY_LOW], pos, BLACK);
    }
}

int main(void)
{
    InitWindow(800, 600, "Text Editor");
    SetTargetFPS(60);

    Font font = font_load_from_file("./charmap-oldschool_white.png");

    editor_insert_text_before_cursor(&editor, "dhjfhskjdfhkjshdf");
    editor_insert_new_line(&editor);
    editor_insert_text_before_cursor(&editor, "3j4k23l4j");
    editor_insert_new_line(&editor);
    editor_insert_text_before_cursor(&editor, "456kj356klj35l6j");
    editor_insert_new_line(&editor);
    editor_insert_text_before_cursor(&editor, "46jkl45jkljclslkj");
    editor_insert_new_line(&editor);
    editor_insert_text_before_cursor(&editor, "tjk5kfkdjgk");

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_BACKSPACE)) {
            editor_backspace(&editor);
        }
        if (IsKeyPressed(KEY_ENTER)) {
            editor_insert_new_line(&editor);
        }
        if (IsKeyPressed(KEY_DELETE)) {
            editor_delete(&editor);
        }
        if (IsKeyPressed(KEY_UP)) {
            if (editor.cursor_row > 0) {
                editor.cursor_row -= 1;
            }
        }
        if (IsKeyPressed(KEY_DOWN)) {
            editor.cursor_row += 1;
        }
        if (IsKeyPressed(KEY_LEFT)) {
            if (editor.cursor_col > 0) {
                editor.cursor_col -= 1;
            }
        }
        if (IsKeyPressed(KEY_RIGHT)) {
            editor.cursor_col += 1;
        }

        if (IsKeyPressed(KEY_TEXTINPUT)) {
            char text[256];
            int length = GetCharPressed();
            while (length > 0)
            {
                for (int i = 0; i < length; i++)
                {
                    text[i] = GetCharPressed();
                }
                text[length] = '\0';
                editor_insert_text_before_cursor(&editor, text);
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (size_t row = 0; row < editor.size; ++row) {
            const Line *line = editor.lines + row;
            render_text_sized(&font, line->chars, line->size, (Vector2){0.0f, row * FONT_CHAR_HEIGHT * FONT_SCALE}, WHITE, FONT_SCALE);
        }
        render_cursor(&font);

        EndDrawing();
    }

    UnloadTexture(font.spritesheet);
    CloseWindow();

    return 0;
}
