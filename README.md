# player


# Dependence

```
mkdir QTAV
cd QTAV
git clone https://github.com/wang-bin/QtAV.git
cd QtAV && git submodule update --init
sudo apt-get install libopenal-dev libpulse-dev libva-dev libxv-dev libass-dev libegl1-mesa-dev ffmpeg libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev 
sudo apt install  libavutil-dev libswresample-dev libswscale-dev libav-tools
mkdir qtavbuild
cd qtavbuild
qmake ../QtAV.pro
make -j$(nproc)
sudo make install
```
