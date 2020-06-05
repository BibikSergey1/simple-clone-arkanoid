#include "sprite.h"
#include <QPainter>
#include <QDebug>

Sprite::Sprite(QPixmap pixmap, QObject* parent)
    : QObject(parent)
    , pixmap_(pixmap)
    , num_frames_(1)
    , cur_frame_(0)
    , frame_delay_(0)
    , frame_trigger_(0)
    , rect_position_(0, 0, pixmap.width(), pixmap.height())
    , point_velocity_(0,0)
    , zorder_(0)
    , rect_bounds_(0, 0, 640, 480)
    , bounds_action_(BA_STOP)
    , hidden_(false)
    , dying_(false)
    , one_cycle_(false)
{
    calcCollisionRect();
}

Sprite::Sprite(QPixmap pixmap, QRect rect_bounds,
               BOUNDSACTION bounds_action, QObject* parent)
    : QObject(parent)
    , pixmap_(pixmap)
    , num_frames_(1)
    , cur_frame_(0)
    , frame_delay_(0)
    , frame_trigger_(0)
    , point_velocity_(0,0)
    , zorder_(0)
    , rect_bounds_(rect_bounds)
    , bounds_action_(bounds_action)
    , hidden_(false)
    , dying_(false)
    , one_cycle_(false)
{
    // Calculate a random position
    int xpos = qrand() % (rect_bounds.right() - rect_bounds.left());
    int ypos = qrand() % (rect_bounds.bottom() - rect_bounds.top());

    // Initialize the member variables
    rect_position_ = QRect(xpos, ypos, pixmap.width(), pixmap.height());
    calcCollisionRect();
}

Sprite::~Sprite()
{
}

void Sprite::calcCollisionRect()
{
    int iXShrink = (rect_position_.left() - rect_position_.right()) / 12;
    int iYShrink = (rect_position_.top() - rect_position_.bottom()) / 12;

    rect_collision_ = rect_position_;
    rect_collision_.translate(iXShrink, iYShrink);
}

void Sprite::updateFrame()
{
  if ((frame_delay_ >= 0) && (--frame_trigger_ <= 0))
  {
    // Reset the frame trigger;
    frame_trigger_ = frame_delay_;

    // Increment the frame
    if (++cur_frame_ >= num_frames_)
    {
        // If it's a one-cycle frame animation, kill the sprite
        if (one_cycle_)
            dying_ = true;
        else
            cur_frame_ = 0;
    }
  }
}

bool Sprite::testCollision(Sprite* pTestSprite)
{
    QRect& rcTest = pTestSprite->getCollision();
    return rect_collision_.left() <= rcTest.right() &&
         rcTest.left() <= rect_collision_.right() &&
         rect_collision_.top() <= rcTest.bottom() &&
         rcTest.top() <= rect_collision_.bottom();
}

void Sprite::setVelocity(int x, int y)
{
    point_velocity_.setX(x);
    point_velocity_.setY(y);
}

void Sprite::setVelocity(QPoint point_velocity)
{
    point_velocity_.setX(point_velocity.x());
    point_velocity_.setY(point_velocity.y());
}

void Sprite::draw(QPainter *p)
{
    // Draw the sprite if it isn't hidden
    if (!hidden_)
    {
        if (num_frames_ == 1)
            p->drawPixmap(rect_position_.left(), rect_position_.top(), pixmap_);
        else
            p->drawTiledPixmap(rect_position_.left(), rect_position_.top(),
                               rect_position_.width(), rect_position_.height(),
                               pixmap_, cur_frame_ * getWidth(), 0);
    }
}

void Sprite::setNumFrames(int num_frames, bool one_cycle)
{
  // Set the number of frames
  num_frames_ = num_frames;
  one_cycle_ = one_cycle;

  // Recalculate the position
  QRect rect = getPosition();
  rect.setRight(rect.left() + ((rect.right() - rect.left()) / num_frames));
  setPosition(rect);
}

void Sprite::setPosition(QPoint point_position)
{
    rect_position_.moveTo(point_position);
    calcCollisionRect();
}

void Sprite::setPosition(int x, int y)
{
    rect_position_.moveTo(x, y);
    calcCollisionRect();
}

void Sprite::setPosition(QRect rcPosition)
{
    rect_position_ = rcPosition;
    calcCollisionRect();
}

