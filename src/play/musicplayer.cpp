#include"musicplayer.h"
#include"metadata.h"
#include <fileref.h>
#include <tag.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <mpegfile.h>
#include <mp4file.h>
#include<QDebug>
#include <QtConcurrent>
#include <fileref.h>
#include <tag.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>
#include <mpegfile.h>
#include <mp4file.h>
MusicPlayer::MusicPlayer()
{


    player=new QMediaPlayer;
    //QtConcurrent::run([this]() {  });
    readList(DataBase::instance(),"locallist");
    connect(player, &QMediaPlayer::mediaStatusChanged, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia) {
            player->play();
        }
    });


}
void MusicPlayer::play(QString url){
    player->setMedia(QUrl::fromLocalFile(url));
   }

///如果数据库没有的话，从本地读元信息传进数据库
void MusicPlayer::initMusicByFilePath(const QString &mediaPath) {


    QDir localdir(mediaPath);
    if (localdir.exists()) {
        QStringList filters;
        filters << "*.mp3" << "*.wav" << "*.flac";
        localdir.setNameFilters(filters);
        QFileInfoList files = localdir.entryInfoList();

        for (const QFileInfo& fileInfo : files) {

            // qDebug()<<fileInfo;
            QString filePath = fileInfo.absoluteFilePath();
            if(isUrlInDatabase(DataBase::instance(),filePath,"locallist")){
                continue;
            }

            // qDebug() << "Processing file: " << filePath;
            TagLib::FileRef f(filePath.toStdString().c_str());
            if (!f.isNull() && f.tag()) {


                loadLocalMusic(filePath,"locallist");

            }
        }


       }
}
///从数据库读元信息
void MusicPlayer::readList(DataBase*db, const QString &playListName){

    QSqlQuery sql_query(db->db);
    QString sqlStatement;
    QByteArray byteArray;
    QString url;
    QString title;
    QString artist;
    QString album;
    QString duration;
    QPixmap poster;
    sqlStatement = QString("select url,title,artist,album,duration,poster from %1;").arg(playListName);
    if (!sql_query.exec(sqlStatement)) {
        qDebug() << "无法更新歌单信息：" << playListName << sql_query.lastError();
        return;
    }
    while(sql_query.next()) {

        url = sql_query.value(0).toString();
        title = sql_query.value(1).toString();
        artist = sql_query.value(2).toString();
        album = sql_query.value(3).toString();

        duration = sql_query.value(4).toString();
        byteArray = sql_query.value(5).toByteArray();


        poster.loadFromData(byteArray);

        if (poster.isNull()) {
            poster.load(":/icon/cd.png");
        }

        if (url.isNull()) {//过滤不存在的歌曲
            continue;
        }
        MMetalist.append(MetaData(url,title,artist,album,duration.toInt(),poster));

    }




}


