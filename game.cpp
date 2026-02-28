#include "game.h"
#include <QDebug>
#include <QPainter>
#include <QBitmap>
#include <QTime>
#include <QTimer>
#include <QRandomGenerator>
#include <widget.h>

std::unique_ptr<Game> Game::m_instance = nullptr;

Game::Game(QObject* parent)
    : QObject(parent)
{
}

Game* Game::getInstance()
{
    if(!m_instance)
    {
        m_instance.reset(new Game());
        qDebug() << "Game instance created";
    }

    return m_instance.get();
}

bool Game::gameInitialize(int w, int h)
{
    if (m_game_engine)
    {
        qDebug() << "Reinitializing game";
    }

    m_game_engine.reset(new GameEngine(w, h));

    m_width_wnd = w;
    m_height_wnd = h;

    qDebug() << "Game initialized";
    return true;
}

void Game::gameEnd()
{
    // Clear the sprites
    m_game_engine->cleanupSprites();
}

void Game::gameStart()
{
    initRandomGenerator();

    if (!loadTextures())
    {
        qCritical() << "Failed to load textures";
        return;
    }

    setupBallMask();

    newGame();

    qDebug() << "Game started with all resources loaded";
}

void Game::createWalls()
{
    QString walls =
        "-------------------"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |"
        "|                 |";

    int x1 = 0;
    int y1 = 0;

    m_num_rows = 14;
    m_num_cols = 19;

    int w_hor = m_pixmap_wall_h.width();
    int h_hor = m_pixmap_wall_h.height();

    int h_ver = m_pixmap_wall_v.height();

    int gap = 2;

    for(int row = 0; row  < m_num_rows; ++row)
    {
        x1 = 0;
        for(int col = 0; col < m_num_cols; ++col)
        {
            if(walls.at(col+row*m_num_cols) == " ")
            {
                x1+=(w_hor*2);
                continue;
            }
            else if(walls.at(col+row*m_num_cols) == "-")
            {
                m_sprite_wall_h = new Sprite(m_pixmap_wall_h, m_bounds, BA_STOP);
                m_sprite_wall_h->setPosition(x1, y1-h_hor);
                m_game_engine->addSprite(m_sprite_wall_h);
            }
            else if(walls.at(col+row*m_num_cols) == "|")
            {
                m_sprite_wall_v = new Sprite(m_pixmap_wall_v, m_bounds, BA_STOP);
                m_sprite_wall_v->setPosition(x1-w_hor, y1);
                m_game_engine->addSprite(m_sprite_wall_v);
            }

            x1+=w_hor+gap;
        }

        if(row == 0)
            y1+=h_hor+gap;
        else
            y1+=h_ver+gap;
    }
}

void Game::createLevel()
{
    m_count_blocks = 0;

    QString level;
    if(m_level == 1)
    {
        level =
        "BBBBBUBBBBB"
        "BBBBUUBBBBB"
        " BBBBUBBBB "
        "  BBBUBBB  "
        "   BBUBB   "
        "    UUU    ";
    }
    else if(m_level == 2)
    {
        level =
        "    UUU    "
        "   BUBUB   "
        "  BBBBUBB  "
        " BBBBUBBBB "
        " BBBUBBBBB "
        "BBBBUUUBBBB";
    }
    else if(m_level == 3)
    {
        m_game_win = true;
        return;
    }

    int m_num_block_rows = 6;
    int m_num_block_cols = 11;

    int m_block_x_gap = 42;
    int m_block_y_gap = 27;

    int m_block_origin_x = 92;
    int m_block_origin_y = 54;

    int x1 = m_block_origin_x;
    int y1 = m_block_origin_y;

    for(int row = 0; row < m_num_block_rows; ++row)
    {
        x1 = m_block_origin_x;
        for(int col = 0; col < m_num_block_cols; ++col)
        {
            if(level.at(col+row*m_num_block_cols) == " ")
            {
                x1+=m_block_x_gap;
                continue;
            }

            if(level.at(col+row*m_num_block_cols) == "B")
            {
                m_sprite_block = new Sprite(m_pixmap_block, m_bounds, BA_BOUNCE);
                ++m_count_blocks;
            }
            else if(level.at(col+row*m_num_block_cols) == "U")
            {
                m_sprite_block = new Sprite(m_pixmap_block_blue, m_bounds, BA_BOUNCE);
                ++m_count_blocks;
            }

            m_sprite_block->setPosition(x1, y1);
            m_game_engine->addSprite(m_sprite_block);

            x1+=m_block_x_gap;
        }
        y1+=m_block_y_gap;
    }
}

