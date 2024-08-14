#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <stdint.h>

#define VEC_ADD(X, Y) ({ (V2){X.x + Y.x, X.y + Y.y}; })
#define VEC_MINUS(X, Y) ({ (V2){X.x - Y.x, X.y - Y.y}; })
#define VEC_SCALAR(X, scalar) ({ (V2){X.x * scalar, X.y * scalar}; })
#define NORMAL_VEC(X) ({ (V2){X.y, -X.x}; })
#define DOT(v0, v1) (v0.x * v1.x + v0.y * v1.y)
#define POW(X) ((X) * (X))
#define LENGTH(v1) (sqrtf(POW(v1.x) + POW(v1.y)))
#define NORMALISE(v1) ({      \
    float l = LENGTH(v1);     \
    (V2){v1.x / l, v1.y / l}; \
})

const int SQUARE_SIZE = 50;

const float PLAYER_SPEED = 0.03;

const int WIDTH = 8;
const int HEIGHT = 8;

uint8_t MAP[] = {1, 1, 1, 1, 1, 1, 1, 1,
                 1, 0, 1, 0, 0, 0, 0, 1,
                 1, 0, 1, 0, 1, 0, 0, 1,
                 1, 0, 1, 0, 1, 0, 0, 1,
                 1, 0, 1, 1, 1, 0, 0, 1,
                 1, 0, 0, 0, 0, 0, 0, 1,
                 1, 0, 0, 0, 0, 0, 0, 1,
                 1, 1, 1, 1, 1, 1, 1, 1};

typedef struct Vec2
{
    float x;
    float y;
} V2;

typedef struct Vec2_i
{
    int x;
    int y;
} V2i;

typedef struct Player
{
    V2 pos;
    float angle;
} Player;

typedef struct RaycastInfo
{
    V2 point;
    float perpDist;
} RaycastInfo;

void draw_map()
{
    for (uint8_t x = 0; x < 8; x++)
    {
        for (uint8_t y = 0; y < 8; y++)
        {

            if (MAP[y * 8 + x] == 0)
            {
                DrawRectangle(x * 50, y * 50, 50, 50, WHITE);
            }
            else
            {
                DrawRectangle(x * 50, y * 50, 50, 50, BLACK);
            }
            DrawLine(x * 50, y * 50, 8 * 50, y * 50, BLACK);
        }
        DrawLine(x * 50, 0, x * 50, 8 * 50, BLACK);
    }
}

void print_vec(V2 vec)
{
    printf("X = %f, Y = %f\n", vec.x, vec.y);
}

V2 rotate_vector(V2 vector, float angle)
{
    return (V2){vector.x * cosf(angle) + vector.y * sinf(angle), -vector.x * sinf(angle) + vector.y * cosf(angle)};
}

void DrawVector(V2 begin, V2 end, Color color)
{
    DrawLine(begin.x * SQUARE_SIZE, begin.y * SQUARE_SIZE, end.x * SQUARE_SIZE, end.y * SQUARE_SIZE, color);
}

RaycastInfo *raycast(Player *player, V2 *newDir, float angle)
{
    V2i map = (V2i){(int)player->pos.x, (int)player->pos.y};
    V2 normPlayer = player->pos;
    V2 dir = (V2){newDir->x, newDir->y};
    dir = NORMALISE(dir);
    V2 sideDist;
    V2 rayUnitStepSize = {sqrtf(1 + POW(dir.y / dir.x)), sqrtf(1 + POW(dir.x / dir.y))};
    V2i step;
    float dist = 0.0;
    int hit = 0;

    if (dir.x < 0)
    {
        step.x = -1;
        sideDist.x = (normPlayer.x - ((float)map.x)) * rayUnitStepSize.x;
    }
    else
    {
        step.x = 1;
        sideDist.x = (((float)map.x + 1.0) - normPlayer.x) * rayUnitStepSize.x;
    }

    if (dir.y < 0)
    {
        step.y = -1;
        sideDist.y = (normPlayer.y - (float)map.y) * rayUnitStepSize.y;
    }
    else
    {
        step.y = 1;
        sideDist.y = (((float)map.y + 1.0) - normPlayer.y) * rayUnitStepSize.y;
    }
    while (hit == 0)
    {
        // jump to next map square, either in x-direction, or in y-direction
        if (sideDist.x < sideDist.y)
        {
            // printf("Advance one x\n");
            dist = sideDist.x;
            sideDist.x += rayUnitStepSize.x;
            map.x += step.x;
        }
        else
        {
            dist = sideDist.y;
            sideDist.y += rayUnitStepSize.y;
            map.y += step.y;
        }

        // Check if ray has hit a wall
        if (MAP[map.y * 8 + map.x] > 0)
        {
            hit = 1;
        }
    }
    RaycastInfo *info = (RaycastInfo *)calloc(1, sizeof(RaycastInfo));
    V2 intersectPoint = VEC_SCALAR(dir, dist);
    info->point = VEC_ADD(normPlayer, intersectPoint);
    return info;
}

int main(int argc, char const *argv[])
{
    char text[500] = {0};
    InitWindow(800, 800, "Raycasting demo");
    Player player = {{2, 2}, 0};
    V2 plane;
    V2 dir;
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {

        if (IsKeyDown(KEY_A))
        {
            player.angle += 0.06;
        }

        if (IsKeyDown(KEY_D))
        {
            player.angle -= 0.05;
        }

        dir = (V2){1, 0};
        plane = NORMALISE(((V2){0.0f, 0.66f}));
        dir = rotate_vector(dir, player.angle);
        plane = rotate_vector(plane, player.angle);
        if (IsKeyDown(KEY_W))
        {
            player.pos = VEC_ADD(player.pos, VEC_SCALAR(dir, PLAYER_SPEED));
        }
        if (IsKeyDown(KEY_S))
        {
            player.pos = VEC_MINUS(player.pos, VEC_SCALAR(dir, PLAYER_SPEED));
        }
        V2 playerDir = VEC_ADD(player.pos, dir);
        V2 point;
        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_map();
        dir = NORMALISE(dir);
        for (int x = 0; x < 8 * SQUARE_SIZE; x++)
        {
            float cameraX = 2 * x / (float)(8 * SQUARE_SIZE) - 1;
            V2 lerp = VEC_SCALAR(plane, cameraX);
            V2 newDir = VEC_ADD(dir, lerp);
            V2 newPlayerDir = VEC_ADD(newDir, player.pos);
            DrawVector(player.pos, point, GREEN);
            point = raycast(&player, &newDir, player.angle);
        }
        snprintf(text, 500, "Player x = %f\nPLayer y = %f", player.pos.x, player.pos.y);
        DrawText(text, SQUARE_SIZE * 8 + 10, 20, 10, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}