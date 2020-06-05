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
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual void paintEvent(QPaintEvent *);
    virtual void timerEvent(QTimerEvent *);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

private:
    std::unique_ptr<Ui_Widget> ui;
    int m_timer_id;

    bool m_left;
    bool m_right;
    bool m_pause;

    int m_width;
    int m_height;
};
