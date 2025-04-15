///最下面控制栏，调音量的
#ifndef CONTROLBAR_H
#define CONTROLBAR_H
#include"musictable.h"
#include<QtAV>
#include<DIconButton>
#include <DWidget>
#include<QTimer>
#include<DLabel>
#include<DSlider>
#include<DFrame>
DWIDGET_USE_NAMESPACE
enum LoopState{

   Loop,
   Random,
   Queue,

};
class ControlBar : public DFrame
{
    Q_OBJECT
public:
    explicit ControlBar(QWidget *parent = nullptr,bool isVideo=0);

~ControlBar();
    void connectVideoFc();
    bool inplay=false;
    bool inHistory=false;
    bool isVideo = false;
    LoopState loopstate=Loop;
    DIconButton *btplay;
    DIconButton *btpre;
    DIconButton *btstop;
    DIconButton *btnex;
    DIconButton *btloop;
    //DIconButton *btvolume=new DIconButton(this);
    DIconButton *btscreen;
    DIconButton *btspeed;
    int speedstate=1;
    MusicTable*table;
    QTimer* cTimer;
    DSlider* processSlider;
    DLabel*playtime;
    DLabel* endtime;
    DSlider* volumeSlider;


    MusicTable * temp;
    ///记录归零之前的音量
    int preVolume=100;
    int  currenttime;

void loadSavedPlayerState();
    void LoadStyleSheet(const QString & url);
//    void changePlayer(bool temp);
void PlaySliderValueReset();

void setSpeedIcon(int speedState) ;

void setLoopBtIcon();

signals:
    void toReturnMediaTable();
private:

    bool m_isLight=false;
    int preVideoUrl=-1;
public slots:
    void musicStateChange(QtAV::AVPlayer::State state);
    void videoStateChang(QtAV::AVPlayer::State state);
    void playslot();
    ///更改进度条
    void handleTimeout();
    void preslot();
    void stopslot();
    void nexslot();
    void musicMediaChange(QtAV::MediaStatus state);
    void videoMediaChange(QtAV::MediaStatus state);
    void sliderchange(int value);
    void volumesetting(int value);
    void processsetting();
    void switchvolume();
    void readVolume(const QString &filePath);

void onVideoMediaChange();
    void onLoopChange();
    void handlePlay();
    void handleChangeLoop();
    void handleNext();
    void handlePrevious();
    void handleVolumeUp();
    void handleVolumeDown();
    void onSetSpeed();
    void onStarted();

    void shiftThemeIcon(bool isLight);



};

#endif // CONTROLBAR_H
