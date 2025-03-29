#include <QFileIconProvider>
#include"videoplayer.h"
#include "pathselector.h"
#include "musictable.h"
#include "settingsmanager.h"
#include <QVBoxLayout>
#include <DTableWidget>
#include <QHeaderView>
#include <QScrollBar>
#include <QMediaMetaData>
#include <QDebug>
#include <DStandardItem>
#include <QPainter>
#include <DLineEdit>
#include <DFileDialog>
#include <QListWidget>
#include <QMouseEvent>
#include <QStringListModel>
MusicTable::MusicTable()
{
    this->setObjectName("localmusic");
   // LoadStyleSheet();
    localMusicLayout();
    initSource();
    initItem();
    initLayout();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &MusicTable::setTheme);
    connect(playAll, &DPushButton::clicked, this, &MusicTable::onBtPlayAll);
    // connect(selectDir,&DPushButton::clicked,this, &MusicTable::bt_selectDir);
    connect(searchEdit, &DLineEdit::textChanged, this, &MusicTable::onSearchTextChange);

    connect(&MusicPlayer::instance(), &MusicPlayer::mediaListAdd, this, &MusicTable::resetMusicTable,Qt::QueuedConnection);
    connect(SettingsManager::instance(), &SettingsManager::pathChange, this, &MusicTable::resetVideoTable);
    connect(&MusicPlayer::instance(),&MusicPlayer::historyListChange,this,&MusicTable::addHistoryItem);
    connect(&MusicPlayer::instance(),&MusicPlayer::historyListRemove,this,&MusicTable::loadHistoryTable);
    connect(&MusicPlayer::instance(), &MusicPlayer::musicToMusictable,
            this, &MusicTable::addMusic, Qt::QueuedConnection);
    connect(&MusicPlayer::instance(),&MusicPlayer::mediaListSub,this,&MusicTable::deleteByDir,Qt::QueuedConnection);
    connect(music_table,&DListView::doubleClicked,this,&MusicTable::onLocalListItemDoubleClicked);

    connect(video_table,&DListView::doubleClicked,this,&MusicTable::videoplay);
    connect(this,&MusicTable::toResizeWidget,this,&MusicTable::onResetWindowSize);

}
void MusicTable::deleteByDir(const QString &dir){
    auto &dirIndex=dirToIndex[dir];
    int n=dirIndex.size();
    if(dirIndex.empty()){
        qWarning()<<"[MusicTable::deleteByDir]dirIndex is empty";
        return;
    }

    int end=*dirIndex.rbegin();
    for (auto row  = dirIndex.rbegin(); row != dirIndex.rend(); ++row) {
        QStandardItem *item = musicListModel->takeItem(*row);  // 从模型中取出项
        if (item) {
            delete item;  // 释放内存
        }
        musicListModel->removeRow(*row);
    }
    for(auto &j:dirToIndex){
        for(auto &i:j)
            if(i>end){
                i-=n;
            }
    }
    dirToIndex.remove(dir);
}

void MusicTable::loadMoreData()
{
    if (m_loaded||m_loading || m_currentHint >= m_totalRows) {
        return;
    }
    m_loading = true;

    // 获取新数据
    QList<MetaData> dataList = DataBase::instance()->getDataFromLocallistwithHint(m_currentHint, qMin(m_loadCount,m_totalRows-m_currentHint));
    if (dataList.isEmpty()) {
        m_loading = false;
        m_loaded=true;
        qDebug() << "No more data to load maybe load all data";
        return;
    }



    for (const MetaData &music : dataList) {

        addMusic(music);
    }

    m_currentHint += dataList.size();
    m_loading = false;
}

