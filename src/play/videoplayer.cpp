#include "videoplayer.h"
#include <QVBoxLayout>
#include<QDebug>
#include<QMouseEvent>
#include<QApplication>
#include<QKeyEvent>
/// --- PlaybackWorker 实现 ---
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
    m_timer1= new QTimer(this);
    m_timer1->setInterval(3000);

    DTitlebar * titleBar= this->titlebar();
    titleBar->setMenuVisible(false);
    connect(m_timer, &QTimer::timeout, this, &VideoPlayer::handleSingleClick);

    // 初始化播放器
    m_player = new QtAV::AVPlayer(this);

    // 初始化视频渲染器
    m_videoOutput = new QtAV::VideoOutput(this);
    if (!m_videoOutput->widget()) {
        DMessageBox::critical(nullptr, "Error", "Failed to create video renderer!");
        return;
    }
    auto widget = m_videoOutput->widget();

    widget->setParent(nullptr);
   widget->setWindowFlags(Qt::Window);

    m_player->setRenderer(m_videoOutput);
    QVBoxLayout * layout = new QVBoxLayout;
    widget->setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 20);
    layout->setSpacing(0);

    layout->addStretch(8);
    QHBoxLayout* DownHLayout= new QHBoxLayout;
    m_controlBar = new ControlBar(this,true);
    m_controlBar->isVideo=true;
    m_controlBar->shiftThemeIcon(false);
    widget->setMouseTracking(true);



    DownHLayout->addWidget(m_controlBar);
    DownHLayout->setContentsMargins(20,0,20,3);

    layout->addLayout(DownHLayout, 1);

    connect(m_player, &QtAV::AVPlayer::positionChanged, this, &VideoPlayer::positionChanged);
    connect(m_player, &QtAV::AVPlayer::durationChanged, this, &VideoPlayer::durationChanged);
    connect(m_player, &QtAV::AVPlayer::stateChanged, this, &VideoPlayer::stateChanged);
    //    connect(m_player, &QtAV::AVPlayer::error, this, [this](const QtAV::AVError &err) {
    //        emit errorOccurred(err.string());
    //    });

    connect(m_player, &QtAV::AVPlayer::mediaStatusChanged, this,&VideoPlayer::onMediaChange);
    connect(m_timer1, &QTimer::timeout, this, &VideoPlayer::hideControlBar);
    connect(m_player,&QtAV::AVPlayer::started,this,&VideoPlayer::onStarted);

    connect(m_player,&QtAV::AVPlayer::stopped,this,&VideoPlayer::onStopped);
    installEventFilter(this);
    widget->installEventFilter(this);
    m_controlBar->installEventFilter(this);


}
void VideoPlayer::closeEvent(QCloseEvent *event) {
    event->ignore();
    hide();
    emit toCloseVideo();
}// —— 捕获中央 videoWidget & controlBar 的鼠标事件
bool VideoPlayer::eventFilter(QObject *obj, QEvent *event) {

    // 视频区双击切换全屏
    if (obj ==widget()) {
        if(event->type()==QEvent::Close){
              event->ignore();
    widget()->hide();
    emit toCloseVideo();
return true;
        }
        if (event->type() == QEvent::MouseButtonDblClick) {
            m_timer->stop();
            onShiftScreen();
            return true;
        }
        if (event->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton)
                m_timer->start(QApplication::doubleClickInterval());
            return false;
        }
           }
       if (event->type() == QEvent::MouseMove) {
            showControlBar();
            m_timer1->start();
                   }

    return DMainWindow::eventFilter(obj, event);
}

void VideoPlayer::showControlBar() {
    if (!m_controlBar->isVisible())
    {
        m_controlBar->setVisible(true);
    }
}
void VideoPlayer::hideControlBar() {
    m_controlBar->setVisible(false);

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
void VideoPlayer::onShiftScreen(){

    if(isFull){

      widget()-> showNormal()
                        ;    }
    else{
      widget()-> showFullScreen();

    }
    isFull^=1;
}
void VideoPlayer::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && isFull) {
       widget()->showNormal();
        isFull = false;
        event->accept();
        return;
    }
    DMainWindow::keyPressEvent(event);
}
void VideoPlayer::mouseMoveEvent(QMouseEvent *event)
{
    showControlBar();
    m_timer1->start();
    DMainWindow::mouseMoveEvent(event);
}

// 播放指定文件
// 第一次播放并打开窗口
void VideoPlayer::play(const QString &url) {

    m_urlList.push_back(url);
    if(m_isPlaying)
    m_player->stop();
    else{
          filePath=url;
    m_player->play(url);
    qDebug()<<url;
    manualStopped=0;
    emit toShowVideo();

    m_timer1->start();

    }

}

bool VideoPlayer::isPlaying(){
    return m_isPlaying;
}
VideoPlayer::~VideoPlayer() {
    stop();
    delete m_videoOutput;
    delete m_player;
}

void VideoPlayer::closeVideoPage(){

    emit toCloseVideo();


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

    m_timer1->stop();
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
void VideoPlayer::onMediaChange(QtAV::MediaStatus state){
   emit mediaStatusChanged(state);

}

void VideoPlayer::onStarted(){
    m_isPlaying=1;

        auto duration = m_player->duration();
            QString title = QFileInfo(filePath).completeBaseName();

       MusicPlayer::instance(). history.addToHistory(
                    HistoryMData(filePath,
                                 title, duration/1000));

        emit historyListChange(MusicPlayer::instance().history.history.first());


   emit videoStarted();
}
void VideoPlayer::onStopped(){

    m_isPlaying=0;
    if(!m_urlList.empty()){
     auto url = m_urlList.front();
     m_urlList.pop_front();
    filePath=url;
    m_player->play(url);
    qDebug()<<url;
    manualStopped=0;
    emit toShowVideo();

    m_timer1->start();

    }

    emit videoStopped();

}
