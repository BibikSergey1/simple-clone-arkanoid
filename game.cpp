#include "game.h"
#include <QDebug>
#include <QPainter>
#include <QBitmap>
#include <QTime>
#include <QTimer>
#include <QRandomGenerator>
#include <widget.h>
#include <cmath>
#include <QPointer>

std::unique_ptr<Game> Game::m_instance = nullptr;

Game::Game(QObject* parent)
    : QObject(parent)
    , m_left(false)
    , m_right(false)
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

    setupMask();

    newGame();

    qDebug() << "Game started with all resources loaded";
}

void Game::createWalls()
{
    QString walls =
        "--------------------"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2"
        "1                  2";

    int x1 = 0;
    int y1 = BLOCK_SIZE.height();

    m_num_rows = GRID_ROWS;
    m_num_cols = GRID_COLS;

    int w_hor = m_pixmap_wall_h.width();
    int h_hor = m_pixmap_wall_h.height();

    int h_ver = m_pixmap_wall_v.height();

    int gap = WOLLS_GAP;

    for(int row = 0; row  < m_num_rows; ++row)
    {
        x1 = 0;
        for(int col = 0; col < m_num_cols; ++col)
        {
            if(walls.at(col + row * m_num_cols) == " ")
            {
                x1 += w_hor;
                continue;
            }
            else if(walls.at(col + row * m_num_cols) == "-")
            {
                m_sprite_wall_h = new Sprite(m_pixmap_wall_h, m_bounds, BA_STOP);
                m_sprite_wall_h->setPosition(x1, y1);
                m_game_engine->addSprite(m_sprite_wall_h);
            }
            else if(walls.at(col+row*m_num_cols) == "1")
            {
                m_sprite_wall_v = new Sprite(m_pixmap_wall_v, m_bounds, BA_STOP);
                m_sprite_wall_v->setPosition(x1 - w_hor, y1);
                m_game_engine->addSprite(m_sprite_wall_v);
            }
            else if(walls.at(col+row*m_num_cols) == "2")
            {
                m_sprite_wall_v = new Sprite(m_pixmap_wall_v, m_bounds, BA_STOP);
                m_sprite_wall_v->setPosition(x1 + w_hor + 2, y1);
                m_game_engine->addSprite(m_sprite_wall_v);
            }

            x1 += w_hor + gap;
        }

        if(row == 0)
            y1 += h_hor + gap;
        else
            y1 += h_ver + gap;
    }
}

void Game::createLevel()
{
    m_count_blocks = 0;
    QString level;
    if (m_level == 1)
    {
        level =
            "RRRRRRRRRRR"
            "RBBBUUBBBBR"
            " RBBBUBBBR "
            "  RBBUBBR  "
            "   RBUBR   "
            "    UUU    ";
    }
    else if (m_level == 2)
    {
        level =
            "    UUU    "
            "   RUBUR   "
            "  RBBBUBR  "
            " RBBBUBBBR "
            " RBBUBBBBR "
            "RRRRUUURRRR";
    }
    else if (m_level == 3)
    {
        level =
            "BBBBUUBBBBB"
            "BBBBBBUBBBB"
            "SBBBUUBBBBS"
            "SBBBBBUBBBS"
            "SBBBUUBBBBS"
            "SSSSSSSSSSS";
    }
    else if(m_level == 4)
    {
        m_game_win = true;
        return;
    }

    int m_num_block_rows = NUM_BLOCK_ROWS;
    int m_num_block_cols = NUM_BLOCK_COLS;
    int m_block_x_gap = BLOCK_X_GAP;
    int m_block_y_gap = BLOCK_Y_GAP;
    int m_block_origin_x = BLOCK_ORIGIN_X;
    int m_block_origin_y = BLOCK_ORIGIN_Y;

    int y1 = m_block_origin_y;

    for(int row = 0; row < m_num_block_rows; ++row)
    {
        int x1 = m_block_origin_x;

        for(int col = 0; col < m_num_block_cols; ++col)
        {
            QChar ch = level.at(col + row * m_num_block_cols);
            if(ch == ' ')
            {
                x1 += m_block_x_gap;
                continue;
            }

            Sprite* pBlock = nullptr;
            if(ch == 'B')
            {
                pBlock = new Sprite(m_pixmap_block, m_bounds, BA_BOUNCE);
                ++m_count_blocks;
            }
            else if(ch == 'U')
            {
                pBlock = new Sprite(m_pixmap_block_blue, m_bounds, BA_BOUNCE);
                ++m_count_blocks;
            }
            else if(ch == 'R')   // тип блока – два удара
            {
                pBlock = new Sprite(m_pixmap_block_2hit, m_bounds, BA_BOUNCE);
                ++m_count_blocks;   // считаем как один блок, но с двумя жизнями
            }
            else if(ch == 'S')   // тип блока – неразбиваемый
            {
                pBlock = new Sprite(m_pixmap_block_solid, m_bounds, BA_BOUNCE);
            }

            if (pBlock)
            {
                pBlock->setPosition(x1, y1);
                m_game_engine->addSprite(pBlock);
            }
            x1 += m_block_x_gap;
        }
        y1 += m_block_y_gap;
    }
}

