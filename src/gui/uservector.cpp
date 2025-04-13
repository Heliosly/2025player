#include "uservector.h"

#include"database.h"
#include <QDebug>
#include <cmath>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include<QVector>

QT_CHARTS_USE_NAMESPACE
// 定义50个标签的顺序列表

UserPreference* UserPreference::s_instance = nullptr;
const QVector<QString> UserPreference::s_tagList = {
    "guitar", "classical", "slow", "techno", "strings",
    "drums", "electronic", "rock", "fast", "piano",
    "ambient", "beat", "violin", "vocal", "synth",
    "female", "indian", "opera", "male", "singing",
    "vocals", "no vocals", "harpsichord", "loud", "quiet",
    "flute", "woman", "male vocal", "no vocal", "pop",
    "soft", "sitar", "solo", "man", "classic",
    "choir", "voice", "new age", "dance", "male voice",
    "female vocal", "beats", "harp", "cello", "no voice",
    "weird", "country", "metal", "female voice", "choral"
};

// 帮助函数：建立标签到索引的映射
QMap<QString, int> UserPreference::createTagMapping() {
    QMap<QString, int>mapping;
    for (int i = 0; i < UserPreference::s_tagList.size(); ++i) {
        mapping[s_tagList[i]] = i;
    }
    return mapping;
}

// 定义静态成员 s_tagToIndex
const QMap<QString, int> UserPreference::s_tagToIndex = createTagMapping();

UserPreference::UserPreference() {
       // 初始化用户向量为50维全0向量
    if(!DataBase::instance()->checkUrlExistsInTags("_user")){

    m_vector.fill(0.0, 50);
    }
    else{
        m_vector= musicTagsToVector(DataBase::instance()->parseTagsToOrderedList(DataBase::instance()->getTagsArrayByUrl("_user")));

    }
}
UserPreference* UserPreference::instance()
{
    if (s_instance == nullptr) {
        s_instance = new UserPreference();  // 创建唯一的实例
    }
    return s_instance;
}

void UserPreference::reWrite()
{
        DataBase::instance()->rewriteTagsWithList("_user",this->vectorToTop10Tags(m_vector));

                             }
void UserPreference::normalize() {
    qreal norm = 0.0;
    for (qreal v : m_vector) {
        norm += v * v;
    }
    norm = std::sqrt(norm);
    if (norm > 0.0) {
        for (int i = 0; i < m_vector.size(); ++i) {
            m_vector[i] /= norm;
        }
    }
}

QVector<qreal> UserPreference::musicTagsToVector(const QList<QPair<QString, qreal>>& top10Tags) const {
    QVector<qreal> vec(50, 0.0);
    for (int i = 0; i < top10Tags.size(); ++i) {
        QString tag = top10Tags[i].first;
        qreal confidence = top10Tags[i].second; // 置信度（整数值）
        auto it = s_tagToIndex.find(tag);
        if (it != s_tagToIndex.end()) {
            int idx =  it.value();
            vec[idx] = confidence;
        }
    }
    return vec;
}

void UserPreference::update(const QList<QPair<QString, qreal>>& top10Tags, qreal alpha) {
    // 将当前音乐的top10标签转换为向量
    QVector<qreal> music_vector = musicTagsToVector(top10Tags);

    // 更新公式：user_vector = alpha * user_vector + (1 - alpha) * music_vector
    for (int i = 0; i < 50; ++i) {
        m_vector[i] = alpha * m_vector[i] + (1.0 - alpha) * music_vector[i];
    }
    // 归一化更新后的向量
    normalize();
    if(temp){
        temp->updateChart(m_vector);

    }
    else{
        qWarning()<<"[UserPreFerencer::update]:chart unloaded";
    }
}

qreal UserPreference::cosineSimilarity(const QList<QPair<QString, qreal>>& candidateTop10Tags,const QString &url)  {
    // 将候选音乐的top10标签转换为向量
    QVector<qreal> candidate_vector = musicTagsToVector(candidateTop10Tags);

    // 归一化候选向量
    qreal normCandidate = 0.0;
    for (qreal v : candidate_vector) {
        normCandidate += v * v;
    }
    normCandidate = std::sqrt(normCandidate);
    if (normCandidate > 0.0) {
        for (int i = 0; i < candidate_vector.size(); ++i) {
            candidate_vector[i] /= normCandidate;
        }
    }

    // 计算余弦相似度
    qreal dot = 0.0, normUser = 0.0;
    for (int i = 0; i < m_vector.size(); ++i) {
        dot += m_vector[i] * candidate_vector[i];
        normUser += m_vector[i] * m_vector[i];
    }
    normUser = std::sqrt(normUser);
    if (normUser == 0.0 || normCandidate == 0.0) return 0.0;
    similarity.push(qMakePair(url, dot / (normUser * normCandidate)));



    return dot / (normUser * normCandidate);
}

