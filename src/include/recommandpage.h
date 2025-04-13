#ifndef RECOMMANDPAGE_H
#define RECOMMANDPAGE_H

#include"metadata.h"
#include <DFrame>
#include <QListWidget>
#include<DStandardItem>
#include<DListView>
#include<DIconButton>
#include<DLabel>
#include<QResizeEvent>
#include<cmath>

#include<DApplicationHelper>
DWIDGET_USE_NAMESPACE

// Forward declare MetaData 和 SongItemWidget（假设你已在其他地方定义）

class RecommandPage : public DFrame
{
    Q_OBJECT
public:
    explicit RecommandPage(QWidget *parent = nullptr);
    QVector<QPair<QString,qreal>> topCandidates;
    QSet<QString> lastRoundRecommended; // 记录上一轮推荐的歌曲
    QSet<QString> thisRoundRecommended;
    QHash<QString, int> playedCount;
    DListView *listView ;
    QStandardItemModel * model;
    const  qreal LAMBDA=0.5;

    int count=0;
    const QSize pixSize=QSize(100,100);
    const QSize itemSize= QSize(150,150);
public slots:

     void refreshList();
void playMusicFromIndex(int index);
     void playMusic(QModelIndex index);

    void deleteByDir(const QString &dir);
private:// 衰减函数：指数型

     void onThemeChange(DGuiApplicationHelper::ColorType type);
    bool m_recommandMutex=0;
     qreal decayFactor(int count);
     DIconButton *refreshBtn;
     QPixmap paintRadius(QPixmap sourcePixmap,int width,int height,int radius);
protected:

};
#endif
// RECOMMANDPAGE_H
