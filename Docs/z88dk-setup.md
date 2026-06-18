# Setting up z88dk sdk
[Back To Doc Home](README.md)

# Setting up Dev Env
Here is what I did to set up the z88dk dev tools on my Ubuntu Linux PC:
```
sudo apt update
sudo apt install -y \
    git build-essential \
    libboost-all-dev \
    libxml2-dev \
    bison flex \
    libgmp-dev \
    libmpfr-dev \
    texinfo \
    git \
    build-essential \
    cmake

cd ~
git clone --recursive git@github.com:z88dk/z88dk.git
cd z88dk
./build.sh

export Z88DK_HOME=$HOME/z88dk
export PATH=$PATH:$Z88DK_HOME/bin
export ZCCCFG=$Z88DK_HOME/lib/config

which zcc
zcc +cpc -v
```
`which zcc` should show where the zcc executable is.

You will need to add the three `export` commands in to your `.bashrc` to make the z88dk tools available in new terminals and after reboot.

# Setting up iDSK to write Disc Images
Here is what I did to set up iDSK so I could create .dsk images to load in to the emulator:
```
cd ~
git clone https://github.com/cpcsdk/iDSK.git
cd iDSK
mkdir build
cd build
cmake ..
make
cp iDSK $Z88DK_HOME/bin
```
[Back To Doc Home](README.md)
