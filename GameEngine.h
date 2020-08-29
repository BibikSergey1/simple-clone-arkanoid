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
    void cleanupSprites(const QPixmap& pix);
    void drawSprites(QPainter* p);
    void updateSprites();
    int countSprites(const QPixmap& pix);

    typedef QList<Sprite*>::const_iterator const_iterator;
    const_iterator begin() const { return sprites_.begin(); }
    const_iterator end()   const { return sprites_.end();   }

protected:
    int width_, height_;
    QList<Sprite*> sprites_;

    // Helper Methods
    bool checkSpriteCollision(Sprite* pTestSprite);
};

#endif // GAMEENGINE_H
