#!/usr/bin/bash

set -x
set -e 

cd /opt 
sudo mkdir jade
sudo chown olivia jade

echo "Using username ${JADE_USER:=jade}"
# This command failure is checked; re-allow bash to continue even on error
set +e
id -u $JADE_USER
if [[ $? != 0 ]]; then
    set -e
    sudo useradd --system --user-group --home-dir / --shell /sbin/nologin $JADE_USER
else
    set -e
    set +x
    echo "User $JADE_USER already exists. It's assumed this is correct."
    echo "If it isn't, you have 10 seconds to press ctrl-c to abort the installation."
    echo "Using an existing user is fine, but may have security implications in the event of"
    echo "a security breach. It's strongly recommended to use a dedicated user for the server"
    echo "NOTE: If you're rerunning the install script to update hazel, and the existing user _is_ for this server, don't."
    echo "Use /opt/jade/scripts/update.sh instead."
    set -x
    sleep 10
fi


# Database init {{{
# For non-interactive use, check for the PSQL_PASSWORD env variable before prompting
if [[ "$PSQL_PASSWORD" == "" ]];
then
    read -s -p "Postgres password for user jade: " PSQL_PASSWORD
    echo
fi

# These can error out; error means stuff already exists, and is probably a consequence of the installer being re-run
set +e
# TODO: This has escaping issues, but ${PSQL_PASSWORD@Q} will likely break the password because erroneous \" 
sudo -u postgres psql -c "CREATE USER jade WITH ENCRYPTED PASSWORD '$PSQL_PASSWORD';"
sudo -u postgres psql -c "CREATE DATABASE jade;"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE jade TO jade;"
set -e
# }}}
