#include "videoplayer.h"
#include <QVBoxLayout>
#include<QDebug>
#include<QMouseEvent>
#include<QApplication>
VideoPlayer*VideoPlayer::s_instance = nullptr;
VideoPlayer*VideoPlayer::instance()
{
    if (s_instance == nullptr) {
        s_instance = new VideoPlayer();  // 创建唯一的实例
    }
    return s_instance;
}
QtAV::AVPlayer* VideoPlayer::player(){
    return m_player;
}
VideoPlayer::VideoPlayer()   {
    m_timer = new QTimer(this);
       m_timer->setSingleShot(true);
       connect(m_timer, &QTimer::timeout, this, &VideoPlayer::handleSingleClick);
    // 初始化播放器
    m_player = new QtAV::AVPlayer(this);

    // 初始化视频渲染器
    m_videoOutput = new QtAV::VideoOutput(this);
    if (!m_videoOutput->widget()) {
        DMessageBox::critical(nullptr, "Error", "Failed to create video renderer!");
        return;
    }
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_videoOutput->widget());
    setLayout(mainLayout);
    m_videoOutput->widget()->installEventFilter(this);

    // 绑定播放器和渲染器
    m_player->setRenderer(m_videoOutput);

    // 连接信号槽
    connect(m_player, &QtAV::AVPlayer::positionChanged, this, &VideoPlayer::positionChanged);
    connect(m_player, &QtAV::AVPlayer::durationChanged, this, &VideoPlayer::durationChanged);
    connect(m_player, &QtAV::AVPlayer::stateChanged, this, &VideoPlayer::stateChanged);
//    connect(m_player, &QtAV::AVPlayer::error, this, [this](const QtAV::AVError &err) {
//        emit errorOccurred(err.string());
//    });
    connect(m_player, &QtAV::AVPlayer::mediaStatusChanged, this,&VideoPlayer::mediaStatusChanged);

}

bool VideoPlayer::isPlaying(){
return m_player->isPlaying();
}
VideoPlayer::~VideoPlayer() {
    stop();
    delete m_videoOutput;
    delete m_player;
}

// 播放指定文件
void VideoPlayer::play(const QString &url) {
    m_player->stop();
    m_player->play(url);
    qDebug()<<url;
    manualStopped=0;
}

// 继续播放
void VideoPlayer::play() {
    if (m_player->isPlaying()) {
        m_player->pause(!m_player->isPaused()); // 切换暂停状态
    }
    else {

        m_player->play();
    }
}


// 暂停
void VideoPlayer::pause() {
    m_player->pause(true);

}
// 停止
void VideoPlayer::stop() {
    manualStopped=1;
    m_player->stop();
}

// 设置音量（0~100 → 转换为 0.0~1.0）
void VideoPlayer::setVolume(int volume) {
    m_player->audio()->setVolume(qBound(0, volume, 100) / 100.0);
}

// 跳转到指定位置（毫秒）
void VideoPlayer::setPosition(qint64 position) {
    m_player->seek(qMax(static_cast<qint64>(0), position));
}

// 设置播放速度（例如 1.0 为正常速度）
void VideoPlayer::setSpeed(qreal speed) {
    m_player->setSpeed(qBound(0.5, speed, 2.0));
}

// 获取总时长（毫秒）
qint64 VideoPlayer::duration() const {
    return m_player->duration();
}

// 获取当前播放位置（毫秒）
qint64 VideoPlayer::position() const {
    return m_player->position();
}

// 获取播放状态
QtAV::AVPlayer::State VideoPlayer::state() const {
    return m_player->state();
}

QWidget* VideoPlayer::widget(){
    return m_videoOutput->widget();
}

void VideoPlayer::shiftScreen(bool isFull){
    if(isFull){
        m_videoOutput->widget()->showNormal();
    }else{
        m_videoOutput->widget()->showFullScreen();
    }
}
bool VideoPlayer::eventFilter(QObject *obj, QEvent *event)
{
    if ( event->type() == QEvent::MouseButtonDblClick) {
        m_timer->stop();
        emit toShiftScreen();
        return true; // 事件已处理
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            m_timer->start(QApplication::doubleClickInterval());



        }
        return true;
    }

    return QWidget::eventFilter(obj, event);
}
void VideoPlayer::handleSingleClick()
{
    if(m_player->isPlaying()){
        play();
    }
    else{
        pause();
    }
  }
