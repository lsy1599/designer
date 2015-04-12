About LibreCAD
==============

[LibreCAD](http://www.librecad.org) is a 2D CAD drawing tool based on the community edition of QCAD (http://www.qcad.org).
LibreCAD has been re-structured, ported to new [Qt](http://qt-project.org) versions and works natively cross platform between OS X, Windows and Linux.
See [LibreCAD Home](http://www.librecad.org).

User Manual and wiki
--------------------

We are in the process of building a user manual and wiki:

[wiki.librecad.org](http://wiki.librecad.org/index.php/Main_Page)

OS X Users
----------

If you use [MacPorts](https://www.macports.org/), see below. If you use brew, or neither one, use this section.

Install Homebrew from [http://brew.sh/](http://brew.sh/).

```bash
gcc --version # you'll need gcc 4.4 or newer. If yours is older:
```

```bash
brew tap homebrew/versions
```

```bash
brew options gcc48
```

```bash
brew install [flags] gcc48
```

```bash
mkdir ~/bin
```

```bash
cd ~/bin
```

```bash
ln -s /usr/local/bin/gcc-4.8 gcc
```

```bash
ln -s /usr/local/bin/g++-4.8 g++
```

```bash
ln -s /usr/local/bin/gcc-ar-4.8 gcc-ar
```

```bash
ln -s /usr/local/bin/gcc-nm-4.8 gcc-nm
```

```bash
ln -s /usr/local/bin/gcc-ranlib-4.8 gcc-ranlib
```bash
source ~/.bashrc
```bash
gcc --version # make sure it's 4.8, as installed and configured in the previous step. if it's not, ~/bin might not be on your path
```

```bash
brew install boost qt
```

Unzip or checkout a version of LibreCAD into a directory.
```bash
cd LibreCAD
./scripts/build-osx.sh
```

This creates an executable "LibreCAD.app/Contents/MacOS/LibreCAD" and package "LibreCAD.dmg".

OS X with MacPorts Users
------------------------

install MacPorts from [http://www.macports.org/](http://www.macports.org/)

You can install LibreCad using MacPorts by:
`$ sudo port install librecad`

You can build LibreCAD manually by following steps:

Install QT and a new gcc, which should have a version 4.4 or later.

Install a version of Qt and boost, for example
`$ sudo port install gcc46 qt4-creator-mac qt4-mac boost`

Select the right compiler, as LibreCAD doesn't build with the default llvm-gcc42,
`$ sudo port select --set gcc mp-gcc46`

Unzip or checkout a version of LibreCAD into a directory.

```
cd LibreCAD
./scripts/build-osx.sh
```

This creates an executable "LibreCAD.app/Contents/MacOS/LibreCAD" and package "LibreCAD.dmg".

Users of Ubuntu/Debian and derivatives
--------------------------------------

Make sure you have the Qt version 4 development packages installed by
running the following commands:

```
$ sudo apt-get install g++ gcc make git-core libqt4-dev qt4-qmake libqt4-help \
qt4-dev-tools libboost-all-dev libmuparser-dev libfreetype6-dev pkg-config
```

Alternatively, you make sure you have deb-src lines enabled in your sources.list file, and run,

```
$ sudo apt-get build-dep librecad
```

For SVN see also: 
http://www.librecad.org/2010/10/debian-64-bit-and-ubuntu-compile-how-to/

For git see also:
http://librecad.org/cms/home/from-source/linux.html

LibreCAD builds with both Qt4 and Qt5. Note that you will most likely need to run __qmake-qt4__ or __qmake-qt5__ instead of just __qmake__.

Users of Red Hat and similar distibutions
-----------------------------------------

Install Qt, Boost and muParser development packages for your respective distribution;
[EPEL](https://fedoraproject.org/wiki/EPEL) and similar repositories may come handy if
your base OS does not include the necessary packages.

As an example, for CentOS 6.4, after adding the EPEL repository,

```bash
yum groupinstall 'Desktop Platform Development' 'Development tools'
```

```bash
yum install qt-devel boost-devel muParser-devel
```

will install the necessary build dependencies.

Note that you will most likely need to run __qmake-qt4__ or __qmake-qt5__ instead of just __qmake__.

__scripts/build-osx.sh__ bash building scripts are supplied. See [OS/X by building scripts](http://wiki.librecad.org/index.php/LibreCAD_Installation_from_Source#By_the_building_script)

FreeBSD users
-------------

See scripts/build-freebsd.sh for the list of ports that need to be installed.

Use the script itself to build LibreCAD.

Windows Users
-------------

Building steps are also given at our wiki page:

[LibreCAD Installation from Source](http://wiki.librecad.org/index.php/LibreCAD_Installation_from_Source)

A sample build batch file is included as scripts/build-windows.bat. If successful, this building script generates a Windows installer file using NSIS(http://nsis.sourceforge.net/Main_Page). 

- Download a copy of Qt SDK,  5.4.1 for example from http://qt-project.org/downloads 

- Download boost, from https://sourceforge.net/projects/boost/files/boost/
- unzip into C:\boost\, for example C:\boost\1_53_0 (in this directory you will find boost root directory, INSTALL, index, Jamroot etc.. etc).

Start Qt Creator and load LibreCAD.pro, from the build menu select "Build All".

###Windows Building from Command Line###
__scritps/build-windows.bat__ batch file for automatic building is supplied. Read [Building Windows by command line](http://wiki.librecad.org/index.php/LibreCAD_Installation_from_Source#Building_Windows_by_command_line) for more details.

Generic Unix Users
------------------

Install necessary dependecy: Qt, boost, muParser (see [http://wiki.librecad.org](http://wiki.librecad.org/index.php/LibreCAD_Installation_from_Source) for details).

Unzip or checkout a version of LibreCAD into a directory LibreCAD.

```bash
cd LibreCAD
```

```bash
qmake -r librecad.pro
```

```bash
make
```

The executible is generated at __unix/librecad__
