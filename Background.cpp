//-----------------------------------------------------------------
// Background Object
// C++ Source - Background.cpp
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Include Files
//-----------------------------------------------------------------
#include "Background.h"
#include <QPainter>
#include <QDebug>

//-----------------------------------------------------------------
// Background Constructor(s)/Destructor
//-----------------------------------------------------------------
Background::Background(int width, int height, QColor color)
{
    // Initialize the member variables
    width_ = width;
    height_ = height;
    color_ = color;
}

Background::Background(QPixmap& pixmap)
{
    // Initialize the member variables
    color_ = QColor(0,0,0);
    pixmap_ = pixmap;
    width_ = pixmap_.width();
    height_ = pixmap_.height();
}

Background::~Background()
{
}

//-----------------------------------------------------------------
// Background General Methods
//-----------------------------------------------------------------
void Background::update()
{
  // Do nothing since the basic background is not animated
}

void Background::draw(QPainter* p)
{
    // Draw the background
    if (p != nullptr && !pixmap_.isNull())
    {
        p->drawPixmap(0, 0, pixmap_.width(), pixmap_.height(), pixmap_);
    }
    else
    {
        QRect rect = QRect(0, 0, width_, height_);
        pixmap_.fill(color_);
        p->drawPixmap(rect, pixmap_);
    }
}

//-----------------------------------------------------------------
// StarryBackground Constructor
//-----------------------------------------------------------------
StarryBackground::StarryBackground(int width, int height, int num_stars, int twinkle_delay)
    : Background(width, height, QColor(0,0,0))
{
    // Initialize the member variables
    num_stars_ = qMin(num_stars, 100);
    twinkle_delay_ = twinkle_delay;

    // Create the stars
    for (int i = 0; i < num_stars; i++)
    {
        stars_[i].setX(qrand() % width);
        stars_[i].setY(qrand() % height);
        star_colors_[i] = QColor(128, 128, 128);
    }
}

StarryBackground::~StarryBackground()
{
}

//-----------------------------------------------------------------
// StarryBackground General Methods
//-----------------------------------------------------------------
void StarryBackground::update()
{
    // Randomly change the shade of the stars so that they twinkle
    int rgb;
    for (int i = 0; i < num_stars_; i++)
    {
        if ((qrand() % twinkle_delay_) == 0)
        {
            rgb = qrand() % 256;
            star_colors_[i] = QColor(rgb, rgb, rgb);
        }
    }
}

void StarryBackground::draw(QPainter* p)
{
    // Draw the solid black background
    QRect rect = QRect(0, 0, width_, height_);
    p->fillRect(rect, QColor(0,0,0));

    // Draw the stars
    for (int i = 0; i < num_stars_; i++)
        p->fillRect(stars_[i].x(), stars_[i].y(), 1, 1, star_colors_[i]);
}
