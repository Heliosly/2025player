#include"musicplayer.h"
#include"metadata.h"
#include"settingsmanager.h"
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


    readHistoryList();

    connect(qApp, &QCoreApplication::aboutToQuit, this,&MusicPlayer::onAppAboutToQuit);

}
MusicPlayer::~MusicPlayer(){
//TODO
}
void MusicPlayer::play(const QString& url){
    if(QFile::exists(url)){
        if(state()==QtAV::AVPlayer::PlayingState)
        {
         player->stop();
        filePath=url;
        player->play(url);
    }
        else{
filePath=url;
     player->play(url);


    }

        emit mediaSetted(url);
    }
    else{
        DataBase::instance()->deleteByUrl(QStringList{url},locallist);

        DataBase::instance()->deleteByUrl(QStringList{url},historylist);

        DataBase::instance()->deleteByUrl(QStringList{url},"tags_table");
    }

}
///如果数据库没有的话，从本地读元信息传进数据库
void MusicPlayer::initMusicByDir(const QString &dir) {


    QDir localdir(dir);
    QStringList notExtUrl;
    if (localdir.exists()) {
        QStringList filters;
        filters << "*.mp3" << "*.wav" << "*.flac";
        localdir.setNameFilters(filters);
        QFileInfoList files = localdir.entryInfoList();

        for (const QFileInfo& fileInfo : files) {

            // qDebug()<<fileInfo;
            if(!fileInfo.exists()){
                notExtUrl.append(fileInfo.absoluteFilePath());
                              continue;
            }
            QString filePath = fileInfo.absoluteFilePath();
            if(isUrlInDatabase(DataBase::instance(),filePath,locallist)){
                continue;
            }

            // qDebug() << "Processing file: " << filePath;
            TagLib::FileRef f(filePath.toStdString().c_str());
            if (!f.isNull() && f.tag()) {


                 loadLocalMusic(filePath,dir);



            }
        }


//TODO
    }
    else{
         }

}
///向数据库中加入某一路径
void MusicPlayer::addPathToDB(const QString &dir){
    
    QDir localdir(dir);
    if (localdir.exists()) {
        QStringList filters;
        filters << "*.mp3" << "*.wav" << "*.flac";
        localdir.setNameFilters(filters);
        QFileInfoList files = localdir.entryInfoList();

        for (const QFileInfo& fileInfo : files) {

            // qDebug()<<fileInfo;
            QString filePath = fileInfo.absoluteFilePath();
            
            if (isUrlInDatabase(DataBase::instance(), filePath, locallist)) {
           
                continue;
            }

            // qDebug() << "Processing file: " << filePath;
            TagLib::FileRef f(filePath.toStdString().c_str());
            if (!f.isNull() && f.tag()) {


              insertItemToMusicTableFromNewDir( filePath,dir);
                
               





            }
        }


    }
    SettingsManager::instance()->isAdding=0;

}

// ///从数据库读元信息
// void MusicPlayer::readMusicList( const QString &playListName){

//    auto db = DataBase::instance();
//     QSqlQuery sql_query(db->db);
//     QString sqlStatement;
//     QByteArray byteArray;
//     QString url;
//     QString title;
//     QString artist;
//     QString album;
//     QString duration;
//     QPixmap poster;
//     sqlStatement = QString("select url,title,artist,album,duration,poster from %1;").arg(playListName);
//     if (!sql_query.exec(sqlStatement)) {
//         qDebug() << "无法更新歌单信息：" << playListName << sql_query.lastError();
//         return;
//     }
//     while(sql_query.next()) {

//         url = sql_query.value(0).toString();
//         title = sql_query.value(1).toString();
//         artist = sql_query.value(2).toString();
//         album = sql_query.value(3).toString();

//         duration = sql_query.value(4).toString();
//         byteArray = sql_query.value(5).toByteArray();


//         poster.loadFromData(byteArray);

//         if (poster.isNull()) {
//             poster.load(":/icon/cd.png");
//         }

//         if (url.isNull() || !QFile::exists(url)) {  // Check if URL is null or file doesn't exist
//             continue;
//         }


//     }




// }


