#include <raylib.h>
#include <vector>

static float speed = 50;
static float speedMod = 240.0f;
float squareSize = 20;
static int offsetX = 0;
static int offsetY = 0;

void sprintBoost(void) {
  if (IsKeyDown(KEY_SPACE))
    speed = 2 * speedMod * GetFrameTime();
  if (IsKeyUp(KEY_SPACE))
    speed = speedMod * GetFrameTime();
}

void WASD_movement(void) {
  sprintBoost();
  if (IsKeyDown(KEY_W)) {
    offsetY -= speed;
  }
  if (IsKeyDown(KEY_A)) {
    offsetX -= speed;
  }
  if (IsKeyDown(KEY_S)) {
    offsetY += speed;
  }
  if (IsKeyDown(KEY_D)) {
    offsetX += speed;
  }
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
  Image kandyImg = LoadImage("kandy.png");
  Texture kandy = LoadTextureFromImage(kandyImg);

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

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    Rectangle square = {centerX + offsetX, centerY + offsetY, squareSize,
                        squareSize};

    WASD_movement();

    for (Circle &c : circles) {
      c.Update();
      if (c.reachedBottom())
        c.Respawn();
      c.Draw();

      if (c.isEnemy)
        DrawTextureEx(kandy, {c.center.x - c.radius, c.center.y - c.radius}, 0,
                      c.radius / kandy.width * 2, RED);

      if (CheckCollisionCircleRec(c.center, c.collisionRadius, square) &&
          c.isEnemy) {
        CloseWindow();
        return 0;
      } else if (CheckCollisionCircleRec(c.center, c.collisionRadius, square)) {
        PlaySound(bubblePop);
        c.Respawn();
      }
    }

    DrawRectangleRec(square, BLACK);

    EndDrawing();
  }

  UnloadSound(bubblePop);
  UnloadImage(kandyImg);
  UnloadTexture(kandy);
  CloseAudioDevice();
  CloseWindow();
  return 0;
}