///判断数据库内是否存在对应url
bool MusicPlayer::isUrlInDatabase(DataBase *db, const QString &url, const QString &playListName){
    QSqlQuery sql_query(db->db);
    QString sqlStatement = QString("SELECT COUNT(*) FROM %1 WHERE url = :url;").arg(playListName);

    sql_query.prepare(sqlStatement);
    sql_query.bindValue(":url", url);

    if (!sql_query.exec()) {
        qDebug() << "查询执行失败:" << sql_query.lastError();
        return false;  // 如果查询失败，返回 false
    }

    if (sql_query.next()) {
        int count = sql_query.value(0).toInt();
        if (count > 0) {

            return true;
        } else {

            return false;
        }
    }
    return false;
}
///将本地元信息传至数据库
void MusicPlayer::loadLocalMusic(const QString &url,const QString &playListName) {

    QFileInfo fileInfo(url);
    QString fileExtension = fileInfo.completeSuffix();

    QString title;
    QString artist;
    QString album;

    int duration;
    QPixmap covpix = QPixmap();


    if (fileExtension == "m4a") {
        // 处理 m4a 文件
        TagLib::MP4::File *mp4File = new TagLib::MP4::File(QFile::encodeName(url).constData());
        if (!mp4File->isOpen()) {
            covpix.load(":/asset/image/logo.png");
            return;
        }

        title = QString(mp4File->tag()->title().toCString(true));
        artist = QString(mp4File->tag()->artist().toCString(true));
        album = QString(mp4File->tag()->album().toCString(true));

        TagLib::MP4::ItemListMap itemListMap = mp4File->tag()->itemListMap();
        TagLib::MP4::Item albumArtItem = itemListMap["covr"];
        TagLib::MP4::CoverArtList albumArtList = albumArtItem.toCoverArtList();
        TagLib::MP4::CoverArt albumArt = albumArtList.front();
        TagLib::AudioProperties *audioProperties = mp4File->audioProperties();
        duration = audioProperties->lengthInSeconds(); // 获取音频时长，单位：秒


        //covpix.loadFromData((const uchar *)albumArt.data().data(), albumArt.data().size());
        covpix.loadFromData(reinterpret_cast<const uchar*>(albumArt.data().data()), albumArt.data().size());
        if (!covpix.isNull()) {
            // qDebug() << "读取M4A封面信息成功";
        } else {
            covpix.load(":/asset/image/logo.png");
            // qDebug() << "读取音乐封面信息失败";
        }
    } else if (fileExtension == "mp3") {
        // 处理 mp3 文件
        TagLib::MPEG::File *mpegFile = new TagLib::MPEG::File(QFile::encodeName(url).constData());
        if (!mpegFile->isOpen()) {
            covpix.load(":/asset/image/logo.png");
            return;
        } else {
            title = QString(mpegFile->tag()->title().toCString(true));
            artist = QString(mpegFile->tag()->artist().toCString(true));
            album = QString(mpegFile->tag()->album().toCString(true));
            TagLib::AudioProperties *audioProperties = mpegFile->audioProperties();
            duration = audioProperties->lengthInSeconds();
            auto tag = mpegFile->ID3v2Tag(false);
            auto list = tag->frameListMap()["APIC"];
            if (!list.isEmpty()) {
                auto frame = list.front();
                auto pic = reinterpret_cast<TagLib::ID3v2::AttachedPictureFrame *>(frame);

                if (pic && !pic->picture().isNull()) {
                    QByteArray coverData(reinterpret_cast<const char *>(pic->picture().data()), pic->picture().size());

                    if (covpix.loadFromData(coverData)) {
                        qDebug() << "读取MP3封面信息成功";
                    }
                }
            } else {
                covpix.load(":/asset/image/logo.png");
                qDebug() << "读取音乐封面信息失败";
            }

        }
    }
    QMap<QString, QString> metaDataMap;
    metaDataMap.insert("url", url);
    metaDataMap.insert("title", title);
    metaDataMap.insert("artist", artist);
    metaDataMap.insert("album", album);
    metaDataMap.insert("duration", QString::number(duration));
    DataBase::instance()->saveMetaData(metaDataMap,playListName,covpix,0);
}

void MusicPlayer::uninstallPath(const QString &filePath) {
    if (!QDir(filePath).exists()) {
      qDebug() << "Error: Path does not exist";
      return;
    }
  
    // Create a file list
    QDirIterator iterator(filePath, musicExtensions, QDir::Files);
  
    // Create a list of absolute file paths
    QStringList filePaths;
    while (iterator.hasNext()) {
      filePaths.append(QDir(filePath).absoluteFilePath(iterator.next()));
    }
    if (!DataBase::instance()->deleteByUrl(filePaths,"locallist")){
        qDebug() << "Error: Failed to delete from database";
    }
    else{

        //异步
        //todo
        MMetalist.clear();
        qDebug()<<"delete MMetalist";

        readList(DataBase::instance(),"locallist");
        emit playListChanged();
    }
  
  }

void MusicPlayer::installPath(const QString & filePath){

      initMusicByFilePath(filePath);

        readList(DataBase::instance(),"locallist");
        emit playListChanged();
     }