///判断数据库内是否存在对应url
bool MusicPlayer::isUrlInDatabase(DataBase *db, const QString &url, const QString &playListName){
    QSqlQuery sql_query(db->db);
    QString sqlStatement = QString("SELECT COUNT(*) FROM %1 WHERE filepath = :url;").arg(playListName);

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
/// @brief 从新目录直接插入至musictable
void MusicPlayer::insertItemToMusicTableFromNewDir(const QString &url, const QString &dir) {

    QFileInfo fileInfo(url);
    QString fileExtension = fileInfo.completeSuffix();

    QString title;
    QString artist;
    QString album;

    int duration=-1;
    QPixmap covpix = QPixmap();

    if (fileExtension == "m4a") {
        // 处理 m4a 文件
        TagLib::MP4::File *mp4File = new TagLib::MP4::File(QFile::encodeName(url).constData());
        if (!mp4File->isOpen()) {
            covpix.load(":/asset/image/unknowcovpix.jpg");
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

        covpix.loadFromData(reinterpret_cast<const uchar*>(albumArt.data().data()), albumArt.data().size());
        if (covpix.isNull()) {
            covpix.load("::/asset/image/unknowcovpix.jpg");
        }
    } else if (fileExtension == "mp3") {
        // 处理 mp3 文件
        TagLib::MPEG::File *mpegFile = new TagLib::MPEG::File(QFile::encodeName(url).constData());
        if (!mpegFile->isOpen()) {
            covpix.load(":/asset/image/unknowcovpix.jpg");
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

                    if (!covpix.loadFromData(coverData)) {
                        covpix.load(":/asset/image/unknowcovpix.jpg");
                    }
                }
            } else {
                covpix.load(":/asset/image/unknowcovpix.jpg");
            }
        }
    }

    if (title==""){
        QFileInfo fileInfo(url)
                        ;
        title = fileInfo.baseName()
                        ;    }
    if (duration==-1){
        duration=0;
    }
      QMap<QString, QString> metaDataMap;
    metaDataMap.insert("dirPath", dir);
    metaDataMap.insert("filePath", url);
    metaDataMap.insert("title", title);
    metaDataMap.insert("artist", artist);
    metaDataMap.insert("album", album);
    metaDataMap.insert("duration", QString::number(duration));

    if (!DataBase::instance()->saveMetaData(metaDataMap, covpix, 0)) {
        qDebug() << "Error: Failed to save metadata to database";
        return;
    }

    // Create a Meta object using the metaDataMap
    MetaData meta;
    meta.url = metaDataMap.value("filepath");
    meta.title = metaDataMap.value("title");
    meta.artist = metaDataMap.value("artist");
    meta.album = metaDataMap.value("album");
    meta.duration = metaDataMap.value("duration").toInt();
    meta.covpix = covpix;


}
///将本地元信息传至数据库
void MusicPlayer::loadLocalMusic(const QString &url,const QString &dir) {

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
            covpix.load(":/asset/image/unknowcovpix.jpg");
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
            covpix.load(":/asset/image/unknowcovpix.jpg");
            // qDebug() << "读取音乐封面信息失败";
        }
    } else if (fileExtension == "mp3") {
        // 处理 mp3 文件
        TagLib::MPEG::File *mpegFile = new TagLib::MPEG::File(QFile::encodeName(url).constData());
        if (!mpegFile->isOpen()) {
            covpix.load(":/asset/image/unknowcovpix.jpg");
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
                covpix.load(":/asset/image/unknowcovpix.jpg");
                qDebug() << "读取音乐封面信息失败";
            }

        }
    }
    if (title==""){
        QFileInfo fileInfo(url)
                        ;
        title = fileInfo.baseName()
                        ;    }
    QMap<QString, QString> metaDataMap;
    metaDataMap.insert("dirPath", dir);
    metaDataMap.insert("filePath", url);
    metaDataMap.insert("title", title);
    metaDataMap.insert("artist", artist);
    metaDataMap.insert("album", album);
    metaDataMap.insert("duration", QString::number(duration));
   if( !DataBase::instance()->saveMetaData(metaDataMap,covpix,0))
            {
                qDebug() << "Error: Failed to save metadata to database";
                return;
            }
   else{
       qDebug()<<"save metadate ok";


      }

}

