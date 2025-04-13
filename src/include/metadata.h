///封装音乐元信息的类
#ifndef METADATA_H
#define METADATA_H
#include<QWidget>
#include<QDir>
#include<QDebug>

#include<QFileInfo>
#include<QFileIconProvider>
class MetaData {
public:
    QString url;
    QString artist;
    QString title;
    QString album;
    QPixmap covpix;
    int duration;

    MetaData():
        artist(""), title(""), album(""), covpix(), duration(3) {}
    MetaData(const QString &url, const QString &title, const QString &artist, const QString &album, const int &duration, const QPixmap &poster)
            : url(url), title(title), artist(artist), album(album), duration(duration), covpix(poster) {}

private:
};
class HistoryMData{
public:

    QString url;
    QString title;
    int duration;
    bool isVideo=0;
    QIcon icon;
    HistoryMData():
       url(""), title(""),duration(3){}
    HistoryMData(const QString &url, const QString &title, const int &duration)
            : url(url), title(title), duration(duration) {

        // 提取文件扩展名
          QString suffix = QFileInfo(url).suffix().toLower();

          // 判断是否为常见视频格式
          static const QSet<QString> videoExtensions = {
              "mp4", "avi", "mkv", "mov", "flv", "wmv", "webm", "mpeg", "mpg"
          };
          isVideo = videoExtensions.contains(suffix);
          QFileInfo fileInfo(url);
          QFileIconProvider iconProvider;
         icon = iconProvider.icon(fileInfo); // 获取文件图标

          if (icon.isNull())
          {
              if(isVideo)
              icon = QIcon(":/asset/image/video2.PNG"); // 使用默认图标路径
              else{

              icon = QIcon(":/asset/image/music2.PNG"); // 使用默认图标路径
              }
          }
    }

    HistoryMData(const QMap <QString,QString> &map){
        url = map["url"];
        title = map["title"];
        duration = map["duration"].toInt();

        // 提取文件扩展名
        QString suffix = QFileInfo(url).suffix().toLower();

        // 判断是否为常见视频格式
        static const QSet<QString> videoExtensions = {
            "mp4", "avi", "mkv", "mov", "flv", "wmv", "webm", "mpeg", "mpg"
        };
        isVideo = videoExtensions.contains(suffix);
            QFileInfo fileInfo(url);
          QFileIconProvider iconProvider;
          QIcon icon = iconProvider.icon(fileInfo); // 获取文件图标

          if (icon.isNull())
          {
              if(isVideo)
              icon = QIcon(":/asset/image/video2.PNG"); // 使用默认图标路径
              else{

              icon = QIcon(":/asset/image/music2.PNG"); // 使用默认图标路径
              }
          }

    }
   bool operator==(const HistoryMData& other) {
          return       url == other.url&&
            title ==other.title&&
            duration==other.duration;
               }
};

#endif // METADATA_H
