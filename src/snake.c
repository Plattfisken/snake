#include <_stdlib.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define GRID_COLUMNS 24
#define GRID_ROWS GRID_COLUMNS
#define GRID_SIZE ((GRID_COLUMNS) * (GRID_ROWS))
#define SQUARE_SIZE 30
#define PLAYER_STARTING_LENGTH 2

typedef enum {
    STATIONARY = -1,
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
    IntVector positions[GRID_SIZE + 2];
    int length;
    DIRECTION direction;
} Player;

typedef struct {
    Player player;
    DIRECTION new_direction;
    IntVector apple_position;
    GAME_RESULT result;
    float time_since_last_update;
    int score_top_10[10];
} GameState;

void insert(int *ints, uint32_t length, int val, uint32_t index) {
    assert(index < length && "index can not be larger than length");
    for(uint32_t i = length - 1; i > index; --i) {
        ints[i] = ints[i - 1];
    }
    ints[index] = val;
}

bool vec_equals(IntVector v1, IntVector v2) {
    return v1.x == v2.x && v1.y == v2.y;
}

void setup_game(GameState *game_state) {
    memset(game_state, 0, sizeof *game_state);
    game_state->player.length = PLAYER_STARTING_LENGTH;


    game_state->player.direction = STATIONARY;
    game_state->player.positions[0].x = arc4random_uniform(GRID_ROWS);
    game_state->player.positions[0].y = arc4random_uniform(GRID_COLUMNS);

    for(int i = 1; i < PLAYER_STARTING_LENGTH; ++i) {
        game_state->player.positions[i].x = game_state->player.positions[0].x - 1;
        game_state->player.positions[i].y = game_state->player.positions[0].y;
    }

    game_state->new_direction = STATIONARY;
    game_state->apple_position.x = arc4random_uniform(GRID_ROWS);
    game_state->apple_position.y = arc4random_uniform(GRID_COLUMNS);

    game_state->result = IN_PROGRESS;

    game_state->time_since_last_update = 0;
}

int get_score(GameState *game_state) {
    return game_state->player.length - PLAYER_STARTING_LENGTH;
}

void save_score(GameState *game_state) {
    FILE *f = fopen("highscore.txt", "a+");
    fprintf(f, "%d\n", get_score(game_state));
    fclose(f);
}

void end_game(GameState *game_state, GAME_RESULT result) {
    save_score(game_state);
    // Bad string functions and memory management
    {
        FILE *f = fopen("highscore.txt", "r");
        char *line = NULL;
        size_t cap = 0;
        while(getline(&line, &cap, f) != -1) {
            int num = atoi(line);
            for(int i = 0; i < 10; ++i) {
                if(num > game_state->score_top_10[i]) {
                    insert(game_state->score_top_10, 10, num, i);
                    break;
                }
            }
        }
        fclose(f);
        if(line) free(line);
    }
    game_state->result = result;
}

void update_game(GameState *game_state, int grid_x_pos, int grid_y_pos) {
    if(game_state->new_direction == game_state->player.direction) {
        if(IsKeyPressed(KEY_UP)) {
            if(game_state->player.direction != DOWN) game_state->new_direction = UP;
        }
        if(IsKeyPressed(KEY_LEFT)) {
            if(game_state->player.direction != RIGHT) game_state->new_direction = LEFT;
        }
        if(IsKeyPressed(KEY_DOWN)) {
            if(game_state->player.direction != UP) game_state->new_direction = DOWN;
        }
        if(IsKeyPressed(KEY_RIGHT)) {
            if(game_state->player.direction != LEFT) game_state->new_direction = RIGHT;
        }
    }

    game_state->time_since_last_update += GetFrameTime();
    float update_time = 0.15;
    if(game_state->time_since_last_update > update_time) {
        game_state->time_since_last_update = 0;
        game_state->player.direction = game_state->new_direction;

        if(game_state->player.direction != STATIONARY) {
            // NOTE: we're saving the location for one segment longer than the length (at index [length]) so we can have a smooth transition for the tail
            for(int i = game_state->player.length; i > 0; --i) {
                game_state->player.positions[i] = game_state->player.positions[i - 1];
            }
            // NOTE: so that the smooth transition looks right when we increase the length, the next length is kept at the tails position
            game_state->player.positions[game_state->player.length + 1] = game_state->player.positions[game_state->player.length];
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
            case STATIONARY:
                break;
        }
    }

    if(game_state->player.positions[0].x >= GRID_COLUMNS || game_state->player.positions[0].x < 0) {
        end_game(game_state, LOSE);
        return;
    }
    if(game_state->player.positions[0].y >= GRID_ROWS || game_state->player.positions[0].y < 0) {
        end_game(game_state, LOSE);
        return;
    }

    for(int i = 1; i < game_state->player.length; ++i) {
        if(vec_equals(game_state->player.positions[0], game_state->player.positions[i]))
        {
            end_game(game_state, LOSE);
            return;
        }
    }

    if(vec_equals(game_state->player.positions[0], game_state->apple_position)) {
        ++game_state->player.length;
        if(game_state->player.length == GRID_SIZE) {
            end_game(game_state, WIN);
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
        int segment_screen_x = grid_x_pos + game_state->player.positions[i].x * SQUARE_SIZE;
        int segment_screen_y = grid_y_pos + game_state->player.positions[i].y * SQUARE_SIZE;
        Vector2 segment_pos = { segment_screen_x, segment_screen_y };
        Vector2 final_pos = segment_pos;
        if(game_state->player.direction != STATIONARY) {
            int prev_segment_screen_x = grid_x_pos + game_state->player.positions[i + 1].x * SQUARE_SIZE;
            int prev_segment_screen_y = grid_y_pos + game_state->player.positions[i + 1].y * SQUARE_SIZE;
            Vector2 prev_segment_pos = { prev_segment_screen_x, prev_segment_screen_y };

            Vector2 diff = Vector2Subtract(segment_pos, prev_segment_pos);
            Vector2 scaled_diff = Vector2Scale(diff, game_state->time_since_last_update / update_time);
            final_pos = Vector2Add(prev_segment_pos, scaled_diff);
        }
        DrawRectangleV(final_pos, (Vector2){SQUARE_SIZE, SQUARE_SIZE}, RED);
    }

    int buf_size = 16;
    char buf[buf_size];
    snprintf(buf, buf_size, "Score: %d", get_score(game_state));
    DrawText(buf, 30, 60, 20, BLACK);
}

void end_screen(GameState *game_state, char *message) {
    if(IsKeyPressed(KEY_SPACE)) {
        game_state->result = NEW_GAME;
    }
    int font_size = 20;
    int text_width = MeasureText(message, font_size);
    DrawText(message, GetScreenWidth()/2 - text_width/2, GetScreenHeight()/4 - font_size/2, font_size, BLACK);

    char *s = "High scores:";
    text_width = MeasureText(s, font_size);
    DrawText(s, GetScreenWidth()/2 - text_width/2, GetScreenHeight()/3, font_size, BLACK);
    for(int i = 0; i < 10; ++i) {
        char buf[16];
        snprintf(buf, 16, "%d", game_state->score_top_10[i]);
        int text_width = MeasureText(buf, font_size);
        DrawText(buf, GetScreenWidth()/2 - text_width/2, GetScreenHeight()/3 + ((font_size + 2) * (i + 1)), font_size, BLACK);
    }
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
