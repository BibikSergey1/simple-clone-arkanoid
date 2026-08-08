#include "GameEngine.h"
#include "sprite.h"
#include "game.h"
#include <QDebug>

GameEngine::GameEngine(int width, int height)
{
    width_ = width;
    height_ = height;
}

GameEngine::~GameEngine()
{
}

bool GameEngine::checkSpriteCollision(Sprite* pTestSprite)
{
    if (!pTestSprite || !pTestSprite->isAlive())
        return false;

    // Собираем все спрайты, с которыми есть пересечение (и которые живы)
    QList<Sprite*> collidedWith;
    for (Sprite* pOther : qAsConst(sprites_))
    {
        if (pOther == pTestSprite || !pOther || !pOther->isAlive())
            continue;

        if (pTestSprite->testCollision(pOther))
        {
            collidedWith.append(pOther);
        }
    }

    if (collidedWith.isEmpty())
        return false;

    // Обрабатываем каждое столкновение
    bool anyCollision = false;
    for (Sprite* pOther : collidedWith)
    {
        // Повторно проверяем, что оба объекта ещё живы (могли быть убиты в предыдущей обработке)
        if (pTestSprite->isAlive() && pOther->isAlive())
        {
            if (Game::getInstance()->spriteCollision(pOther, pTestSprite))
                anyCollision = true;
        }
    }

    return anyCollision;
}

void GameEngine::addSprite(Sprite* pSprite)
{
    if (!pSprite)
        return;

    // Find insertion position based on z-order (ascending)
    auto it = sprites_.begin();
    while (it != sprites_.end() && (*it)->getZOrder() <= pSprite->getZOrder())
    {
        ++it;
    }
    sprites_.insert(it, pSprite);
}

void GameEngine::cleanupSprites()
{
    // Delete and remove the sprites in the sprite vector
    qDeleteAll(sprites_);
    sprites_.clear();
}

void GameEngine::cleanupSprites(const QPixmap& pix)
{
    QList<Sprite*>::iterator siSprite;
    for (siSprite = sprites_.begin(); siSprite != sprites_.end(); ++siSprite)
    {
        if ((*siSprite)->getPixmap() == pix)
        {
            (*siSprite)->kill();
        }
    }
}

int GameEngine::countSprites(const QPixmap& pix)
{
    int res = std::count_if(sprites_.begin(), sprites_.end(), [&](Sprite* item){return(item->getPixmap() == pix);});
    return res;
}

void GameEngine::drawSprites(QPainter* p)
{
    // Draw the sprites in the sprite vector
    QList<Sprite*>::iterator siSprite;
    for (siSprite = sprites_.begin(); siSprite != sprites_.end(); ++siSprite)
    {
        (*siSprite)->draw(p);
    }
}

void GameEngine::updateSprites()
{
    for (int i = 0; i < sprites_.size(); )
    {
        Sprite* sprite = sprites_.at(i);
        if (!sprite)
        {
            ++i;
            continue;
        }

        // 1. Сохраняем старую позицию
        QRect oldPos = sprite->getPosition();

        // 2. Обновляем спрайт
        SPRITEACTION action = sprite->update();

        // 3. Добавление нового спрайта (сразу)
        if (action == SA_ADDSPRITE)
        {
            Sprite* newSprite = sprite->addSprite();
            if (newSprite)
                addSprite(newSprite);   // добавит в конец sprites_
        }

        // 4. Удаление текущего спрайта (если нужно)
        if (action == SA_KILL)
        {
            Game::getInstance()->spriteDying(sprite);
            auto sprite = sprites_.takeAt(i);
            if (sprite)
            {
                delete sprite;
                sprite = nullptr;
            }
            // i не увеличиваем — на место i встал следующий элемент
            continue;
        }

        // 5. Коллизия (только если спрайт жив)
        if (checkSpriteCollision(sprite))
        {
            sprite->setPosition(oldPos);
        }

        ++i;  // переходим к следующему спрайту
    }
}