void Game::createBall()
{
    Sprite* newBall = new Sprite(m_pixmap_ball, m_game_bounds, BA_BOUNCE, this);
    newBall->setPosition(m_width_wnd/2 - newBall->getWidth()/2,
                         m_sprite_paddle->getPosition().y() - newBall->getHeight());
    m_game_engine->addSprite(newBall);
    m_sprite_ball[0] = newBall;  // сохраняем указатель
}

void Game::newGame()
{
    // Clear the sprites
    m_game_engine->cleanupSprites();

    m_bounds = QRect(0, 0, m_width_wnd, m_height_wnd);

    m_game_bounds = QRect(BALL_MARGIN_LEFT,
                          BLOCK_SIZE.height() * 2 - UP_MARGIN,
                          m_width_wnd-BALL_MARGIN_RIGHT,
                          m_height_wnd);
    // paddle
    m_paddle_bounds = QRect(PADDLE_MARGIN_LEFT,
                            BLOCK_SIZE.height() * 2 - UP_MARGIN,
                            m_width_wnd-PADDLE_MARGIN_RIGHT,
                            m_height_wnd);

    m_sprite_paddle = new Sprite(m_pixmap_paddle, m_paddle_bounds, BA_STOP, this);
    m_sprite_paddle->setPosition(m_width_wnd/2-m_sprite_paddle->getWidth()/2, PADDLE_Y);
    m_game_engine->addSprite(m_sprite_paddle);

    // walls
    createWalls();

    // blocks
    m_level = 1;
    createLevel();

    // balls
    createBall();

    QTimer::singleShot(DELAY_MS, this, [this]()
    {
        if (!m_game_engine)
            return;

        m_vel_x = BALL_SPEED;
        m_vel_y = BALL_SPEED;

        for (auto& sprite : *m_game_engine)
        {
            if (sprite->getPixmap().cacheKey() == m_pixmap_ball.cacheKey())
            {
                sprite->setVelocity(random(-m_vel_x, m_vel_x), -m_vel_y);
            }
        }
    });

    // Background
    m_background = std::make_unique<StarryBackground>(m_width_wnd, m_height_wnd);

    m_sprite_saucer = new Sprite(m_pixmap_saucer, m_game_bounds, BA_BOUNCE);
    m_sprite_saucer->setPosition(m_width_wnd / 2, BLOCK_SIZE.height() * 2);
    m_sprite_saucer->setVelocity(3, 1);
    m_game_engine->addSprite(m_sprite_saucer);

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
                p->drawPixmap((m_width_wnd-(m_pixmap_paddle_sm.width()+7)*3)+((m_pixmap_paddle_sm.width()+5)*ii), 8, m_pixmap_paddle_sm);

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

                updateSaucer();

                for (int i = 0; i < BALLS; ++i)
                {
                    Sprite* ball = m_sprite_ball[i];
                    if (ball)
                    {
                        if (ball->getPosition().bottom() > m_height_wnd)
                        {
                            ball->kill();
                        }
                    }
                }
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
                                     pHittee == m_pixmap_block_2hit ||
                                     pHittee == m_pixmap_block_damaged ||
                                     pHittee == m_pixmap_block_solid))
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

    // Столкновение мяча с летающей тарелкой
    if ((pHitter == m_pixmap_ball && pHittee == m_pixmap_saucer))
    {
        collisBallSaucer(pSpriteHitter, pSpriteHittee);
        return true;
    }

    return false;
}

