#include "mainwindow.h"
#include<DApplicationHelper>
MainWindow::~MainWindow() {

}
MainWindow::MainWindow()

{
    this->titlebar()->setIcon(QIcon(":asset/image/logo.png"));
    this->setTitlebarShadowEnabled(false);
    this->setWindowRadius(8);
    this->setObjectName("main_window");
    this->resize(QSize(1450, 900));
    this->setEnableSystemResize(true);

    //标题栏

    DataBase::instance();
    VideoPlayer::instance()->m_controlBar->connectVideoFc();
    MusicPlayer::instance().enable=1;

    MusicPlayer::instance().player= VideoPlayer::instance()->player();
    MusicPlayer::instance().initConnect();
    cbar->loadSavedPlayerState();


    DTitlebar * bar=this->titlebar();
    ;
    DMenu *menu =  bar->menu();
    QAction *action = new QAction("设置", this);
    menu->addAction(action);

    music_table = new MusicTable();


    recommandPage = new RecommandPage (this);
    recommandPage->refreshList();
    VideoPlayer::instance()->m_controlBar->temp=music_table;


    bar->setSeparatorVisible(false);
    bar->setSwitchThemeMenuVisible(true);
    bar->setFixedHeight(90);
    Navw->setAutoFillBackground(true);
    Navw->setMaximumWidth(250);

    cbar->setAutoFillBackground(true);

    cbar->temp=music_table;
    moveToCenter(this); //把窗口移动到屏幕中间
    //把主窗口分为上下两个垂直布局

    music_table->setParent(this);
    music_table->window=this;
    settingPage = new SettingPage();
    settingPage->setParent(this);
    settingPage->setObjectName("setting_page");

    page = new QStackedWidget(this);
    mainPage=new QStackedWidget(this);
    recommandPage ->setObjectName("recommandFrame");

    DFrame *history = new DFrame(this);
    history->setObjectName("historyFrame");
    QVBoxLayout *historyLayout = new QVBoxLayout(history);
    history->setLayout(historyLayout);

    // 2. 把 historyTable 从旧 parent “解绑”，再加入到 historyLayout
    //    （这样可以避免原来在 music_table 中的约束）
    music_table->historyTable->setParent(nullptr);
    historyLayout->addWidget(music_table->historyTable);

    // 3. 确保它是可见的
    ///
    music_table->historyTable->setVisible(true);
    DFrame* historyEmptyFrame = new DFrame(this);
    historyEmptyFrame->setObjectName("History_Empty_Frame");

    QVBoxLayout* layout = new QVBoxLayout(historyEmptyFrame);
    layout->setAlignment(Qt::AlignCenter);

    DLabel* label = new DLabel("尚未播放过媒体文件", historyEmptyFrame);
     QFont font;
      font.setFamily("Noto Sans CJK SC");  // 设置字体家族
            font.setPointSize(24);  // 设置字体大小
           label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    ///

    layout->addWidget(label);
    page->addWidget(music_table);//0
    page->addWidget(settingPage);//1
    page->addWidget(recommandPage);//2
    page->addWidget(history);//3
    page->addWidget(historyEmptyFrame);//4
    RightHLayout->addWidget(page);

    cw->setLayout(MainVLayout);

    MainVLayout->addWidget(mainPage);
    //cw->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //    QPalette palette = this->palette();
    //    palette.setColor(QPalette::Background,Qt::transparent);
    //    cw->setPalette(palette);
    mainPage->addWidget(cw2);

    cw2->setLayout(UpHLayout);

    MainVLayout->addLayout(DownHLayout);
    LeftHLayout->addWidget(Navw);
    LeftHLayout->setContentsMargins(20,0,20,20);
    UpHLayout->addLayout(LeftHLayout);

    UpHLayout->addLayout(RightHLayout);

    MainVLayout->setSpacing(0);

    DownHLayout->addWidget(cbar);
    DownHLayout->setContentsMargins(20,0,20,3);

    UpHLayout->setStretch(0, 1);
    UpHLayout->setStretch(1, 4);

    MainVLayout->setStretch(0, 8);
    MainVLayout->setStretch(1, 1);
    this->setCentralWidget(cw);
    //切换主题
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,this,&MainWindow::setTheme);
    //对应左侧导航栏
    connect(Navw->ListView1->selectionModel(), &QItemSelectionModel::currentChanged, this, &MainWindow::currentchange);

    connect(action, &QAction::triggered,this,&MainWindow::showSettingPage);

    Navw->ListView1->setCurrentIndex(Navw->ListView1->model()->index(0, 0));
    //    MainVLayout->setContentsMargins(0,0,0,0);

    //    DownHLayout->setContentsMargins(0,0,0,0);
    auto cbar2=VideoPlayer::instance()->m_controlBar;
    connect(qApp, &QCoreApplication::aboutToQuit, this,&MainWindow::onAppAboutToQuit);
    // 在构造函数末尾添加快捷键连接
    connect(ShortcutManager::instance(), &ShortcutManager::playTriggered,
            cbar, &ControlBar::handlePlay);
    connect(ShortcutManager::instance(), &ShortcutManager::switchLoopTriggered,
            cbar, &ControlBar::handleChangeLoop);
    connect(ShortcutManager::instance(), &ShortcutManager::nextTriggered,
            cbar, &ControlBar::handleNext);
    connect(ShortcutManager::instance(), &ShortcutManager::previousTriggered,
            cbar, &ControlBar::handlePrevious);
    connect(ShortcutManager::instance(), &ShortcutManager::volumeUpTriggered,
            cbar, &ControlBar::handleVolumeUp);
    connect(ShortcutManager::instance(), &ShortcutManager::volumeDownTriggered,
            cbar, &ControlBar::handleVolumeDown);
    connect(ShortcutManager::instance(), &ShortcutManager::playTriggered,
               cbar2, &ControlBar::handlePlay);
       connect(ShortcutManager::instance(), &ShortcutManager::switchLoopTriggered,
               cbar2, &ControlBar::handleChangeLoop);
       connect(ShortcutManager::instance(), &ShortcutManager::nextTriggered,
               cbar2, &ControlBar::handleNext);
       connect(ShortcutManager::instance(), &ShortcutManager::previousTriggered,
               cbar2, &ControlBar::handlePrevious);
       connect(ShortcutManager::instance(), &ShortcutManager::volumeUpTriggered,
               cbar2, &ControlBar::handleVolumeUp);
       connect(ShortcutManager::instance(), &ShortcutManager::volumeDownTriggered,
               cbar2, &ControlBar::handleVolumeDown);
