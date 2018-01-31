#!/bin/sh

#instalar nodejs na maquina!

#instalar node-gyp
#sudo npm install -g node-gyp

#instalar uuid-dev - universally unique id library - headers and static libraries
#sudo apt-get install uuid-dev
#fedora:
#cd opensource/util-linux-2.28.rc1
#./configure && make -j4 && sudo make install
#osx
#brew install ossp-uuid

#remover build anterior
rm -rf node_modules

#remover fontes re2c anteriores:
rm request.cpp

#transformar fonte re2c:
./re2c --case-insensitive --input custom request.re.c > request.cpp

#compilar modulo
node-gyp configure build

#criar diretorio do modulo
mkdir node_modules

#copiar modulo para destino
cp build/Release/neurose.node node_modules/neurose.node

#remover diretorio de temporarios do build
rm -rf build

#server
#node run_server.js

#rodar testes
node check_001.js
