#!/bin/bash

set -e

TOPDIR=/data/app/dpl
LIBDIR=/data/lib

VERSION=v3.1.3
SHELL_EXE=dplShell
SHELL_VERSION_EXE=${SHELL_EXE}_${VERSION}
SERVICE_EXE=dplService
SERVICE_VERSION_EXE=${SERVICE_EXE}_${VERSION}

echo "--> Creating subdirectories..."
mkdir -p ${TOPDIR}/shell/bin
mkdir -p ${TOPDIR}/shell/cfg
mkdir -p ${TOPDIR}/shell/log
mkdir -p ${TOPDIR}/service/bin
mkdir -p ${TOPDIR}/service/log

echo "--> Setting up dpl shell"
mv ${TOPDIR}/${SHELL_EXE} ${TOPDIR}/shell/bin/${SHELL_VERSION_EXE}
ln -sfn ${SHELL_VERSION_EXE} ${TOPDIR}/shell/bin/${SHELL_EXE}

echo "--> Setting up dpl service"
mv ${TOPDIR}/${SERVICE_EXE} ${TOPDIR}/service/bin/${SERVICE_VERSION_EXE}
ln -sfn ${SERVICE_VERSION_EXE} ${TOPDIR}/service/bin/${SERVICE_EXE}
cp -r ${TOPDIR}/cfg/ ${TOPDIR}/service && rm -rf ${TOPDIR}/cfg
cp -r ${TOPDIR}/sample/ ${TOPDIR}/service && rm -rf ${TOPDIR}/sample

echo "--> Installing core libs to $LIBDIR"
mv ${TOPDIR}/libDataProxy.so.* ${LIBDIR}
ln -sfn libDataProxy.so.3.1.3 ${LIBDIR}/libDataProxy.so.3.1
ln -sfn libDataProxy.so.3.1 ${LIBDIR}/libDataProxy.so.3
ln -sfn libDataProxy.so.3 ${LIBDIR}/libDataProxy.so
ln -sfn libDataProxy.so.3.0.2 ${LIBDIR}/libDataProxy.so.3.0
for lib in `find ${TOPDIR} -name lib*.so.*`; do ln -sfn $(basename $lib) ${LIBDIR}/$(basename $lib .3.1.3); done
mv ${TOPDIR}/lib* ${LIBDIR}

echo "--> change user:group to adlearn:optimization"
chown -R adlearn:optimization ${TOPDIR}
chown -R adlearn:optimization ${LIBDIR}

echo "--> DONE!"
