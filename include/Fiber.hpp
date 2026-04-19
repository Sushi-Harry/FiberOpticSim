#ifndef FIBER_HPP
#define FIBER_HPP

#include "raylib.h"
#include <vector>
#include <cmath>

class Fiber{
public:
    float coreRadius = 30.0f;
    float cladThickness = 15.0f;
    float n1 = 1.480f;
    float n2 = 1.460f;
    bool curved = false;

    std::vector<Vector2> nodes;

    Fiber(){
        // i< 5 because I'm adding 5 default curve control nodes
        for(int i = 0; i < 5; i++){
            nodes.push_back({i * 300.0f, 400.0f});
        }
    }

    void PhysConstraints(){
        if(n1 < 1.0f) n1 = 1.0f;
        if(n2 < 1.0f) n1 = 1.0f;
        if(n2 > n1) n2 = n1;
    }

    // Catmull Rom Spline interpolation thing. Not sure how this work, But I just applied the basic formula so yeah...
    float CatmullRom_Spline(float p0, float p1, float p2, float p3, float t) const {
        float t2 = t * t;
        float t3 = t2 * t;
        return 0.5f * ((2.0f * p1) + (-p0 + p2) * t + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 + (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
    }

    // Calculating the y position of the draggable fibre
    float getCenterY(float x, float screenHeight) const {
        if(!curved) return screenHeight/ 2.0f;
        float interval = 300.0f;
        float i = (int)(x / interval);
        float t = (x - (i*interval)) / interval;

        if(i < 0) return nodes[0].y;
        if(i >= (int)nodes.size() - 1) return nodes.back().y;

        // Getting data for the surrounding 4 nodes for cubic interpolation
        float p0 = (i - 1 >= 0) ? nodes[i-1].y : nodes[i].y;
        float p1 = nodes[i].y;
        float p2 = nodes[i+1].y;
        float p3 = (i + 2 < (int)nodes.size()) ? nodes[i+2].y : nodes[i+1].y;

        return CatmullRom_Spline(p0, p1, p2, p3, t);
    }

    float getCriticalAngle() const { return asinf(n2 / n1) * RAD2DEG; }
    float getNumericalAperture() const { return sqrtf(n1 * n1 - n2 * n2); }
    float getAcceptanceAngle() const { return asinf(getNumericalAperture()) * RAD2DEG; }
};

#endif