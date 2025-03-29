#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>

#include<QMediaPlayer>
#include <QtAV>
#include <DMessageBox>
DWIDGET_USE_NAMESPACE
class VideoPlayer : public QWidget {
    Q_OBJECT

public:
    ~VideoPlayer();

        static VideoPlayer* instance();
    // 播放控制
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

signals:
    void positionChanged(qint64 position); // 播放位置变化
    void durationChanged(qint64 duration); // 总时长变化
    void stateChanged(QtAV::AVPlayer::State state); // 播放状态变化
    void errorOccurred(const QString &error);       // 错误信息
    void    mediaStatusChanged(QtAV::MediaStatus status);

private:
    QtAV::AVPlayer *m_player;      // QtAV 播放器核心
    QtAV::VideoOutput *m_videoOutput; // 视频渲染器

    VideoPlayer(QWidget *parent = nullptr);
    static VideoPlayer *s_instance;
};

#endif // VIDEOPLAYER_H
