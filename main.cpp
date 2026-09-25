#include <raylib.h>
#include <vector>

static float speed = 50;
static float speedMod = 240.0f;
float squareSize = 50;
float centerX = ((GetScreenWidth() - squareSize) / 2.0);
float centerY = ((GetScreenHeight() - squareSize) / 2.0);
static int offsetX = 0;
static int offsetY = 0;

void sprintBoost(void) {
  if (IsKeyDown(KEY_RIGHT_SHIFT))
    speed = 2 * speedMod * GetFrameTime();
  if (IsKeyUp(KEY_RIGHT_SHIFT))
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
  Color color;

  Circle(Vector2 pos, float r, Color c) : center(pos), radius(r), color(c) {}

  void Draw() const { DrawCircleV(center, radius, color); }
};

int main() {
  InitWindow(800, 450, "raylib");
  SetTargetFPS(60);

  std::vector<Circle> circles;

  for (int i = 1; i <= 10; i++) {
    float x = 60 + i * 70;
    circles.push_back(Circle({x, 200}, 30, PINK));
  }

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    Rectangle square = {centerX + offsetX, centerY + offsetY, squareSize,
                        squareSize};

    WASD_movement();

    DrawRectangleRec(square, BLACK);
    for (const Circle &c : circles) {
      c.Draw();
    }
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