void Game::newGame()
{
    // Clear the sprites
    m_game_engine->cleanupSprites();

    m_bounds = QRect(0, 0, m_width_wnd, m_height_wnd);

    // paddle
    QRect bounds_paddle = QRect(20, 0, m_width_wnd-38, m_height_wnd);
    m_sprite_paddle = new Sprite(m_pixmap_paddle, bounds_paddle, BA_STOP, this);
    m_sprite_paddle->setPosition(m_width_wnd/2-m_sprite_paddle->getWidth()/2, 400);
    m_game_engine->addSprite(m_sprite_paddle);

    // walls
    createWalls();

    // blocks
    m_level = 1;
    createLevel();

    // ball
    m_count_balls = 1;
    for(int ii = 0; ii < m_count_balls; ++ii)
    {
        m_sprite_ball[ii] = new Sprite(m_pixmap_ball, m_bounds, BA_DIE, this);
        m_sprite_ball[ii]->setPosition(m_width_wnd/2-m_sprite_ball[ii]->getWidth()/2,
                                   m_sprite_paddle->getPosition().y()-m_sprite_ball[ii]->getHeight());
        m_game_engine->addSprite(m_sprite_ball[ii]);
    }

    auto lbd = [&]()
    {
        m_vel_x = 2;
        m_vel_y = 2;

        for(auto it = m_game_engine.get()->begin(); it != m_game_engine.get()->end(); ++it)
        {
            if ((*it)->getPixmap() == m_pixmap_ball)
                (*it)->setVelocity(random(-m_vel_x, m_vel_x), -m_vel_y);
        }
    };

    QTimer::singleShot(1500, this, lbd);

    // Background
    m_background.reset(new StarryBackground(m_width_wnd, m_height_wnd));

    m_pause = false;
    m_num_lives = 3;
    m_game_over = false;
    m_game_win = false;
    next_level = false;
}

void Game::gameActivate()
{
    m_pause = false;
}

void Game::gameDeactivate()
{
    m_pause = true;
}

void Game::gamePaint(QPainter* p)
{
    m_background->draw(p);

    if (!m_game_over)
    {
        if (!m_game_win)
        {
            m_game_engine->drawSprites(p);

            for (int ii = 0; ii < m_num_lives; ++ii)
                p->drawPixmap((m_width_wnd-(m_pixmap_paddle_sm.width()+7)*3)+((m_pixmap_paddle_sm.width()+5)*ii), 4, m_pixmap_paddle_sm);

            if (m_pause)
                p->drawPixmap(m_width_wnd/2-m_pixmap_pause.width()/2, m_height_wnd/3, m_pixmap_pause);
        }
        else
        {
            p->drawPixmap(m_width_wnd/2-m_pixmap_win.width()/2, m_height_wnd/3, m_pixmap_win);
        }
    }
    else
    {
        p->drawPixmap(m_width_wnd/2-m_pixmap_game_over.width()/2, m_height_wnd/3, m_pixmap_game_over);
    }
}

void Game::gameCycle()
{
    m_background->update();

    if (!m_game_over)
    {
        if (!m_game_win)
        {
            if(!m_pause)
            {
                processKeys();

                m_game_engine->updateSprites();
            }
        }
    }
}

bool Game::spriteCollision(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
    const QPixmap& pHitter = pSpriteHitter->getPixmap();
    const QPixmap& pHittee = pSpriteHittee->getPixmap();

    if(pHitter == m_pixmap_ball && (pHittee == m_pixmap_block ||
                                                        pHittee == m_pixmap_block_blue ||
                                                        pHittee == m_pixmap_wall_h ||
                                                        pHittee == m_pixmap_wall_v))
    {
        collisBallBricks(pSpriteHitter, pSpriteHittee);

        return true;
    }

    if(pHitter == m_pixmap_ball && pHittee == m_pixmap_paddle)
    {
        collisBallPaddle(pSpriteHitter, pSpriteHittee);

        return true;
    }

    if((pHitter == m_bonus_red_star) && pHittee == m_pixmap_paddle)
    {
        collisBonusPaddle(pSpriteHitter, pSpriteHittee);

        return true;
    }

    return false;
}