void Game::collisBallPaddle(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
    // 1. Отскок только когда мяч движется ВНИЗ (y > 0)
    //    Если мяч летит вверх или горизонтально, столкновение не обрабатываем,
    //    чтобы избежать многократных отскоков при застревании.
    if (pSpriteHitter->getVelocity().y() <= 0)
        return;

    // 2. Точка на нижней грани мяча (центр)
    //    Берём середину нижней стороны мяча – именно она первой касается ракетки.
    QPoint ballBottom(pSpriteHitter->getPosition().x() + pSpriteHitter->getWidth() / 2,
                      pSpriteHitter->getPosition().y() + pSpriteHitter->getHeight());

    // 3. Прямоугольник ракетки
    QRect paddleRect(pSpriteHittee->getPosition().x(),
                     pSpriteHittee->getPosition().y(),
                     pSpriteHittee->getWidth(),
                     pSpriteHittee->getHeight());

    // 4. Если точка не внутри ракетки — выходим
    if (!paddleRect.contains(ballBottom))
        return;

    // 5. Смещение от центра ракетки (-1..1)
    //    Вычисляем, как далеко от центра ракетки пришёлся удар.
    //    Значение -1 означает крайний левый край, +1 – крайний правый.
    double paddleCenterX = pSpriteHittee->getPosition().x() + pSpriteHittee->getWidth() / 2.0;
    double offset = (ballBottom.x() - paddleCenterX) / (pSpriteHittee->getWidth() / 2.0);
    offset = qBound(-1.0, offset, 1.0);

    // 6. Максимальный угол отскока (60 градусов)
    //    Угол изменяется от -60° (влево) до +60° (вправо) пропорционально смещению.
    //    При ударе в центр угол = 0 → мяч летит строго вверх.
    const double MAX_ANGLE = M_PI / 3.0;
    double angle = offset * MAX_ANGLE;

    // 7. Текущая скорость (модуль)
    //    Вычисляем длину вектора скорости мяча до удара.
    QPoint oldVel = pSpriteHitter->getVelocity();
    double speed = std::hypot(oldVel.x(), oldVel.y());
    //    Ограничиваем скорость: минимум = BALL_SPEED * 1.25 (чуть выше стартовой),
    //    максимум = MAX_BALL_SPEED. Так мяч не будет слишком медленным или быстрым.
    speed = qBound(static_cast<double>(BALL_SPEED) * 1.25, speed, static_cast<double>(MAX_BALL_SPEED));

    // 8. Новые компоненты скорости
    //    Пересчитываем скорость из желаемого модуля и угла.
    //    newVelY отрицательная, потому что мяч должен лететь вверх (против оси Y).
    double newVelX = speed * std::sin(angle);
    double newVelY = -speed * std::cos(angle); // минус, чтобы лететь вверх

    // 9. Горизонтальная скорость не должна быть слишком маленькой
    //    Иначе мяч будет почти вертикально ходить, что скучно.
    if (std::abs(newVelX) < 1.5)
        newVelX = (newVelX >= 0) ? 1.5 : -1.5;

    // 10. Устанавливаем скорость
    //     Приводим к int, т.к. в Sprite скорость хранится как QPoint (целые).
    pSpriteHitter->setVelocity(static_cast<int>(newVelX),
                               static_cast<int>(newVelY));

    // 11. Выталкиваем мяч вверх, чтобы не застревал
    //     Без этого мяч может остаться внутри ракетки на следующий кадр
    //     и вызвать повторный отскок или залипание.
    pSpriteHitter->setPosition(pSpriteHitter->getPosition().x(),
                               pSpriteHitter->getPosition().y() - 2);
}

