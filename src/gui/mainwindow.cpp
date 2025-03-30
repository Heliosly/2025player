#include "mainwindow.h"
#include"shortcutmanager.h"
#include"videoplayer.h"
#include<QVBoxLayout>
#include<QScreen>
#include<QHBoxLayout>
#include<DPushButton>
#include<DLabel>
#include<DScrollBar>
#include<DTitlebar>
#include<DWidgetUtil>
#include<DPaletteHelper>
#include<DScrollArea>
#include<QStackedWidget>
MainWindow::~MainWindow() {

    delete &MusicPlayer::instance();
    delete DataBase::instance();

}
MainWindow::MainWindow()

{
    //this->setWindowFlags(Qt::FramelessWindowHint);

    //this->titlebar()->setIcon(QIcon(":asset/image/logo.png"));
    this->setTitlebarShadowEnabled(false);
    this->setWindowRadius(18);
    this->setObjectName("main_window");
    this->resize(QSize(1450, 900));
    this->setShadowColor(Qt::transparent);
    this->setEnableSystemResize(true);
    this->LoadStyleSheet(":/asset/qss/mainwindow_dark.qss");
    //标题栏
    DTitlebar * bar=this->titlebar()
                    ;

    bar->setSeparatorVisible(false);
    bar->setSwitchThemeMenuVisible(true);
    DIconButton* leftButton = new DIconButton(bar);
    leftButton->setIcon(QIcon(":/asset/image/settings.PNG"));
    leftButton->setFixedSize(36, 36);
    bar->addWidget(leftButton,Qt::AlignRight);
    Navw->setAutoFillBackground(true);
    cbar->setAutoFillBackground(true);

    moveToCenter(this); //把窗口移动到屏幕中间
    //把主窗口分为上下两个垂直布局

    music_table = new MusicTable();
    music_table->window=this;
    settingPage = new SettingPage();
    cbar->temp=music_table;

    page = new QStackedWidget(this);
    mainPage=new QStackedWidget(this);

    page->addWidget(music_table);
    page->addWidget(settingPage);
    RightHLayout->addWidget(page);

    cw->setLayout(MainVLayout);

    MainVLayout->addWidget(mainPage);
    //cw->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    //    QPalette palette = this->palette();
    //    palette.setColor(QPalette::Background,Qt::transparent);
    //    cw->setPalette(palette);
    mainPage->addWidget(cw2);

    mainPage->addWidget(VideoPlayer::instance());
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

    connect(leftButton, &DIconButton::clicked,this,&MainWindow::showSettingPage);

    Navw->ListView1->setCurrentIndex(Navw->ListView1->model()->index(0, 0));
    //    MainVLayout->setContentsMargins(0,0,0,0);

    //    DownHLayout->setContentsMargins(0,0,0,0);
    connect(qApp, &QCoreApplication::aboutToQuit, this,&MainWindow::onAppAboutToQuit);
    // 在构造函数末尾添加快捷键连接
    connect(ShortcutManager::instance(), &ShortcutManager::playTriggered,
            cbar, &ControlBar::handlePlay);
    connect(ShortcutManager::instance(), &ShortcutManager::pauseTriggered,
            cbar, &ControlBar::handleChangeLoop);
    connect(ShortcutManager::instance(), &ShortcutManager::nextTriggered,
            cbar, &ControlBar::handleNext);
    connect(ShortcutManager::instance(), &ShortcutManager::previousTriggered,
            cbar, &ControlBar::handlePrevious);
    connect(ShortcutManager::instance(), &ShortcutManager::volumeUpTriggered,
            cbar, &ControlBar::handleVolumeUp);
    connect(ShortcutManager::instance(), &ShortcutManager::volumeDownTriggered,
            cbar, &ControlBar::handleVolumeDown);
    ShortcutManager::instance()->bindActionsTo(this);
    connect(cbar,&ControlBar::toReturnMediaTable,this,&MainWindow::closeVideoPage);

    connect(cbar->btscreen, &DIconButton::clicked, this, &MainWindow::onShiftScreen);
    connect(music_table,&MusicTable::videoPlaying,this,&MainWindow::showVideoPage);
    connect(VideoPlayer::instance(),&VideoPlayer::toShiftScreen,this,&MainWindow::onShiftScreen);
}
void MainWindow::closeVideoPage(){

    if(isFull){
    showNormal();
    isFull =!isFull;
    }
    mainPage->setCurrentIndex(0);
    isVideoPage=0;

}
///切换到设置页面
void MainWindow::showSettingPage(){
   page->setCurrentIndex(1);

}
///显示播放器播放视频
void MainWindow::showVideoPage(){
    mainPage->setCurrentIndex(1);
    isVideoPage=1;
}
///深色浅色两套主题的颜色/图标设置
void MainWindow::setTheme(DGuiApplicationHelper::ColorType theme)
{

    //    if(theme==DGuiApplicationHelper::LightType){
    //        QPalette palette = this->palette();
    //        cbar->setPalette(palette);
    //    }else {
    //        QPalette palette = this->palette();
    //        cbar->setPalette(palette);
    //    }
}


void MainWindow::currentchange(const QModelIndex &current,const QModelIndex &previous)
{
    int row=current.row();


    if(page->currentIndex()==1){
        page->setCurrentIndex(0);
    }
    if (row==0)
    {
        music_table->page->setCurrentIndex(0);
        //        if(cbar->mediaPlayer!=nullptr){
        //            cbar->mediaPlayer->stop();
        //        }
        cbar->mediaPlayer=&MusicPlayer::instance();
        cbar->readVolume("");
        cbar->changePlayer(0);
    }
    else if(row==1){

        cbar->changePlayer(1);
        music_table->page->setCurrentIndex(1);
        //video
    }
    else if (row==2){
        music_table->page->setCurrentIndex(2);
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
}

void MainWindow::onAppAboutToQuit() {

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
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // 当全屏且按下 ESC 键时退出全屏
    if (event->key() == Qt::Key_Escape && isFull) {
        showNormal();
        isFull = false;
        event->accept();
        return;
    }
    QMainWindow::keyPressEvent(event);
}




