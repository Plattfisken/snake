#include <_stdlib.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>

#define GRID_COLUMNS 24
#define GRID_ROWS GRID_COLUMNS
#define GRID_SIZE ((GRID_COLUMNS) * (GRID_ROWS))
#define SQUARE_SIZE 30

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT,
} DIRECTION;

typedef enum {
    NEW_GAME,
    IN_PROGRESS,
    WIN,
    LOSE
} GAME_RESULT;

typedef struct {
    int x;
    int y;
} IntVector;

typedef struct {
    IntVector positions[GRID_SIZE];
    int length;
    DIRECTION direction;
} Player;

typedef struct {
    Player player;
    DIRECTION new_direction;
    IntVector apple_position;
    GAME_RESULT result;
    float time_since_last_update;
} GameState;

bool vec_equals(IntVector v1, IntVector v2) {
    return v1.x == v2.x && v1.y == v2.y;
}

void setup_game(GameState *game_state) {
    memset(game_state, 0, sizeof *game_state);
    game_state->player.length = 1;
    game_state->player.direction = -1;
    game_state->player.positions[0].x = arc4random_uniform(GRID_ROWS);
    game_state->player.positions[0].y = arc4random_uniform(GRID_COLUMNS);

    game_state->new_direction = -1;
    game_state->apple_position.x = arc4random_uniform(GRID_ROWS);
    game_state->apple_position.y = arc4random_uniform(GRID_COLUMNS);

    game_state->result = IN_PROGRESS;

    game_state->time_since_last_update = 0;
}

void update_game(GameState *game_state, int grid_x_pos, int grid_y_pos) {
    if(IsKeyPressed(KEY_W)) {
        if(game_state->player.direction != DOWN) game_state->new_direction = UP;
    }
    else if(IsKeyPressed(KEY_A)) {
        if(game_state->player.direction != RIGHT) game_state->new_direction = LEFT;
    }
    else if(IsKeyPressed(KEY_S)) {
        if(game_state->player.direction != UP) game_state->new_direction = DOWN;
    }
    else if(IsKeyPressed(KEY_D)) {
        if(game_state->player.direction != LEFT) game_state->new_direction = RIGHT;
    }

    game_state->time_since_last_update += GetFrameTime();
    float update_time = 0.2;
    if(game_state->time_since_last_update > update_time) {
        game_state->time_since_last_update = 0;
        game_state->player.direction = game_state->new_direction;

        for(int i = game_state->player.length; i > 0; --i) {
            game_state->player.positions[i] = game_state->player.positions[i - 1];
        }

        // update head
        switch(game_state->player.direction) {
            case UP:
                --game_state->player.positions[0].y;
                break;
            case DOWN:
                ++game_state->player.positions[0].y;
                break;
            case LEFT:
                --game_state->player.positions[0].x;
                break;
            case RIGHT:
                ++game_state->player.positions[0].x;
                break;
        }
    }

    if(game_state->player.positions[0].x >= GRID_COLUMNS || game_state->player.positions[0].x < 0) {
        game_state->result = LOSE;
        return;
    }
    if(game_state->player.positions[0].y >= GRID_ROWS || game_state->player.positions[0].y < 0) {
        game_state->result = LOSE;
        return;
    }

    for(int i = 1; i < game_state->player.length; ++i) {
        if(vec_equals(game_state->player.positions[0], game_state->player.positions[i]))
        {
            game_state->result = LOSE;
            return;
        }
    }

    if(vec_equals(game_state->player.positions[0], game_state->apple_position)) {
        ++game_state->player.length;
        if(game_state->player.length == GRID_SIZE) {
            game_state->result = WIN;
            return;
        }
        bool apple_in_snake = true;
        while(apple_in_snake) {
            game_state->apple_position.x = arc4random_uniform(GRID_COLUMNS);
            game_state->apple_position.y = arc4random_uniform(GRID_ROWS);
            apple_in_snake = false;
            for(int i = 0; i < game_state->player.length; ++i) {
                if(vec_equals(game_state->player.positions[i], game_state->apple_position)) {
                    apple_in_snake = true;
                    break;
                }
            }
        }
    }

    int apple_x_pos = grid_x_pos + game_state->apple_position.x * SQUARE_SIZE;
    int apple_y_pos = grid_y_pos + game_state->apple_position.y * SQUARE_SIZE;
    DrawRectangle(apple_x_pos, apple_y_pos, SQUARE_SIZE, SQUARE_SIZE, GREEN);

    for(int i = 0; i < game_state->player.length; ++i) {
        int player_segment_screen_x_pos = grid_x_pos + game_state->player.positions[i].x * SQUARE_SIZE;
        int player_segment_screen_y_pos = grid_y_pos + game_state->player.positions[i].y * SQUARE_SIZE;
        DrawRectangle(player_segment_screen_x_pos, player_segment_screen_y_pos, SQUARE_SIZE, SQUARE_SIZE, RED);
    }

    int buf_size = 16;
    char buf[buf_size];
    snprintf(buf, buf_size, "Score: %d", game_state->player.length - 1);
    DrawText(buf, 30, 60, 20, BLACK);
}

void end_screen(GameState *game_state, char *message) {
    if(IsKeyPressed(KEY_SPACE)) {
        game_state->result = NEW_GAME;
    }
    int font_size = 20;
    int text_width = MeasureText(message, font_size);
    DrawText(message, GetScreenWidth()/2 - text_width/2, GetScreenHeight()/2 - font_size/2, font_size, BLACK);
}

int main(void) {
    InitWindow(0, 0, "Snake");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(GRID_COLUMNS * SQUARE_SIZE, GRID_ROWS * SQUARE_SIZE);
    SetTargetFPS(60);

    GameState game_state = {};
    setup_game(&game_state);

    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        // drawing grid
        int grid_width = GRID_COLUMNS * SQUARE_SIZE;
        int grid_height = GRID_ROWS * SQUARE_SIZE;
        int grid_x_pos = GetScreenWidth() / 2 - grid_width / 2;
        int grid_y_pos = GetScreenHeight() / 2 - grid_height / 2;
        DrawRectangle(grid_x_pos, grid_y_pos, grid_width, grid_height, GRAY);

        switch(game_state.result) {
            case NEW_GAME:
                setup_game(&game_state);
                break;
            case IN_PROGRESS:
                update_game(&game_state, grid_x_pos, grid_y_pos);
                break;
            case WIN:
                end_screen(&game_state, "You win! Press space to play again.");
                break;
            case LOSE:
                end_screen(&game_state, "You lose! Press space to play again.");
                break;
        }
        DrawFPS(30, 30);
        EndDrawing();
    }
}
