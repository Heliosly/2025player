#include <QFileIconProvider>
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
    LoadStyleSheet();
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

}
void MusicTable::deleteByDir(const QString &dir){
    auto &dirIndex=dirToIndex[dir];
      for (auto it = dirIndex.rbegin(); it != dirIndex.rend(); ++it) {
        QListWidgetItem *item = music_table->takeItem(*it);
           delete item;  // 释放内存
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
    music_table = new QListWidget(this);

    setMusicCount
(DataBase::instance()->getListCount("locallist"));
   setLoadParameters(0, 50); // 每次加载 50 行
    music_table->setObjectName("table_music");
    QList<QString> tableList; //
    // QStandardItemModel* headmodel = new QStandardItemModel;
    music_table->setSortingEnabled(false);

    tableList << "#" << "音乐标题" << "专辑" << "时长";
    music_table->setIconSize(QSize(50, 50));

    // 打开右键菜单属性
    music_table->setContextMenuPolicy(Qt::CustomContextMenu);

    music_table->setAlternatingRowColors(false);
    // 设置表格内容不可编辑
    music_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 设置为行选择
    music_table->setSelectionBehavior(QAbstractItemView::SelectRows);
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
    music_table->clear();
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

void MusicTable::addMusic(const MetaData &music)
{

    int minutes = music.duration / 60;
    int seconds = music.duration % 60;
    QPixmap roundedPixmap(music.covpix.size());
    roundedPixmap.fill(Qt::transparent); // 设置透明背景

    // 绘制圆角矩形
    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
    painter.setBrush(QBrush(music.covpix));
    painter.setPen(Qt::NoPen);                                                            // 不需要边框
    painter.drawRoundedRect(0, 0, music.covpix.width(), music.covpix.height(), 100, 100); // 绘制圆角矩形
    painter.end();

    int row = music_table->count();

    // music_Table->setRowHeight(row,200);
    //  设置每列的值;
    //  设置第一列（序号）
    QString rowNumber = (row + 1 >= 10) ? QString::number(row + 1) : "0" + QString::number(row + 1);

    QString duration = QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0'));
    // 设置第二列（标题）
    CustomListView *view = new CustomListView();
    view->tableWidget = this;
    view->number = row;
    view->url = music.url;
    QStandardItemModel *model = new QStandardItemModel(view);

    view->setModel(model);
    DStandardItem *item0 = new DStandardItem(rowNumber);
    DStandardItem *item11 = new DStandardItem();
    item11->setIcon(roundedPixmap.scaled(QSize(50, 50)));
    DStandardItem *item1 = new DStandardItem(music.title);

    DStandardItem *item2 = new DStandardItem(music.album);
    DStandardItem *item3 = new DStandardItem(duration);
    view->setBackgroundType(DStyledItemDelegate::BackgroundType::NoBackground);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setIconSize(QSize(50, 50));
    view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // view->setViewMode(QListView::IconMode);
    view->setOrientation(QListView::LeftToRight, false);
    view->setResizeMode(QListView::Adjust);

    view->itemdelegate = new CustomItemDelegate(view);
    view->setItemDelegate(view->itemdelegate);
    DViewItemAction *act = new DViewItemAction;

    act->setText(music.artist);
    act->setFontSize(DFontSizeManager::T8);
    act->setTextColorRole(DPalette::TextTitle);
    act->setParent(view);

    item1->setTextAlignment(Qt::AlignTop | Qt::AlignLeft);
    item1->setTextActionList({act});
    item0->setTextAlignment(Qt::AlignCenter);
    item11->setTextAlignment(Qt::AlignCenter);
    item1->setTextAlignment(Qt::AlignCenter);
    item2->setTextAlignment(Qt::AlignCenter);
    item3->setTextAlignment(Qt::AlignCenter);
    item0->setSizeHint(QSize(40, 70));
    item11->setSizeHint(QSize(90, 70));
    item1->setSizeHint(QSize(150, 70));
    item2->setSizeHint(QSize(300, 70));
    item3->setSizeHint(QSize(100, 70));
    item0->setEditable(false);
    item11->setEditable(false);
    item1->setEditable(false);
    item2->setEditable(false);
    item3->setEditable(false);
    view->addItem(rowNumber);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    model->appendRow(item0);
    model->appendRow(item11);
    model->appendRow(item1);
    model->appendRow(item2);
    model->appendRow(item3);

    QListWidgetItem *item02 = new QListWidgetItem(music_table);

    item02->setSizeHint(QSize(70, 70));

    music_table->setItemWidget(item02, view);
    music_table->addItem(item02);

    QFileInfo fileInfo(music.url);
    QString path = fileInfo.absolutePath();  // 获取目录路径
         if(!dirToIndex.contains(path)){

         dirToIndex[path] = QList<int>();
         }
         dirToIndex[path].append(row);
}

void MusicTable::onBtPlayAll()
{

    playFromListView(0);
}
QString MusicTable::getUrlFromListView(int index)
{
    QListWidgetItem *item = music_table->item(index);
    if (item)
    {

        QWidget *widget = music_table->itemWidget(item);
        if (widget)
        {

            CustomListView *listView = qobject_cast<CustomListView *>(widget);
            return listView->url;
        }
    }
    qDebug() << "Can't get information from CustomListView[0]";
    return QString();
}

void MusicTable::playFromListView(int index)
{
    QListWidgetItem *item = music_table->item(index);
    if (item)
    {

        QWidget *widget = music_table->itemWidget(item);
        if (widget)
        {

            CustomListView *listView = qobject_cast<CustomListView *>(widget);

            listView->play();

            music_table->setCurrentRow(index);
            return;
        }
    }
    qDebug() << "Can't get information from CustomListView[0]";
}

void CustomListView::mouseDoubleClickEvent(QMouseEvent *event)
{

    this->play();
}
void CustomListView::play()
{
    MusicPlayer::instance().play(url);
}
void MusicTable::LoadStyleSheet()
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
    for (int i = 0; i < music_table->count(); ++i) {
        QListWidgetItem *item = music_table->item(i);

        // 获取与 QListWidgetItem 关联的 widget（即 CustomListView）
        CustomListView* customListView = qobject_cast<CustomListView*>(music_table->itemWidget(item));

        // 确保 customListView 不为空
        if (customListView) {
            customListView->itemdelegate->factor = ((width - 900) * 5 / 18);
            // qDebug() << "factor:" << customListView->itemdelegate->factor;
        } else {
            qWarning() << "Failed to get CustomListView from itemWidget.";
        }
    }
}

void MusicTable::onSearchTextChange(QString text)
{
    for (int i = 0; i < music_table->count(); ++i)
    {
        bool matchFound = false;
        QListWidgetItem *item = music_table->item(i);
        if (item)
        {
            QWidget *widget = music_table->itemWidget(item);
            if (widget)
            {
                QListView *listView = qobject_cast<QListView *>(widget);
                if (listView)
                {
                    QModelIndex index = listView->model()->index(3, 0);
                    QString text1 = listView->model()->data(index).toString();

                    if (text1.contains(text, Qt::CaseInsensitive))
                    {
                        matchFound = true;
                    }
                }
            }
        }
        music_table->setRowHidden(i, !matchFound);
    }
}
void MusicTable::resetMusicTable()
{        
        m_loaded=0;
        setMusicCount(DataBase::instance()->countLocallist);


        if (windowsWidth != 0) {
            QMetaObject::invokeMethod(this, [this]() {
                onResetWindowSize(windowsWidth);
            }, Qt::QueuedConnection);
        }
  }
void MusicTable::resetVideoTable()
{
    clearVideoTable();
    loadVideoTable();
}
void HistoryTable::mouseDoubleClickEvent(QMouseEvent *event)
{

    this->play();
}
void HistoryTable::play()
{
    MusicPlayer::instance().play(url);
}