void Game::collisBallPaddle(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
    int gap = 10;
    QRect rect_centr_paddle(pSpriteHittee->getPosition().x()+gap, pSpriteHittee->getPosition().y(),
                            pSpriteHittee->getWidth()-gap*2, pSpriteHittee->getHeight());
    m_rect_centr_paddle = rect_centr_paddle;

    QRect rect_left_paddle(pSpriteHittee->getPosition().x(), pSpriteHittee->getPosition().y(),
                           pSpriteHittee->getWidth()-(pSpriteHittee->getWidth()-gap), pSpriteHittee->getHeight());
    m_rect_left_paddle = rect_left_paddle;

    QRect rect_right_paddle(pSpriteHittee->getPosition().x()+(pSpriteHittee->getWidth()-gap), pSpriteHittee->getPosition().y(),
                            gap, pSpriteHittee->getHeight());
    m_rect_right_paddle = rect_right_paddle;

    QPoint bottomPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
            QPoint(pSpriteHitter->getWidth()/2,pSpriteHitter->getHeight());


    if(rect_left_paddle.contains(bottomPosBall))
    {
        if(pSpriteHitter->getVelocity().y() > 0)
        {
            pSpriteHitter->setVelocity(-m_vel_x, -pSpriteHitter->getVelocity().y());
        }

        return;
    }

    if(rect_centr_paddle.contains(bottomPosBall))
    {
        if(pSpriteHitter->getVelocity().y() > 0)
        {
            pSpriteHitter->setVelocity(pSpriteHitter->getVelocity().x(),
                                       -pSpriteHitter->getVelocity().y());
        }

        return;
    }

    if(rect_right_paddle.contains(bottomPosBall))
    {
        if(pSpriteHitter->getVelocity().y() > 0)
        {
            pSpriteHitter->setVelocity(2, -pSpriteHitter->getVelocity().y());
        }

        return;
    }
}

void Game::collisBonusPaddle(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
    QRect rect_paddle(pSpriteHittee->getPosition().x(), pSpriteHittee->getPosition().y(),
                            pSpriteHittee->getWidth(), pSpriteHittee->getHeight());

    QPoint leftPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
            QPoint(0, pSpriteHitter->getHeight()/2);

    QPoint rightPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
            QPoint(pSpriteHitter->getWidth(),pSpriteHitter->getHeight()/2);

    QPoint bottomBonus = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
            QPoint(pSpriteHitter->getWidth()/2,pSpriteHitter->getHeight());


    if(rect_paddle.contains(bottomBonus) || rect_paddle.contains(leftPosBall)
            || rect_paddle.contains(rightPosBall))
    {
        pSpriteHitter->kill();

        if (pSpriteHitter->getPixmap() == m_bonus_red_star)
        {
            m_count_balls = 2;
            for(int ii = 0; ii < m_count_balls; ++ii)
            {
                Sprite* sprite_ball = new Sprite(m_pixmap_ball, m_bounds, BA_DIE, this);

                for(auto it = m_game_engine.get()->begin(); it != m_game_engine.get()->end(); ++it)
                {
                    if ((*it)->getPixmap() == m_pixmap_ball)
                    {
                        sprite_ball->setPosition((*it)->getPosition());
                    }
                }

                sprite_ball->setVelocity(random(-m_vel_x, m_vel_x), -m_vel_y);
                m_game_engine->addSprite(sprite_ball);
            }
        }
    }
}

void Game::createNewLevel(Sprite* pSpriteHitter)
{
    pSpriteHitter->setPosition(m_width_wnd/2-m_pixmap_ball.width()/2,
                               m_sprite_paddle->getPosition().y()-m_pixmap_ball.height());
    pSpriteHitter->kill();
    m_game_engine.get()->cleanupSprites(m_pixmap_ball);

    next_level = true;
    ++m_level;
    createLevel();
    m_sprite_paddle->setPosition(m_width_wnd/2-m_sprite_paddle->getWidth()/2, 400);
}

void Game::checkRandomBonus(Sprite* pSpriteHitter)
{
    int roll = random(0, 20);
    if(roll >= 19 && roll <= 20)
    {
        // bonus balls
        Sprite* sprt_red_star = new Sprite(m_bonus_red_star, m_bounds, BA_DIE, this);
        sprt_red_star->setPosition(pSpriteHitter->getPosition());
        sprt_red_star->setVelocity(0, 1);
        m_game_engine->addSprite(sprt_red_star);
    }
}

