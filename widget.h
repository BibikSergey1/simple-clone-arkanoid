#pragma once

#include <QWidget>
#include <QEvent>
#include <memory>

namespace
{
constexpr int CELL_SIZE = 32;
constexpr int GRID_GAP = 1;
constexpr int WOLLS_GAP = 1;

constexpr int GRID_ROWS = 20;
constexpr int GRID_COLS = 20;

constexpr int WINDOW_WIDTH = GRID_COLS * CELL_SIZE + GRID_COLS * WOLLS_GAP;
constexpr int WINDOW_HEIGHT = GRID_ROWS * CELL_SIZE + GRID_ROWS * WOLLS_GAP;

constexpr QSize BLOCK_SIZE(40, 25);
constexpr QSize BALL_SIZE(12, 12);
constexpr QSize PADDLE_SMALL(22, 8);
constexpr QSize PADDLE_NORMAL(80, 20);

static constexpr int PADDLE_Y = 600;
static constexpr int PADDLE_GAP_LEFT = 5;
static constexpr int PADDLE_GAP_RIGHT = 10;
static constexpr int PADDLE_MARGIN_LEFT = BLOCK_SIZE.height() - PADDLE_GAP_LEFT;
static constexpr int PADDLE_MARGIN_RIGHT = BLOCK_SIZE.height() * 2 - PADDLE_GAP_RIGHT;
static constexpr int PADDLE_MAX_SPEED = 6;

static constexpr int DELAY_MS = 1500;

constexpr int BALL_SPEED = 4;  // 3 Умеренная скорость, 4 Быстрая скорость
constexpr int MAX_BALL_SPEED = 4;

static constexpr int NUM_BLOCK_ROWS = 6;
static constexpr int NUM_BLOCK_COLS = 11;
static constexpr int BLOCK_X_GAP = BLOCK_SIZE.width() + GRID_GAP;
static constexpr int BLOCK_Y_GAP = BLOCK_SIZE.height() + GRID_GAP;
static constexpr int BLOCK_ORIGIN_X = BLOCK_SIZE.width() * 2 + BLOCK_SIZE.height();
static constexpr int BLOCK_ORIGIN_Y = 160;

}

class Ui_Widget;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void timerEvent(QTimerEvent *) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    std::unique_ptr<Ui_Widget> ui;
    int m_timer_id = 0;

    bool m_left = false;
    bool m_right = false;
    bool m_pause = false;

    int m_contentWidth = 0;   // Ширина игрового поля/контента
    int m_contentHeight = 0;  // Высота игрового поля/контента
};
