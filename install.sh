./build.sh
if [ $? -eq 0 ];
then
  sudo cp ./build/bin/serverstatus /usr/local/bin
  sudo cp serverstatus.service /etc/systemd/system/serverstatus.service
  sudo systemctl daemon-reload
  sudo systemctl enable serverstatus
  sudo systemctl start serverstatus
fi