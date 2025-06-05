echo "Using username ${JADE_USER:=jade}"

cd /opt/jade

# TODO: stick to tagged versions unless flags say otherwise
git pull
cd build 
make -j $(nproc)
sudo cmake --install . --prefix /opt/jade/dist
sudo systemctl restart jade
