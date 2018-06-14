# nn-i2c-kernel-driver
## WSL
```bash

nproc
vim .bashrc
ls
repo
ccache -M 25G
ls
git config --global user.name "Erik Rosengren"
git config --global user.email e@roseng.ren
sudo apt-get unzip
sudo apt-get install unzip
cp /mnt/c/Users/erikr/Downloads/8009W_Open-Q_2100_Android_BSP-N-1.2 .
cp /mnt/c/Users/erikr/Downloads/8009W_Open-Q_2100_Android_BSP-N-1.2.zip .
unzip 8009W_Open-Q_2100_Android_BSP-N-1.2.zip
cp -ar 8009W_Open-Q_2100_Android_BSP-1.2/Source_Package workdir/
ls
cd workdir/
ls
cd ..
mv workdir Source_Package
ls
mkdir workdir
mv Source_Package workdir/
ls
cd workdir/o
cd workdir/Source_Package/
ls
cat getSource_and_build.sh
sudo apt-get install openjdk-8-jdk
sudo apt-get update
sudo apt-get install openjdk-8-jdk
sudo apt-get install git ccache automake lzop bison gperf build-essential zip curl zlib1g-dev zlib1g-dev:i386 g++-multilib python-networkx libxml2-utils bzip2 libbz2-dev libbz2-1.0 libghc-bzlib-dev squashfs-tools pngcrush schedtool dpkg-dev liblz4-tool make optipng maven
sudo apt full-upgrade -y
sudo apt install -y android-sdk-platform-tools bc build-essential ccache curl g++-multilib gcc-multilib git gnupg gperf imagemagick lib32ncurses5-dev lib32readline-dev lib32z1-dev liblz4-tool libncurses5-dev libsdl1.2-dev libssl-dev libwxgtk3.0-dev libxml2 libxml2-utils lzop m4 openjdk-8-jdk pngcrush repo rsync schedtool squashfs-tools xsltproc zip zlib1g-dev
source ~/.bashrc
./getSource_and_build.sh
. build/envsetup.sh m
lunch msm8909w-userdebug
make bison && make ijar
cat ~/.bash_history
vim build/core/config.mk
vim build/core/dex_preopt_libart.mk
repo diff >> ~/wsl-changes.diff
cp ~/wsl-changes.diff /mnt/c/jobb/MrT/android/nn-i2c-kernel-driver/
```