QVector<qreal> UserPreference::getVector() const {
    return m_vector;
}

QList<QPair<QString, qreal>> UserPreference::vectorToTop10Tags(const QVector<qreal>& vector) const {
    QList<QPair<QString, qreal>> top10Tags;

    // 将用户向量与标签一起放入一个列表
    for (int i = 0; i < vector.size(); ++i) {
        QString tag = s_tagList[i];
        qreal confidence = vector[i] ;  // 将qreal置信度转换为0-100的整数
        top10Tags.append(qMakePair(tag, confidence));
    }



    return top10Tags;
}
PieChartWidget::PieChartWidget(const QVector<qreal>& m_vector, QWidget* parent)
    : QWidget(parent)
{
    // 初始化图表对象和图表视图
    m_chart = new QChart();
    m_chart->setMargins(QMargins(10, 10, 10, 10));
    m_chart->legend()->hide();
    defaultLabel = new DLabel();
    defaultLabel ->setText("尚未播放过音乐");
    QFont font;
      font.setFamily("Noto Sans CJK SC");  // 设置字体家族
            font.setPointSize(24);  // 设置字体大小
            defaultLabel->setFont(font);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    layout = new QStackedLayout(this);
    layout->addWidget(m_chartView);
    layout ->addWidget(defaultLabel);
    setLayout(layout);

    // 根据初始数据生成图表
       updateChart(m_vector);
}

// 新方法：传入新的 m_vector 数据，更新图表内容
void PieChartWidget::updateChart(const QVector<qreal>& m_vector)
{
    m_chart->removeAllSeries();

    // 构造 (tag, value) 对（假定 m_vector 至少包含 s_tagList.size() 个数据，否则取最小值）
    QVector<QPair<QString, qreal>> tagValues;
    int count = qMin(UserPreference::instance()-> s_tagList.size(), m_vector.size());
    for (int i = 0; i < count; ++i) {
        tagValues.append(qMakePair(UserPreference::instance()-> s_tagList[i], m_vector[i]));
    }

    int ct=10;
    // 按数值降序排序
    std::sort(tagValues.begin(), tagValues.end(),
              [](const QPair<QString, qreal>& a, const QPair<QString, qreal>& b) {
                  return a.second > b.second;
              });

    // 将第11个及以后的数据合并为“Other”
    double otherSum = 0.0;
       for(int i=0;i<10;i++){
        if(tagValues[i].second==0.0){
        ct=i;

        break;

        }

    }

       if(ct <10){

        otherSum=0;
       }
       else
       {
        for (int i = 10; i < tagValues.size(); ++i) {
        otherSum += tagValues[i].second;
    }
       }


    // 创建新的 QPieSeries 对象
    QPieSeries *series = new QPieSeries();

    // 添加前 10 个数据切片，并设置颜色渐变（从红到蓝）
    int sliceCount = qMin(ct, tagValues.size());
    for (int i = 0; i < sliceCount; ++i) {
        QPieSlice *slice = series->append(tagValues[i].first, tagValues[i].second);
        slice->setLabelVisible(true);
        slice->setLabelColor(Qt::black);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        slice->setExploded(false);
        slice->setPen(QPen(Qt::NoPen));

        // 计算颜色渐变：红 (255,0,0) 到蓝 (0,0,255)
        double ratio = static_cast<double>(i) / sliceCount;
        int r = 255 + static_cast<int>((0 - 255) * ratio);
        int g = 0;
        int b = 0 + static_cast<int>((255 - 0) * ratio);
        slice->setBrush(QColor(r, g, b));
    }

    // 添加“Other”切片（若合并的值大于0）
    if (otherSum > 0) {
        QPieSlice *otherSlice = series->append("Other", otherSum);
        otherSlice->setLabelVisible(true);
        otherSlice->setLabelColor(Qt::black);
        otherSlice->setLabelPosition(QPieSlice::LabelOutside);
        otherSlice->setExploded(false);
        otherSlice->setPen(QPen(Qt::NoPen));
        otherSlice->setBrush(QColor(0, 0, 255));
    }

    // 将新的系列添加到图表中
    m_chart->addSeries(series);
    m_chart->setTitle("Music Likeness Distribution");


    // 更新视图
    m_chartView->update();
    if(ct==0){
        layout->setCurrentIndex(1);
    }
    else{
        layout->setCurrentIndex(0);
    }
}

void UserPreference::emitLoadSomeMusicMedia(){
   emit loadSomeMusicMedia();
}

void PieChartWidget::changeStackLayout(int index){
    if(index>1){
        qDebug()<<"invalid index in changeStackLayout";
        return ;
    }
    this->layout->setCurrentIndex(index);
}

void PieChartWidget::setDefaultLabeltText(const QString &text){
    if(defaultLabel)
    defaultLabel->setText(text);
}
