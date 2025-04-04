#include "controlbar.h"
// #include"videoplayer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <DLabel>
#include <DIconButton>
#include <QFile>
#include <QRandomGenerator>
ControlBar::ControlBar(QWidget *parent) : DFrame(parent)
{

    this->setObjectName("control_bar");
    btspeed= new DIconButton(this);
    btplay = new DIconButton(this);
    btpre = new DIconButton(btplay);
    btstop = new DIconButton(btplay);
    btnex = new DIconButton(btplay);

    btloop = new DIconButton(btplay);
    btscreen = new DIconButton(btplay);

    btscreen->setDisabled(true);
    currenttime = 0;
    cTimer = new QTimer(this);

    // this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    QVBoxLayout *MainVlayout = new QVBoxLayout(this);
    this->setLayout(MainVlayout);
    MainVlayout->setContentsMargins(20,10,20,3);


    QHBoxLayout *UpHlayout = new QHBoxLayout();
    QHBoxLayout *DownHlayout = new QHBoxLayout();
    playtime = new DLabel(this);
    endtime = new DLabel(this);
    processSlider = new DSlider(Qt::Horizontal, this);
    volumeSlider = new DSlider(Qt::Horizontal, this);
    volumeSlider->setMinimum(0);
    volumeSlider->setMaximum(100);
    volumeSlider->setMaximumWidth(120);

    volumeSlider->setIconSize(QSize(22, 22));
    btplay->setIconSize(QSize(24, 24));
    btplay->setFixedSize(QSize(36, 36));
    btplay->setObjectName("bt_play");

    btpre->setIconSize(QSize(18, 18));
    btpre->setFixedSize(QSize(30, 30));
    btpre->setObjectName("bt_pre");
    // btpre->setStyleSheet("#bt_pre{ background-color: transparent;}");
    btspeed->setIconSize(QSize(36, 36));
    btspeed->setFixedSize(QSize(36, 36));
    btspeed->setObjectName("bt_speed");



    btstop->setIconSize(QSize(18, 18));
    btstop->setFixedSize(QSize(30, 30));
    btstop->setObjectName("bt_stop");

    btnex->setIconSize(QSize(18, 18));
    btnex->setFixedSize(QSize(30, 30));
    btnex->setObjectName("bt_next");

    //    btvolume->setIconSize(QSize(18, 18));
    //    btvolume->setObjectName("bt_volume");
    //    btvolume->setStyleSheet("#bt_volume{ background-color: transparent;}");
    btscreen->setIconSize(QSize(24, 24));
    btscreen->setObjectName("bt_screen");

    btscreen->setFixedSize(QSize(36, 36));


    btloop->setIconSize(QSize(36, 36));

    btloop->setObjectName("bt_loop");
    btloop->setFixedSize(QSize(36, 36));
    this->ChangeLoopBtIcon();
    //    QString styleSheet = "#bt_play:hover, #bt_pre:hover, #bt_stop:hover, #bt_next:hover, #bt_volume:hover, #bt_screen:hover { background-color: #f1f1f1; }"
    //                             "#bt_play:active, #bt_pre:active, #bt_stop:active, #bt_next:active, #bt_volume:active, #bt_screen:active { background-color: #ccc; }"
    //                             "#bt_play, #bt_pre, #bt_stop, #bt_next, #bt_volume, #bt_screen { background-color: transparent; }";

    //        // 应用样式到所有按钮
    //        btplay->setStyleSheet(styleSheet);
    //        btpre->setStyleSheet(styleSheet);
    //        btstop->setStyleSheet(styleSheet);
    //        btnex->setStyleSheet(styleSheet);
    //        btvolume->setStyleSheet(styleSheet);
    //        btscreen->setStyleSheet(styleSheet);

    playtime->setText("--.--");
    endtime->setText("--.--");
    MainVlayout->addLayout(UpHlayout);
    MainVlayout->addLayout(DownHlayout);
    MainVlayout->setStretch(0, 2);
    MainVlayout->setStretch(1, 1);
    UpHlayout->addWidget(playtime);
    UpHlayout->addWidget(processSlider);
    UpHlayout->addWidget(endtime);

    DownHlayout->addSpacing(4);
    DownHlayout->addWidget(btplay);
    DownHlayout->addSpacing(1);
    DownHlayout->addWidget(btpre);
    DownHlayout->addSpacing(0);
    DownHlayout->addWidget(btstop);
    DownHlayout->addWidget(btnex);
    DownHlayout->addSpacing(5);
    // DownHlayout->addWidget(btvolume);

    DownHlayout->addWidget(volumeSlider);
    DownHlayout->addSpacing(5);
    DownHlayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    DownHlayout->addWidget(btspeed);
    DownHlayout->addWidget(btloop);
    DownHlayout->addWidget(btscreen);
    connect(btplay, &DIconButton::clicked, this, &ControlBar::playslot);
    connect(btpre, &DIconButton::clicked, this, &ControlBar::preslot);
    connect(btstop, &DIconButton::clicked, this, &ControlBar::stopslot);
    connect(btnex, &DIconButton::clicked, this, &ControlBar::nexslot);
    connect(&MusicPlayer::instance(), &MusicPlayer::stateChanged, this, &ControlBar::musicStateChange);
    connect(&MusicPlayer::instance(), &MusicPlayer::mediaStatusChanged, this, &ControlBar::musicMediaChange);

    connect(VideoPlayer::instance(), &VideoPlayer::stateChanged, this, &ControlBar::videoStateChang);
    connect(VideoPlayer::instance(), &VideoPlayer::mediaStatusChanged, this, &ControlBar::videoMediaChange);
    connect(cTimer, &QTimer::timeout, this, &ControlBar::handleTimeout);
    connect(processSlider, &DSlider::valueChanged, this, &ControlBar::sliderchange);
    connect(processSlider, &DSlider::sliderReleased, this, &ControlBar::processsetting);
    connect(volumeSlider, &DSlider::valueChanged, this, &ControlBar::volumesetting);
    connect(btloop, &DIconButton::clicked, this, &ControlBar::onLoopChange);

    connect(btspeed, &DIconButton::clicked, this, &ControlBar::onSetSpeed);


    //    connect(volumeSlider, &DSlider::valueChanged, mediaPlayer, &QMediaPlayer::setVolume);
}
/// 读取设置之前的音量设置(todo
void ControlBar::readVolume(const QString &filePath)
{
    volumeSlider->setValue(preVolume);
    MusicPlayer::instance().setVolume(preVolume);
}
void ControlBar::LoadStyleSheet(const QString & url)
{
    QFile file(url);
    file.open(QIODevice::ReadOnly);

    if (file.isOpen())
    {
        QString style = this->styleSheet();
        style += QLatin1String(file.readAll());
        this->setStyleSheet(style);
        file.close();
    }
    else{
        qDebug()<<"controlbar Qss load failed";
    }
}

