#ifndef GAME_H
#define GAME_H

#include "GameEngine.h"
#include "sprite.h"
#include "Background.h"
#include <qobject.h>
#include <memory>
#include <QSoundEffect>

class Game : public QObject
{
    Q_OBJECT

public:
    static Game* getInstance();

    bool gameInitialize(int w, int h);
    void gameEnd();
    void gameStart();
    void gameActivate();
    void gameDeactivate();
    void gamePaint(QPainter* p);
    void gameCycle();
    void handleKeys(bool left, bool right, bool pause);
    bool spriteCollision(Sprite* pSpriteHitter, Sprite* pSpriteHittee);
    void spriteDying(Sprite* pSprite);

private:
    void addEnemy();
    void newGame();
    void updateSaucer();

private slots:
    void resetPaddleWidth();
    void disableFastBall();

private:
    explicit Game(QObject* parent = nullptr);

    Game(const Game&) = delete;
    Game& operator=(Game&) = delete;

    Game(Game&&) = delete;
    Game& operator=(Game&&) = delete;

    void createLevel();
    void createNewLevel(Sprite* pSpriteHitter);
    void createBall();
    void collisBallBricks(Sprite* pSpriteHitter, Sprite* pSpriteHittee);
    /**
     * Обрабатывает столкновение мяча с ракеткой.
     * Правила отскока:
     * - Отскок происходит только при движении мяча вниз (y > 0).
     * - Угол отскока зависит от места удара относительно центра ракетки:
     *   смещение к краю увеличивает горизонтальную составляющую (до ±60°).
     * - Модуль скорости сохраняется, но ограничивается в диапазоне
     *   [BALL_SPEED*1.25, MAX_BALL_SPEED].
     * - Минимальная горизонтальная скорость после отскока не менее 1.5 пикселя/кадр.
     * - Мяч принудительно смещается вверх на 2 пикселя для предотвращения застревания.
     */
    void collisBallPaddle(Sprite* pSpriteHitter, Sprite* pSpriteHittee);
    void collisBonusPaddle(Sprite* pSpriteHitter, Sprite* pSpriteHittee);
    void collisBallSaucer(Sprite* pBall, Sprite* pSaucer);
    int random(int a, int b);
    void processKeys();
    void checkRandomBonus(Sprite* pSpriteHitter);
    void initRandomGenerator();
    bool loadTextures();
    void scaleTextures();
    void setupMask();
    void drawWalls(QPainter* p);
    void setAllBallsPixmap(bool fast);
    int countBalls() const;

    QRect m_bounds;
    QRect m_game_bounds;
    QRect m_paddle_bounds;

    std::unique_ptr<GameEngine> m_game_engine;

    QPixmap m_pixmap_block;
    QPixmap m_pixmap_block_blue;
    QPixmap m_pixmap_ball;
    QPixmap m_pixmap_ball_fast;
    QPixmap m_pixmap_wall_h;
    QPixmap m_pixmap_wall_v;
    QPixmap m_pixmap_paddle;
    QPixmap m_pixmap_paddle_sm;
    QPixmap m_pixmap_game_over;
    QPixmap m_pixmap_win;
    QPixmap m_pixmap_pause;
    QPixmap m_pixmap_bonus_red_star;
    QPixmap m_pixmap_bonus_green_star;
    QPixmap m_pixmap_bonus_blue_star;
    QPixmap m_pixmap_block_2hit;   // блок с двумя жизнями (неповреждённый)
    QPixmap m_pixmap_block_damaged; // блок после первого удара
    QPixmap m_pixmap_block_solid;// не разрушаемый блок
    QPixmap m_pixmap_block_solid_flash;
    QPixmap m_pixmap_saucer;

    enum BonusType
    {
        BONUS_RED_STAR,
        BONUS_GREEN_STAR,
        BONUS_BLUE_STAR,
        // ... сюда можно добавлять новые
        BONUS_COUNT
    };
    QPixmap* m_bonusPixmaps[BONUS_COUNT];

    std::unique_ptr<StarryBackground> m_background;

    enum {BALLS = 3};

    Sprite* m_sprite_wall_h = nullptr;
    Sprite* m_sprite_wall_v = nullptr;
    Sprite* m_sprite_block = nullptr;
    Sprite* m_sprite_ball[BALLS];
    Sprite* m_sprite_paddle = nullptr;
    Sprite* m_sprite_saucer = nullptr;

    int m_width_wnd;
    int m_height_wnd;

    static std::unique_ptr<Game> m_instance;

    // paddle
    bool m_left;
    bool m_right;
    int m_num_lives;
    bool m_game_over;
    bool m_game_win;
    bool next_level;

    // level
    int m_level;
    int m_count_blocks;

    // balls
    int m_vel_x;
    int m_vel_y;

    bool m_pause;

    bool m_paddleBonusActive;
    QTimer* m_paddleBonusTimer;

    int m_originalPaddleWidth;
    int m_originalPaddleHeight;

    bool m_fastBallActive;
    QTimer* m_fastBallTimer;
    float m_fastBallSpeedFactor;   // множитель скорости (например, 1.5)
    int m_currentMaxBallSpeed;

    QSoundEffect* m_blockHitSound;
    QSoundEffect* m_solidBlockHitSound;
    QSoundEffect* m_paddleHitSound;
};

#endif // GAME_H
