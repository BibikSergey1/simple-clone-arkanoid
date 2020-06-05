//-----------------------------------------------------------------
// Background Object
// C++ Header - Background.h
//-----------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include <QPixmap>

//-----------------------------------------------------------------
// Background Class
//-----------------------------------------------------------------
class Background
{
protected:
    // Member Variables
    int       width_, height_;
    QColor    color_;
    QPixmap   pixmap_;

public:
    // Constructor(s)/Destructor
    Background(int width, int height, QColor color);
    Background(QPixmap& pixmap);
    virtual ~Background();

    // General Methods
    virtual void  update();
    virtual void  draw(QPainter* p);

    // Accessor Methods
    int getWidth()  { return width_; }
    int getHeight() { return height_; }
};

//-----------------------------------------------------------------
// Starry Background Class
//-----------------------------------------------------------------
class StarryBackground : Background
{
protected:
    // Member Variables
    int       num_stars_;
    int       twinkle_delay_;
    QPoint    stars_[100];
    QColor  star_colors_[100];

public:
    // Constructor(s)/Destructor
    StarryBackground(int width, int height, int num_stars = 100, int twinkle_delay = 50);
    virtual ~StarryBackground();

    // General Methods
    virtual void  update();
    virtual void  draw(QPainter* p);
};