void Game::collisBallBricks(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
    if (next_level)
        return;

    QRect rect_brick(pSpriteHittee->getPosition().x(), pSpriteHittee->getPosition().y(),
                     pSpriteHittee->getWidth(), pSpriteHittee->getHeight());

    QPoint topPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
            QPoint(pSpriteHitter->getWidth()/2,0);

    QPoint bottomPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
            QPoint(pSpriteHitter->getWidth()/2,pSpriteHitter->getHeight());

    QPoint leftPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
            QPoint(0, pSpriteHitter->getHeight()/2);

    QPoint rightPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
            QPoint(pSpriteHitter->getWidth(),pSpriteHitter->getHeight()/2);

    if(rect_brick.contains(leftPosBall))
    {
        if(pSpriteHitter->getVelocity().x() < 0)
        {
            pSpriteHitter->setVelocity(-pSpriteHitter->getVelocity().x(),
                                        pSpriteHitter->getVelocity().y());
        }

        if(pSpriteHittee->getPixmap() == m_pixmap_block || pSpriteHittee->getPixmap() == m_pixmap_block_blue)
        {
            checkRandomBonus(pSpriteHitter);
            pSpriteHittee->kill();
            --m_count_blocks;
        }

        if(m_count_blocks == 0)
        {
            createNewLevel(pSpriteHitter);
        }

        return;
    }

    if(rect_brick.contains(rightPosBall))
    {
        if(pSpriteHitter->getVelocity().x() > 0)
        {
            pSpriteHitter->setVelocity(-pSpriteHitter->getVelocity().x(),
                                       pSpriteHitter->getVelocity().y());
        }

        if(pSpriteHittee->getPixmap() == m_pixmap_block || pSpriteHittee->getPixmap() == m_pixmap_block_blue)
        {
            checkRandomBonus(pSpriteHitter);
            pSpriteHittee->kill();
            --m_count_blocks;
        }

        if(m_count_blocks == 0)
        {
            createNewLevel(pSpriteHitter);
        }

        return;
    }

    if(rect_brick.contains(topPosBall))
    {
        if(pSpriteHitter->getVelocity().y() < 0)
        {
            pSpriteHitter->setVelocity(pSpriteHitter->getVelocity().x(),
                                       -pSpriteHitter->getVelocity().y());
        }

        if(pSpriteHittee->getPixmap() == m_pixmap_block || pSpriteHittee->getPixmap() == m_pixmap_block_blue)
        {
            checkRandomBonus(pSpriteHitter);
            pSpriteHittee->kill();
            --m_count_blocks;
        }

        if(m_count_blocks == 0)
        {
            createNewLevel(pSpriteHitter);
        }

        return;
    }

    if(rect_brick.contains(bottomPosBall))
    {
        if(pSpriteHitter->getVelocity().y() > 0)
        {
            pSpriteHitter->setVelocity(pSpriteHitter->getVelocity().x(),
                                       -pSpriteHitter->getVelocity().y());
        }

        if(pSpriteHittee->getPixmap() == m_pixmap_block || pSpriteHittee->getPixmap() == m_pixmap_block_blue)
        {
            checkRandomBonus(pSpriteHitter);
            pSpriteHittee->kill();
            --m_count_blocks;
        }

        if(m_count_blocks == 0)
        {
            createNewLevel(pSpriteHitter);
        }

        return;
    }
}

void Game::addEnemy()
{

}