void Game::collisBonusPaddle(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
    if (!pSpriteHitter || !pSpriteHittee)
    {
        return;
    }

    QRect rectHitter(pSpriteHitter->getPosition().x(),
                     pSpriteHitter->getPosition().y(),
                     pSpriteHitter->getWidth(),
                     pSpriteHitter->getHeight());

    QRect rectHittee(pSpriteHittee->getPosition().x(),
                     pSpriteHittee->getPosition().y(),
                     pSpriteHittee->getWidth(),
                     pSpriteHittee->getHeight());

    if (!rectHitter.intersects(rectHittee))
    {
        return;
    }

    pSpriteHitter->kill();

    if (pSpriteHitter->getPixmap() == m_bonus_red_star)
    {
        QPoint existingBallPos;
        int currentBalls = 0;

        for (auto it = m_game_engine->begin(); it != m_game_engine->end(); ++it)
        {
            Sprite* sprite = *it;
            if (!sprite)
            {
                continue;
            }
            if (sprite->getPixmap() == m_pixmap_ball)
            {
                ++currentBalls;
                if (existingBallPos.isNull())
                {
                    existingBallPos = sprite->getPosition().topLeft();
                }
            }
        }

        int ballsToAdd = 3 - currentBalls;

        if (ballsToAdd > 0 && !existingBallPos.isNull())
        {
            for (int i = 0; i < BALLS && ballsToAdd > 0; ++i)
            {
                if (m_sprite_ball[i] == nullptr)
                {
                    Sprite* newBall = new Sprite(m_pixmap_ball, m_game_bounds, BA_BOUNCE, this);
                    if (!newBall)
                        continue;
                    newBall->setPosition(existingBallPos);
                    newBall->setVelocity(random(-m_vel_x, m_vel_x), -m_vel_y);
                    m_game_engine->addSprite(newBall);
                    m_sprite_ball[i] = newBall;
                    --ballsToAdd;
                }
            }
        }
    }
}

void Game::collisBallSaucer(Sprite* pBall, Sprite* pSaucer)
{
    QRect ballRect = pBall->getPosition();
    QRect saucerRect = pSaucer->getPosition();
    QPoint ballVel = pBall->getVelocity();

    // 1. Рассчитаем перекрытие по осям
    int overlapLeft   = ballRect.right() - saucerRect.left();
    int overlapRight  = saucerRect.right() - ballRect.left();
    int overlapTop    = ballRect.bottom() - saucerRect.top();
    int overlapBottom = saucerRect.bottom() - ballRect.top();

    // 2. Выбираем ось с минимальным перекрытием
    int minOverlap = std::min({overlapLeft, overlapRight, overlapTop, overlapBottom});
    if (minOverlap == overlapLeft || minOverlap == overlapRight)
        ballVel.setX(-ballVel.x());
    else
        ballVel.setY(-ballVel.y());

    // 3. Минимальная скорость, чтобы избежать "залипания"
    if (abs(ballVel.x()) < 2) ballVel.setX(ballVel.x() > 0 ? 2 : -2);
    if (abs(ballVel.y()) < 2) ballVel.setY(ballVel.y() > 0 ? 2 : -2);

    pBall->setVelocity(ballVel);

    // 4. Разводим объекты, чтобы они больше не пересекались
    // Выбираем направление разведения по той же оси, что и отскок
    QPoint separation(0,0);
    if (minOverlap == overlapLeft)
        separation.setX(-(overlapLeft + 1));
    else if (minOverlap == overlapRight)
        separation.setX(overlapRight + 1);
    else if (minOverlap == overlapTop)
        separation.setY(-(overlapTop + 1));
    else if (minOverlap == overlapBottom)
        separation.setY(overlapBottom + 1);

    pBall->setPosition(ballRect.translated(separation));
}

