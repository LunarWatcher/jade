echo "Using username ${JADE_USER:=jade}"

cd /opt/jade

sudo -u $JADE_USER git pull
cd build 
sudo -u $JADE_USER make -j $(nproc) install
sudo systemctl restart jade
