#include "game.h"
#include <QDebug>
#include <QPainter>
#include <QBitmap>
#include <QTime>
#include <QTimer>

std::unique_ptr<Game> Game::m_instance = nullptr;

Game::Game(QObject* parent)
    : QObject(parent)
{
}

Game* Game::getInstance()
{
    if(!m_instance.get())
        m_instance.reset(new Game());

    return m_instance.get();
}

bool Game::gameInitialize(int w, int h)
{
    m_game_engine.reset(new GameEngine(w, h));
    if(m_game_engine == nullptr)
        return false;

    m_width_wnd = w;
    m_height_wnd = h;

    return true;
}

void Game::gameEnd()
{
    // Clear the sprites
    m_game_engine->cleanupSprites();
}

void Game::gameStart()
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));

    m_pixmap_paddle.load(":/images/paddle.png");
    m_pixmap_paddle_sm = m_pixmap_paddle.scaled(22, 8);
    m_pixmap_wall_h.load(":/images/wall_hor.png");
    m_pixmap_wall_v.load(":/images/wall_ver.png");

    m_pixmap_block.load(":/images/block.png");
    //m_pixmap_block = m_pixmap_block.scaled(150, 100);//40, 20
    m_pixmap_block = m_pixmap_block.scaled(40, 25);

    m_pixmap_block_blue.load(":/images/block_blue.png");
    m_pixmap_block_blue = m_pixmap_block_blue.scaled(40, 25);

    m_pixmap_ball.load(":/images/ball.png");
    m_pixmap_ball = m_pixmap_ball.scaled(12,12);
    m_pixmap_ball.setMask(m_pixmap_ball.createMaskFromColor(QColor(0,0,0)));

    m_pixmap_game_over.load(":/images/game_over.png");
    m_pixmap_win.load(":/images/win.png");
    m_pixmap_pause.load(":/images/pause.png");

    newGame();
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

    int num_rows = 14;
    int num_cols = 19;

    int w_hor = m_pixmap_wall_h.width();
    int h_hor = m_pixmap_wall_h.height();

    int h_ver = m_pixmap_wall_v.height();

    int gap = 2;

    for(int row = 0; row < num_rows; ++row)
    {
        x1 = 0;
        for(int col = 0; col < num_cols; ++col)
        {
            if(walls.at(col+row*num_cols) == " ")
            {
                x1+=(w_hor*2);
                continue;
            }
            else if(walls.at(col+row*num_cols) == "-")
            {
                m_sprite_wall_h = new Sprite(m_pixmap_wall_h, m_bounds, BA_STOP);
                m_sprite_wall_h->setPosition(x1, y1);
                m_game_engine->addSprite(m_sprite_wall_h);
            }
            else if(walls.at(col+row*num_cols) == "|")
            {
                m_sprite_wall_v = new Sprite(m_pixmap_wall_v, m_bounds, BA_STOP);
                m_sprite_wall_v->setPosition(x1, y1);
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


    //QString level_1 = "BB";

//    m_num_block_rows = 2;
//    m_num_block_cols = 1;

    m_num_block_rows = 6;
    m_num_block_cols = 11;

    m_block_width  = 40;
    m_block_height = 25;

//    m_block_width  = 150;
//    m_block_height = 100;

    m_block_x_gap = 42;
    m_block_y_gap = 27;
//    m_block_x_gap = 282;
//    m_block_y_gap = 222;

    m_block_origin_x = 92;
    m_block_origin_y = 54;

//    m_block_origin_x = 94;
//    m_block_origin_y = 132;

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
                //m_sprite_block->setPosition(x1, y1);
                //m_game_engine->addSprite(m_sprite_block);
                ++m_count_blocks;
            }
            else if(level.at(col+row*m_num_block_cols) == "U")
            {
                m_sprite_block = new Sprite(m_pixmap_block_blue, m_bounds, BA_BOUNCE);
                //m_sprite_block->setPosition(x1, y1);
                //m_game_engine->addSprite(m_sprite_block);
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

    QRect bounds_paddle = QRect(20, 0, m_width_wnd-38, m_height_wnd);
    m_sprite_paddle = new Sprite(m_pixmap_paddle, bounds_paddle, BA_STOP, this);
    m_sprite_paddle->setPosition(m_width_wnd/2-m_sprite_paddle->getWidth()/2, 400);
    m_game_engine->addSprite(m_sprite_paddle);

    createWalls();
    m_level = 1;
    createLevel();

    m_sprite_ball = new Sprite(m_pixmap_ball, m_bounds, BA_DIE, this);
    m_sprite_ball->setPosition(m_width_wnd/2-m_sprite_ball->getWidth()/2,
                               m_sprite_paddle->getPosition().y()-m_sprite_ball->getHeight());
    m_game_engine->addSprite(m_sprite_ball);

    m_vel_x = 2;
    m_vel_y = 2;

    auto lbd = [&]()
    {
        m_sprite_ball->setVelocity(random(-m_vel_x, m_vel_x), -m_vel_y);
    };

    QTimer::singleShot(1500, this, lbd);

    m_background.reset(new StarryBackground(m_width_wnd, m_height_wnd));

    m_num_lives = 3;
    m_game_over = false;
    m_game_win = false;
    next_level = false;
    m_pause = false;
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

            for (int ii = 0; ii < m_num_lives; ii++)
                p->drawPixmap((m_width_wnd-(m_pixmap_paddle_sm.width()+7)*3)+((m_pixmap_paddle_sm.width()+5)*ii), 4, m_pixmap_paddle_sm);

            if (m_pause)
                p->drawPixmap(m_width_wnd/2-m_pixmap_pause.width()/2, m_height_wnd/3, m_pixmap_pause);

            //p->setPen(Qt::blue);
            //p->drawRect(m_rect_centr_paddle);
            //p->setPen(Qt::red);
            //p->drawRect(m_rect_left_paddle);
            //p->drawRect(m_rect_right_paddle);
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
    if (!m_game_over)
    {
        if (!m_game_win)
        {
            if(!m_pause)
            {
                processKeys();

                m_background->update();
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
        collisionBallWithBricks(pSpriteHitter, pSpriteHittee);

        return true;
    }

    if(pHitter == m_pixmap_ball && pHittee == m_pixmap_paddle)
    {
        collisionBallWithPaddle(pSpriteHitter, pSpriteHittee);

        return true;
    }

    return false;
}

void Game::collisionBallWithPaddle(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
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

void Game::collisionBallWithBricks(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
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
            pSpriteHittee->kill();

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
            pSpriteHittee->kill();

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
            pSpriteHittee->kill();

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
            pSpriteHittee->kill();

        return;
    }
}

void Game::addEnemy()
{

}

void Game::spriteDying(Sprite* pSprite)
{
    if(pSprite->getPixmap() == m_pixmap_block || pSprite->getPixmap() == m_pixmap_block_blue)
    {
        --m_count_blocks;

        if(m_count_blocks == 0)
        {
            m_sprite_ball->setPosition(m_width_wnd/2-m_sprite_ball->getWidth()/2,
                                       m_sprite_paddle->getPosition().y()-m_sprite_ball->getHeight());
            m_sprite_ball->kill();
            next_level = true;
            ++m_level;
            createLevel();
            m_sprite_paddle->setPosition(m_width_wnd/2-m_sprite_paddle->getWidth()/2, 400);
        }
    }

    if (pSprite->getPixmap() == m_pixmap_ball)
    {
        if (!next_level)
        {
            --m_num_lives;
        }

        next_level = false;

        if (m_num_lives == 0)
        {
            m_game_over = true;
        }

        m_sprite_paddle->setPosition(m_width_wnd/2-m_sprite_paddle->getWidth()/2, 400);

        m_sprite_ball = new Sprite(m_pixmap_ball, m_bounds, BA_DIE, this);
        m_sprite_ball->setPosition(m_width_wnd/2-m_sprite_ball->getWidth()/2,
                                   m_sprite_paddle->getPosition().y()-m_sprite_ball->getHeight());
        m_game_engine->addSprite(m_sprite_ball);

        auto lbd = [&]()
        {
            m_sprite_ball->setVelocity(random(-m_vel_x, m_vel_x), -m_vel_y);
        };

        QTimer::singleShot(1500, this, lbd);

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
