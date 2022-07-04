mysql -uroot < "./source/server/sql/skydrive.sql"
mkdir /home/skydrive_real_path/
rm -rf  /var/www/html/
cp -rf ./source/web/html /var/www
cd ./config
./init.sh