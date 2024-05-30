#include <glm/glm.hpp>
#include <iostream>

class CollisionBox
{
    float minX, minY, maxX, maxY;

public:
    CollisionBox(glm::vec2 pos, float min1, float max1, float min2, float max2)
    {
        minX = pos.x - min1;
        minY = pos.y - min2;

        maxX = pos.x + max1;
        maxY = pos.y + max2;
    }

    bool checkCollision(CollisionBox &obj)
    {
        return checkCollisionOnAxis(minX, maxX, obj.minX, obj.maxX) &&
               checkCollisionOnAxis(minY, maxY, obj.minY, obj.maxY);
    }

    bool checkCollisionOnAxis(float min1, float max1, float min2, float max2)
    {
        if (min1 < min2)
        {
            return min2 < max1;
        }
        else
        {
            return min1 < max2;
        }
    }
};