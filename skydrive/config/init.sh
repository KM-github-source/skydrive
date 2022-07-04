cd ../source/server/src
make clean
make
./skydrive_server
dirname=$(pwd)
cd /etc/rc.d/init.d
echo "#!/bin/sh" > skydrive
echo "#chkconfig: 2345 80 90">> skydrive
echo "${dirname}/skydrive_server" >> skydrive

chown root.root /etc/rc.d/init.d/skydrive
chmod 755 /etc/rc.d/init.d/skydrive
chkconfig --add skydrive
chkconfig skydrive on