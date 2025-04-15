#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H


#include"controlbar.h"
#include <QWidget>
#include<QTimer>
#include<DTitlebar>

#include<DMainWindow>
#include <QtAV>
#include <DMessageBox>
#include<QList>
DWIDGET_USE_NAMESPACE
class VideoPlayer : public DMainWindow {
    Q_OBJECT

public:
    QWidget* widget();
    ~VideoPlayer();

    bool enable=0;
    QtAV::AVPlayer *m_player;      // QtAV 播放器核心
    bool manualStopped=0;
    ControlBar * m_controlBar;
    QString filePath="1";
    static VideoPlayer* instance();
    // 播放控制
    bool isPlaying();
    void play(const QString &url);
    void play();
    void pause();
    void stop();

    QtAV::AVPlayer* player();
    // 属性设置
    void setVolume(int volume);          // 0~100
    void setPosition(qint64 position);   // 毫秒
    void setSpeed(qreal speed);          // 0.5x~2.0x

    // 状态获取
    qint64 duration() const;             // 总时长（毫秒）
    qint64 position() const;             // 当前播放位置（毫秒）
    QtAV::AVPlayer::State state() const; // 播放状态
    void closeVideoPage();

signals:
//    void positionChanged(qint64 position); // 播放位置变化
//    void durationChanged(qint64 duration); // 总时长变化
    void stateChanged(QtAV::AVPlayer::State state); // 播放状态变化
    void errorOccurred(const QString &error);       // 错误信息
    void mediaStatusChanged(QtAV::MediaStatus status);
    void toCloseVideo();

    void toShowVideo();


signals:
    void historyListChange(HistoryMData& item);
signals:
    void historyListRemove(int index);
    void videoStarted();
    void videoStopped();

private:

    QList<QString> m_urlList;
    QtAV::VideoOutput *m_videoOutput; // 视频渲染器
    QTimer *m_timer;
    //用于为鼠标移动事件计时
    QTimer *m_timer1;
    bool isFull=0;
    QPoint m_dragPos;
    bool m_mousePressing=0;


    void showControlBar();

    void hideControlBar() ;


void onMediaChange(QtAV::MediaStatus state);

    VideoPlayer();
    static VideoPlayer *s_instance;
protected:
    void handleSingleClick();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

public slots:
    void onStateChanged(QtAV::AVPlayer::State state);
    void onShiftScreen();
    void onStopped();

    void onStarted();
};

#endif // VIDEOPLAYER_H
