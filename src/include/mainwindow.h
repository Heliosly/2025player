#ifndef MainWindow_H
#define MainWindow_H
#include"settingpage.h"
#include "navwidget.h"
#include"musictable.h"
#include<controlbar.h>
#include <DMainWindow>
#include <DGuiApplicationHelper>//用来适配深色模式
#include<QVBoxLayout>
#include<QMediaPlayer>
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
    QWidget *cw = new QWidget(this);
    NavWidget *Navw = new  NavWidget;
    ControlBar * cbar = new ControlBar(this);
    //主布局
    QVBoxLayout* MainVLayout = new QVBoxLayout;

    QHBoxLayout* UpHLayout = new QHBoxLayout;
    QHBoxLayout* DownHLayout = new QHBoxLayout;
    //有奖
    QHBoxLayout* RightHLayout = new QHBoxLayout;   
    QHBoxLayout* LeftHLayout = new QHBoxLayout;
    QMediaPlayer *player = new QMediaPlayer(this);
    QStackedWidget * page;
     DFrame * recommandPage ;
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
public emit:
    void showSettingPage();
private:
     QWidget*videoOriginalParent;
    QLayout*videoOriginalLayout;
protected:
    void keyPressEvent(QKeyEvent *event) override;

};

#endif // MAINWINDOW_H
