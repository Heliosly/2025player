# player


# Dependence

```
mkdir QTAV
cd QTAV
git clone https://github.com/wang-bin/QtAV.git
cd QtAV && git submodule update --init
sudo apt-get install libopenal-dev libpulse-dev libva-dev libxv-dev libass-dev libegl1-mesa-dev ffmpeg libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev 
sudo apt-get install  libavutil-dev libswresample-dev libswscale-dev 
mkdir qtavbuild
cd qtavbuild
qmake ../QtAV.pro
make -j$(nproc)
sudo make install

git clone https://github.com/Skycoder42/QHotkey.git
 cd QHotkey
 cmake -B build -S . -DQT_DEFAULT_MAJOR_VERSION=5
 cmake --build build
sudo cmake --install build
```
