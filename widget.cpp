#include "widget.h"
#include "ui_widget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QShowEvent>
#include <QDebug>
#include "game.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui_Widget)
    , m_left(false)
    , m_right(false)
    , m_pause(false)
    , m_width(644)
    , m_height(458)
{
    ui->setupUi(this);

    resize(m_width, m_height);
    m_timer_id = startTimer(8);

    Game::getInstance()->gameInitialize(width(), height());
    Game::getInstance()->gameStart();
}

Widget::~Widget()
{
    killTimer(m_timer_id);
    Game::getInstance()->gameEnd();
}

void Widget::showEvent(QShowEvent*)
{
    Game::getInstance()->gameActivate();
}

void Widget::hideEvent(QHideEvent *)
{
    Game::getInstance()->gameDeactivate();
}

void Widget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

//    int side = qMin(width(), height());
//    int x = ((width() - side)/2);
//    int y = ((height() - side)/2);
//    p.setViewport(x, y, side, side);

    p.setWindow(0,0, m_width, m_height);

    Game::getInstance()->gamePaint(&p);
}

void Widget::timerEvent(QTimerEvent *)
{
    Game::getInstance()->gameCycle();
    update();
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left)
    {
        m_left = true;
    }
    else if(event->key() == Qt::Key_Right)
    {
        m_right = true;
    }

    if(event->key() == Qt::Key_P)
    {
        m_pause = !m_pause;
    }

    Game::getInstance()->handleKeys(m_left, m_right, m_pause);
}

void Widget::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Left)
    {
        m_left = false;
    }
    else if(event->key() == Qt::Key_Right)
    {
        m_right = false;
    }

    Game::getInstance()->handleKeys(m_left, m_right, m_pause);
}