void Game::spriteDying(Sprite* pSprite)
{
    if (pSprite->getPixmap() == m_pixmap_ball)
    {
        m_count_balls = m_game_engine.get()->countSprites(m_pixmap_ball);

        if (next_level == false && m_count_balls == 1)
        {
            --m_num_lives;
        }

//        next_level = false;

        if (m_num_lives == 0)
        {
            m_game_over = true;
        }

        if(m_count_balls == 1)
        {
            m_game_engine.get()->cleanupSprites(m_bonus_red_star);

            m_sprite_paddle->setPosition(m_width_wnd/2-m_sprite_paddle->getWidth()/2, 400);

            m_count_balls = 1;
            for(int ii = 0; ii < m_count_balls; ++ii)
            {
                m_sprite_ball[ii] = new Sprite(m_pixmap_ball, m_bounds, BA_DIE, this);
                m_sprite_ball[ii]->setPosition(m_width_wnd/2-m_sprite_ball[ii]->getWidth()/2,
                                           m_sprite_paddle->getPosition().y()-m_sprite_ball[ii]->getHeight());
                m_game_engine->addSprite(m_sprite_ball[ii]);
            }

            auto lbd = [&]()
            {
                m_vel_x = 2;
                m_vel_y = 2;

                for(auto it = m_game_engine.get()->begin(); it != m_game_engine.get()->end(); ++it)
                {
                    next_level = false;
                    if ((*it)->getPixmap() == m_pixmap_ball)
                        (*it)->setVelocity(random(-m_vel_x, m_vel_x), -m_vel_y);
                }
            };

            QTimer::singleShot(1500, this, lbd);
        }
    }
}

void Game::handleKeys(bool left, bool right, bool pause)
{
    m_left = left;
    m_right = right;
    m_pause = pause;
}

void Game::processKeys()
{
    QPoint ptVelocity = m_sprite_paddle->getVelocity();
    if (m_left)
    {
        // Move left
        ptVelocity.setX(qMax(ptVelocity.x() - 1, -4));
        m_sprite_paddle->setVelocity(ptVelocity);
    }
    else if (m_right)
    {
        // Move right
        ptVelocity.setX(qMin(ptVelocity.x() + 1, 4));
        m_sprite_paddle->setVelocity(ptVelocity);
    }

    if(!m_right && !m_left)
    {
        m_sprite_paddle->setVelocity(0, m_sprite_paddle->getVelocity().y());
    }
}

// Returns a random number in [low, high].
int Game::random(int low, int high)
{
    return low + rand() % ((high + 1) - low);
}

void Game::initRandomGenerator()
{
    auto seed = QTime::currentTime().msecsSinceStartOfDay();
    QRandomGenerator::securelySeeded().seed(seed);
    qDebug() << "Random generator initialized with seed:" << seed;
}

bool Game::loadTextures()
{
    bool success = true;

    auto loadPixmap = [&success](const QString& path) -> QPixmap
    {
        QPixmap pixmap(path);
        if (pixmap.isNull())
        {
            qWarning() << "Failed to load:" << path;
            success = false;
        }
        return pixmap;
    };

    m_pixmap_paddle = loadPixmap(":/images/paddle.png");
    m_pixmap_wall_h = loadPixmap(":/images/wall_hor.png");
    m_pixmap_wall_v = loadPixmap(":/images/wall_ver.png");
    m_pixmap_block = loadPixmap(":/images/block.png");
    m_pixmap_block_blue = loadPixmap(":/images/block_blue.png");
    m_pixmap_ball = loadPixmap(":/images/ball.png");
    m_pixmap_game_over = loadPixmap(":/images/game_over.png");
    m_pixmap_win = loadPixmap(":/images/win.png");
    m_pixmap_pause = loadPixmap(":/images/pause.png");
    m_bonus_red_star = loadPixmap(":/images/star1.png");

    // Масштабирование с правильными параметрами
    scaleTextures();

    return success;
}

void Game::scaleTextures()
{
    // Масштабируем с отключенным сглаживанием для четкости
    auto scaleFast = [](QPixmap& pix, const QSize& size)
    {
        if (!pix.isNull())
        {
            pix = pix.scaled(size, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        }
    };

    scaleFast(m_pixmap_paddle_sm, PADDLE_SMALL);
    scaleFast(m_pixmap_block, BLOCK_SIZE);
    scaleFast(m_pixmap_block_blue, BLOCK_SIZE);
    scaleFast(m_pixmap_ball, BALL_SIZE);

    // Маленькая версия платформы
    if (!m_pixmap_paddle.isNull())
    {
        m_pixmap_paddle_sm = m_pixmap_paddle.scaled(PADDLE_SMALL.width(), PADDLE_SMALL.height(),
                                                    Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
}

void Game::setupBallMask()
{
    if (!m_pixmap_ball.isNull())
    {
        // Создаем маску на основе черного цвета (0,0,0)
        QColor black(0, 0, 0);
        QBitmap mask = m_pixmap_ball.createMaskFromColor(black);
        m_pixmap_ball.setMask(mask);
        qDebug() << "Ball mask created";
    }
}
