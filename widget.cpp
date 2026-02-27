#include "widget.h"
#include "ui_widget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QShowEvent>
#include <QDebug>
#include <QTime>
#include "game.h"

namespace
{
constexpr int CELL_SIZE = 32;
constexpr int GRID_GAP = 2;
constexpr int BORDER_WIDTH = 2;
constexpr int FIRST_ROW_OFFSET = 16;

constexpr int GRID_COLS = 19;
constexpr int GRID_ROWS = 13;

constexpr int WINDOW_WIDTH = GRID_COLS * CELL_SIZE + (GRID_COLS - 1) * GRID_GAP - BORDER_WIDTH;
constexpr int WINDOW_HEIGHT = GRID_ROWS * CELL_SIZE + (GRID_ROWS - 1) * GRID_GAP - FIRST_ROW_OFFSET - BORDER_WIDTH;
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui_Widget)
    , m_left(false)
    , m_right(false)
    , m_pause(false)
    , m_contentWidth(WINDOW_WIDTH)
    , m_contentHeight(WINDOW_HEIGHT)
{
    ui->setupUi(this);
    resize(m_contentWidth, m_contentHeight);

    m_timer_id = startTimer(8);

    auto* game = Game::getInstance();
    if (game)
    {
        game->gameInitialize(m_contentWidth, m_contentHeight);
        game->gameStart();
        qDebug() << "Game started";
    }
    else
    {
        qCritical() << "Failed to initialize game";
        close();
    }

    qDebug() << "Widget created, size:" << m_contentWidth << "x" << m_contentHeight;
}

Widget::~Widget()
{
    if (m_timer_id != 0)
    {
        killTimer(m_timer_id);
        m_timer_id = 0;
        qDebug() << "Timer stopped";
    }

    // Уведомляем игру о завершении
    if (auto* game = Game::getInstance())
    {
        game->gameEnd();
        qDebug() << "Game ended";
    }
    else
    {
        qWarning() << "Game instance already destroyed";
    }
}

void Widget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    if (auto* game = Game::getInstance())
    {
        game->gameActivate();
        qDebug() << "Game activated (widget shown)";
    }
}

void Widget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);

    if (auto* game = Game::getInstance())
    {
        game->gameDeactivate();
        qDebug() << "Game deactivated (widget hidden)";
    }
}

void Widget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    static int frameCount = 0;
    static int fps = 0;
    static QTime lastTime = QTime::currentTime();

    frameCount++;

    if (lastTime.msecsTo(QTime::currentTime()) >= 1000)
    {
        fps = frameCount;
        frameCount = 0;
        lastTime = QTime::currentTime();
        qDebug() << "FPS:" << fps;  // Для отладки
    }

    QPainter p(this);
    //    int side = qMin(width(), height());
    //    int x = ((width() - side)/2);
    //    int y = ((height() - side)/2);
    //    p.setViewport(x, y, side, side);
    p.setWindow(0, 0, m_contentWidth, m_contentHeight);

    if (auto* game = Game::getInstance())
    {
        game->gamePaint(&p);
    }

    // Опционально: показываем FPS на экране
    p.setPen(Qt::green);
    p.drawText(10, 20, QString("FPS: %1").arg(fps));
}

void Widget::timerEvent(QTimerEvent *event)
{
    QWidget::timerEvent(event);

    // Проверяем ID таймера
    if (event->timerId() != m_timer_id)
    {
        return;
    }

    // Проверяем паузу
    if (m_pause)
    {
        return;
    }

    // Проверяем игру
    auto* game = Game::getInstance();
    if (!game)
    {
        // Если игры нет — останавливаем таймер
        killTimer(m_timer_id);
        m_timer_id = 0;
        return;
    }

    // Игровой цикл
    game->gameCycle();
    update();

    // Для отладки (можно убрать в релизе)
    static int tickCount = 0;
    tickCount++;
    if (tickCount % 125 == 0)
    {  // Примерно раз в секунду
        qDebug() << "Game cycle tick:" << tickCount;
    }

}

void Widget::keyPressEvent(QKeyEvent *event)
{
    // Игнорируем автоповтор, чтобы не спамить
    if (event->isAutoRepeat())
    {
        return;
    }

    bool changed = false;

    switch(event->key())
    {
    case Qt::Key_Left:
        m_left = true;
        changed = true;
        qDebug() << "Left pressed";
        break;

    case Qt::Key_Right:
        m_right = true;
        changed = true;
        qDebug() << "Right pressed";
        break;

    case Qt::Key_P:
    case Qt::Key_Pause:  // Поддержка клавиши Pause
        m_pause = !m_pause;
        changed = true;
        qDebug() << "Pause toggled:" << (m_pause ? "ON" : "OFF");
        break;

    case Qt::Key_Escape:  // Добавим выход по Escape
        close();
        return;

    default:
        QWidget::keyPressEvent(event);
        return;
    }

    if (changed)
    {
        if (auto* game = Game::getInstance())
        {
            game->handleKeys(m_left, m_right, m_pause);
        }
    }
}

void Widget::keyReleaseEvent(QKeyEvent *event)
{
    // Игнорируем автоповтор
    if (event->isAutoRepeat())
    {
        return;
    }

    bool changed = false;

    switch(event->key())
    {
    case Qt::Key_Left:
        m_left = false;
        changed = true;
        qDebug() << "Left released";
        break;

    case Qt::Key_Right:
        m_right = false;
        changed = true;
        qDebug() << "Right released";
        break;

    default:
        QWidget::keyReleaseEvent(event);
        return;
    }

    if (changed)
    {
        if (auto* game = Game::getInstance())
        {
            game->handleKeys(m_left, m_right, m_pause);
        }
    }
}