void MusicPlayer::uninstallPath(const QString &filePath) {
    if (!QDir(filePath).exists()) {
        qDebug() << "Error: Path does not exist";
        return;
    }

    QtConcurrent::run([this, filePath]() {
        // Create a file list
        QDirIterator iterator(filePath, musicExtensions, QDir::Files);

        // Create a list of absolute file paths
        QStringList filePaths;
        while (iterator.hasNext()) {
            filePaths.append(QDir(filePath).absoluteFilePath(iterator.next()));
        }
        
        if (!DataBase::instance()->deleteByDirPath(filePath, locallist)) {
            qDebug() << "Error: Failed to delete from database in locallist";
        }
        else if (!DataBase::instance()->deleteByDirPath(filePath, historylist)) {
            qDebug() << "Error: Failed to delete from database in historylist";
        }
        else {
            qDebug() << "delete MMetalist";
        DataBase::instance()->reSetListCount();
                emit mediaListSub(filePath);
        }

//        DataBase::instance()->cleanupThreadDatabase();
    });
}

void MusicPlayer::installPath(const QString & filePath){

    QtConcurrent::run([this, filePath]() {
        addPathToDB(filePath);
        DataBase::instance()->reSetListCount();

//        DataBase::instance()->cleanupThreadDatabase();
            emit mediaListAdd();
    });
}
void MusicPlayer::pause(){
    player->pause();
}

void MusicPlayer::play(){
    player->play();

}
void MusicPlayer::stop(){
    player->stop();
}
QtAV::AVPlayer::State MusicPlayer::state(){
    return player->state();
}
void MusicPlayer::setVolume(int volume){
    VideoPlayer::instance()->setVolume(volume);
}
void MusicPlayer::setPosition(qint64 position){
    player->setPosition(position);
}
qint64 MusicPlayer::duration(){
    player->duration();
}
void MusicPlayer::setSpeed(qreal speed){
     VideoPlayer::instance()->setSpeed(speed);

}
void MusicPlayer::onMediaChange(QtAV::MediaStatus state){
    if(!enable){
        return ;
    }
    if (state==QtAV::MediaStatus::BufferedMedia)
    {
        auto duration = player->duration();
            QString title = QFileInfo(filePath).completeBaseName();
         history.addToHistory(
                    HistoryMData(filePath,
                                 title, duration/1000));
        emit historyListChange(history.history.first());
    }


    emit mediaStatusChanged(state);
}

void MusicPlayer::readHistoryList() {
    QSqlQuery query(DataBase::instance()->db);
        QString sqlStatement = QString("SELECT filePath, title, duration FROM historylist;");

        if (!query.exec(sqlStatement)) {
            qDebug() << "Unable to read history list: " << query.lastError();
            return;
        }

        while (query.next()) {
            QString url      = query.value(0).toString();
            QString title    = query.value(1).toString();
            int     duration = query.value(2).toInt();

            history.history.append(HistoryMData(url, title, duration));
        }
}
void MusicPlayer::onAppAboutToQuit(){

    DataBase::instance()->clearTable(historylist);
    DataBase::instance()->saveHistoryData(history.history);
}

void MusicPlayer::initConnect(){
    connect(player, &QtAV::AVPlayer::stateChanged, this, &MusicPlayer::onStateChanged);
    connect(player,&QtAV::AVPlayer::mediaStatusChanged,this,&MusicPlayer::onMediaChange);
    connect(player,&QtAV::AVPlayer::started,this,&MusicPlayer::onStarted);

    connect(player,&QtAV::AVPlayer::stopped,this,&MusicPlayer::onStopped);

}

void HistoryList::sendHistoryRemove(){
    ///bug
 emit historyListRemove(0);
}
void MusicPlayer::onStateChanged(QtAV::AVPlayer::State state) {
    if (!enable)
        return;

    emit stateChanged(state);  // 如果你之前就是发这个信号
}

//void MusicPlayer::onMediaStatusChanged(QtAV::MediaStatus status) {
//    if (!enable)
//        return;

//    emit mediaStatusChanged(status);
//}
void MusicPlayer::onStarted(){


    emit started();
}
void MusicPlayer::onStopped(){

}
