#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QWidget>
#include <vector>
#include "sprite.h"

class Game;
class GameEngine
{
public:
    GameEngine(int width = 640, int height = 480);
    virtual ~GameEngine();

    int getWidth() { return width_; }
    int getHeight() { return height_; }

    void addSprite(Sprite* pSprite);
    void cleanupSprites();
    void drawSprites(QPainter* p);
    void updateSprites();

protected:
    int width_, height_;
    QList<Sprite*> sprites_;

    // Helper Methods
    bool checkSpriteCollision(Sprite* pTestSprite);
};

#endif // GAMEENGINE_H
