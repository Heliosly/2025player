#ifndef USERVECTOR_H
#define USERVECTOR_H

#include <QMutex>
#include <QList>
#include <QPair>
#include <QString>
#include <QVector>
#include <map>
#include <QtCharts/QtCharts>
#include<queue>
#include<vector>
#include<DLabel>
#include"recommandpage.h"
QT_CHARTS_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

struct CompareQPair {
 bool operator()(const QPair<QString, qreal>& a, const QPair<QString, qreal>& b) const {
            return a.second < b.second; // 按 qreal 从大到小排序
        }
    };


class PieChartWidget;
class UserPreference :public QObject{
    Q_OBJECT
public:

    friend class RecommandPage;
    std::priority_queue<QPair<QString, qreal>, std::vector<QPair<QString, qreal>>, CompareQPair> similarity;
    PieChartWidget * temp=nullptr;
        static UserPreference* instance();
    // 使用top10标签更新用户喜好向量，alpha控制更新比例
    void update(const QList<QPair<QString, qreal>>& top10Tags, qreal alpha = 0.8);

    // 计算当前用户喜好向量与候选音乐（通过top10标签表示）的余弦相似度
    qreal cosineSimilarity(const QList<QPair<QString, qreal>>& candidateTop10Tags,const QString & url);

    // 获取当前用户喜好向量
    QVector<qreal> getVector() const;

    static QMap<QString, int>createTagMapping() ;

    static const QVector<QString> s_tagList;


    QVector<qreal> m_vector;
void reWrite();

    void emitLoadSomeMusicMedia();
    void rollBackByUrlList(const QStringList & urlList);

private:
  //暂时用于锁优先队列
    QMutex m_mutex;
    UserPreference();
    static UserPreference *s_instance;
    // 用户喜好向量（固定50维）

    // 对内部向量归一化
    void normalize();

    // 将top10标签（QList<QPair<QString, int>>）转换为50维向量
    QVector<qreal> musicTagsToVector(const QList<QPair<QString, qreal>>& top10Tags) const;

    // 50个标签的顺序列表

    // 标签到索引的映射（使用std::map保证值排序）
    static const  QMap<QString, int> s_tagToIndex;
//说是10tags其实是无序50tag
QList<QPair<QString, qreal>> vectorToTop10Tags(const QVector<qreal>& vector) const ;

signals:
    void loadSomeMusicMedia();

};
// PieChartWidget 类声明
class PieChartWidget : public QWidget {
    Q_OBJECT
public:
    // 构造函数接收初始数据
    explicit PieChartWidget(const QVector<qreal>& m_vector, QWidget* parent = nullptr);

    // 新方法：传入新的 m_vector 数据，更新图表内容
    void updateChart(const QVector<qreal>& m_vector);
    void  setDefaultLabeltText(const QString & text);
    void changeStackLayout(int index);

private:
    QChart *m_chart;           // QChart 对象，显示饼图的主图表
    QChartView *m_chartView;   // QChartView 对象，承载 QChart 并显示图表
    DLabel *defaultLabel;
    QStackedLayout *layout;
};

#endif // USERVECTOR_H
