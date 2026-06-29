#include "EnemySprite.h"
#include "widget.h"
#include "game.h"

#include <QDebug>

EnemySprite::EnemySprite(Game* game, QPixmap& pixmap, QPixmap& pixmap_m, QRect& rcBounds, BOUNDSACTION baBoundsAction)
    : Sprite(pixmap, rcBounds, baBoundsAction)
    , m_game(game)
    , m_batPixmap(pixmap)
    , m_missilePixmap(pixmap_m)
{

}

EnemySprite::~EnemySprite()
{
}

SPRITEACTION EnemySprite::update()
{
    SPRITEACTION sa = Sprite::update();

    // Если горизонтальная скорость равна нулю, задаём случайную ненулевую
    if (getVelocity().x() == 0)
    {
        int newVx = (rand() % 2 == 0) ? 1 : -1;
        setVelocity(newVx, getVelocity().y());
    }

    if (m_game && m_game->isGameActive() && m_game->enemyExists())
    {
        // 180	3 сек	исходно
        // 300	5 сек	реже
        // 600	10 сек	значительно реже
        // 900	15 сек	очень редко
        if ((rand() % 300) == 0)
            sa = SA_ADDSPRITE;
    }
    return sa;
}

Sprite* EnemySprite::addSprite()
{
    QRect rcBounds = QRect(BALL_MARGIN_LEFT,
                           BLOCK_SIZE.height() * 2 - UP_MARGIN,
                           WINDOW_WIDTH-BALL_MARGIN_RIGHT,
                           WINDOW_HEIGHT - 1);
    QRect rcPos = getPosition();

    Sprite* pSprite = nullptr;
    if (getPixmap().toImage() == m_batPixmap.toImage())
    {
        pSprite = new Sprite(m_missilePixmap, rcBounds, BA_DIE);
        // Текущий	    2 + rand() % 7	2..8	Исходный
        // Умеренный	2 + rand() % 4	2..5	Максимум 5, комфортно
        // Медленный	1 + rand() % 4	1..4	Очень медленно, для обучения
        // Средний	    2 + rand() % 5	2..6	Баланс
        pSprite->setVelocity(0, 2 + rand() % 8);
    }

    if (!pSprite)
        return nullptr;

    pSprite->setPosition(rcPos.left() + getWidth() / 2, rcPos.bottom());
    return pSprite;
}
