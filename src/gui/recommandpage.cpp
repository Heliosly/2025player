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

void RecommandPage::refreshList(){

    auto similarity = UserPreference::instance()->similarity;
    model->clear();
    topCandidates.clear();
    // 第1阶段：取出前10首，加折损
    QVector<QPair<QString, qreal>> topCandidates;
    for (int i = 0; i < 10 && !similarity.empty(); ++i) {
        auto temp = similarity.top();
        similarity.pop();

        QString song = temp.first;
        qreal score = temp.second;

        int count = playedCount.value(song, 0);
        if (count > 0) score *= this->decayFactor(count);

        topCandidates.push_back(qMakePair(song, score));
    }
    for (const auto &p : topCandidates)
        similarity.push(p);

    // 冷却处理
    lastRoundRecommended = thisRoundRecommended;
    thisRoundRecommended.clear();

    // 推荐阶段
    int added = 0;
    while (!similarity.empty() && added < 5) {
        auto [song, sim] = similarity.top();
                        similarity.pop();

                        if (lastRoundRecommended.contains(song)) continue;

                        thisRoundRecommended.insert(song);
                        MetaData data = DataBase::instance()->getMetaDataByUrl(song);

                        auto *item = new DStandardItem();
                        item->setIcon(paintRadius(data.covpix, pixSize.width(), pixSize.height(), 18));
                        item->setText(data.title);
                        item->setData(data.url, Qt::UserRole + 1);
                        model->appendRow(item);

                        added++;
                   }
 qDebug()<<"trace1";

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

    m_recommandMutex=1;
//    QtConcurrent::run([this,dir]() {
           auto& similarity = UserPreference::instance()->similarity;

           // 临时 vector 保存保留项
           std::vector<QPair<QString, qreal>> retainedItems;

           // 弹出并筛选
           while (!similarity.empty()) {
               const auto& pair = similarity.top();
               QFileInfo fileInfo(pair.first);
               if (fileInfo.absolutePath() != dir) {
                   retainedItems.push_back(pair);
               }
               similarity.pop();
           }

           // 重新压入保留项
           for (const auto& item : retainedItems) {
               similarity.push(item);
           }

          m_recommandMutex=0;
                   this->refreshList();
           qDebug() << "[RecommandPage] Deleted similarity items in dir:" << dir;
            if(similarity.empty()){

//                QMetaObject::invokeMethod(this, [=]() {
                     UserPreference::instance()->temp->changeStackLayout(1);

//                }, Qt::QueuedConnection);




          }

//       });

        }


void RecommandPage::onThemeChange(DGuiApplicationHelper::ColorType type){
    if(type == DGuiApplicationHelper::ColorType::LightType)
    refreshBtn->setIcon(QIcon(":/asset/image/refresh.PNG"));
    else{

    refreshBtn->setIcon(QIcon(":/asset/image/refresh_dark.PNG"));
    }
       refreshBtn->setFixedSize(30, 30);
}
