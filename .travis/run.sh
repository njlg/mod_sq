#!/bin/sh

# install the pre-reqs
sudo apt-get install apache2 apache2-prefork-dev
wget http://squirrel.googlecode.com/files/squirrel_3_0_4_stable.tar.gz
tar xzvf squirrel_3_0_4_stable.tar.gz
wget https://raw.github.com/njlg/gentoo-overlay/master/dev-lang/squirrel/files/squirrel-3.0.4-autotools.patch
patch -p0 < squirrel-3.0.4-autotools.patch
cd SQUIRREL3
mkdir m4
autoreconf -vfi
./configure && make
sudo make install
sudo ldconfig

# build it
cd ..
autoreconf -vfi
./configure && make

# install it
sudo make install

# prep to test it
echo "$(curl https://gist.github.com/raw/4545578/82ef48d68db1b76e6a784b6ceb44c026f12d289c/squirrel.apacheconf)" | sudo tee /etc/apache2/mods-available/squirrel.load > /dev/null
sudo a2enmod squirrel
echo "$(curl https://gist.github.com/raw/4545575/c62ecef2ce0bb50b1110ce7f2ad59efb2366c9d8/default)" | sed -e "s,PATH,`pwd`/examples,g" | sudo tee /etc/apache2/sites-available/default > /dev/null
sudo perl -pi -e 's/LogLevel warn/LogLevel debug/' /etc/apache2/apache2.conf
sudo service apache2 restart

# test all the nuts
curl http://localhost/welcome.nut
curl http://localhost/globals.nut
curl http://localhost/file.nut
curl http://localhost/header.nut

# test stock nuts
mv SQUIRREL3/samples examples
curl http://localhost/samples/ackermann.nut
curl http://localhost/samples/array.nut
curl http://localhost/samples/class.nut
curl http://localhost/samples/classattributes.nut
curl http://localhost/samples/coroutines.nut
curl http://localhost/samples/delegation.nut
curl http://localhost/samples/fibonacci.nut
curl http://localhost/samples/flow.nut
curl http://localhost/samples/generators.nut
curl http://localhost/samples/hello.nut
curl http://localhost/samples/list.nut
curl http://localhost/samples/loops.nut
curl http://localhost/samples/matrix.nut
curl http://localhost/samples/metamethods.nut
curl http://localhost/samples/methcall.nut
curl http://localhost/samples/tailstate.nut

# check the log
cat /var/log/apache2/error.log
