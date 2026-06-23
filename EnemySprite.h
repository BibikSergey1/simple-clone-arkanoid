#pragma once

#include "Sprite.h"

class Game;

class EnemySprite : public Sprite
{
public:
    EnemySprite(Game* game, QPixmap& pixmap, QPixmap& pixmap_m, QRect& rcBounds, BOUNDSACTION baBoundsAction = BA_STOP);
    virtual ~EnemySprite();

    // General Methods
    virtual SPRITEACTION  update();
    virtual Sprite*       addSprite();

private:
    Game* m_game;
    QPixmap& m_batPixmap;
    QPixmap& m_missilePixmap;

};
