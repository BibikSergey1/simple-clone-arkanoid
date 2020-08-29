#ifndef GAME_H
#define GAME_H

#include "GameEngine.h"
#include "sprite.h"
#include "Background.h"
#include <qobject.h>
#include <memory>

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

private slots:

private:
    Game(QObject* parent = nullptr);
    Game( const Game& );
    Game& operator=( Game& );

    void createLevel();
    void createNewLevel(Sprite* pSpriteHitter);
    void createWalls();
    void collisBallBricks(Sprite* pSpriteHitter, Sprite* pSpriteHittee);
    void collisBallPaddle(Sprite* pSpriteHitter, Sprite* pSpriteHittee);
    void collisBonusPaddle(Sprite* pSpriteHitter, Sprite* pSpriteHittee);
    int random(int a, int b);
    void processKeys();
    void checkRandomBonus(Sprite* pSpriteHitter);

    QRect m_bounds;
    std::unique_ptr<GameEngine> m_game_engine;

    QPixmap m_pixmap_block;
    QPixmap m_pixmap_block_blue;
    QPixmap m_pixmap_ball;
    QPixmap m_pixmap_wall_h;
    QPixmap m_pixmap_wall_v;
    QPixmap m_pixmap_paddle;
    QPixmap m_pixmap_paddle_sm;
    QPixmap m_pixmap_game_over;
    QPixmap m_pixmap_win;
    QPixmap m_pixmap_pause;
    QPixmap m_bonus_red_star;

    std::unique_ptr<StarryBackground> m_background;

    Sprite* m_sprite_wall_h;
    Sprite* m_sprite_wall_v;
    Sprite* m_sprite_block;
    enum {BALLS=2};
    Sprite* m_sprite_ball[BALLS];
    Sprite* m_sprite_paddle;

    int m_width_wnd;
    int m_height_wnd;

    static std::unique_ptr<Game> m_instance;

    // field
    int m_num_rows = 14;
    int m_num_cols = 19;

    // paddle
    bool m_left;
    bool m_right;
    QRect m_rect_centr_paddle;
    QRect m_rect_left_paddle;
    QRect m_rect_right_paddle;
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
    int m_count_balls;

    bool m_pause;
};

#endif // GAME_H