/// 计算分秒
QString formatTime(int timeInSeconds)
{
    if (timeInSeconds == 0)
    {
        return "00.00";
    }
    int minutes = timeInSeconds / 60;
    int seconds = timeInSeconds % 60;
    QString a;
    if (minutes >= 10)
    {
        a += QString("%1").arg(minutes);
    }
    else
    {
        a += QString("0%1").arg(minutes);
    }
    a += ".";
    if (seconds >= 10)
    {
        a += QString("%1").arg(seconds);
    }
    else
    {
        a += QString("0%1").arg(seconds);
    }
    return a;
}

void ControlBar::changePlayer(bool temp){
    stopslot();
    if(m_isLight)
    btplay->setIcon(QIcon(":/asset/image/play.PNG"));
    else

    btplay->setIcon(QIcon(":/asset/image/play_dark.PNG"));



    currenttime = 0;

    playtime->setText("--.--");
    endtime->setText("--.--");
    cTimer->stop();
    processSlider->setValue(0);
    if(temp==1){
        isVideo=1;
        btscreen->setDisabled(false);

    }
    else {
        isVideo=0;
        btscreen->setDisabled(true);
    }

}
void ControlBar::videoStateChang(QtAV::AVPlayer::State state){

    if (state == QtAV::AVPlayer::StoppedState)
    {
    if(m_isLight)
        btplay->setIcon(QIcon(":/asset/image/play.PNG"));
    else

        btplay->setIcon(QIcon(":/asset/image/play_dark.PNG"));
        currenttime = 0;

        playtime->setText("--.--");
        endtime->setText("--.--");
        cTimer->stop();
        processSlider->setValue(0);
    }
    else if (state == QtAV::AVPlayer::PlayingState)
    {

    if(m_isLight)
        btplay->setIcon(QIcon(":/asset/image/pause.PNG"));

    else
        btplay->setIcon(QIcon(":/asset/image/pause_dark.PNG"));
        if (currenttime)
        {
            cTimer->start();
        }
    }
    else
    {

    if(m_isLight)
        btplay->setIcon(QIcon(":/asset/image/play.PNG"));
    else

        btplay->setIcon(QIcon(":/asset/image/play_dark.PNG"));
        cTimer->stop();
    }
}

