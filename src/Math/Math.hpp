#pragma once

#include "../Common.hpp"

// Forward declarations
class World;
struct BlockData;

// Axis-Aligned Bounding Box
struct AABB
{
    glm::vec3 minPoint;
    glm::vec3 maxPoint;

    explicit constexpr AABB(const glm::vec3 &minPoint, const glm::vec3 &maxPoint)
        : minPoint(minPoint),
          maxPoint(maxPoint) {}

    static AABB fromBlockPosition(const glm::vec3 &position) { return AABB(position, position + glm::vec3{1, 1, 1}); }

    [[nodiscard]] bool intersect(const AABB &aabb) const
    {
        return (getMinX() <= aabb.getMaxX() && getMaxX() >= aabb.getMinX()) &&
               (getMinY() <= aabb.getMaxY() && getMaxY() >= aabb.getMinY()) &&
               (getMinZ() <= aabb.getMaxZ() && getMaxZ() >= aabb.getMinZ());
    }

    [[nodiscard]] glm::vec3 getMinPoint() const { return minPoint; };
    [[nodiscard]] glm::vec3 getMaxPoint() const { return maxPoint; };

    [[nodiscard]] float getMinX() const { return minPoint.x; };
    [[nodiscard]] float getMinY() const { return minPoint.y; };
    [[nodiscard]] float getMinZ() const { return minPoint.z; };
    [[nodiscard]] float getMaxX() const { return maxPoint.x; };
    [[nodiscard]] float getMaxY() const { return maxPoint.y; };
    [[nodiscard]] float getMaxZ() const { return maxPoint.z; };
};

// Axis-aligned plane intersection
class AxisPlane
{
    glm::vec3 planeNormal;

    float offsetDirection;
    float planeOffset;

    glm::vec3 rayPosition;
    glm::vec3 rayDirection;

    glm::vec3 hitPosition;
    float hitDistance;

    [[nodiscard]] float intersect() const;

    [[nodiscard]] float calculateOffsetDirection(const glm::vec3 &direction) const;
    [[nodiscard]] float calculateStartOffset(const glm::vec3 &position, const glm::vec3 &direction) const;

    [[nodiscard]] float calculateHitDistanceToPosition() const { return glm::distance(rayPosition, hitPosition); }
    [[nodiscard]] glm::vec3 calculateHitPosition() const;

public:
    static std::optional<glm::ivec3> rayHitsToBlockPosition(const glm::vec3 &hit1, const glm::vec3 &hit2);
    AxisPlane(glm::vec3 planeNormal, glm::vec3 rayPosition, glm::vec3 rayDirection);

    [[nodiscard]] glm::vec3 getHitPosition() const { return hitPosition; };
    [[nodiscard]] float getHitDistance() const { return hitDistance; };

    bool operator<(const AxisPlane &other) const { return hitDistance < other.hitDistance; }

    void advanceOffset();
};

// World ray casting result
struct HitTarget
{
    glm::vec3 position;
    const BlockData* block;
    glm::vec3 neighbor;
    bool hasNeighbor = false;
};

// World ray casting
class WorldRayCast
{
    bool successful;
    HitTarget hitTarget;

public:
    WorldRayCast(glm::vec3 position, glm::vec3 direction, World &world, float reach);

    [[nodiscard]] bool hasHit() const { return successful; };
    [[nodiscard]] HitTarget getHitTarget() const { return hitTarget; };
    explicit operator bool() const { return hasHit(); }
};

// Inline implementations for AxisPlane
inline AxisPlane::AxisPlane(glm::vec3 planeNormal, glm::vec3 rayPosition, glm::vec3 rayDirection)
    : planeNormal(planeNormal),
      rayPosition(rayPosition),
      rayDirection(rayDirection)
{
    offsetDirection = calculateOffsetDirection(rayDirection);
    planeOffset = calculateStartOffset(rayPosition, rayDirection);
    hitPosition = calculateHitPosition();
    hitDistance = calculateHitDistanceToPosition();
}

inline glm::vec3 AxisPlane::calculateHitPosition() const
{
    float t = intersect();

    // if the intersection result is less than 0 then it either did not hit or the hit position is behind the cast position
    if (t < 0)
        return glm::vec3(std::numeric_limits<float>::infinity());

    return rayPosition + t * rayDirection;
}

inline float AxisPlane::intersect() const
{
    float d = glm::dot(planeNormal, rayDirection);
    if (d == 0)
        return -std::numeric_limits<float>::infinity(); // the plane and the ray are parallel

    float t = glm::dot(planeNormal, planeNormal * planeOffset - rayPosition);
    float td = t / d;
    return td;
}

inline float AxisPlane::calculateOffsetDirection(const glm::vec3 &direction) const
{
    return glm::dot(planeNormal, direction) < 0 ? -1.0f : 1.0f;
}

inline float AxisPlane::calculateStartOffset(const glm::vec3 &position, const glm::vec3 &direction) const
{
    return std::floor(glm::dot(planeNormal, position)) + (glm::dot(planeNormal, direction) > 0 ? 1.0f : 0.0f);
}

inline void AxisPlane::advanceOffset()
{
    planeOffset += offsetDirection;
    hitPosition = calculateHitPosition();
    hitDistance = calculateHitDistanceToPosition();
}

inline std::optional<glm::ivec3> AxisPlane::rayHitsToBlockPosition(const glm::vec3 &hit1, const glm::vec3 &hit2)
{
    glm::vec3 diff = glm::abs(hit1 - hit2);

    if (diff.x > 1.001f || diff.y > 1.001f || diff.z > 1.001f)
        return std::nullopt;
    return glm::floor((hit1 + hit2) / 2.0f);
}