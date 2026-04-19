#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "Simulation.hpp"
#include <algorithm> // For std::min

class Renderer{
public:
    void DrawScene(const Fiber& fiber, const OpticalSimulation& sim, int screenWidth, int screenHeight ){
        // Drawing the fiber
        for(float x = 0.0f; x < screenWidth; x+= 2.0f){
            float y = fiber.getCenterY(x, screenHeight);
            DrawRectangle(x, y - fiber.coreRadius, 3, fiber.coreRadius * 2, Fade(BLUE, 0.2f)); 
            DrawRectangle(x, y - fiber.coreRadius - fiber.cladThickness, 3, fiber.cladThickness, Fade(DARKGRAY, 0.4f)); 
            DrawRectangle(x, y + fiber.coreRadius, 3, fiber.cladThickness, Fade(DARKGRAY, 0.4f)); 
            DrawPixel(x, y - fiber.coreRadius, SKYBLUE); 
            DrawPixel(x, y + fiber.coreRadius, SKYBLUE);
        }

        // Drawing the spline nodes
        if(fiber.curved){
            for(const auto& node: fiber.nodes){
                DrawCircleV(node, 10.0f, ORANGE);
                DrawCircleLines(node.x, node.y, 10.0f, WHITE);
            }
        }

        // Drawing the escaped rays
        for(size_t i = 0; i < sim.escapedRays.size(); i+=2){
            DrawLineEx(sim.escapedRays[i], sim.escapedRays[i+1], 2.0f, Fade(RED, 0.6f));
            DrawCircleV(sim.escapedRays[i], 4.0f, RED);
        }

        // Draw TIR ray path
        for(size_t i = 1; i < sim.rayPath.size(); i++){
            DrawLineEx(sim.rayPath[i-1], sim.rayPath[i], 2.5f, LIME);
        }
    }

    void DrawUI(const Fiber& fiber, const OpticalSimulation& sim) {
        int uiX = 20;
        DrawRectangle(uiX, 20, 360, 280, Fade(BLACK, 0.8f));
        DrawRectangleLines(uiX, 20, 360, 280, SKYBLUE);
        
        DrawText("FIBER OPTICS SIMULATOR", uiX + 15, 35, 20, SKYBLUE);
        DrawLine(uiX + 15, 60, uiX + 345, 60, SKYBLUE);
        
        DrawText(TextFormat("Launch Angle    : %.1f deg", fabsf(sim.launchAngle)), uiX + 15, 75, 16, RAYWHITE);
        DrawText(TextFormat("Core Index (n1) : %.3f", fiber.n1), uiX + 15, 95, 16, RAYWHITE);
        DrawText(TextFormat("Clad Index (n2) : %.3f", fiber.n2), uiX + 15, 115, 16, RAYWHITE);
        
        DrawText(TextFormat("Critical Angle  : %.1f deg", fiber.getCriticalAngle()), uiX + 15, 150, 14, GRAY);
        DrawText(TextFormat("Num. Aperture   : %.3f", fiber.getNumericalAperture()), uiX + 15, 170, 14, GRAY);
        DrawText(TextFormat("Acceptance Ang. : %.1f deg", fiber.getAcceptanceAngle()), uiX + 15, 190, 14, GRAY);

        if (sim.hasEscaped) DrawText("STATUS: SIGNAL LOST (Refraction)", uiX + 15, 225, 16, RED);
        else DrawText("STATUS: TOTAL INTERNAL REFLECTION", uiX + 15, 225, 16, GREEN);

        DrawText("UP/DOWN: Angle | W/S: n1 | E/D: n2 | P: Save", uiX, 260, 12, GRAY);
        if (fiber.curved) DrawText("C: Straighten | MOUSE: Drag Orange Nodes", uiX, 280, 12, ORANGE);
        else DrawText("C: Toggle Curved Fiber", uiX, 280, 12, GRAY);
    }
};

class Application {
private:
    const int virtualWidth = 1200;
    const int virtualHeight = 800;

    int screenWidth = 1920;
    int screenHeight = 1080;
    
    Fiber fiber;
    OpticalSimulation sim;
    Renderer renderer;
    
    int draggedNodeIndex = -1; 

    void HandleInput(Vector2 virtualMouse) {
        if (IsKeyDown(KEY_UP)) sim.launchAngle -= 0.5f;
        if (IsKeyDown(KEY_DOWN)) sim.launchAngle += 0.5f;
        
        if (IsKeyDown(KEY_W)) fiber.n1 += 0.005f;
        if (IsKeyDown(KEY_S)) fiber.n1 -= 0.005f;
        if (IsKeyDown(KEY_E)) fiber.n2 += 0.005f;
        if (IsKeyDown(KEY_D)) fiber.n2 -= 0.005f;
        
        if (IsKeyPressed(KEY_C)) fiber.curved = !fiber.curved;
        if (IsKeyPressed(KEY_P)) TakeScreenshot("Fiber_Optics_Result.png");

        fiber.PhysConstraints();

        // --- Drag & Drop Node Logic ---
        if (fiber.curved) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                for (size_t i = 0; i < fiber.nodes.size(); i++) {
                    if (CheckCollisionPointCircle(virtualMouse, fiber.nodes[i], 15.0f)) {
                        draggedNodeIndex = i;
                        break;
                    }
                }
            }
            
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                draggedNodeIndex = -1;
            }
            
            if (draggedNodeIndex != -1) {
                fiber.nodes[draggedNodeIndex].y = virtualMouse.y;
                if (fiber.nodes[draggedNodeIndex].y < 50) fiber.nodes[draggedNodeIndex].y = 50;
                if (fiber.nodes[draggedNodeIndex].y > virtualHeight - 50) fiber.nodes[draggedNodeIndex].y = virtualHeight - 50;
            }
        }
    }

public:
    void Run() {
        SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
        InitWindow(screenWidth, screenHeight, "Fiber Optics Simulator");
        SetTargetFPS(60);

        Camera2D camera = { 0 };
        camera.rotation = 0.0f;

        while (!WindowShouldClose()) {
            // 1. Calculate uniform scale (Letterboxing) to maintain exact aspect ratio
            float scale = std::min((float)GetScreenWidth() / virtualWidth, (float)GetScreenHeight() / virtualHeight);

            // 2. Setup Camera for Native Vector Scaling
            camera.zoom = scale;
            camera.offset = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
            camera.target = { virtualWidth / 2.0f, virtualHeight / 2.0f };

            // 3. Perfect Mouse Mapping via Matrix Transform
            Vector2 virtualMouse = GetScreenToWorld2D(GetMousePosition(), camera);

            HandleInput(virtualMouse);
            sim.calculatePath(fiber, virtualWidth, virtualHeight);

            BeginDrawing();
                // Fill the dead space (letterbox borders) with true black
                ClearBackground(BLACK); 

                BeginMode2D(camera);
                    // Draw the actual lab background strictly inside the virtual bounds
                    DrawRectangle(0, 0, virtualWidth, virtualHeight, GetColor(0x0A0F14FF));

                    renderer.DrawScene(fiber, sim, virtualWidth, virtualHeight);
                    renderer.DrawUI(fiber, sim);
                EndMode2D();
            EndDrawing();
        }
        CloseWindow();
    }
};

#endif