void ControlBar::musicStateChange(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState)
    {

    if(m_isLight)
        btplay->setIcon(QIcon(":/asset/image/play.PNG"));
    else

        btplay->setIcon(QIcon(":/asset/image/play_dark.PNG"));
        currenttime = 0;

        playtime->setText("--.--");
        endtime->setText("--.--");
        cTimer->stop();
        processSlider->setValue(0);
    }
    else if (state == QMediaPlayer::PlayingState)
    {

    if(m_isLight)
        btplay->setIcon(QIcon(":/asset/image/pause.PNG"));
        else

        btplay->setIcon(QIcon(":/asset/image/pause_dark.PNG"));
        if (currenttime)
        {
            cTimer->start();
        }
    }
    else
    {

    if(m_isLight)
        btplay->setIcon(QIcon(":/asset/image/play.PNG"));
    else {

        btplay->setIcon(QIcon(":/asset/image/play_dark.PNG"));
    }
        cTimer->stop();
    }
}
void ControlBar::playslot()
{
    if (isVideo)
    {
        auto player = VideoPlayer::instance();
        auto state = player->state();
        if (state == QtAV::AVPlayer::State::PlayingState)
        {
            player->pause();
        }
        else
        {
            player->play();
        }
        return;
    }
    auto &player = MusicPlayer::instance();

    if (player.state() == QMediaPlayer::PlayingState)
    {

        player.pause();
    }
    else if(player.state()==QMediaPlayer::StoppedState)
    {


        player.play();
    }
    else{
        player.play();
    }
}
void ControlBar::preslot()
{
    if (isVideo)
    {
        QModelIndex currentIndex = temp->video_table->currentIndex();
        int index = currentIndex.row();
        if (index > 0)
        {
            temp->playVideoFromIndex(index - 1);
        }
        return;
    }
    QModelIndex currentIndex = temp->music_table->currentIndex();
    int index = currentIndex.row();
    if (index > 0)
    {
        temp->playMusicFromIndex(index - 1);
    }
}
void ControlBar::stopslot()
{
    if (isVideo)
    {
        VideoPlayer::instance()->stop();
        emit toReturnMediaTable();}

    else
        MusicPlayer::instance().stop();
}
void ControlBar::nexslot()
{
    if (isVideo)
    {
        QModelIndex currentIndex = temp->video_table->currentIndex();
        int index = currentIndex.row();
        if (index > 0)
        {
            temp->playVideoFromIndex(index + 1);
        }
        else
        {
            temp->playVideoFromIndex(0);
        }
        return;
    }
    QModelIndex currentIndex = temp->music_table->currentIndex();
    int index = currentIndex.row();
    if (index < temp->music_table->count())
    {
        temp->playMusicFromIndex(index + 1);
        index++;

    }

    else
    {
        temp->playMusicFromIndex(0);

       temp->music_table->setCurrentIndex(currentIndex);
       index=0;
    }
}

void ControlBar::onLoopChange()
{

    switch (loopstate)
    {
    case LoopState::Loop:
        loopstate = LoopState::Random; // 如果是 Loop，改成 Random
        break;

    case LoopState::Random:
        loopstate = LoopState::Queue; // 如果是 Random，改成 Queue
        break;

    case LoopState::Queue:
        loopstate = LoopState::Loop; // 如果是 Queue，改成 Loop
        break;
    }

    this->ChangeLoopBtIcon();
}

