///所有的多媒体文件都在这里展示,page目前存储了音视频页面
#ifndef MEDIATABLE_H
#define MEDIATABLE_H
#include"metadata.h"
#include"pathselector.h"
#include<QDir>
#include<DFrame>
#include<DTableWidget>
#include<QStandardItemModel>
#include<QWidget>
#include<DListView>
#include<DPushButton>
#include<QLabel>
#include<QHBoxLayout>
#include<DGuiApplicationHelper>
#include<QFrame>
#include<DLabel>
#include<QItemDelegate>
#include<QListWidget>
#include<DLineEdit>
#include<DMainWindow>
#include<QStackedWidget>
#include <QFuture>
#include <QMutex>
#include <QFutureWatcher>
 DWIDGET_USE_NAMESPACE
 class CustomListView;  
 class TableItemDelegate;
 class normalItemDelegate;
 class MusicTable : public DFrame {
     Q_OBJECT
 public:

     bool haveHistory=0;
     bool haveVideo=0;
     bool haveRecommand=0;
     bool haveMusic=0;
     bool inMusic=1;
     MusicTable();
     QMap<QString,QList<QPair<QString, double>>> musicFavority;

     QStringList musicUrlList;

     QStringList musicUrlListAll;

     QMap<QString,QList<int>> dirToIndex;

     void setThemeType(bool isLight);
     void loadMoreData();
void cancelTask();  // 新增中断方法
     void deleteByDir(const QString&dir);
     void onScrollValueChanged(int value);

     void setLoadParameters(int initialHint, int loadCount);

     void setMusicCount(int value);

     bool enableFavorite=1;
     DListView *music_table;
     DListView *video_table;
     DMainWindow*window;
     TableItemDelegate *delegate;
     normalItemDelegate *normalDelegate;

     DListView *historyTable;
     QPushButton *playAll;
     DLineEdit *searchEdit ;
     PathSelector*pathSelector;
     QStackedWidget *page;
     QVector<CustomListView*> listDlistView;
     DFrame *qf;
     QStandardItemModel*videoListModel;

     QStandardItemModel*musicListModel;
     QStandardItemModel*historyListModel;
     int m_totalRows;      // 固定的总行数
     int m_currentHint;    // 当前已加载的数据量
     int m_loadCount;      // 每次加载的条数
     bool m_loading=0;      //防止重复加载
     bool m_loaded=0;
     int m_historyPre=-1;
     int  m_VideoPre=-1;
     // 线程控制相关成员
         QFuture<void> m_future;
         QFutureWatcher<void> m_watcher;
         QMutex m_mutex;
         bool m_abort = false;

     void checkIsEmpty();
     void onResetWindowSize(int width);
///Controlbar的上一曲下一曲会经过这里
  void playMusicFromIndex(int index);
void playMediaFromIndexInHistory(QModelIndex index);
  void playVideoFromIndex(int index);
     void LoadStyleSheet(const QString & url);

 void   onLocalListItemDoubleClicked(const QModelIndex &index );
         void setTheme(DGuiApplicationHelper::ColorType);
         void clearMusicTable();
         void clearVideoTable();
         void loadVideoTable();
         void loadHistoryTable();
         void resetMusicTable();
         void resetVideoTable();
         void videoplay(const QModelIndex &index );

       void addMusic(const MetaData& music);
         void onHistoryListRemove(int index);
       void addHistoryItem(const HistoryMData& item);

 private slots:
     void onBtPlayAll();

void   onHistoryListItemDoubleClicked(const QModelIndex &index );
     // void bt_selectDir();
     void onSearchTextChange(QString text);

void deleteHistoryByDir(const QString &dir);
 private:
void toApi();
     bool isLightTheme;
void updateUserVec(const QString &url);
  const int MAX_HISTORY = 100; // 限制历史记录数量
     void localMusicLayout();
     void initLayout();
     void initItem();
     void initSource();

    signals:
    void toResizeWidget();
    void videoPlaying();
    void toRefreshRecommand();

    void mediaChange();

    void mediaDeletedByDir(const QString &url);
 };

 ///调整Dlistview项间距
 // 外部因子变量，用于调节列宽比例（例如可根据窗口大小进行修改）
 static int factor = 800;
 // 自定义代理，负责绘制每一行的内容
 class TableItemDelegate : public QStyledItemDelegate
 {
 public:
     TableItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

     bool isLight=0;
     void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
 };
 class normalItemDelegate : public QStyledItemDelegate {
public:
    bool isLight=0;
    normalItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override    ;
};
#endif // MEDIATABLE_H
