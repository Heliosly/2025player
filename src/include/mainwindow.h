#ifndef MainWindow_H
#define MainWindow_H
#include"settingpage.h"
#include "navwidget.h"
#include"musictable.h"
#include"recommandpage.h"
#include"controlbar.h"
#include"uservector.h"
#include"shortcutmanager.h"
#include"musicplayer.h"
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
#include <DMainWindow>
#include <DGuiApplicationHelper>
#include<QVBoxLayout>
#include <QResizeEvent>


DWIDGET_USE_NAMESPACE

class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    void closeVideoPage();
    ~MainWindow();

private:
    bool isFull=0;
    bool isVideoPage=0;
    DWidget *cw = new DWidget(this);
    NavWidget *Navw = new  NavWidget;
    ControlBar * cbar = new ControlBar(this,false);
    //主布局
    QVBoxLayout* MainVLayout = new QVBoxLayout;
    LoopState preState;

    QHBoxLayout* UpHLayout = new QHBoxLayout;
    QHBoxLayout* DownHLayout = new QHBoxLayout;
    //有奖
    QHBoxLayout* RightHLayout = new QHBoxLayout;   
    QHBoxLayout* LeftHLayout = new QHBoxLayout;
    QStackedWidget * page;
     RecommandPage * recommandPage ;
    MusicTable *music_table;
    SettingPage *settingPage ;
    QWidget*cw2=new QWidget(this);
    QStackedWidget * mainPage;
    int m_lastPosition=0;

    void LoadStyleSheet(QString url);

public slots:
    void setTheme(DGuiApplicationHelper::ColorType);
    void currentchange(const QModelIndex &current,const QModelIndex &previous);
    void showVideoPage();
    protected:
    void resizeEvent(QResizeEvent *event) override;
    /// 程序退出时调用
    void onAppAboutToQuit();
    // void setupConnections() ;

    void onShiftScreen();

    void onQuit();
    void syncControlBarState(bool isVideo);
public emit:
    void showSettingPage();
private:

void closeEvent(QCloseEvent *event) ;
  protected:

};

#endif // MAINWINDOW_H