void MusicTable::onScrollValueChanged(int value)
{
    Q_UNUSED(value);

    if (music_table->verticalScrollBar()->maximum() - value < 50) {
        loadMoreData();
    }
}
void MusicTable::setLoadParameters(int initialHint, int loadCount)
{
    m_currentHint = initialHint;
    m_loadCount = loadCount;

}
void MusicTable::setMusicCount(int value)
{
    music_table->verticalScrollBar()->setValue(value);
    m_totalRows=value;
}
void MusicTable::initSource(){
    pathSelector = new PathSelector(this);

}
void MusicTable::initItem()
{

    m_loading=0;
    music_table = new DListView(this);
    musicListModel = new QStandardItemModel();
    music_table->setModel(musicListModel);
    TableItemDelegate *delegate = new TableItemDelegate(music_table);
    music_table->setItemDelegate(delegate);

    setMusicCount
                    (DataBase::instance()->getListCount("locallist"));
    setLoadParameters(0, 50); // 每次加载 50 行
    music_table->setObjectName("table_music");
    QList<QString> tableList; //
    // QStandardItemModel* headmodel = new QStandardItemModel;

    tableList << "#" << "音乐标题" << "专辑" << "时长";
    music_table->setIconSize(QSize(60, 60));
    music_table->setUniformItemSizes(true);  // 启用统一 item 大小
    music_table->setGridSize(QSize(music_table->width(),80 ));
    music_table->setSpacing(20);



    music_table->setBackgroundType(DStyledItemDelegate::BackgroundType::NoBackground);

    music_table->setAlternatingRowColors(false);
    // 设置表格内容不可编辑
    music_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置为行选择
    //    music_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    // 设置最后一栏自适应长度
    //     title_table->horizontalHeader()->setStretchLastSection(true);
    // 删除网格表
    //     title_table->setShowGrid(false);
    // 去除边框线
    music_table->setFrameShape(QFrame::NoFrame);
    // 去除选中虚线框
    music_table->setFocusPolicy(Qt::NoFocus);

    // 设置点击单元格时，表头项不高亮
    //     title_table->horizontalHeader()->setHighlightSections(false);
    //     title_table->verticalHeader()->setHighlightSections(false);
    // 设置只能选中一个目标
    music_table->setSelectionMode(QAbstractItemView::SingleSelection);
    // 设置垂直滚动条最小宽度
    music_table->verticalScrollBar()->setMaximumWidth(7);
    music_table->setResizeMode(QListView::Adjust);
    //    title_table->verticalHeader()->setObjectName("music_verticalHeader");
    if(DataBase::instance()->countLocallist)loadMoreData();
    else {
        m_loaded=true;
    }
    video_table = new DListView(this);
    video_table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    video_table->setViewMode(QListView::IconMode);
    video_table->setIconSize(QSize(140, 140));
    video_table->setGridSize(QSize(200, 200));
    video_table->setSpacing(10);
    video_table->setResizeMode(QListView::Adjust);
    video_table->setMovement(QListView::Static);
    video_table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    //            video_table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    video_table->setSpacing(30);
    videoListModel = new QStandardItemModel(this);
    loadVideoTable();

    video_table->setModel(videoListModel);

    historyTable = new HistoryTable();
    historyTable->setParent(this);
    historyListModel = new QStandardItemModel(this);
    historyTable->setModel(historyListModel);

    historyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    historyTable->setSpacing(10);
    loadHistoryTable();

    playAll = new DPushButton(this);
    playAll->setText("播放全部");
    playAll->setMaximumSize(100, 40);
    playAll->setMinimumSize(100, 40);
    playAll->setObjectName("playallBtn");
    playAll->setIcon(QIcon(":/images/stackWidget/localMusic/btn_playall.png"));
}

void MusicTable::loadHistoryTable() {
    auto &history = MusicPlayer::instance().history;
    historyListModel->clear();

    for(const auto &item : history.history) {
        QStandardItem *newItem = new QStandardItem(QIcon(":/asset/image/video2.PNG"), item.title);
        historyListModel->appendRow(newItem);
    }

}

