#ifndef SPRITE_H
#define SPRITE_H

#include <QPixmap>
#include "common.h"
#include <QObject>

class Sprite : public QObject
{

public:
    Sprite(QPixmap pixmap, QObject* parent = nullptr);
    Sprite(QPixmap pixmap, QRect rect_bounds,
        BOUNDSACTION bounds_action = BA_STOP, QObject* parent = nullptr);

    virtual ~Sprite();

    void setVelocity(int x, int y);
    void setVelocity(QPoint point_velocity);

    QRect& getPosition(){ return rect_position_; }
    void setPosition(QPoint point_position);
    void setPosition(QRect rcPosition);
    void setPosition(int x, int y);

    virtual SPRITEACTION update();
    void draw(QPainter* p);

    QRect& getCollision() { return rect_collision_; }
    bool testCollision(Sprite* pTestSprite);

    QPoint getVelocity() { return point_velocity_; }

    bool getZOrder(){ return zorder_; }
    void setZOrder(int iZOrder){ zorder_ = iZOrder; }

    int getWidth() { return (pixmap_.width() / num_frames_); }
    int getHeight() { return pixmap_.height(); }

    void setNumFrames(int num_frames, bool one_cycle = false);
    void setFrameDelay(int frame_delay) { frame_delay_ = frame_delay; }

    const QPixmap& getPixmap() { return pixmap_; }
    void setPixmap(const QPixmap& pixmap) { pixmap_ = pixmap; }

    void kill() { dying_ = true; }

    virtual Sprite* addSprite();

    bool isHidden() { return hidden_; }
    void setHidden(bool hidden) { hidden_ = hidden; }

protected:
    QPixmap pixmap_;

    int num_frames_;
    int cur_frame_;
    int frame_delay_;
    int frame_trigger_;

    QRect rect_position_;
    QRect rect_collision_;
    QPoint point_velocity_;
    int zorder_;
    QRect rect_bounds_; // ограничивающий прямоугольник

    BOUNDSACTION  bounds_action_;
    bool hidden_;

    bool dying_;
    bool one_cycle_;

    // Helper Methods
    virtual void  calcCollisionRect();
    void updateFrame();
};

#endif // SPRITE_H