void Game::createNewLevel(Sprite* pSpriteHitter)
{
    pSpriteHitter->kill();
    m_game_engine->cleanupSprites(m_pixmap_ball);   // удаляем все мячи

    // Удаляем все типы блоков
    m_game_engine->cleanupSprites(m_pixmap_block);
    m_game_engine->cleanupSprites(m_pixmap_block_blue);
    m_game_engine->cleanupSprites(m_pixmap_block_2hit);
    m_game_engine->cleanupSprites(m_pixmap_block_damaged);
    m_game_engine->cleanupSprites(m_pixmap_block_solid);

    // Сброс тарелки для нового уровня
    if (m_sprite_saucer)
    {
        m_sprite_saucer->setPosition(m_width_wnd / 2, BLOCK_SIZE.height() * 2);
        m_sprite_saucer->setVelocity(3, 1);
    }

    next_level = true;
    ++m_level;
    createLevel();
    m_sprite_paddle->setPosition(m_width_wnd/2 - m_sprite_paddle->getWidth()/2, PADDLE_Y);
}

void Game::checkRandomBonus(Sprite* pSpriteHitter)
{
    if (!pSpriteHitter)
        return;

    constexpr int BONUS_CHANCE_PERCENT = 10; // 10%
    if (random(0, 99) >= BONUS_CHANCE_PERCENT)
        return;

    // Пока только бонус "красная звезда" (multi-ball)
    const int FALL_SPEED = 1;
    Sprite* bonus = new Sprite(m_bonus_red_star, m_bounds, BA_DIE, this);
    bonus->setPosition(pSpriteHitter->getPosition().topLeft());
    bonus->setVelocity(0, FALL_SPEED);
    m_game_engine->addSprite(bonus);
}

