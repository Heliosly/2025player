#include "controlbar.h"
 #include"videoplayer.h"

#include"musicplayer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <DLabel>
#include <DIconButton>
#include <QFile>
#include <QRandomGenerator>
ControlBar::ControlBar(QWidget *parent,bool _isVideo) : DFrame(parent)
{

    isVideo=_isVideo;
    this->setMaximumHeight(130);
    this->setObjectName("control_bar");
    btspeed= new DIconButton(this);
    btplay = new DIconButton(this);
    btpre = new DIconButton(btplay);
    btstop = new DIconButton(btplay);
    btnex = new DIconButton(btplay);

    btloop = new DIconButton(btplay);
    btscreen = new DIconButton(btplay);

    if(!isVideo)

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
    this->setLoopBtIcon();
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
    MainVlayout->setContentsMargins(5,10,5,10);
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

    connect(cTimer, &QTimer::timeout, this, &ControlBar::handleTimeout);
    connect(processSlider, &DSlider::valueChanged, this, &ControlBar::sliderchange);
    connect(processSlider, &DSlider::sliderReleased, this, &ControlBar::processsetting);
    connect(volumeSlider, &DSlider::valueChanged, this, &ControlBar::volumesetting);
    connect(btloop, &DIconButton::clicked, this, &ControlBar::onLoopChange);

    connect(btspeed, &DIconButton::clicked, this, &ControlBar::onSetSpeed);
    if(!isVideo){
           connect(&MusicPlayer::instance(), &MusicPlayer::stateChanged, this, &ControlBar::musicStateChange);
    connect(&MusicPlayer::instance(), &MusicPlayer::mediaStatusChanged, this, &ControlBar::musicMediaChange);

    connect(&MusicPlayer::instance(), &MusicPlayer::started, this, &ControlBar::onStarted);
    }


   }
