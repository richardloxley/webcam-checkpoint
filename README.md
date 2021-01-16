# webcam-checkpoint
Save and restore UVC parameters for USB webcams via the command line. The values are written to a text file as name=value pairs.

Tested on MacOS, should work on other UNIX-like operating systems.

Tested with Logitech Brio and Logitech Webcam Pro 9000.  Should work with any other webcam that has settings that can be set over UVC.  I haven't implemented all features of UVC, only those supported by my two webcams.  Others should be fairly straighforward to add.

Dependencies: libuvc and libusb.

## Building

Requires these libraries to be built and installed:

        https://github.com/libuvc/libuvc

                git clone https://github.com/libuvc/libuvc
                cd libuvc/
                mkdir build
                cd build/
                cmake ..
                make
                sudo make install

        https://libusb.info/

                Download and extract:
                        https://github.com/libusb/libusb/releases/download/v1.0.24/libusb-1.0.24.tar.bz2

                cd libusb-1.0.24/
                ./configure && make && make install

Build with:

        cc -O3 -l uvc -o webcam-checkpoint webcam-checkpoint.c
        
Static build on MacOS:

        cc -O3 -framework CoreFoundation -framework IOKit /usr/local/lib/libuvc.a /usr/local/lib/libusb-1.0.a -o webcam-checkpoint webcam-checkpoint.c
        
## Usage
        ./webcam-checkpoint load|save <filename>
