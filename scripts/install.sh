#!/usr/bin/bash

set -x
set -e 

cd /opt 

if [ -d /opt/jade ]; then
    echo "/opt/jade already exists. Do not rerun the installer to update. Use ./scripts/update.sh instead."
    return -69
fi


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

sudo mkdir jade
sudo chown -R $JADE_USER /opt/jade
sudo -u $JADE_USER git clone https://codeberg.org/LunarWatcher/jade
cd /opt/jade

# TODO: I think this might count as insecure. Keeping /opt/jade and /opt/jade/dist/bin owned by $USER or root (root probably just for the bin folder), and
# making the data dir itself owned by $JADE_USER might be safer?
sudo -u $JADE_USER mkdir build
cd build
sudo -u $JADE_USER cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/opt/jade/dist/ 
sudo -u $JADE_USER make -j $(nproc)
sudo -u $JADE_USER make install

# Database init {{{
# For non-interactive use, check for the PSQL_PASSWORD env variable before prompting
if [[ "$PSQL_PASSWORD" == "" ]];
then
    set +x
    echo "Postgres password for [postgres] user jade: "
    read -s PSQL_PASSWORD
    if [[ $? != 0 ]] || [[ "$PSQL_PASSWORD" == "" ]]; then
        echo "Panic: failed to read password"
        echo "Did you curl | bash instead of bash <(curl)?"
        exit -2
    fi
    echo
    set -x
fi

# These can error out; error means stuff already exists, and is probably a consequence of the installer being re-run
set +e
# TODO: This has escaping issues, but ${PSQL_PASSWORD@Q} will likely break the password because erroneous \" 
set +x
sudo -u postgres psql -c "CREATE USER jade WITH ENCRYPTED PASSWORD '$PSQL_PASSWORD';"
set -x
sudo -u postgres psql -c "CREATE DATABASE jade;"
sudo -u postgres psql -c "GRANT ALL PRIVILEGES ON DATABASE jade TO jade;"
set -e
# }}}

cat <<EOF | sudo tee /etc/nginx/conf.d/jade.conf
server {
    listen 443 ssl http2;
    server_name ebooks.${JADE_DOMAIN:-FIXME};
    ssl_certificate         ${JADE_CERT:-FIXME};
    ssl_certificate_key     ${JADE_CERT_KEY:-FIXME};

    location / {
        proxy_set_header   X-Real-IP \$remote_addr;
        proxy_set_header   X-Forwarded-For \$proxy_add_x_forwarded_for;
        proxy_set_header   Host \$host;
        proxy_pass         http://localhost:6969/;
        # Required to prevent downgrades during redirects
        # This is caused by an oddity in how Crow handles redirects. See
        #     https://github.com/CrowCpp/Crow/blob/ad337a8a868d1d6bc61e9803fc82f36c61f706de/include/crow/http_connection.h#L257
        # for the code in question
        proxy_redirect     http://\$host/ https://\$host/;
        proxy_http_version 1.1;
        proxy_set_header   Upgrade \$http_upgrade;
        proxy_set_header   Connection "upgrade";

        # Required for HTTP basic auth services
        proxy_set_header   Authorization \$http_authorization;
        proxy_pass_header  Authorization;
    }
}
EOF

cd /opt/jade
sudo cp etc/systemd/system/jade.service /etc/systemd/system/jade.service
sudo mkdir -p /etc/jade
sudo cp config.example.json /etc/jade/config.json
sudo systemctl daemon-reload

echo "Jade successfully installed. Next steps:"
echo " 1. Edit /etc/jade/config.json"
echo " 2. Run 'sudo systemctl start jade', and verify that it started."
echo "\tNote that depending on how you edit config.json, you may also need to edit the following files:"
echo "\t * /etc/systemd/system/jade.service"
echo "\t * /etc/nginx/conf.d/jade.conf [Non-Ubuntu users may need to modify /etc/nginx/nginx.conf instead of using this file]"
echo "\tIf any of the files have disappeared, they're available in install.sh or elsewhere in the repo: https://codeberg.org/LunarWatcher/jade"
echo " 3. Run 'sudo systemctl restart nginx'"
echo ""
echo "If you run into problems, feel free to open an issue on Codeberg:"
echo "\thttps://codeberg.org/LunarWatcher/jade/issues"

