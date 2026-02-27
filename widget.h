#pragma once

#include <QWidget>
#include <QEvent>
#include <memory>

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
