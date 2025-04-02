///左侧的导航栏
#ifndef NAVWIDGET_H
#define NAVWIDGET_H

#include<DListView>
#include<DLabel>
#include<DFrame>
 DWIDGET_USE_NAMESPACE
 ///最左边导航栏
class NavWidget : public DFrame
{
    Q_OBJECT
public:
    NavWidget();
    DListView *ListView1;

    void LoadStyleSheet(QString url);
};
class LabelItemDelegate : public QStyledItemDelegate {
public:
    LabelItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override    ;
};
#endif // NAVWIDGET_H
