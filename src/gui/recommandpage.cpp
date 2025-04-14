#include "recommandpage.h"
#include "uservector.h"

#include"musicplayer.h"
#include"database.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <DListView>
#include <QStandardItemModel>
#include <DStandardItem>
#include<DIconButton>
#include<DWidget>
#include<QtConcurrent>

RecommandPage::RecommandPage(QWidget *parent)
        : DFrame(parent)
{

    // 1) 列表视图
    listView = new DListView(this);
    model= new QStandardItemModel;
    listView->setModel(model);
    listView->setIconSize(pixSize);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    listView->setSelectionMode(QAbstractItemView::SingleSelection);

    listView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);


    QWidget *leftW  = new QWidget(this);
    QVBoxLayout *leftL = new QVBoxLayout;
    // 2) 刷新按钮，浮在列表右下角

    refreshBtn = new DIconButton(this);
    refreshBtn->setToolTip(QStringLiteral("刷新推荐"));
    refreshBtn->setFixedSize(30, 30);

    leftL->addWidget(listView,9);
    leftL->addWidget(refreshBtn,1);
    leftW->setLayout(leftL);

    // ——— 右侧：用户信息（右上）+ 饼图（右下） ———
    DFrame *infoFrame = new DFrame(this);

    // 获取当前样式
    QString style = infoFrame->styleSheet();

    // 修改并设置新的样式
    style += "border-radius: 50px;";  // 添加圆角样式
    infoFrame->setStyleSheet(style);  // 应用新的样式
    QVBoxLayout *rightLayout = new QVBoxLayout(infoFrame);
    infoFrame->setLayout(rightLayout);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(10);

    // （这里不做 QSS，保持默认样式，你可以后面再加样式）

    // 饼状图
    auto *userChart = new PieChartWidget(UserPreference::instance()->m_vector, this);

    // 如果需要在 UserPreference 保存指针，可按需恢复下面这行
    UserPreference::instance()->temp = userChart;

    // 布局：上信息、下饼图，两者都靠右
    auto *imageLabel = new DLabel(this);
    imageLabel->setPixmap(QPixmap(":/asset/image/logo.png").scaled(QSize(250,250), Qt::KeepAspectRatio, Qt::SmoothTransformation));


    rightLayout->addWidget(imageLabel, /*stretch=*/1,
                           Qt::AlignCenter | Qt::AlignCenter);

    rightLayout->addSpacing(10);
    rightLayout->addWidget(userChart  ,1);
    userChart->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ——— 总布局 ———
    auto *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(20);
    // 左右各占 1 份空间
    mainLayout->addWidget(leftW, 2);
    mainLayout->addWidget(infoFrame,1);

    setLayout(mainLayout);

    connect(refreshBtn, &DIconButton::clicked, this, &RecommandPage::refreshList);
    connect(listView,&DListView::doubleClicked,this,&RecommandPage::playMusic);
    connect(DApplicationHelper::instance(),&DApplicationHelper::themeTypeChanged,this,&RecommandPage::onThemeChange);
}

void RecommandPage::playMusic(QModelIndex index){
    QString url = index.data(Qt::UserRole+1).toString();
    playedCount[url]++;
    MusicPlayer::instance().play( url);

}

void RecommandPage::playMusicFromIndex(int index){
    QModelIndex Index = model->index(index, 0);
    QString url = Index.data(Qt::UserRole+1).toString();
    playedCount[url]++;
    listView->setCurrentIndex(Index );
    MusicPlayer::instance().play( url);


}

