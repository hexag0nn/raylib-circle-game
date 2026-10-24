#include <cmath>
#include <raylib.h>
#include <string>
#include <unistd.h>
#include <vector>

static constexpr int g_windowRezX = 800;
static constexpr int g_windowRezY = 800;

enum class circleType { BUBBLE, ENEMY, FLY, CHUNGUS };

struct Player {
  Rectangle rect;
  float speed = 500.f;
  static constexpr float NORMAL = 500.f;
  static constexpr float SPRINT = 750.f;
  float size = 20.f;

  Player() { Reset(); }

  void Reset() {
    rect = {(GetScreenHeight() - size) / 2.f, (GetScreenWidth() - size) / 2.f,
            size, size};
    speed = NORMAL;
  };

  Rectangle GetRect() const { return rect; };

  void WASD_movement() {
    speed = IsKeyDown(KEY_SPACE) ? SPRINT : NORMAL;

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

    if (rect.x + moveX * speed * GetFrameTime() >= g_windowRezX) {
      rect.x = size / 2;
      rect.x += moveX * speed * GetFrameTime();
    } else if (rect.x + moveX * speed * GetFrameTime() <= -size) {
      rect.x = g_windowRezX - size / 2;
      rect.x += moveX * speed * GetFrameTime();
    } else
      rect.x += moveX * speed * GetFrameTime();

    // doesnt allow the player to go out of bounds on the Y coordinate
    if (rect.y + moveY * speed * GetFrameTime() >= g_windowRezY - size) {
      rect.y = g_windowRezY - size;
    } else if (rect.y + moveY * speed * GetFrameTime() <= 0) {
      rect.y = 0;
    } else
      rect.y += moveY * speed * GetFrameTime();
  }
};

class Circle {
public:
  float spawnTime;
  Vector2 center;
  float radius;
  float collisionRadius;
  Color color;
  float velocity;
  bool isEnemy;
  circleType type;

