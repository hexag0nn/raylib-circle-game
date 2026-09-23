#include <raylib.h>

static float speed = 50;
float squareSize = 50;
float centerX = ((GetScreenWidth() - squareSize) / 2.0);
float centerY = ((GetScreenHeight() - squareSize) / 2.0);
static int offsetX = 0;
static int offsetY = 0;
Rectangle square = {centerX + offsetX, centerY + offsetY, squareSize,
                    squareSize};
void sprintBoost(void) {
  if (IsKeyDown(KEY_RIGHT_SHIFT))
    speed = 480.0f * GetFrameTime();
  if (IsKeyUp(KEY_RIGHT_SHIFT))
    speed = 240.0f * GetFrameTime();
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

int main() {
  InitWindow(800, 450, "raylib");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    Rectangle square = {centerX + offsetX, centerY + offsetY, squareSize,
                        squareSize};

    WASD_movement();

    DrawRectangleRec(square, BLACK);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
