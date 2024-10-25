#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

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
#define min(a, b) ({ __typeof__(a) _a = (a), _b = (b); _a < _b ? _a : _b; })
#define max(a, b) ({ __typeof__(a) _a = (a), _b = (b); _a > _b ? _a : _b; })

const int SQUARE_SIZE = 50;

const float PLAYER_SPEED = 0.03;

const int SIZE = 8;

const int WIDTH = SIZE * SQUARE_SIZE;
const int HEIGHT = SIZE * SQUARE_SIZE;

const float texSize = 64;

uint8_t MAP[] = {1, 1, 1, 1, 1, 1, 1, 1,
                 1, 0, 2, 0, 0, 0, 0, 1,
                 1, 0, 3, 0, 4, 0, 0, 1,
                 1, 0, 4, 0, 3, 0, 0, 1,
                 1, 0, 2, 1, 2, 0, 0, 1,
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
    Color color;
    int mapX;
    float wallX;
    int texX;
} RaycastInfo;

void draw_map()
{
    for (uint8_t x = 0; x < 8; x++)
    {
        for (uint8_t y = 0; y < 8; y++)
        {

            if (MAP[y * 8 + x] == 0)
            {
                DrawRectangle(x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, WHITE);
            }
            else
            {
                DrawRectangle(x * SQUARE_SIZE, y * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, BLACK);
            }
            DrawLine(x * SQUARE_SIZE, y * SQUARE_SIZE, SIZE * SQUARE_SIZE, y * SQUARE_SIZE, BLACK);
        }
        DrawLine(x * SQUARE_SIZE, 0, x * SQUARE_SIZE, SIZE * SQUARE_SIZE, BLACK);
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

void DrawRenderScene()
{
    DrawRectangle(0, WIDTH, HEIGHT, HEIGHT, GREEN);
}

RaycastInfo *raycast(Player *player, V2 *newDir, float angle)
{
    V2i map = (V2i){(int)player->pos.x, (int)player->pos.y};
    V2 normPlayer = player->pos;
    V2 dir = (V2){newDir->x, newDir->y};

    V2 rayUnitStepSize = {
        fabsf(dir.x) < 1e-20 ? 1e30 : fabsf(1.0f / dir.x),
        fabsf(dir.y) < 1e-20 ? 1e30 : fabsf(1.0f / dir.y),
    };

    V2 sideDist = {
        rayUnitStepSize.x * (dir.x < 0 ? (normPlayer.x - map.x) : (map.x + 1 - normPlayer.x)),
        rayUnitStepSize.y * (dir.y < 0 ? (normPlayer.y - map.y) : (map.y + 1 - normPlayer.y)),
    };
    V2i step;
    float dist = 0.0;
    int hit = 0;
    int side = 0;

    if (dir.x < 0)
    {
        step.x = -1;
    }
    else
    {
        step.x = 1;
    }

    if (dir.y < 0)
    {
        step.y = -1;
    }
    else
    {
        step.y = 1;
    }
    while (hit == 0)
    {
        if (sideDist.x < sideDist.y)
        {
            dist = sideDist.x;
            sideDist.x += rayUnitStepSize.x;
            map.x += step.x;
            side = 0;
        }
        else
        {
            dist = sideDist.y;
            sideDist.y += rayUnitStepSize.y;
            map.y += step.y;
            side = 1;
        }
        // Check if ray has hit a wall
        hit = MAP[map.y * 8 + map.x];
    }

    RaycastInfo *info = (RaycastInfo *)calloc(1, sizeof(RaycastInfo));
    V2 intersectPoint = VEC_SCALAR(dir, dist);
    info->point = VEC_ADD(normPlayer, intersectPoint);

    // Calculate the perpendicular distance
    if (side == 0){
        info->perpDist = sideDist.x - rayUnitStepSize.x;
    }
    else{
        info->perpDist = sideDist.y - rayUnitStepSize.y;
    }

    //Calculate wallX 
    float wallX;
    if (side == 0) wallX = player->pos.y + info->perpDist * dir.y;
    else wallX = player->pos.x + info->perpDist * dir.x;
    wallX -= floorf(wallX);

    // Calculate texX
    int texX = (int)(wallX * (float)texSize);
    if (side == 0 && dir.x > 0) texX = texSize - texX - 1;
    if (side == 1 && dir.y < 0) texX = texSize - texX - 1;
    texX = texX & (int)(texSize - 1); // Ensure texX is within the valid range

    // Set the calculated values in the hit structure
    info->wallX = wallX;
    info->texX = texX;
    info->mapX = map.x;
    return info;
}

int main(int argc, char const *argv[])
{
   
    char text[500] = {0};
    InitWindow(HEIGHT * 2, HEIGHT * 2, "Raycasting demo");
    Player player = {{1.5, 1.5}, 0};
    V2 plane;
    V2 dir;
    SetTargetFPS(60);
    RaycastInfo *hit = (RaycastInfo *)malloc(sizeof(RaycastInfo));
    Texture2D texture = LoadTexture("./asset/texture/bluestone.png");
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
        plane = NORMALISE(((V2){0.0f, 0.50f}));
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
        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw_map();
        dir = NORMALISE(dir);
        for (int x = 0; x < HEIGHT; x++)
        {
            float cameraX = 2 * x / (float)(8 * SQUARE_SIZE) - 1;
            V2 newDir = VEC_ADD(dir, VEC_SCALAR(plane, cameraX));

            hit = raycast(&player, &newDir, player.angle);
            DrawVector(player.pos, hit->point, GREEN);
                
            int h, y0, y1;
            h = (int)(HEIGHT / hit->perpDist);
            y0 = max((HEIGHT / 2) - (h / 2), 0);
            y1 = min((HEIGHT / 2) + (h / 2), HEIGHT - 1);
            Rectangle source = (Rectangle){
                .x = hit->texX,
                .y = 0,
                .width = 1,
                .height = texture.height
            };
            Rectangle dest = (Rectangle){
                .x = x,
                .y = HEIGHT + y0,
                .width = 1,
                .height = y1 - y0,
            };
             DrawTexturePro(texture, source, dest,(Vector2){0,0},0.0f, RAYWHITE);
            //DrawLine(x, y0 + HEIGHT, x, y1 + HEIGHT, hit->color);
        }
        snprintf(text, 500, "Player x = %f\nPLayer y = %f", player.pos.x, player.pos.y);
        DrawText(text, SQUARE_SIZE * 8 + 10, 20, 10, RED);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}