//       connect(cbar,&ControlBar::toSyncControlBarState,this,&MainWindow::syncControlBarState);

//       connect(cbar2,&ControlBar::toSyncControlBarState,this,&MainWindow::syncControlBarState);
       connect(music_table,&MusicTable::mediaChange,cbar2,&ControlBar::onVideoMediaChange);
    connect(VideoPlayer::instance(),&VideoPlayer::toCloseVideo,this,&MainWindow::closeVideoPage);

    connect(VideoPlayer::instance(),&VideoPlayer::toShowVideo,this,&MainWindow::showVideoPage);

    connect(music_table, &MusicTable::toResizeWidget,
            music_table, [this](){  // 注意括号和可调用对象声明
                music_table->onResetWindowSize(this->width());
            });
    connect(music_table,&MusicTable::mediaDeletedByDir,recommandPage,&RecommandPage::deleteByDir);

    connect(music_table,&MusicTable::toRefreshRecommand,recommandPage,&RecommandPage::refreshList,Qt::QueuedConnection);
}
void MainWindow::closeVideoPage(){
      auto cbar2 = VideoPlayer::instance()->m_controlBar;
    int volume=cbar2->volumeSlider->value();
    auto loopState=cbar2->loopstate;
    auto speedState= cbar2->speedstate;
    cbar->volumeSlider->setValue(volume);
    cbar->speedstate=speedState;
    cbar->loopstate=loopState;
    cbar->setSpeedIcon(speedState);
    cbar->setLoopBtIcon();

    isVideoPage=0;
 VideoPlayer::instance()->enable=0;
    MusicPlayer::instance().enable=1;

    auto widget = VideoPlayer::instance()->widget();
    widget->hide();
    this->show();
    VideoPlayer::instance()->stop();

}

