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
    // See if the sprite has collided with any other sprites
    QList<Sprite*>::iterator siSprite;
    for (siSprite = sprites_.begin(); siSprite != sprites_.end(); ++siSprite)
    {
        // Make sure not to check for collision with itself
        if (pTestSprite == (*siSprite))
            continue;

        // Test the collision
        if (pTestSprite->testCollision(*siSprite))
        {
            // Collision detected
            return Game::getInstance()->spriteCollision((*siSprite), pTestSprite);
        }
    }

    // No collision
    return false;
}

void GameEngine::addSprite(Sprite* pSprite)
{
    // Add a sprite to the sprite vector
    if (pSprite != nullptr)
    {
        // See if there are sprites already in the sprite vector
        if (sprites_.size() > 0)
        {
            // Find a spot in the sprite vector to insert the sprite by its z-order
            QList<Sprite*>::iterator siSprite;
            for (siSprite = sprites_.begin(); siSprite != sprites_.end(); ++siSprite)
            {
                if (pSprite->getZOrder() < (*siSprite)->getZOrder())
                {
                    // Insert the sprite into the sprite vector
                    sprites_.insert(siSprite, pSprite);
                    return;
                }
            }
        }

    // The sprite's z-order is highest, so add it to the end of the vector
    sprites_.push_back(pSprite);
    }
}

void GameEngine::cleanupSprites()
{
    // Delete and remove the sprites in the sprite vector

    qDeleteAll(sprites_);
    sprites_.clear();

//    QList<Sprite*>::iterator siSprite;
//    for (siSprite = sprites_.begin(); siSprite != sprites_.end(); ++siSprite)
//    {
//        delete (*siSprite);
//        sprites_.erase(siSprite);
//    }
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
    // Update the sprites in the sprite vector
    QRect rcOldSpritePos;
    SPRITEACTION  saSpriteAction;

    for (int i=0;i<sprites_.count();++i)
    {
        if(sprites_.at(i))
        {
            // Save the old sprite position in case we need to restore it
            rcOldSpritePos = sprites_.at(i)->getPosition();

            // Update the sprite
            saSpriteAction = sprites_.at(i)->update();

            // Handle the SA_ADDSPRITE sprite action
            if (saSpriteAction == SA_ADDSPRITE)
            {
                // Allow the sprite to add its sprite
                addSprite(sprites_.at(i)->addSprite());
            }

            // Handle the SA_KILL sprite action
            if (saSpriteAction == SA_KILL)
            {
                // Notify the game that the sprite is dying
                Game::getInstance()->spriteDying(sprites_.at(i));

                delete sprites_.takeAt(i);

                continue;
            }

            // See if the sprite collided with any others
            if (checkSpriteCollision(sprites_.at(i)))
            {
                // Restore the old sprite position
                sprites_.at(i)->setPosition(rcOldSpritePos);
            }
        }
    }
}