SPRITEACTION Sprite::update()
{
    // See if the sprite needs to be killed
    if (dying_)
        return SA_KILL;

    // Update the frame
    updateFrame();

    // Update the position
    QPoint ptNewPosition, ptSpriteSize, ptBoundsSize;

    ptNewPosition.setX(rect_position_.left() + point_velocity_.x());
    ptNewPosition.setY(rect_position_.top() + point_velocity_.y());

    ptSpriteSize.setX(rect_position_.right() - rect_position_.left());
    ptSpriteSize.setY(rect_position_.bottom() - rect_position_.top());

    ptBoundsSize.setX(rect_bounds_.right() - rect_bounds_.left());
    ptBoundsSize.setY(rect_bounds_.bottom() - rect_bounds_.top());

    // Check the bounds
    // Wrap?
    if (bounds_action_ == BA_WRAP)
    {
        if ((ptNewPosition.x() + ptSpriteSize.x()) < rect_bounds_.left())
          ptNewPosition.setX(rect_bounds_.right());
        else if (ptNewPosition.x() > rect_bounds_.right())
          ptNewPosition.setX(rect_bounds_.left() - ptSpriteSize.x());

        if ((ptNewPosition.y() + ptSpriteSize.y()) < rect_bounds_.top())
          ptNewPosition.setY(rect_bounds_.bottom());
        else if (ptNewPosition.y() > rect_bounds_.bottom())
          ptNewPosition.setY(rect_bounds_.top() - ptSpriteSize.y());
    }
    // Bounce?
    else if (bounds_action_ == BA_BOUNCE)
    {
        bool bBounce = false;
        QPoint ptNewVelocity = point_velocity_;
        if (ptNewPosition.x() < rect_bounds_.left())
        {
            bBounce = true;
            ptNewPosition.setX(rect_bounds_.left());
            ptNewVelocity.setX(-ptNewVelocity.x());
        }
        else if ((ptNewPosition.x() + ptSpriteSize.x()) > rect_bounds_.right())
        {
            bBounce = true;
            ptNewPosition.setX(rect_bounds_.right() - ptSpriteSize.x());
            ptNewVelocity.setX(-ptNewVelocity.x());
        }
        if (ptNewPosition.y() < rect_bounds_.top())
        {
            bBounce = true;
            ptNewPosition.setY(rect_bounds_.top());
            ptNewVelocity.setY(-ptNewVelocity.y());
        }
        else if ((ptNewPosition.y() + ptSpriteSize.y()) > rect_bounds_.bottom())
        {
            bBounce = true;
            ptNewPosition.setY(rect_bounds_.bottom() - ptSpriteSize.y());
            ptNewVelocity.setY(-ptNewVelocity.y());
        }

        if (bBounce)
            setVelocity(ptNewVelocity);
    }
    // Die?
    else if (bounds_action_ == BA_DIE)
    {
        if ((ptNewPosition.x() + ptSpriteSize.x()) < rect_bounds_.left() ||
                ptNewPosition.x() > rect_bounds_.right() ||
                (ptNewPosition.y() + ptSpriteSize.y()) < rect_bounds_.top() ||
                ptNewPosition.y() > rect_bounds_.bottom())
            return SA_KILL;
    }
    // Stop (default)
    else
    {
        if (ptNewPosition.x()  < rect_bounds_.left() ||
                ptNewPosition.x() > (rect_bounds_.right() - ptSpriteSize.x()))
        {
            ptNewPosition.setX(qMax(rect_bounds_.left(), qMin(ptNewPosition.x(),
                                         rect_bounds_.right() - ptSpriteSize.x())));
            setVelocity(0, 0);
        }
        if (ptNewPosition.y()  < rect_bounds_.top() ||
        ptNewPosition.y() > (rect_bounds_.bottom() - ptSpriteSize.y()))
        {
            ptNewPosition.setY(qMax(rect_bounds_.top(), qMin(ptNewPosition.y(),
                                                             rect_bounds_.bottom() - ptSpriteSize.y())));
            setVelocity(0, 0);
        }
    }

  setPosition(ptNewPosition);

  return SA_NONE;
}

Sprite* Sprite::addSprite()
{
    return nullptr;
}