void Game::collisBallBricks(Sprite* pSpriteHitter, Sprite* pSpriteHittee)
{
    if (next_level)
        return;

    QRect rect_brick(pSpriteHittee->getPosition().x(), pSpriteHittee->getPosition().y(),
                     pSpriteHittee->getWidth(), pSpriteHittee->getHeight());

    QPoint topPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
                        QPoint(pSpriteHitter->getWidth()/2, 0);

    QPoint bottomPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
                           QPoint(pSpriteHitter->getWidth()/2,pSpriteHitter->getHeight());

    QPoint leftPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
                         QPoint(0, pSpriteHitter->getHeight()/2);

    QPoint rightPosBall = QPoint(pSpriteHitter->getPosition().x(),pSpriteHitter->getPosition().y()) +
                          QPoint(pSpriteHitter->getWidth(),pSpriteHitter->getHeight()/2);

    // === ОТЛАДКА ===
    // static int frameCounter = 0;
    // frameCounter++;
    // if (frameCounter % 60 == 0)
    // { // не засорять лог каждый кадр
    //     qDebug() << "--- Frame" << frameCounter << "---";
    //     qDebug() << "Ball pos:" << pSpriteHitter->getPosition() << "vel:" << pSpriteHitter->getVelocity();
    //     qDebug() << "Brick pos:" << rect_brick.topLeft() << "size:" << rect_brick.size();
    // }

    bool collided = false;

    if(rect_brick.contains(leftPosBall))
    {
        if(pSpriteHitter->getVelocity().x() < 0)
        {
            pSpriteHitter->setVelocity(-pSpriteHitter->getVelocity().x(),
                                       pSpriteHitter->getVelocity().y());
            collided = true;
            //qDebug() << "leftPosBall  -> reflected X, new vel =" << pSpriteHitter->getVelocity();
        }
    }
    if(rect_brick.contains(rightPosBall))
    {
        if(pSpriteHitter->getVelocity().x() > 0)
        {
            pSpriteHitter->setVelocity(-pSpriteHitter->getVelocity().x(),
                                       pSpriteHitter->getVelocity().y());
            collided = true;
            //qDebug() << "rightPosBall  -> reflected X, new vel =" << pSpriteHitter->getVelocity();
        }
    }
    if(rect_brick.contains(topPosBall))
    {
        if(pSpriteHitter->getVelocity().y() < 0)
        {
            pSpriteHitter->setVelocity(pSpriteHitter->getVelocity().x(),
                                       -pSpriteHitter->getVelocity().y());
            collided = true;
            //qDebug() << "topPosBall  -> reflected Y, new vel =" << pSpriteHitter->getVelocity();
        }
    }
    if(rect_brick.contains(bottomPosBall))
    {
        if(pSpriteHitter->getVelocity().y() > 0)
        {
            pSpriteHitter->setVelocity(pSpriteHitter->getVelocity().x(),
                                       -pSpriteHitter->getVelocity().y());
            collided = true;
            //qDebug() << "bottomPosBall  -> reflected Y, new vel =" << pSpriteHitter->getVelocity();
        }
    }

    if (collided)
    {
        // Ограничиваем скорость после отскока
        QPoint vel = pSpriteHitter->getVelocity();
        vel.setX(qBound(-MAX_BALL_SPEED, vel.x(), MAX_BALL_SPEED));
        vel.setY(qBound(-MAX_BALL_SPEED, vel.y(), MAX_BALL_SPEED));

        // Гарантируем ненулевую скорость
        if (vel.x() == 0 && vel.y() != 0)
        {
            vel.setX((random(0, 1) == 0) ? 2 : -2);
        }
        if (vel.y() == 0 && vel.x() != 0)
        {
            vel.setY((random(0, 1) == 0) ? 2 : -2);
        }

        pSpriteHitter->setVelocity(vel);

        const QPixmap& blockPix = pSpriteHittee->getPixmap();

        if (blockPix == m_pixmap_block_2hit)
        {
            // первый удар – заменяем текстуру на повреждённую
            pSpriteHittee->setPixmap(m_pixmap_block_damaged);
            // m_count_blocks не уменьшаем, блок ещё жив
        }
        else if (blockPix == m_pixmap_block_damaged)
        {
            // второй удар – уничтожаем
            checkRandomBonus(pSpriteHitter);
            pSpriteHittee->kill();
            --m_count_blocks;
        }
        else if (blockPix == m_pixmap_block || blockPix == m_pixmap_block_blue)
        {
            // обычный блок – уничтожаем сразу
            checkRandomBonus(pSpriteHitter);
            pSpriteHittee->kill();
            --m_count_blocks;
        }

        if (m_count_blocks == 0)
        {
            createNewLevel(pSpriteHitter);
        }
    }
}

void Game::addEnemy()
{

}