  Circle(Vector2 pos, float r, Color c, float vel, circleType t)
      : center(pos), radius(r), color(c), velocity(vel), type(t),
        spawnTime(GetTime()) {

    if (type == circleType::ENEMY) {
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

  bool isThreatening() {
    if (type == circleType::ENEMY || type == circleType::FLY)
      return true;
    else
      return false;
  }
};

// function for making circles
std::vector<Circle> generateCircles(int amount, float minVelocity,
                                    float maxVelocity, int minRadius,
                                    int maxRadius, Color color,
                                    circleType type) {
  std::vector<Circle> batch;
  for (int i = 1; i <= amount; i++) {
    float x = GetRandomValue(0, GetScreenWidth());
    float y = -GetScreenHeight();
    float velocity = GetRandomValue(minVelocity, maxVelocity);
    int radius = GetRandomValue(minRadius, maxRadius);
    batch.push_back(Circle({x, y}, radius, color, velocity, type));
  }
  return batch;
}

// big chunky function to put all circle types in one big array
void initializeAllCircles(std::vector<Circle> &circles) {
  circles.clear();

  std::vector<Circle> bubbles =
      generateCircles(75, 50, 300, 10, 10, PINK, circleType::BUBBLE);
  std::vector<Circle> enemies =
      generateCircles(25, 50, 400, 10, 50, RED, circleType::ENEMY);
  std::vector<Circle> flies =
      generateCircles(100, 10, 60, 5, 5, GREEN, circleType::FLY);

  circles.insert(circles.end(), bubbles.begin(), bubbles.end());
  circles.insert(circles.end(), enemies.begin(), enemies.end());
  circles.insert(circles.end(), flies.begin(), flies.end());
}

void initChungusCircles(std::vector<Circle> &circles) {
  circles.clear();

  std::vector<Circle> chungus =
      generateCircles(1, 450, 500, 30, 45, DARKGRAY, circleType::CHUNGUS);

  circles.insert(circles.end(), chungus.begin(), chungus.end());
}

void launchMainGame() {
  InitAudioDevice();
  Sound bubblePop = LoadSound("universfield-bubble-pop-06-351337.wav");
  Sound death = LoadSound("undertale-death.wav");
  Sound explosion = LoadSound("roblox-explosion-sound.wav");
  SetSoundVolume(explosion, 0.4);
  SetMasterVolume(0.5f);
  Image sonImg = LoadImage("son.png");
  Texture son = LoadTextureFromImage(sonImg);

  Player player;
  player.Reset();

  // initializing all the circles in a vector array called circles
  std::vector<Circle> circles;
  std::vector<Circle> chungusCircles;
  initializeAllCircles(circles);
  initChungusCircles(chungusCircles);

  // other settings to have before the main game loop
  bool isGameOver = false;
  float deathTime = 0;
  float currentTime = 0;
  int score = 0;
  std::string scoreText = "Score: " + std::to_string(score);

  HideCursor();
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    currentTime = GetTime();

    Rectangle square = player.GetRect(); // needed for drawing the rectangle
                                         // (player) at the end of the game loop

    // game loop to run if player is not dead
    if (!isGameOver) {

      player.WASD_movement();

      for (Circle &chungus : chungusCircles) {
        for (Circle &c : circles) {
          if (CheckCollisionCircles(chungus.center, chungus.radius, c.center,
                                    c.radius)) {
            c.Respawn();
          }
        }
        chungus.Update();

        if (score >= 50 && chungus.center.y > g_windowRezY + chungus.radius) {
          chungus.Respawn();
        }
      }

      for (Circle &c : circles) {

        if (CheckCollisionCircleRec(c.center, c.collisionRadius, square) &&
            c.isThreatening()) {
          deathTime = GetTime();
          PlaySound(death);
          isGameOver = true;
          break;
        } else if (CheckCollisionCircleRec(c.center, c.collisionRadius,
                                           square) &&
                   !c.isThreatening()) {
          PlaySound(bubblePop);
          score++;
          c.Respawn();
        }

        bool pulled = false;
        if (!c.isThreatening())
          pulled = c.MagnetizeToObject(
              {square.x + player.size / 2, square.y + player.size / 2}, 100,
              200);

        if (c.type == circleType::FLY)
          pulled = c.MagnetizeToObject(
              {square.x + player.size / 2, square.y + player.size / 2},
              1000, // magnetizes the FLY enemy type
              GetRandomValue(10, 270));

        if (currentTime - c.spawnTime >= 30.f && c.type == circleType::FLY) {
          PlaySound(explosion);
          c.Respawn();
        }

        if (!pulled)
          c.Update();
        if (c.reachedBottom())
          c.Respawn();
      }
    }

    BeginDrawing();
    ClearBackground(WHITE);

    // extra logic that will be run on each turn, regardless if player is dead
    // or not

    for (Circle &chungus : chungusCircles) {
      chungus.Draw();
    }

    for (Circle &c : circles) {
      c.Draw();
      if (c.type == circleType::ENEMY)
        DrawTextureEx(son, {c.center.x - c.radius, c.center.y - c.radius}, 0,
                      c.radius / son.width * 2, RED);
    }

    DrawRectangleRec(square, BLACK);

    if (isGameOver && GetTime() - deathTime >= 0.9f) {
      DrawRectangleRec({15, g_windowRezY / 2.f - 36, 550, 135}, MAROON);
      DrawText("Game Over!", 30, GetRenderHeight() / 2 - 30, 64, BLACK);
      DrawText("Press R to restart the game or\nESCAPE to go to the main menu.",
               30, GetRenderHeight() / 2 + 30, 32, BLACK);
      if (IsKeyPressed(KEY_R)) {
        player.Reset();

        initializeAllCircles(circles);
        initChungusCircles(chungusCircles);
        score = 0;
        isGameOver = false;
      }
    }
    scoreText = "Score: " + std::to_string(score);
    DrawText(scoreText.c_str(), GetScreenWidth() - 200, 20, 32, BLACK);

    EndDrawing();
  }

  UnloadSound(bubblePop);
  UnloadSound(death);
  UnloadSound(explosion);
  UnloadImage(sonImg);
  UnloadTexture(son);
  CloseAudioDevice();
}

void launchMainMenu() {
  bool clickable = false;
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);
    Vector2 textMeasures =
        MeasureTextEx(GetFontDefault(), "Video Game", 114, 16);
    DrawText("Press ESCAPE to exit the game.", 10, g_windowRezY - 20, 20,
             BLACK);
    DrawTextEx(GetFontDefault(), "Video game",
               {g_windowRezX / 2.f - textMeasures.x / 2.f,
                g_windowRezY / 2.f - textMeasures.y},
               114, 16, BLACK);

    Vector2 rectanglePos = {g_windowRezX / 2.f - 300 / 2.f,
                            g_windowRezY / 2.f + 100 / 2.f};

    DrawRectangleRec({g_windowRezX / 2.f - 300 / 2.f,
                      g_windowRezY / 2.f + 100 / 2.f, 300, 100},
                     BLACK);
    DrawTextEx(GetFontDefault(), "Play",
               {g_windowRezX / 2.f - 75.f, g_windowRezY / 2.f + 75}, 48, 16,
               WHITE);

    // stupid function to check if mouse is in button area, need to make a
    // better solution
    if (GetMousePosition().x >= rectanglePos.x &&
        GetMousePosition().y >= rectanglePos.y &&
        GetMousePosition().x <= rectanglePos.x + 300 &&
        GetMousePosition().y <= rectanglePos.y + 100)
      clickable = true;
    else
      clickable = false;

    if (!clickable)
      SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    if (clickable) {
      SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
      if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        launchMainGame();
        ShowCursor();
      }
    }

    if (IsKeyPressed(KEY_SPACE)) {
      launchMainGame();
      ShowCursor();
    }
    EndDrawing();
  }
}

int main() {
  InitWindow(g_windowRezX, g_windowRezY, "raylib game");

  // this whole launching the game mechanism is stupid, i need to make a better
  // one
  launchMainMenu();
  CloseWindow();
  return 0;
}
