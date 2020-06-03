cd resource/upload
gcc upload.c
cd ../..
/usr/bin/env php build-fpga.php
sudo ./resource/upload/a.out
