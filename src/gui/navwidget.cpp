#include "navwidget.h"
#include<DLabel>
#include<QVBoxLayout>
#include<QFile>
///左侧导航栏
NavWidget::NavWidget()
{

    QFont chineseFont("Noto Sans CJK SC");

    ListView1=new DListView(this);
    this->stackUnder(this);
    this->setObjectName("navigate_frame");
    ListView1->setObjectName("navigate_list");
    ListView1->setIconSize(QSize(50,50));

    ListView1->setSpacing(5);
    auto VLayoutLeft = new QVBoxLayout(this);
    VLayoutLeft->addSpacing(15);

    DLabel *label1 = new DLabel(this);
        LabelItemDelegate * labelDelegate = new LabelItemDelegate();
    ListView1->setItemDelegate(labelDelegate);



    label1->setAlignment(Qt::AlignLeft);
               label1->setText(" 播放器");
    label1->setFont(chineseFont);
    QFont font = label1->font();
    font.setPointSize(14);
    label1->setFont(font);
    QStandardItemModel *model = new QStandardItemModel();
    QStandardItem *separatorItem = new QStandardItem();
    separatorItem->setData("separator", Qt::UserRole);  // 设置自定义数据标识
    separatorItem->setSizeHint(QSize(0,70));

    auto AddItems = [&model](QIcon icon, QString name){
        DStandardItem *item = new DStandardItem(icon, name);
        item->setEditable(false);
        model->appendRow(item);
        item->setSizeHint(QSize(0,60));
        return item;

    };
    AddItems( QIcon(":/asset/image/music2.PNG"),"Recommend");

    model->appendRow(separatorItem);
    AddItems( QIcon(":/asset/image/music2.PNG"),"Music");
    AddItems(QIcon(":asset/image/video2.PNG"),"Video");
    AddItems(QIcon(":asset/image/video2.PNG"),"History");
   // ListView1->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ListView1->setModel(model);
    VLayoutLeft->addSpacing(5);
    VLayoutLeft->addWidget(label1);
    VLayoutLeft->addSpacing(10);
    VLayoutLeft->addWidget(ListView1);
    ListView1->setItemSpacing(10);


}
void NavWidget::LoadStyleSheet( QString url)
{
    QFile file(url);
    file.open(QIODevice::ReadOnly);

    if (file.isOpen())
    {
        QString style = this->styleSheet();
        style += QLatin1String(file.readAll());
        this->setStyleSheet(style);
        file.close();
    }
    else{
        qDebug()<<"navgate widget qss load failed";
    }
}


void LabelItemDelegate:: paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        if (index.data(Qt::UserRole).toString() == "separator") {
            painter->setPen(Qt::gray);

            QFont font = painter->font();  // 获取当前字体
            font.setFamily("Noto Sans CJK SC");  // 设置字体家族
            font.setPointSize(14);  // 设置字体大小
            painter->setFont(font);  // 应用字体到 painte

             painter->drawText(option.rect, Qt::AlignLeft | Qt::AlignVCenter, "媒体列表");
                } else {
//            painter->save();
//           QPainterPath path;
//               // 绘制选中背景（带圆角效果）
//               if (option.state & QStyle::State_Selected) {
//                   path.addRoundedRect(option.rect.adjusted(2, 2, -2, -2), 18, 18);  // 向内缩进 2px，圆角半径 18px
//                   painter->setRenderHint(QPainter::Antialiasing, true);  // 开启抗锯齿
//                   painter->setPen(Qt::NoPen);
//                   painter->setBrush(option.palette.highlight());
//                   painter->drawPath(path);
//               } else if (option.state & QStyle::State_MouseOver) {
//                   // 悬停时的背景（灰色）
//                   path.addRoundedRect(option.rect.adjusted(2, 2, -2, -2), 18, 18);
//                   painter->setRenderHint(QPainter::Antialiasing, true);
//                   painter->setPen(Qt::NoPen);
//                   painter->setBrush(QColor("#f0f0f0"));
//                   painter->drawPath(path);
//               } else {
//                   // 默认背景色（灰色）
//                   path.addRoundedRect(option.rect.adjusted(2, 2, -2, -2), 18, 18);
//                   painter->setRenderHint(QPainter::Antialiasing, true);
//                   painter->setPen(Qt::NoPen);
//                   painter->setBrush(QColor("#f7f7f7"));
//                   painuuter->drawPath(path);
//               }


//                             painter->restore();
            QStyledItemDelegate::paint(painter, option, index);
        }
    }