void ControlBar::loadSavedPlayerState(){
  int volume,speedState,loopState;
    SettingsManager::instance()->loadPlayerStates(volume,speedState,loopState);
    volumeSlider->setValue(volume);
    speedstate=speedState;
     if (loopState == 0) {
        loopstate = LoopState::Loop;
    } else if (loopState == 1) {
        loopstate = LoopState::Random;
    } else if (loopState == 2) {
        loopstate = LoopState::Queue;
    } else {
        loopstate = LoopState::Loop;
    }
     setLoopBtIcon();
     if(speedstate==0){
         speedstate=3;
         onSetSpeed();
     }else{
         speedstate--;
         onSetSpeed();
     }

}
ControlBar::~ControlBar(){
    if(isVideo){
        int loopState = 0; // 初始化
        if (loopstate == LoopState::Loop) {
            loopState = 0;
        } else if (loopstate == LoopState::Random) {
            loopState = 1;
        } else if (loopstate == LoopState::Queue) {
            loopState = 2;
        } else {
            loopState = 0; // 默认值，兜底
        }
        int volume = volumeSlider->value();
        SettingsManager::instance()->savePlayerStates(volume,speedstate,loopState);


    }

}
void ControlBar::connectVideoFc(){
     connect(VideoPlayer::instance(), &VideoPlayer::stateChanged, this, &ControlBar::videoStateChang);
    connect(VideoPlayer::instance(), &VideoPlayer::mediaStatusChanged, this, &ControlBar::videoMediaChange);
    //    connect(volumeSlider, &DSlider::valueChanged, mediaPlayer, &QtAV::setVolume);

    connect(btscreen, &DIconButton::clicked,VideoPlayer::instance() ,&VideoPlayer::onShiftScreen);
    volumeSlider->setValue(100);


    connect(VideoPlayer::instance(), &VideoPlayer::videoStarted, this, &ControlBar::onStarted);

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

//void ControlBar::changePlayer(bool temp){
//    stopslot();
//    if(m_isLight)
//        btplay->setIcon(QIcon(":/asset/image/play.PNG"));
//    else

//        btplay->setIcon(QIcon(":/asset/image/play_dark.PNG"));



//    currenttime = 0;

//    playtime->setText("--.--");
//    endtime->setText("--.--");
//    cTimer->stop();
//    processSlider->setValue(0);
//    if(temp==1){
//        isVideo=1;
//        btscreen->setDisabled(false);

//    }
//    else {
//        isVideo=0;
//        btscreen->setDisabled(true);
//    }

//}
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

void ControlBar::musicStateChange(QtAV::AVPlayer::State state)
{
    if (state == QtAV::AVPlayer::StoppedState)
    {
        inplay=false;

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
        inplay=true;

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

        inplay=false;
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
      if(inHistory){
            auto &player = MusicPlayer::instance();

             auto state = player.state();
    if (state ==QtAV::AVPlayer::PlayingState)
    {
        player.pause();
    }
    else
    {
        player.play();
    }
    return;

    }


    if (isVideo)
    {

        auto player = VideoPlayer::instance();
        if(player->m_player->mediaStatus()==QtAV::MediaStatus::NoMedia){
            temp->playVideoFromIndex(0);
        }
        else{
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
           }
    auto &player = MusicPlayer::instance();
    if(player.player->mediaStatus()==QtAV::MediaStatus::NoMedia){
        temp->playVideoFromIndex(0);
    }
    else{

         auto state = player.state();
    if (state ==QtAV::AVPlayer::PlayingState)
    {
        player.pause();
    }
    else
    {
        player.play();
    }
    return;


    }
}
void ControlBar::preslot()
{
       if(inHistory){

               return;
//        QModelIndex currentIndex = temp->historyTable->currentIndex();

//            temp->playMediaFromIndexInHistory(currentIndex);
//               return;
    }


    if (isVideo)
    {

        QModelIndex currentIndex = temp->video_table->currentIndex();
        int index = currentIndex.row();
        if (index > 0)
        {
            temp->playVideoFromIndex(index - 1);
        }

       onVideoMediaChange();
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
        VideoPlayer::instance()->closeVideoPage();

    }

    else
        MusicPlayer::instance().stop();
}
void ControlBar::nexslot()
{
    if(inHistory){

               return;
        QModelIndex currentIndex = temp->historyTable->currentIndex();

            temp->playMediaFromIndexInHistory(currentIndex);
               return;
    }



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

       onVideoMediaChange();
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

    this->setLoopBtIcon();
}

void ControlBar::setLoopBtIcon()
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

void ControlBar::onVideoMediaChange(){
     // 播放器加载完毕
        PlaySliderValueReset();

}

void ControlBar::videoMediaChange(QtAV::MediaStatus state){

   if (state == QtAV::MediaStatus::EndOfMedia&&!VideoPlayer::instance()->manualStopped)  // QtAV 的 EndOfMedia
    {
        // 播放结束

        stopslot();
        return;
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
void ControlBar::musicMediaChange(QtAV::MediaStatus state)
{
    /// 换媒体文件
    if (state==QtAV::MediaStatus::BufferedMedia)
    {

          }
    else if (state == QtAV::MediaStatus::EndOfMedia)
    {
        cTimer->stop();
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


            if (!temp->music_table){
                qDebug()<<"mtable not existed";
            }
            if( !temp->music_table->currentIndex().isValid()){
                qDebug()<<"index is not valid";
            }
            QModelIndex currentIndex = temp->music_table->currentIndex();
            auto count=temp->musicListModel->rowCount();
            int index = currentIndex.row();
            int randomNumber = QRandomGenerator::global()->bounded(0, qMin(index+10,count-1));
            if(count<2){

            }
            else
            if(randomNumber==index){
               if(index!=1) index--;
               else index++;
            }
            qDebug()<<randomNumber;
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

void ControlBar::setSpeedIcon(int speedState) {
    QString iconPath;

    if (m_isLight) {
        // 浅色模式图标
        switch (speedState) {
            case 0:
                iconPath = ":/asset/image/0.5xspeed.PNG";
                break;
            case 1:
                iconPath = ":/asset/image/1xspeed.PNG";
                break;
            case 2:
                iconPath = ":/asset/image/1.5xspeed.PNG";
                break;
            case 3:
                iconPath = ":/asset/image/2xspeed.PNG";
                break;
            default:
                return; // 无效状态不处理
        }
    } else {
        // 深色模式图标
        switch (speedState) {
            case 0:
                iconPath = ":/asset/image/0.5xspeed_dark.PNG";
                break;
            case 1:
                iconPath = ":/asset/image/1xspeed_dark.PNG";
                break;
            case 2:
                iconPath = ":/asset/image/1.5xspeed_dark.PNG";
                break;
            case 3:
                iconPath = ":/asset/image/2xspeed_dark.PNG";
                break;
            default:
                return;
        }
    }

    btspeed->setIcon(QIcon(iconPath));
}




void ControlBar:: shiftThemeIcon(bool isLight){
    if(!isLight){
        if(!inplay)
        btplay->setIcon(QIcon(":/asset/image/play_dark.PNG"));
        else

        btplay->setIcon(QIcon(":/asset/image/pause_dark.PNG"));
        btpre->setIcon(QIcon(":/asset/image/previous_dark.PNG"));
        btspeed->setIcon(QIcon(":/asset/image/1xspeed_dark.PNG"));
        btstop->setIcon(QIcon(":/asset/image/stop_dark.PNG"));
        btnex->setIcon(QIcon(":/asset/image/next_dark.PNG"));
        btscreen->setIcon(QIcon(":/asset/image/fscreen_dark.PNG"));


        volumeSlider->setLeftIcon(QIcon(":/asset/image/fvolume_dark.PNG"));
        volumeSlider->setIconSize(QSize(22, 22));
        btplay->setIconSize(QSize(24, 24));
        btpre->setIconSize(QSize(18, 18));
        btspeed->setIconSize(QSize(36, 36));
        btstop->setIconSize(QSize(18, 18));
        btnex->setIconSize(QSize(18, 18));
        btscreen->setIconSize(QSize(24, 24));
        btloop->setIconSize(QSize(36, 36));
        setLoopBtIcon();
    }
    else{
          if(!inplay)
        btplay->setIcon(QIcon(":/asset/image/play.PNG"));
        else

        btplay->setIcon(QIcon(":/asset/image/pause.PNG"));

        btpre->setIcon(QIcon(":/asset/image/previous.PNG"));
        btspeed->setIcon(QIcon(":/asset/image/1xspeed.PNG"));
        btstop->setIcon(QIcon(":/asset/image/stop.PNG"));
        btnex->setIcon(QIcon(":/asset/image/next.PNG"));
        btscreen->setIcon(QIcon(":/asset/image/fscreen.PNG"));

        volumeSlider->setLeftIcon(QIcon(":/asset/image/fvolume.PNG"));
        volumeSlider->setIconSize(QSize(22, 22));
        btplay->setIconSize(QSize(24, 24));
        btpre->setIconSize(QSize(18, 18));
        btspeed->setIconSize(QSize(36, 36));
        btstop->setIconSize(QSize(18, 18));
        btnex->setIconSize(QSize(18, 18));
        btscreen->setIconSize(QSize(24, 24));

        setLoopBtIcon();

    }

    m_isLight=isLight	;

    //changeloopicon依赖m_isLight;
    setLoopBtIcon();
}
void ControlBar::onStarted(){

       if(isVideo)
        {
           qDebug()<<"started AV";

        qint64 duration = VideoPlayer::instance()->duration();  // 使用 VideoPlayer 实例

        if(duration==0) return ;
          endtime->setText(QString(formatTime(duration / 1000 + 1)));
        processSlider->setMaximum(duration / 1000 + 1);
       }
       else{
           qDebug()<<"stated AU";
             PlaySliderValueReset();
        endtime->setText(QString(formatTime(MusicPlayer::instance().duration() / 1000 + 1)));
        processSlider->setMaximum(MusicPlayer::instance().duration() / 1000 + 1);

       }

}