void ControlBar::ChangeLoopBtIcon()
{

    if(m_isLight)
        switch (loopstate)
        {
        case LoopState::Loop:
            btloop->setIcon(QIcon(":/asset/image/loop.png"));
            break;

        case LoopState::Random:
            btloop->setIcon(QIcon(":/asset/image/shuffle.png"));
            break;

        case LoopState::Queue:
            btloop->setIcon(QIcon(":/asset/image/queue.png"));
            break;

        default:
            qDebug() << "trap in ChangeLoopBtIcon";
            break;
        }
    else{
        switch (loopstate)
            {
            case LoopState::Loop:
                btloop->setIcon(QIcon(":/asset/image/loop_dark.png"));
                break;

            case LoopState::Random:
                btloop->setIcon(QIcon(":/asset/image/shuffle_dark.png"));
                break;

            case LoopState::Queue:
                btloop->setIcon(QIcon(":/asset/image/queue_dark.png"));
                break;

            default:
                qDebug() << "trap in ChangeLoopBtIcon";
                break;
            }
    }

}

void ControlBar::handleTimeout()
{
    currenttime += 1;

    playtime->setText(formatTime(currenttime));
    if (processSlider->value() != currenttime)
        processSlider->setValue(currenttime);
}
void ControlBar::PlaySliderValueReset()
{
    currenttime = 0;
    cTimer->start(1000);
    processSlider->setMinimum(0);

    playtime->setText(formatTime(currenttime));
    processSlider->setValue(currenttime);
}
void ControlBar::videoMediaChange(QtAV::MediaStatus state){

    // 视频处理
    if (state == QtAV::MediaStatus::LoadedMedia||state== QtAV::MediaStatus::BufferedMedia)  // QtAV 的 LoadedState
    {
        // 播放器加载完毕
        auto player = VideoPlayer::instance();  // 使用 VideoPlayer 实例
        PlaySliderValueReset();
        endtime->setText(QString(formatTime(player->duration() / 1000 + 1)));
        processSlider->setMaximum(player->duration() / 1000 + 1);
    }
    else if (state == QtAV::MediaStatus::EndOfMedia&&!VideoPlayer::instance()->manualStopped)  // QtAV 的 EndOfMedia
    {
        // 播放结束
        auto m_player = VideoPlayer::instance()->player();
        if(m_player->position() == m_player->duration()){ cTimer->stop();
            if (loopstate == Loop)
            {
                PlaySliderValueReset();
                playslot();
            }
            else if (loopstate == Queue)
            {
                nexslot();
            }
            else
            {
                QModelIndex currentIndex = temp->video_table->currentIndex();
                int index = currentIndex.row();
                int randomNumber = QRandomGenerator::global()->bounded(0, index);
                if (index < temp->video_table->count())
                {
                    temp->playMusicFromIndex(randomNumber);
                }
            }}

    }

}
void ControlBar::musicMediaChange(QMediaPlayer::MediaStatus state)
{
    /// 换媒体文件
    if (state == QMediaPlayer::MediaStatus::LoadedMedia||state==QMediaPlayer::MediaStatus::BufferedMedia)
    {
        PlaySliderValueReset();
        endtime->setText(QString(formatTime(MusicPlayer::instance().duration() / 1000 + 1)));
        processSlider->setMaximum(MusicPlayer::instance().duration() / 1000 + 1);
    }
    else if (state == QMediaPlayer::MediaStatus::EndOfMedia)
    {
        cTimer->stop();
        if (loopstate = Loop)
        {
            PlaySliderValueReset();
            playslot();
        }
        else if (loopstate == Queue)
        {
            nexslot();
        }
        else
        {


            QModelIndex currentIndex = temp->music_table->currentIndex();
            int index = currentIndex.row();
            int randomNumber = QRandomGenerator::global()->bounded(0, index);
            if (index < temp->music_table->count())
            {
                temp->playMusicFromIndex(randomNumber);
            }
        }
    }
}
void ControlBar::sliderchange(int value)
{
    currenttime = value;
    playtime->setText(formatTime(currenttime));
}
void ControlBar::volumesetting(int value)
{
    if (isVideo)

        VideoPlayer::instance()->setVolume(value);
    else
        MusicPlayer::instance().setVolume(value);
}
void ControlBar::processsetting()
{
    if(isVideo)
        VideoPlayer::instance()->setPosition(processSlider->value()*1000);
    else
        MusicPlayer::instance().setPosition(processSlider->value() * 1000);
}
void ControlBar::switchvolume()
{
    if (volumeSlider->value())
    {
        // 关
        preVolume = volumeSlider->value();
        volumeSlider->setValue(0);
    }
    else
    {
        volumeSlider->setValue(preVolume);
    }
}

void ControlBar::handlePlay()
{
    playslot();
}

void ControlBar::handleChangeLoop()
{
    onLoopChange();
}