void MusicTable::onHistoryListRemove(int index){
    historyListModel->removeRow(index);
}
// 添加新的历史记录项目
void MusicTable::addHistoryItem(const HistoryMData& item) {
    QStandardItem *newItem = new QStandardItem(QIcon(":/asset/image/video2.PNG"), item.title);

    historyListModel->insertRow(0, newItem);


}
void MusicTable::loadVideoTable()
{
    // 使用 Lambda 读取文件并添加到 video_table
    auto addVideoItems = [&](QFileInfoList files)
    {
        for (const QFileInfo &fileInfo : files)
        {
            // 获取文件图标
            QFileIconProvider iconProvider;
            QIcon icon = iconProvider.icon(fileInfo); // 获取文件图标

            if (icon.isNull())
            {
                icon = QIcon(":/asset/image/video2.PNG"); // 使用默认图标路径
            }

            QStandardItem *item = new QStandardItem(icon, fileInfo.fileName());
            item->setData(fileInfo.absoluteFilePath(),Qt::UserRole+1);

            videoListModel->appendRow(item);
        }
    };

    for (const QString &mediaPath : SettingsManager::instance()->paths)
    {
        QDir dir(mediaPath);
        if (!dir.exists())
            return;

        QStringList filters = {"*.mp4", "*.avi", "*.mkv", "*.mov", "*.flv", "*.wmv"};
        QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
        addVideoItems(fileList);
    }
}
void MusicTable::clearMusicTable()
{
    musicListModel->clear();
    listDlistView.clear();
}
void MusicTable::clearVideoTable()
{
    videoListModel->clear();
}

