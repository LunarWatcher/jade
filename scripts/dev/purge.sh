#!/usr/bin/bash

sudo -u postgres psql -d jade -c "DROP SCHEMA Jade CASCADE;"
sudo -u postgres psql -d jadetest -c "DROP SCHEMA Jade CASCADE;"