void RecommandPage::refreshList() {
    // 1. 先把整个 priority_queue 在短暂锁内复制一份到本地
    std::priority_queue<
        QPair<QString, qreal>,
        std::vector<QPair<QString, qreal>>,
        CompareQPair
    > localSim;
    {
        QMutexLocker locker(&UserPreference::instance()->m_mutex);
        localSim = UserPreference::instance()->similarity;
    } //  锁在这里就释放了

    // 2. 清 UI 模型
    model->clear();

    // 3. 第一阶段：取出前10条并做衰减
    QVector<QPair<QString, qreal>> topCandidates;
    for (int i = 0; i < 10 && !localSim.empty(); ++i) {
        auto [song, score] = localSim.top();
        localSim.pop();
        int cnt = playedCount.value(song, 0);
        if (cnt > 0) score *= decayFactor(cnt);
        topCandidates.push_back(qMakePair(song, score));
    }
    // 把它们再塞回 localSim，以便第二阶段使用
    for (auto &p : topCandidates) {
        localSim.push(p);
    }

    // 4. 冷却：上轮推荐 → 本轮推荐
    lastRoundRecommended = thisRoundRecommended;
    thisRoundRecommended.clear();

    // 5. 第二阶段：从 localSim 中拿出最多5条新推荐
    int added = 0;
    QVector<QPair<QString, qreal>> recList;
    while (!localSim.empty() && added < 5) {
        auto [song, sim] = localSim.top();
        localSim.pop();
        if (lastRoundRecommended.contains(song)) continue;
        recList.push_back(qMakePair(song, sim));
        thisRoundRecommended.insert(song);
        ++added;
    }

    // 6. 更新 UI（完全在无锁状态下）
    for (auto &p : recList) {
        MetaData data = DataBase::instance()->getMetaDataByUrl(p.first);
        auto *item = new DStandardItem();
        item->setIcon(paintRadius(data.covpix,
                                 pixSize.width(),
                                 pixSize.height(),
                                 18));
        item->setText(data.title);
        item->setData(data.url, Qt::UserRole + 1);
        model->appendRow(item);
    }
    qDebug() << "trace1";

    // 7. 最后再短暂加锁检查原队列是否已空
    bool isEmpty;
    {
        QMutexLocker locker(&UserPreference::instance()->m_mutex);
        isEmpty = UserPreference::instance()->similarity.empty();
    }
    if (isEmpty) {
        UserPreference::instance()->temp->changeStackLayout(1);
    }
}

QPixmap RecommandPage::paintRadius(QPixmap sourcePixmap,int width,int height,int radius){
            int margin = 10;
            // 生成目标 pixmap 尺寸：图片区域加上 margin
            QPixmap roundedPixmap(width + 2 * margin, height + 2 * margin);
            roundedPixmap.fill(Qt::transparent);

            QPainter painter(&roundedPixmap);
            painter.setRenderHint(QPainter::Antialiasing);

            // 目标区域，即中间区域
            QRect targetRect(margin, margin, width, height);

            // 可选：如果 sourcePixmap 尺寸不合适，进行缩放，保持平滑和宽高比
            QPixmap scaledPixmap = sourcePixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

            // 设置剪裁区域为圆角矩形
            QPainterPath path;
            path.addRoundedRect(targetRect, radius, radius);  // 圆角半径设为18，可根据需要调整
            painter.setClipPath(path);

            painter.drawPixmap(targetRect, scaledPixmap);
            painter.end();
            return roundedPixmap;
        }
        qreal RecommandPage::decayFactor(int count){
            return std::exp(-LAMBDA * count);
        }

void RecommandPage::deleteByDir(const QString &dir){

    auto& similarity = UserPreference::instance()->similarity;
      QMutex& mutex = UserPreference::instance()->m_mutex;

      // 临时 vector 保存保留项
      std::vector<QPair<QString, qreal>> retainedItems;

      // 提取阶段，只锁读取和 pop
      while (true) {
          QPair<QString, qreal> pair;
          {
              QMutexLocker locker(&mutex);
              if (similarity.empty()) break;
              pair = similarity.top();
              similarity.pop();
          }

          // 判断是否保留
          QFileInfo fileInfo(pair.first);
          if (fileInfo.absolutePath() != dir) {
              retainedItems.push_back(pair);
          }
      }

      // 重压阶段，只锁 push
      {
          QMutexLocker locker(&mutex);
          for (const auto& item : retainedItems) {
              similarity.push(item);
          }
      }

      this->refreshList();
      qDebug() << "[RecommandPage] Deleted similarity items in dir:" << dir;

      {
          QMutexLocker locker(&mutex);
          if (similarity.empty()) {
              UserPreference::instance()->temp->changeStackLayout(1);
          }
      }

}


void RecommandPage::onThemeChange(DGuiApplicationHelper::ColorType type){
    if(type == DGuiApplicationHelper::ColorType::LightType)
    refreshBtn->setIcon(QIcon(":/asset/image/refresh.PNG"));
    else{

    refreshBtn->setIcon(QIcon(":/asset/image/refresh_dark.PNG"));
    }
       refreshBtn->setFixedSize(30, 30);
}
