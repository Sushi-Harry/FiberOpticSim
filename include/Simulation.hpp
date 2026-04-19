#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include "Fiber.hpp"
#include "raymath.h"

class OpticalSimulation{

public:
    float launchAngle = 10.0f;
    std::vector<Vector2> rayPath;
    std::vector<Vector2> escapedRays;
    bool hasEscaped = false;

    void calculatePath(const Fiber& fiber, int screenWidth, int screenHeight){
        rayPath.clear();
        escapedRays.clear();
        hasEscaped = false;

        float currentX = 20.0f;
        float currentY = fiber.getCenterY(currentX, screenHeight);
        Vector2 pos = {currentX, currentY};
        Vector2 dir = { cosf(launchAngle*DEG2RAD), sinf(launchAngle*DEG2RAD)};

        rayPath.push_back(pos);

        for(int step = 0; step < 3000; step++){
            Vector2 nextPos = Vector2Add(pos, Vector2Scale(dir, 1.5f));
            float centerY = fiber.getCenterY(nextPos.x, screenHeight);
            float distFromCenter = nextPos.y - centerY;

            if(distFromCenter > fiber.coreRadius || distFromCenter < -fiber.coreRadius){
                rayPath.push_back(nextPos);

                float slope = (fiber.getCenterY(nextPos.x + 1.0f, screenHeight) - fiber.getCenterY(nextPos.x -1.0f, screenHeight)) / 2.0f;
                Vector2 N = (distFromCenter > 0) ? Vector2{ slope, -1.0f } : Vector2{ -slope, 1.0f};
                N = Vector2Normalize(N);

                Vector2 invDir = Vector2Scale(dir, -1.0f);
                float cos_i = Clamp(Vector2DotProduct(invDir, N), 0.0f, 1.0f);
                float sin_i = sqrtf(1.0f - cos_i * cos_i);

                if(fiber.n1 * sin_i > fiber.n2){
                    float dot = Vector2DotProduct(dir, N);
                    dir.x -= 2.0f * dot * N.x;
                    dir.y -= 2.0f * dot * N.y;
                    dir = Vector2Normalize(dir);
                    nextPos = Vector2Add(nextPos, Vector2Scale(dir, 3.0f));
                }else{
                    escapedRays.push_back(nextPos);
                    escapedRays.push_back(Vector2Add(nextPos, Vector2Scale(dir, 50.0f)));
                    hasEscaped = true;
                    break;
                }
            }

            pos = nextPos;
            if(step % 5 == 0) rayPath.push_back(pos);
            if(pos.x > screenWidth) { rayPath.push_back(pos); break; }
        }
    }
};

#endif