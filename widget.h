#pragma once

#include <QWidget>
#include <QEvent>
#include <memory>

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

constexpr QSize BLOCK_SIZE(40, 25);
constexpr QSize BALL_SIZE(12, 12);
constexpr QSize PADDLE_SMALL(22, 8);
constexpr QSize PADDLE_NORMAL(80, 20);
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