void Game::spriteDying(Sprite* pSprite)
{
    // Определяем, что именно умирает
    bool isBall = (pSprite->getPixmap() == m_pixmap_ball);
    if (!isBall)
        return; // другие типы спрайтов (бонусы, враги) обрабатываем отдельно, если нужно

    // Обнуляем указатель в массиве мячей
    for (int i = 0; i < BALLS; ++i)
    {
        if (m_sprite_ball[i] == pSprite)
        {
            m_sprite_ball[i] = nullptr;
            break;
        }
    }

    // Уменьшаем счётчик мячей (не вызывая countSprites)
    // Но лучше вести учёт при добавлении/удалении мячей.
    // Пока используем countSprites для простоты, но запоминаем результат.
    int ballsLeft = m_game_engine->countSprites(m_pixmap_ball) - 1; // текущий мяч ещё не удалён из контейнера

    if (ballsLeft == 0)  // был последний мяч
    {
        // Потеря жизни
        if (!next_level)  // только если не переходим на следующий уровень
        {
            --m_num_lives;
        }
        // Не сбрасываем next_level здесь (его сброс происходит при начале уровня)

        if (m_num_lives == 0)
        {
            m_game_over = true;
            return;  // не продолжаем — игра окончена
        }

        // Очистить бонусы (звёзды)
        m_game_engine->cleanupSprites(m_bonus_red_star);

        // Переместить ракетку в центр
        m_sprite_paddle->setPosition(m_width_wnd/2 - m_sprite_paddle->getWidth()/2, PADDLE_Y);

        // Создать новый мяч
        createBall();

        QPointer<Game> self(this);
        float velX = m_vel_x;
        float velY = m_vel_y;

        QTimer::singleShot(1500, this, [self, velX, velY]()
        {
            if (self.isNull())
                return;
            for (auto it = self->m_game_engine->begin(); it != self->m_game_engine->end(); ++it)
            {
                if ((*it)->getPixmap() == self->m_pixmap_ball)
                {
                   (*it)->setVelocity(self->random(-velX, velX), -velY);
                }
            }
            self->next_level = false;
        });
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
        ptVelocity.setX(qMax(ptVelocity.x() - 1, -PADDLE_MAX_SPEED));
        m_sprite_paddle->setVelocity(ptVelocity);
    }
    else if (m_right)
    {
        // Move right
        ptVelocity.setX(qMin(ptVelocity.x() + 1, PADDLE_MAX_SPEED));
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
    m_pixmap_block_2hit = loadPixmap(":/images/block_2hit.png");
    m_pixmap_block_damaged = loadPixmap(":/images/block_damaged.png");
    m_pixmap_block_solid = loadPixmap(":/images/block_solid.png");
    m_pixmap_saucer = loadPixmap(":/images/saucer.bmp");

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
    scaleFast(m_pixmap_block_2hit, BLOCK_SIZE);
    scaleFast(m_pixmap_block_damaged, BLOCK_SIZE);
    scaleFast(m_pixmap_block_solid, BLOCK_SIZE);

    // Маленькая версия платформы
    if (!m_pixmap_paddle.isNull())
    {
        m_pixmap_paddle_sm = m_pixmap_paddle.scaled(PADDLE_SMALL.width(), PADDLE_SMALL.height(),
                                                    Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
}

void Game::setupMask()
{
    if (!m_pixmap_ball.isNull())
    {
        QColor black(0, 0, 0);
        QBitmap mask = m_pixmap_ball.createMaskFromColor(black);
        m_pixmap_ball.setMask(mask);
    }

    if (!m_pixmap_saucer.isNull())
    {
        QColor madjenta(255, 0, 255);
        QBitmap mask = m_pixmap_saucer.createMaskFromColor(madjenta);
        m_pixmap_saucer.setMask(mask);
    }
}

void Game::updateSaucer()
{
    if (!m_sprite_saucer)
        return;

    auto isBlock = [&](Sprite* s) -> bool
    {
        if (!s)
            return false;

        QPixmap pix = s->getPixmap();
        return (pix == m_pixmap_block ||
                pix == m_pixmap_block_blue ||
                pix == m_pixmap_block_2hit ||
                pix == m_pixmap_block_damaged ||
                pix == m_pixmap_block_solid);
    };

    QRect saucerRect = m_sprite_saucer->getPosition();
    QPoint velocity = m_sprite_saucer->getVelocity();

    // ========== 1. Выбор случайной цели (каждую секунду) ==========
    static QPoint targetPoint(0, 0);
    static int targetTicks = 0;
    targetTicks++;
    // Меняем цель каждые 60 кадров (≈1 секунда) или если уже почти достигли
    bool reached = (abs(targetPoint.x() - saucerRect.center().x()) < 10 &&
                    abs(targetPoint.y() - saucerRect.center().y()) < 10);
    if (targetTicks > 60 || reached)
    {
        targetTicks = 0;
        // Случайная точка внутри игровых границ
        targetPoint.setX(m_bounds.left() + 20 + rand() % (m_bounds.width() - 40));
        targetPoint.setY(m_bounds.top()  + 20 + rand() % (m_bounds.height() - 40));
    }

    // Вычисляем желаемое направление к цели
    int dxToTarget = targetPoint.x() - saucerRect.center().x();
    int dyToTarget = targetPoint.y() - saucerRect.center().y();
    int desiredX = (dxToTarget > 0) ? 1 : (dxToTarget < 0) ? -1 : 0;
    int desiredY = (dyToTarget > 0) ? 1 : (dyToTarget < 0) ? -1 : 0;

    // Плавно меняем скорость в сторону цели (максимум ±1 за кадр)
    if (velocity.x() < desiredX) velocity.setX(velocity.x() + 1);
    if (velocity.x() > desiredX) velocity.setX(velocity.x() - 1);
    if (velocity.y() < desiredY) velocity.setY(velocity.y() + 1);
    if (velocity.y() > desiredY) velocity.setY(velocity.y() - 1);

    // ========== 2. Поиск ближайшего блока (как раньше) ==========
    int centerX = saucerRect.center().x();
    int centerY = saucerRect.center().y();
    int closestDx = 0, closestDy = 0;
    int minDistance = 10000;

    for (auto it = m_game_engine->begin(); it != m_game_engine->end(); ++it)
    {
        Sprite* sprite = *it;
        if (!sprite || sprite == m_sprite_saucer)
            continue;
        if (!isBlock(sprite))
            continue;

        QRect blockRect = sprite->getPosition();
        int blockCenterX = blockRect.center().x();
        int blockCenterY = blockRect.center().y();
        int dx = centerX - blockCenterX;
        int dy = centerY - blockCenterY;
        int dist = abs(dx) + abs(dy);

        if (dist < minDistance)
        {
            minDistance = dist;
            closestDx = dx;
            closestDy = dy;
        }
    }

    // ========== 3. Уклонение от близкого блока (плавное) ==========
    const int THRESHOLD = 70;
    if (minDistance < THRESHOLD)
    {
        if (abs(closestDx) < THRESHOLD)
        {
            if (closestDx < 0)
                velocity.setX(qMax(velocity.x() - 1, -6));
            else
                velocity.setX(qMin(velocity.x() + 1, 6));
        }
        if (abs(closestDy) < THRESHOLD)
        {
            if (closestDy < 0)
                velocity.setY(qMax(velocity.y() - 1, -6));
            else
                velocity.setY(qMin(velocity.y() + 1, 6));
        }
    }

    // ========== 4. Отталкивание от границ экрана ==========
    if (saucerRect.left() < m_bounds.left() && velocity.x() < 0)
        velocity.setX(1);
    if (saucerRect.right() > m_bounds.right() && velocity.x() > 0)
        velocity.setX(-1);
    if (saucerRect.top() < m_bounds.top() && velocity.y() < 0)
        velocity.setY(1);
    if (saucerRect.bottom() > m_bounds.bottom() && velocity.y() > 0)
        velocity.setY(-1);

    // ========== 5. Гарантия минимальной скорости (чтобы не стоял) ==========
    if (abs(velocity.x()) < 1 && abs(velocity.y()) < 1)
    {
        // Задаём случайное маленькое движение
        velocity.setX((rand() % 3) - 1);  // -1,0,1
        velocity.setY((rand() % 3) - 1);
        // Если оба нуля – принудительно даём импульс
        if (velocity.x() == 0 && velocity.y() == 0)
        {
            velocity.setX(1);
            velocity.setY(0);
        }
    }

    // Ограничиваем максимальную скорость
    velocity.setX(qBound(-6, velocity.x(), 6));
    velocity.setY(qBound(-6, velocity.y(), 6));

    m_sprite_saucer->setVelocity(velocity);
}
