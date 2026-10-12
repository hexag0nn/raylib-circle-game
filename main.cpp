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
  float spawnTime;
  Vector2 center;
  float radius;
  float collisionRadius;
  Color color;
  float velocity;
  bool isEnemy;

  Circle(Vector2 pos, float r, Color c, float vel, bool enemy)
      : center(pos), radius(r), color(c), velocity(vel), isEnemy(enemy),
        spawnTime(GetTime()) {

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
    spawnTime = GetTime();
  }

  bool MagnetizeToObject(Vector2 target, float reqDistance, float pullSpeed) {
    Vector2 toTarget = {target.x - center.x, target.y - center.y};
    float absDistance =
        sqrtf(toTarget.x * toTarget.x + toTarget.y * toTarget.y);

    if (absDistance > 0.0f && absDistance <= reqDistance) {
      center.x += (toTarget.x / absDistance) * pullSpeed * GetFrameTime();
      center.y += (toTarget.y / absDistance) * pullSpeed * GetFrameTime();
      return true;
    }
    return false;
  };
};

std::vector<Circle> generateCircles(int amount, float minVelocity,
                                    float maxVelocity, int minRadius,
                                    int maxRadius, Color color,
                                    bool isEnemyType) {
  std::vector<Circle> batch;
  for (int i = 1; i <= amount; i++) {
    float x = GetRandomValue(0, GetScreenWidth());
    float y = -GetScreenHeight();
    float velocity = GetRandomValue(minVelocity, maxVelocity);
    int radius = GetRandomValue(minRadius, maxRadius);
    batch.push_back(Circle({x, y}, radius, color, velocity, isEnemyType));
  }
  return batch;
}

int main() {
  InitWindow(800, 800, "raylib");
  InitAudioDevice();
  SetMasterVolume(0.5f);
  Sound bubblePop = LoadSound("universfield-bubble-pop-06-351337.wav");
  Sound death = LoadSound("undertale-death.wav");
  Image kandyImg = LoadImage("kandy.png");
  Texture kandy = LoadTextureFromImage(kandyImg);

  std::string scoreText = "Score: " + std::to_string(score);

  float centerX = ((GetScreenWidth() - squareSize) / 2.0);
  float centerY = ((GetScreenHeight() - squareSize) / 2.0);

  SetTargetFPS(60);

  std::vector<Circle> circles;

  std::vector<Circle> bubbles =
      generateCircles(75, 50, 300, 10, 10, PINK, false);
  std::vector<Circle> enemies = generateCircles(25, 50, 400, 10, 50, RED, true);
  std::vector<Circle> flies = generateCircles(100, 10, 60, 5, 5, GREEN, true);

  circles.insert(circles.end(), bubbles.begin(), bubbles.end());
  circles.insert(circles.end(), enemies.begin(), enemies.end());
  circles.insert(circles.end(), flies.begin(), flies.end());

  // for (int i = 1; i <= 75; i++) {
  //   float x = GetRandomValue(0, GetScreenWidth());
  //   float y = -GetScreenHeight();
  //   float velocity = GetRandomValue(50, 300);
  //   circles.push_back(Circle({x, y}, 10, PINK, velocity, false));
  // }

  // for (int i = 1; i <= 25; i++) {
  //   float x = GetRandomValue(0, GetScreenWidth());
  //   float y = -GetScreenHeight() + 250;
  //   float velocity = GetRandomValue(50, 400);
  //   circles.push_back(
  //       Circle({x, y}, GetRandomValue(10, 50), RED, velocity, true));
  // }

  bool isGameOver = false;
  float deathTime = 0;
  float lastTime = 0;
  float currentTime = 0;

  while (!WindowShouldClose()) {
    currentTime = GetTime();
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

        bool pulled = false;
        if (!c.isEnemy)
          pulled = c.MagnetizeToObject(
              {square.x + squareSize / 2, square.y + squareSize / 2}, 100, 200);

        if (c.isEnemy && c.radius <= 5)
          pulled = c.MagnetizeToObject(
              {square.x + squareSize / 2, square.y + squareSize / 2}, 1000,
              GetRandomValue(10, 270));

        if (currentTime - c.spawnTime >= 30.f && c.isEnemy && c.radius <= 5) {
          c.Respawn();
        }

        if (!pulled)
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
      if (c.isEnemy && c.radius > 5)
        DrawTextureEx(kandy, {c.center.x - c.radius, c.center.y - c.radius}, 0,
                      c.radius / kandy.width * 2, RED);
    }
    if (isGameOver && GetTime() - deathTime >= 0.9f) {
      DrawText("Game Over!", 30, GetRenderHeight() / 2 - 30, 64, BLACK);
      DrawText("Press R to restart the game.", 30, GetRenderHeight() / 2 + 30,
               32, BLACK);
      if (IsKeyPressed(KEY_R)) {
        offsetX = 0;
        offsetY = 0;
        circles.clear();
        std::vector<Circle> bubbles =
            generateCircles(75, 50, 300, 10, 10, PINK, false);
        std::vector<Circle> enemies =
            generateCircles(25, 50, 400, 10, 50, RED, true);
        std::vector<Circle> flies =
            generateCircles(100, 10, 60, 5, 5, GREEN, true);

        circles.insert(circles.end(), bubbles.begin(), bubbles.end());
        circles.insert(circles.end(), enemies.begin(), enemies.end());
        circles.insert(circles.end(), flies.begin(), flies.end());

        score = 0;
        isGameOver = false;
      }
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
