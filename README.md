# PC Remote

## Build
```sh
arduino-cli compile --fqbn esp8266:esp8266:nodemcu pc-remote.ino && \
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp8266:esp8266:nodemcu pc-remote.ino
```

## Upload SPIFFS
```sh
./makespiffs.sh /dev/ttyUSB0
```

## Example systemd service
```
[Unit]
Description=PC Remote

[Service]
Environment=GO_ENV=production
Environment=PC_REMOTE_SECRETS=approved_secret1234,acidburn1337
User=remote_user
Type=simple
Restart=always
RestartSec=5s
WorkingDirectory=/home/remote_user/pc-remote
ExecStart=/home/remote_user/pc-remote/pc-remote

[Install]
WantedBy=multi-user.target
```

Example nginx proxy config snippet
```
location /pc-remote {
    rewrite /pc-remote$ /pc-remote/ permanent;
    rewrite /pc-remote/(.*) /$1  break;
    proxy_pass http://127.0.0.1:8000;
}
```

