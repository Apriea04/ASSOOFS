cmd_/home/osboxes/Desktop/ASSOOFS/assoofs.mod := printf '%s\n'   assoofs.o | awk '!x[$$0]++ { print("/home/osboxes/Desktop/ASSOOFS/"$$0) }' > /home/osboxes/Desktop/ASSOOFS/assoofs.mod
