# RemoteDisplay

RemoteDisplay is a RDP client library built upon [Qt](http://qt.digia.com/) and [FreeRDP](http://www.freerdp.com/).

The library consists of a single widget [RemoteDisplayWidget](src/remotedisplaywidget.h)
which renders the remote display inside it. The widgets sends any mouse movement
or keyboard button presses to the remote host as well as playback any audio the
remote host might playback. 

## Usage example
```C++
#include <QApplication>
#include <QString>
#include <remotedisplaywidget.h>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

	// configuration options
	quint16 width  = 800;
	quint16 height = 600;
	QString host   = "1.2.3.4";
	quint16 port   = 3389;

	// create and show the RemoteDisplay widget
    RemoteDisplayWidget w;
    w.resize(width, height);
    w.setDesktopSize(width, height);
    w.connectToHost(host, port);
    w.show();

	// exit program if disconnected
    QObject::connect(&w, SIGNAL(disconnected()), &a, SLOT(quit()));

    return a.exec();
}
```
For full example, see the [example subdirectory](example).

## Building and installing
In order to use this library you need to build it first. Here's instructions
for Windows and Linux.

### Linux
Prerequisites:
* CMake 2.8.10 or newer
* FreeRDP from https://github.com/sailfish-sdk/FreeRDP, branch 'build-fixes'
* Qt 4.8 or newer (5.x not yet supported)
* gcc 4.5 or newer (for some c++11 features)

#### Building FreeRDP
First make sure that you have cmake version 2.8.10 or newer installed, this is
required for the FreeRDP to generate cmake config files for finding its libraries.
If you don't have high enough version, then you need to built it yourself,
refer to [install instructions at cmake's site](http://www.cmake.org/cmake/help/install.html).

To build the FreeRDP, clone its repo first:
```
$ git clone https://github.com/sailfish-sdk/FreeRDP.git
```
Next, cd to its directory and configure the project with cmake:
```
$ cmake -DCMAKE_BUILD_TYPE=Release -DWITH_SERVER:BOOL=OFF -DCHANNEL_AUDIN:BOOL=OFF -DCHANNEL_CLIPRDR:BOOL=OFF -DCHANNEL_DISP:BOOL=OFF -DCHANNEL_DRDYNVC:BOOL=OFF -DCHANNEL_DRIVE:BOOL=OFF -DCHANNEL_ECHO:BOOL=OFF -DCHANNEL_PRINTER:BOOL=OFF -DCHANNEL_RAIL:BOOL=OFF -DCHANNEL_RDPEI:BOOL=OFF -DCHANNEL_RDPGFX:BOOL=OFF -DCHANNEL_TSMF:BOOL=OFF .
```
There is a lot of features that RemoteDisplay doesn't use currently and so they
can be left off to produce slightly leaner binaries.
Finally build and install FreeRDP:
```
$ make
$ sudo make install
```

#### Building RemoteDisplay
First clone the repo:
```
$ git clone https://github.com/sailfish-sdk/RemoteDisplay.git
```
Next, cd to its directory and configure, build and install it:
```
$ cmake -DCMAKE_BUILD_TYPE=Release .
$ make
$ sudo make install
```
Note, that if cmake complains of missing qtmultimedia component then sound playback
might not work, but it will not prevent compiling and using of the library.

Verify that the RemoteDisplay works by starting your RDP server and running
RemoteDisplay's example:
```
$ RemoteDisplayExample 1.2.3.4 3389 800 600
```
Run RemoteDisplayExample without parameters for explanation what the parameters do.

### Windows
TBD

## License
Licensed under the ([MIT license](LICENSE)).
