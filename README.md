Requried external packages to compile:
OPENSSL - LIBSSL-DEV

IMPORTANT! CREATE A FILE NAMED: temp.txt UPON CLONING THE FOLDER!!!!

Place it in /bin/

Make sure that your script is executable with:
    chmod u+x /bin/cloudy

Start it:
    sudo systemctl start cloudy

Enable it to run at boot:
    sudo systemctl enable cloudy

Stop it:
    sudo systemctl stop cloudy