void MusicTable::localMusicLayout()
{
    //    display_HBoxLayout = new QHBoxLayout();

    //    displayLabel[0] = new DLabel(this);
    //    displayLabel[0]->setText("本地音乐");
    //    displayLabel[0]->setObjectName("localLabel");
    //    displayLabel[1] = new DLabel(this);
    //    displayLabel[1]->setText("共0首");
    //    displayLabel[1]->setObjectName("numberlabel");

    //    QSpacerItem *display_hSpacer = new QSpacerItem(200,10,
    //                                                   QSizePolicy::Expanding,
    //                                                   QSizePolicy::Expanding);

    //    display_HBoxLayout->addWidget(displayLabel[0]);
    //    display_HBoxLayout->addWidget(displayLabel[1]);
    //    display_HBoxLayout->addSpacerItem(display_hSpacer);
    //    display_HBoxLayout->addWidget(selectDir);
    //    display_HBoxLayout->addSpacing(30);
}
void MusicTable::initLayout()
{

    QVBoxLayout *VLayout = new QVBoxLayout();

    QSpacerItem *Button_HSpacer = new QSpacerItem(200, 20,
                                                  QSizePolicy::Expanding,
                                                  QSizePolicy::Expanding);
    // 从左到右 存储table上方控件的布局
    QHBoxLayout *button_HBoxLayout = new QHBoxLayout();
    searchEdit = new DLineEdit();
    searchEdit->setPlaceholderText("搜索本地音乐");
    searchEdit->setObjectName("localSearch");
    searchEdit->setMaximumSize(200, 25);
    QAction *searchAction = new QAction(searchEdit);
    searchAction->setIcon(QIcon(":/images/stackWidget/localMusic/icon_search.png"));
    // 设置ICON在搜索框右边
    searchEdit->addAction(searchAction);
    button_HBoxLayout->addWidget(playAll);
    // 右边存储搜索栏和selectdir的布局

    QVBoxLayout *right_VBoxLayout = new QVBoxLayout();

    right_VBoxLayout->addWidget(searchEdit);
    right_VBoxLayout->addWidget(pathSelector);

    button_HBoxLayout->addSpacerItem(Button_HSpacer);
    button_HBoxLayout->addLayout(right_VBoxLayout);
    qf = new QFrame();
    qf->setObjectName("tableqf");
    QVBoxLayout *temp = new QVBoxLayout();
    // temp->addLayout(display_HBoxLayout);
    // temp->addSpacing(10);
    temp->addLayout(button_HBoxLayout);
    temp->addSpacing(10);
    qf->setLayout(temp);
    // temp->setContentsMargins(10,10,0,0);
    VLayout->addWidget(qf);
    page = new QStackedWidget(this);
    page->addWidget(music_table);
    page->addWidget(video_table);
    page->addWidget(historyTable);
    VLayout->addWidget(page);
    // VLayout->setContentsMargins(0,0,0,0);
    VLayout->setStretch(0, 1);
    VLayout->setStretch(1, 8);
    this->setLayout(VLayout);
    music_table->setBackgroundRole(QPalette::NoRole);
    music_table->setSpacing(10);
}
void MusicTable::playMusicFromIndex(int index){
    QModelIndex Index = musicListModel->index(index, 0);
    MusicPlayer::instance().play( Index.data(Qt::UserRole + 5).toString());


}
void MusicTable::addMusic(const MetaData &music)
{

    int minutes = music.duration / 60;
    int seconds = music.duration % 60;


    QPixmap sourcePixmap;

    // 检查原始图像是否有效
    if (music.covpix.isNull()) {
        qWarning() << "Cover image is null, using placeholder";
        sourcePixmap = QPixmap(":/asset/image/unknowcovpix.jpg");
    } else {
        sourcePixmap = music.covpix;
    }
    int margin = 10;
    int imageSize = 60;
    // 生成目标 pixmap 尺寸：图片区域加上 margin
    QPixmap roundedPixmap(imageSize + 2 * margin, imageSize + 2 * margin);
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 目标区域，即中间区域
    QRect targetRect(margin, margin, imageSize, imageSize);

    // 可选：如果 sourcePixmap 尺寸不合适，进行缩放，保持平滑和宽高比
    QPixmap scaledPixmap = sourcePixmap.scaled(imageSize, imageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 设置剪裁区域为圆角矩形
    QPainterPath path;
    path.addRoundedRect(targetRect, 22, 22);  // 圆角半径设为18，可根据需要调整
    painter.setClipPath(path);

    painter.drawPixmap(targetRect, scaledPixmap);
    painter.end();
    int row = music_table->count();

    // music_Table->setRowHeight(row,200);
    //  设置每列的值;
    //  设置第一列（序号）


    QString duration = QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    // 设置第二列（标题）
    DStandardItem *item = new DStandardItem(music.title);
    item->setIcon(roundedPixmap.scaled(QSize(80, 80), Qt::KeepAspectRatio, Qt::SmoothTransformation));


    item->setData(music.artist,Qt::UserRole+1);
    item->setData(music.album, Qt::UserRole + 2);
    item->setData(duration, Qt::UserRole + 3);

    item->setData(music.url,Qt::UserRole+5);

    item->setSizeHint(QSize(0,80));


    // view->setViewMode(QListView::IconMode);

    DViewItemAction *act = new DViewItemAction;

    act->setText(music.artist);
    act->setFontSize(DFontSizeManager::T8);
    act->setTextColorRole(DPalette::TextTitle);
    act->setParent(music_table);


    musicListModel->appendRow(item );




    QFileInfo fileInfo(music.url);
    QString dirpath = fileInfo.absolutePath();  // 获取目录路径
    if(!dirToIndex.contains(dirpath)){

        dirToIndex[dirpath] = QList<int>();
    }
    dirToIndex[dirpath].append(row);
}

void MusicTable::onBtPlayAll()
{

    QStandardItem *firstItem = musicListModel->item(0, 0);
    if (firstItem) {
        QString musicUrl = firstItem->data(Qt::UserRole + 5).toString();
        MusicPlayer::instance().play(musicUrl);
    }
    else qWarning()<<"[MusicTable::onBtPlayAll]:cant get first Item";

}void MusicTable::LoadStyleSheet()
{
    QFile file(":/asset/qss/musictb.qss");
    file.open(QIODevice::ReadOnly);

    if (file.isOpen())
    {
        QString style = this->styleSheet();
        style += QLatin1String(file.readAll());
        this->setStyleSheet(style);
        file.close();
    }
}

void MusicTable::setTheme(DGuiApplicationHelper::ColorType theme)
{

    //    if(theme==DGuiApplicationHelper::LightType){
    //        QPalette palette = this->palette();
    //        palette.setColor(QPalette::Background, Qt::white);

    //    }else {
    //        QPalette palette = this->palette();
    //        palette.setColor(QPalette::Background,Qt::black);

    //    }
}
void MusicTable::onResetWindowSize(int width)
{


    factor = ((width - 900) * 5 / 18);
}

void MusicTable::onSearchTextChange(QString text)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel *>(music_table);
    if (!model) return;

    for (int i = 0; i < model->rowCount(); ++i) {
        QString itemText = model->index(i, 0).data(Qt::DisplayRole).toString();

        bool matchFound = itemText.contains(text, Qt::CaseInsensitive);
        music_table->setRowHidden(i, !matchFound);
    }
}
void MusicTable::resetMusicTable()
{        
    m_loaded=0;
    setMusicCount(DataBase::instance()->countLocallist);
    int width =music_table ->width();
    width*=1.25;
    emit toResizeWidget(width);


}
void MusicTable::resetVideoTable()
{
    clearVideoTable();
    loadVideoTable();
}

