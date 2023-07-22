#pragma once

#include "bounding_box.h"
#include "plane.h"
#include <iostream>

struct Frustum {
public:
    Plane planes[6];
    enum {
        LeftFace = 0,
        RightFace = 1,
        BottomFace = 2,
        TopFace = 3,
        NearFace = 4,
        FarFace = 5
    };

    bool intersect(const BoundingBox& aabb, const glm::mat4& modelMatrix) const {
        // TODO: judge whether the frustum intersects the bounding box
        // Note: this is for Bonus project 'Frustum Culling'
        // write your code here
        // ------------------------------------------------------------
        // return true;
        // ------------------------------------------------------------

        glm::vec3 point;

        // for 2 x 2 x 2 points
        for (int i = 0; i < 2; ++i) {
            point.x = i ? aabb.max.x : aabb.min.x;
            for (int j = 0; j < 2; ++j) {
                point.y = j ? aabb.max.y : aabb.min.y;
                for (int k = 0; k < 2; ++k) {
                    point.z = k ? aabb.max.z : aabb.min.z;
                    // change the point from model space to world space
                    point = glm::vec3(modelMatrix * glm::vec4(point, 1.0f));
                    // for all planes
                    if (planes[0].getSignedDistanceToPoint(point) > 0.0f
                        && planes[1].getSignedDistanceToPoint(point) > 0.0f
                        && planes[2].getSignedDistanceToPoint(point) > 0.0f
                        && planes[3].getSignedDistanceToPoint(point) > 0.0f
                        && planes[4].getSignedDistanceToPoint(point) > 0.0f
                        && planes[5].getSignedDistanceToPoint(point) > 0.0f) {
                        return true;
                    }
                }
            }
        }

        return false;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Frustum& frustum) {
    os << "frustum: \n";
    os << "planes[Left]:   " << frustum.planes[0] << "\n";
    os << "planes[Right]:  " << frustum.planes[1] << "\n";
    os << "planes[Bottom]: " << frustum.planes[2] << "\n";
    os << "planes[Top]:    " << frustum.planes[3] << "\n";
    os << "planes[Near]:   " << frustum.planes[4] << "\n";
    os << "planes[Far]:    " << frustum.planes[5] << "\n";

    return os;
}