void MainWindow::showVideoPage(){
    auto cbar2 = VideoPlayer::instance()->m_controlBar;
    int volume=cbar->volumeSlider->value();
    auto loopState=cbar->loopstate;
    auto speedState= cbar->speedstate;
    cbar2->volumeSlider->setValue(volume);
    cbar2->speedstate=speedState;
    cbar2->loopstate=loopState;
    cbar2->setSpeedIcon(speedState);
    cbar2->setLoopBtIcon();

    isVideoPage=1;
    VideoPlayer::instance()->enable=1;
    MusicPlayer::instance().enable=0;
    auto widget = VideoPlayer::instance()->widget();
    // 复制 MainWindow 的几何信息（含位置+大小）
    widget->setGeometry(this->geometry());
    widget->show();
    this->hide();
    VideoPlayer::instance()->m_controlBar->LoadStyleSheet(":/asset/qss/cbar_dark.qss");
    VideoPlayer::instance()->m_controlBar->shiftThemeIcon(0);
}

///切换到设置页面
void MainWindow::showSettingPage(){
    page->setCurrentIndex(1);

}
///显示播放器播放视频///深色浅色两套主题的颜色/图标设置
void MainWindow::setTheme(DGuiApplicationHelper::ColorType theme)
{
    this->setStyleSheet("");  // 先清空
    music_table->setStyleSheet("");
    if(theme==DGuiApplicationHelper::LightType){

        music_table->LoadStyleSheet(":/asset/qss/musictb_light.qss");
        Navw->LoadStyleSheet(":/asset/qss/navwidget_light.qss");
        cbar->LoadStyleSheet(":/asset/qss/cbar_light.qss");

        this->LoadStyleSheet(":/asset/qss/mainwindow_light.qss");
        music_table->setThemeType(1);
        cbar->shiftThemeIcon(1);

        Navw->shiftTheme(1);

    }else {

        music_table->LoadStyleSheet(":/asset/qss/musictb_dark.qss");
        Navw->LoadStyleSheet(":/asset/qss/navwidget_dark.qss");

        cbar->LoadStyleSheet(":/asset/qss/cbar_dark.qss");



        this->LoadStyleSheet(":/asset/qss/mainwindow_dark.qss");

        music_table->setThemeType(0);

        cbar->shiftThemeIcon(0);
        Navw->shiftTheme(0);
    }
}


void MainWindow::currentchange(const QModelIndex &current,const QModelIndex &previous)
{
    int row=current.row();

    if(m_lastPosition == 4){

        cbar->btnex->setDisabled(false);

        cbar->btpre->setDisabled(false);

            cbar->btloop->setDisabled(false);

            cbar->inHistory= false;

    }


    if(page->currentIndex()!=1){
        page->setCurrentIndex(0);
        m_lastPosition=-1;
    }
    if(row==0)
    {

        page->setCurrentIndex(2);

        m_lastPosition=0;
    }
    else
        if (row==2)
        {
            page->setCurrentIndex(0);
            music_table->page->setCurrentIndex(0);
            //        if(cbar->mediaPlayer!=nullptr){
            //            cbar->mediaPlayer->stop();
            //        }
            cbar->readVolume("");

            m_lastPosition=2;
        }
        else if(row==3){

            page->setCurrentIndex(0);
            music_table->page->setCurrentIndex(1);
            //video

            m_lastPosition=3;
        }
        else if (row==4){
            m_lastPosition=4;
            cbar->btnex->setDisabled(true);

            cbar->btpre->setDisabled(true);
            cbar->btloop->setDisabled(true);

            cbar->inHistory=true;
            if(music_table->haveHistory)
            page->setCurrentIndex(3);
            else{
                page->setCurrentIndex(4);
            }
        }

}

///重设大小
void MainWindow::resizeEvent(QResizeEvent *event)  {
    DMainWindow::resizeEvent(event);
    music_table->onResetWindowSize(event->size().width());


}
void MainWindow::LoadStyleSheet( QString url)
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
        qWarning()<<"qss windows failed";
    }
}

void MainWindow::onAppAboutToQuit() {

    UserPreference::instance()->reWrite();
    onQuit();

}

// 实现代码
void MainWindow::onShiftScreen()
{

    if(!isFull) {


        showFullScreen();

    } else {



        showNormal();


    }

    isFull = !isFull;

}

void MainWindow::closeEvent(QCloseEvent *event) {
    // 可以在这里处理一些清理工作
    // 如果需要询问用户是否退出，或者保存数据等

    event->accept();  // 确保事件被接受，窗口关闭
}

void MainWindow::onQuit(){
   DataBase::instance()->abortRequest();
   music_table->m_abort=1;

}