void ControlBar::handleNext()
{
    nexslot();
}

void ControlBar::handlePrevious()
{
    preslot();
}

void ControlBar::handleVolumeUp()
{
    volumeSlider->setValue(volumeSlider->value() + 5);
}

void ControlBar::handleVolumeDown()
{
    volumeSlider->setValue(volumeSlider->value() - 5);
}
void ControlBar::onSetSpeed(){
    qreal speed;

    if(speedstate==3){
        speedstate=0;

    }else{
        speedstate+=1;

    }
    if(m_isLight){
    if (speedstate == 0) {
        speed = 0.5;
        btspeed->setIcon(QIcon(":/asset/image/0.5xspeed.PNG"));
        VideoPlayer::instance()->setSpeed(speed);
        MusicPlayer::instance().setSpeed(speed);

    } else if (speedstate == 1) {
        speed = 1;
        btspeed->setIcon(QIcon(":/asset/image/1xspeed.PNG"));
        VideoPlayer::instance()->setSpeed(speed);
        MusicPlayer::instance().setSpeed(speed);

    } else if (speedstate == 2) {
        speed = 1.5;
        btspeed->setIcon(QIcon(":/asset/image/1.5xspeed.PNG"));
        VideoPlayer::instance()->setSpeed(speed);
        MusicPlayer::instance().setSpeed(speed);

    } else if (speedstate == 3) {
        speed = 2;
        btspeed->setIcon(QIcon(":/asset/image/2xspeed.PNG"));
        VideoPlayer::instance()->setSpeed(speed);
        MusicPlayer::instance().setSpeed(speed);
    }
}
    else{
        if (speedstate == 0) {
            speed = 0.5;
            btspeed->setIcon(QIcon(":/asset/image/0.5xspeed_dark.PNG"));
            VideoPlayer::instance()->setSpeed(speed);
            MusicPlayer::instance().setSpeed(speed);

        } else if (speedstate == 1) {
            speed = 1;
            btspeed->setIcon(QIcon(":/asset/image/1xspeed_dark.PNG"));
            VideoPlayer::instance()->setSpeed(speed);
            MusicPlayer::instance().setSpeed(speed);

        } else if (speedstate == 2) {
            speed = 1.5;
            btspeed->setIcon(QIcon(":/asset/image/1.5xspeed_dark.PNG"));
            VideoPlayer::instance()->setSpeed(speed);
            MusicPlayer::instance().setSpeed(speed);

        } else if (speedstate == 3) {
            speed = 2;
            btspeed->setIcon(QIcon(":/asset/image/2xspeed_dark.PNG"));
            VideoPlayer::instance()->setSpeed(speed);
            MusicPlayer::instance().setSpeed(speed);
        }
    }

    }







void ControlBar:: shiftThemeIcon(bool isLight){
    if(!isLight){
        btplay->setIcon(QIcon(":/asset/image/play_dark.PNG"));
        btpre->setIcon(QIcon(":/asset/image/previous_dark.PNG"));
        btspeed->setIcon(QIcon(":/asset/image/1xspeed_dark.PNG"));
        btstop->setIcon(QIcon(":/asset/image/stop_dark.PNG"));
        btnex->setIcon(QIcon(":/asset/image/next_dark.PNG"));
        btscreen->setIcon(QIcon(":/asset/image/fscreen_dark.PNG"));
        btloop->setIcon(QIcon(":/asset/image/queue_dark.PNG"));


    volumeSlider->setLeftIcon(QIcon(":/asset/image/fvolume_dark..PNG"));

    }
    else{
        btplay->setIcon(QIcon(":/asset/image/play.PNG"));
        btpre->setIcon(QIcon(":/asset/image/previous.PNG"));
        btspeed->setIcon(QIcon(":/asset/image/1xspeed.PNG"));
        btstop->setIcon(QIcon(":/asset/image/stop.PNG"));
        btnex->setIcon(QIcon(":/asset/image/next.PNG"));
        btscreen->setIcon(QIcon(":/asset/image/fscreen.PNG"));
        btloop->setIcon(QIcon(":/asset/image/queue.PNG"));

    volumeSlider->setLeftIcon(QIcon(":/asset/image/fvolume.PNG"));

    }

        m_isLight=isLight	;

        //changeloopicon依赖m_isLight;
    ChangeLoopBtIcon();
}
