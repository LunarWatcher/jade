#!/usr/bin/bash

# For non-interactive use, check for the PSQL_PASSWORD env variable before prompting
if [[ "$PSQL_PASSWORD" == "" ]];
then
    read -s -p "Postgres password for [postgres] user jade: " PSQL_PASSWORD
    echo
fi

# These can error out; error means stuff already exists, and is probably a consequence of the installer being re-run
set +e
# TODO: This has escaping issues, but ${PSQL_PASSWORD@Q} will likely break the password because erroneous \" 
sudo -u postgres psql -c "CREATE USER jade WITH ENCRYPTED PASSWORD '$PSQL_PASSWORD';"
sudo -u postgres psql -c "CREATE DATABASE jade;"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE jade TO jade;"
set -e
