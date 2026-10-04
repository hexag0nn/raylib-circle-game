#include <cmath>
#include <cstdlib>
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
    center.y = -GetScreenHeight() + 250; // slightly closer to the screen border
    center.x = GetRandomValue(0, GetScreenWidth());
  }

  void MagnetizeToObject(Rectangle player, float reqDistance) {
    float absDistance = sqrtf(pow(std::abs(center.x - player.x), 2) +
                              pow(std::abs(center.y - player.y), 2));

    float distanceX = center.x - player.x; // negative if circle is on the left,
    float distanceY = center.y - player.y; // negative if circle is higher up.

    if (absDistance <= reqDistance) {
      if (distanceX > 0 && distanceY > 0) { // circle is in bottom right corner
        center = {center.x - 100 * GetFrameTime(),
                  center.y - 250 * GetFrameTime()};
      } else if (distanceX < 0 &&
                 distanceY < 0) { // circle is in top left corner
        center = {center.x + 100 * GetFrameTime(),
                  center.y + 100 * GetFrameTime()};
      } else if (distanceX > 0 &&
                 distanceY < 0) { // circle is in top right corner
        center = {center.x - 100 * GetFrameTime(),
                  center.y + 100 * GetFrameTime()};
      } else { // circle is in bottom left corner
        center = {center.x + 100 * GetFrameTime(),
                  center.y - 250 * GetFrameTime()};
      }
      // 100 is a magic number, its here
      // to make it so the things dont
      // move at an abysmal speed.
    }
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
  float deathTime = 0;

  while (!WindowShouldClose()) {
    Rectangle square = {centerX + offsetX, centerY + offsetY, squareSize,
                        squareSize}; // in this case, he is the player that
                                     // will be controlled.
    if (!isGameOver) {

      for (Circle &c : circles) {

        if (CheckCollisionCircleRec(c.center, c.collisionRadius, square) &&
            c.isEnemy) {
          deathTime = GetTime();
          PlaySound(death);
          isGameOver = true;
          break;
        } else if (CheckCollisionCircleRec(c.center, c.collisionRadius,
                                           square)) {
          PlaySound(bubblePop);
          score++;
          c.Respawn();
        }

        if (!c.isEnemy)
          c.MagnetizeToObject(square, 200);

        c.Update();
        if (c.reachedBottom())
          c.Respawn();
      }
      WASD_movement();
    }

    BeginDrawing();
    ClearBackground(WHITE);

    for (Circle &c : circles) {
      c.Draw();
      if (c.isEnemy)
        DrawTextureEx(kandy, {c.center.x - c.radius, c.center.y - c.radius}, 0,
                      c.radius / kandy.width * 2, RED);
    }
    if (isGameOver && GetTime() - deathTime >= 0.9f) {
      DrawText("Game Over!", 30, GetRenderHeight() / 2 - 30, 64, BLACK);
    }

    DrawRectangleRec(square, BLACK);

    scoreText = "Score: " + std::to_string(score);
    DrawText(scoreText.c_str(), GetScreenWidth() - 200, 20, 32, BLACK);

    EndDrawing();
  }

  UnloadSound(bubblePop);
  UnloadImage(kandyImg);
  UnloadTexture(kandy);
  CloseAudioDevice();
  CloseWindow();
  return 0;
}
