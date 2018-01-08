#! /bin/bash

mkdir ./out/odroid_highweb_browser
cp ./out/OdroidRelease/chrome ./out/odroid_highweb_browser
cp ./out/OdroidRelease/*.pak ./out/odroid_highweb_browser
cp ./out/OdroidRelease/*.so ./out/odroid_highweb_browser
cp ./out/OdroidRelease/icudtl.dat ./out/odroid_highweb_browser
cp ./out/OdroidRelease/*.bin ./out/odroid_highweb_browser
cp -R ./out/OdroidRelease/locales ./out/odroid_highweb_browser/
cp -R ./out/OdroidRelease/resources ./out/odroid_highweb_browser/
tar czf odroid_highweb_browser.tar.gz -C ./out/ odroid_highweb_browser
sshpass -p 'odroid' ssh odroid@192.168.10.45 'rm odroid_highweb_browser.tar.gz'
sshpass -p 'odroid' scp odroid_highweb_browser.tar.gz odroid@192.168.10.45:/home/odroid/
sshpass -p 'odroid' ssh odroid@192.168.10.45 'tar xzf odroid_highweb_browser.tar.gz'
rm -rf ./out/odroid_highweb_browser
rm odroid_highweb_browser.tar.gz