void HistoryTable::mouseDoubleClickEvent(QMouseEvent *event)
{

    this->musicPlay();
}
void HistoryTable::musicPlay()
{
    MusicPlayer::instance().play(url);
}

void MusicTable::onLocalListItemDoubleClicked(const QModelIndex &index ){
    QStandardItem *item = musicListModel->itemFromIndex(index);
    QString musicUrl =item->data(Qt::UserRole + 5).toString();
    MusicPlayer::instance().play(musicUrl);
}

void TableItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const  {

    // 获取数据：图标、文本及自定义数据
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
    QString text = index.data(Qt::DisplayRole).toString();
    QString dataA = index.data(Qt::UserRole + 1).toString();
    QString dataB = index.data(Qt::UserRole + 2).toString();
    QString dataC = index.data(Qt::UserRole + 3).toString();
    int row = index.row();
    QString rowNumber = (row + 1 >= 10) ? QString::number(row + 1)
                                        : "0" + QString::number(row + 1);

    // 定义各列宽度
    int numColWidth         = 30;
    int iconColWidth        = 80;
    int textAndDataAColWidth = 200 + factor;  // 第三列：同时显示 text 和 dataA
    int dataBColWidth       = 200 + factor;
    int dataCColWidth       = option.rect.width() - (numColWidth + iconColWidth + textAndDataAColWidth + dataBColWidth);

    // 构造各列区域
    QRect rectNum(option.rect.x(), option.rect.y(), numColWidth, option.rect.height());
    QRect rectIcon(rectNum.right() + 20, option.rect.y(), iconColWidth, option.rect.height());
    QRect rectTextDataA(rectIcon.right() + 1, option.rect.y(), textAndDataAColWidth, option.rect.height());
    QRect rectDataB(rectTextDataA.right() + 1, option.rect.y(), dataBColWidth, option.rect.height());
    QRect rectDataC(rectDataB.right() + 1, option.rect.y(), dataCColWidth, option.rect.height());

    // 开始绘制
    painter->save();

    // 绘制选中背景（带圆角效果）
    if (option.state & QStyle::State_Selected) {
        QPainterPath path;
        path.addRoundedRect(option.rect.adjusted(2, 2, -2, -2), 18, 18); // 向内缩进 2px，圆角半径 18px
        painter->setRenderHint(QPainter::Antialiasing,QPainter::SmoothPixmapTransform);
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.highlight());
        painter->drawPath(path);
    }

    painter->drawText(rectNum, Qt::AlignCenter, rowNumber);

    if (!icon.isNull()) {
        QSize iconSize = rectIcon.size().boundedTo(QSize(80, 80)); // 限制最大尺寸
        painter->drawPixmap(rectIcon, icon.pixmap(iconSize));
    }

    QRect rectText = rectTextDataA;
    rectText.setHeight(rectTextDataA.height() / 2);

    QRect rectDataA = rectTextDataA;
    rectDataA.setY(rectTextDataA.y() + rectTextDataA.height() / 2);
    rectDataA.setHeight(rectTextDataA.height() / 2);


    painter->setPen(Qt::black);
    painter->drawText(rectText, Qt::AlignLeft | Qt::AlignBottom, text);

    painter->setPen(QColor(128, 128, 128));  // 灰色
    painter->drawText(rectDataA, Qt::AlignLeft | Qt::AlignTop, dataA);

    painter->setPen(Qt::black);
    painter->drawText(rectDataB, Qt::AlignCenter, dataB);

    painter->drawText(rectDataC, Qt::AlignCenter, dataC);

    painter->restore();
}

void MusicTable::videoplay(const QModelIndex &index ){

 QStandardItem *item = videoListModel->itemFromIndex(index);
    QString url =item->data(Qt::UserRole + 1).toString();


    VideoPlayer::instance()->play(url);
    emit videoPlaying();
}

void MusicTable::playVideoFromIndex(int index){
    QModelIndex Index = videoListModel->index(index, 0);
    VideoPlayer::instance()->play( Index.data(Qt::UserRole + 1).toString());

}
