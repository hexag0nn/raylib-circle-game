#include <cmath>
#include <raylib.h>
#include <string>
#include <unistd.h>
#include <vector>

static float speed = 500.f;
// static float speedMod = 2.0f;
float squareSize = 20;
static float offsetX = 0;
static float offsetY = 0;
static int score = 0;

void sprintBoost(void) {
  if (IsKeyDown(KEY_SPACE))
    speed = 750.f;
  if (IsKeyUp(KEY_SPACE))
    speed = 500.f;
}

void WASD_movement(void) {
  sprintBoost();

  float moveX = 0.f;
  float moveY = 0.f;

  if (IsKeyDown(KEY_W))
    moveY -= 1.0f;

  if (IsKeyDown(KEY_A))
    moveX -= 1.0f;

  if (IsKeyDown(KEY_S))
    moveY += 1.0f;

  if (IsKeyDown(KEY_D))
    moveX += 1.0f;

  float length = sqrtf(pow(moveX, 2) + pow(moveY, 2));

  if (length > 1.0f) {
    moveX /= length;
    moveY /= length;
  }

  offsetX += moveX * speed * GetFrameTime();
  offsetY += moveY * speed * GetFrameTime();
}

class Circle {
public:
  Vector2 center;
  float radius;
  float collisionRadius;
  Color color;
  float velocity;
  bool isEnemy;

  Circle(Vector2 pos, float r, Color c, float vel, bool enemy)
      : center(pos), radius(r), color(c), velocity(vel), isEnemy(enemy) {

    if (isEnemy) {
      collisionRadius = r - 10.f;
    } else {
      collisionRadius = r;
    }
  }

  void Draw() const { DrawCircleV(center, radius, color); }

  void Update(void) { center.y += velocity * GetFrameTime(); }

  bool reachedBottom(void) const {
    return center.y - radius > GetScreenHeight();
  }

  void Respawn() {
    center.y = -GetScreenHeight() + 250;
    center.x = GetRandomValue(0, GetScreenWidth());
  }
};

int main() {
  InitWindow(800, 800, "raylib");
  InitAudioDevice();
  SetMasterVolume(1);
  Sound bubblePop = LoadSound("universfield-bubble-pop-06-351337.wav");
  Sound death = LoadSound("undertale-death.wav");
  Image kandyImg = LoadImage("kandy.png");
  Texture kandy = LoadTextureFromImage(kandyImg);

  std::string scoreText = "Score: " + std::to_string(score);

  float centerX = ((GetScreenWidth() - squareSize) / 2.0);
  float centerY = ((GetScreenHeight() - squareSize) / 2.0);

  SetTargetFPS(60);

  std::vector<Circle> circles;

  for (int i = 1; i <= 75; i++) {
    float x = GetRandomValue(0, GetScreenWidth());
    float y = -GetScreenHeight();
    float velocity = GetRandomValue(50, 300);
    circles.push_back(Circle({x, y}, 15, PINK, velocity, false));
  }

  for (int i = 1; i <= 25; i++) {
    float x = GetRandomValue(0, GetScreenWidth());
    float y = -GetScreenHeight() + 250;
    float velocity = GetRandomValue(50, 400);
    circles.push_back(
        Circle({x, y}, GetRandomValue(10, 50), RED, velocity, true));
  }

  bool isGameOver = false;

  while (!WindowShouldClose()) {
    Rectangle square = {centerX + offsetX, centerY + offsetY, squareSize,
                        squareSize};
    if (!isGameOver) {
      WASD_movement();

      for (Circle &c : circles) {
        c.Update();
        if (c.reachedBottom())
          c.Respawn();

        if (CheckCollisionCircleRec(c.center, c.collisionRadius, square) &&
            c.isEnemy) {
          PlaySound(death);
          usleep(900000); // 0.9 seconds
          BeginDrawing();
          isGameOver = true;
        } else if (CheckCollisionCircleRec(c.center, c.collisionRadius,
                                           square)) {
          PlaySound(bubblePop);
          score++;
          c.Respawn();
        }
      }
    }

    BeginDrawing();
    ClearBackground(WHITE);

    DrawRectangleRec(square, BLACK);

    scoreText = "Score: " + std::to_string(score);
    DrawText(scoreText.c_str(), GetScreenWidth() - 200, 20, 32, BLACK);

    EndDrawing();

    for (Circle &c : circles) {
      c.Draw();
      if (c.isEnemy)
        DrawTextureEx(kandy, {c.center.x - c.radius, c.center.y - c.radius}, 0,
                      c.radius / kandy.width * 2, RED);
    }
    if (isGameOver) {
      DrawText("Game Over!", 30, GetRenderHeight() / 2 - 30, 64, BLACK);
    }
  }

  UnloadSound(bubblePop);
  UnloadImage(kandyImg);
  UnloadTexture(kandy);
  CloseAudioDevice();
  CloseWindow();
  return 0;
}
