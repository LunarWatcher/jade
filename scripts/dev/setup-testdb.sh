#!/usr/bin/bash

echo "Make sure you run dbinit.sh before this script."
sudo -u postgres psql -c "CREATE DATABASE jadetest;"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE jadetest TO jade;"
