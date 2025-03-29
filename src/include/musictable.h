///所有的多媒体文件都在这里展示,page目前存储了音视频页面
#ifndef MEDIATABLE_H
#define MEDIATABLE_H
#include"musicplayer.h"
#include"pathselector.h"
#include<QDir>
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
#include<QStackedWidget>
 DWIDGET_USE_NAMESPACE
 class CustomListView;  
 class MusicTable : public QFrame {
     Q_OBJECT
 public:
     MusicTable();

     void loadMoreData();

     void deleteByDir(const QString&dir);
     void onScrollValueChanged(int value);

     void setLoadParameters(int initialHint, int loadCount);

     void setMusicCount(int value);

     QMap<QString,QList<int>> dirToIndex;
     DListView *music_table;
     DListView *video_table;


     DListView *historyTable;
     DPushButton *playAll;
     DLineEdit *searchEdit ;
//     DLabel *displayLabel[2];
//     QVBoxLayout *local_VBoxLayout;
     PathSelector*pathSelector;
     QHBoxLayout *display_HBoxLayout;
     QStackedWidget *page;
     QVector<CustomListView*> listDlistView;
     QFrame *qf;
     QStandardItemModel*videoListModel;

     QStandardItemModel*musicListModel;
     QStandardItemModel*historyListModel;
     int m_totalRows;      // 固定的总行数
     int m_currentHint;    // 当前已加载的数据量
     int m_loadCount;      // 每次加载的条数
     bool m_loading;      //防止重复加载
     bool m_loaded;


     void onResetWindowSize(int width);
///Controlbar的上一曲下一曲会经过这里
  void playFromListView(int index);
 public slots:
 void   onLocalListItemDoubleClicked(const QModelIndex &index );
         void setTheme(DGuiApplicationHelper::ColorType);
         void clearMusicTable();
         void clearVideoTable();
         void loadVideoTable();
         void loadHistoryTable();
         void resetMusicTable();
         void resetVideoTable();

       void addMusic(const MetaData& music);
         void onHistoryListRemove(int index);
       void addHistoryItem(const HistoryMData& item);

 private slots:
     void onBtPlayAll();
     // void bt_selectDir();
     void onSearchTextChange(QString text);

 private:

  const int MAX_HISTORY = 100; // 限制历史记录数量
     void LoadStyleSheet();
     void localMusicLayout();
     void initLayout();
     void initItem();
     void initSource();

    signals:
    void toResizeWidget(int width);
 };

 ///调整Dlistview项间距
 // 外部因子变量，用于调节列宽比例（例如可根据窗口大小进行修改）
 static int factor = 800;
 // 自定义代理，负责绘制每一行的内容
 class TableItemDelegate : public QStyledItemDelegate
 {
 public:
     TableItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

     void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
 };
class HistoryTable : public DListView {
    Q_OBJECT
public:
    QString url;
    MusicTable *tableWidget;
    
    int number;
    void mouseDoubleClickEvent(QMouseEvent *event);

    void play();
};
#endif // MEDIATABLE_H
