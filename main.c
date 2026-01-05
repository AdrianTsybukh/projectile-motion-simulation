#include <raylib.h>
#include <raymath.h>
#include <string.h>

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600
#define BALL_RADIUS 50
#define BALL_COLOR YELLOW
#define FPS 60
#define MAX_OBSTACLES 100

typedef struct {
  Vector2 position;
  Vector2 velocity;
} Ball;

typedef struct {
  Rectangle rect;
  Color color;
} Obstacle;

Vector2 CheckObstacleCollision(Obstacle obstacle, Ball ball) {
  Vector2 closestPoint = {
    .x = Clamp(ball.position.x, obstacle.rect.x, obstacle.rect.x + obstacle.rect.width),
    .y = Clamp(ball.position.y, obstacle.rect.y, obstacle.rect.y + obstacle.rect.height),
  };

  Vector2 normal = Vector2Subtract(ball.position, closestPoint);
  float distance = Vector2Length(normal);

  if (distance < BALL_RADIUS) {
    if (distance <= 0.0001f) return (Vector2){ 0, -1 };

    return Vector2Scale(normal, 1.0f / distance);
  }

  return (Vector2){ 0, 0 };
}

int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Projectile Simulation");
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetTargetFPS(FPS);

  Ball ball = {0};

  Obstacle map[MAX_OBSTACLES] = {
    {{ 400, 300, 200, 40 }, GRAY},
    {{ 200, 450, 100, 100 }, GRAY}
  };
  int obstacleCount = 2;
  Vector2 obstacleStartPos =  {0};

  bool isDragging = false;
  bool isDraggingRight = false;
  bool isBallMoving = false;
  bool isBallSpawned = false;


  while(!WindowShouldClose()) {
    Vector2 mousePos = GetMousePosition();
    float deltaTime = GetFrameTime();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      isBallMoving = false;
      ball.position = mousePos;
      isDragging = true;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      isDragging = false;
      isBallSpawned = true;
      isBallMoving = true;
      ball.velocity = Vector2Scale(Vector2Subtract(ball.position, mousePos), 5);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
      obstacleStartPos = mousePos;
      isDraggingRight = true;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
      isDraggingRight = false;
      if (obstacleCount < MAX_OBSTACLES) {
        float x = fminf(obstacleStartPos.x, mousePos.x);
        float y = fminf(obstacleStartPos.y, mousePos.y);

        float w = fabsf(mousePos.x - obstacleStartPos.x);
        float h = fabsf(mousePos.y - obstacleStartPos.y);

        if (w > 5 && h > 5) {
	  map[obstacleCount] = (Obstacle){{ x, y, w, h }, GRAY};
	  obstacleCount++;
        }
      }
    }

    if (isBallMoving) {
      ball.velocity.y += 980.0f * deltaTime;
      ball.position = Vector2Add(ball.position, Vector2Scale(ball.velocity, deltaTime));

      Vector2 normal = { 0, 0 };
      bool hit = false;

      if (ball.position.y + BALL_RADIUS > GetScreenHeight()) {
        normal = (Vector2){ 0, -1 };
        ball.position.y = GetScreenHeight() - BALL_RADIUS;
        hit = true;
      }
      else if (ball.position.y - BALL_RADIUS < 0) {
        normal = (Vector2){ 0, 1 };
        ball.position.y = BALL_RADIUS;
        hit = true;
      }

      if (ball.position.x + BALL_RADIUS > GetScreenWidth()) {
        normal = (Vector2){ -1, 0 };
        ball.position.x = GetScreenWidth() - BALL_RADIUS;
        hit = true;
      }
      else if (ball.position.x - BALL_RADIUS < 0) {
        normal = (Vector2){ 1, 0 };
        ball.position.x = BALL_RADIUS;
        hit = true;
      }

      for (int i = 0; i < obstacleCount; i++) {
	Vector2 closestPoint = {
	  .x = Clamp(ball.position.x, map[i].rect.x, map[i].rect.x + map[i].rect.width),
	  .y = Clamp(ball.position.y, map[i].rect.y, map[i].rect.y + map[i].rect.height),
	};

	Vector2 diff = Vector2Subtract(ball.position, closestPoint);
	float currentDist = Vector2Length(diff);

	if (currentDist < BALL_RADIUS) {
	  Vector2 n;
	  if (currentDist <= 0.0001f) n = (Vector2){ 0, -1 };
	  else n = Vector2Scale(diff, 1.0f / currentDist);

	  float penetration = BALL_RADIUS - currentDist;
	  ball.position = Vector2Add(ball.position, Vector2Scale(n, penetration));

	  ball.velocity = Vector2Reflect(ball.velocity, n);
	  ball.velocity = Vector2Scale(ball.velocity, 0.75f);
	}
      }

      if (hit) {
        ball.velocity = Vector2Reflect(ball.velocity, normal);
        ball.velocity = Vector2Scale(ball.velocity, 0.75f);

        if (Vector2Length(ball.velocity) < 40.0f) {
	  ball.velocity = (Vector2){ 0, 0 };
	  isBallMoving = false;
        }
      }
    }

    BeginDrawing();
    ClearBackground(DARKGRAY);
    DrawFPS(10, 10);


    if (isDragging) {
      DrawLineEx(ball.position, mousePos, 5.0f, LIGHTGRAY);
      DrawCircleV(ball.position, BALL_RADIUS, BALL_COLOR);
      DrawCircleV(mousePos, 5, WHITE);
    }

    if (isBallSpawned) {
      DrawCircleV(ball.position, BALL_RADIUS, BALL_COLOR);
    }

    if (isDraggingRight) {
      float x = fminf(obstacleStartPos.x, mousePos.x);
      float y = fminf(obstacleStartPos.y, mousePos.y);
      float w = fabsf(mousePos.x - obstacleStartPos.x);
      float h = fabsf(mousePos.y - obstacleStartPos.y);

      DrawRectangleLinesEx((Rectangle){ x, y, w, h }, 2, WHITE);
    }

    for(int i = 0; i < obstacleCount; i++) {
      DrawRectangleRec(map[i].rect, map[i].color);
    }

    if (IsKeyPressed(KEY_R)) {
      obstacleCount = 0;
    }

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
