#!/bin/bash

set -e

TOPDIR=/data/app/dpl
LIBDIR=/data/lib

VERSION=3.2.0
MIN_VER=${VERSION:0:3}
MAJ_VER=${VERSION:0:1}
SHELL_EXE=dplShell
SHELL_VERSION_EXE=${SHELL_EXE}_v${VERSION}
SERVICE_EXE=dplService
SERVICE_VERSION_EXE=${SERVICE_EXE}_v${VERSION}

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
mkdir -p $LIBDIR
mv ${TOPDIR}/DataProxy.mexa64 ${LIBDIR}
mv ${TOPDIR}/libDataProxy.so.* ${LIBDIR}
ln -sfn libDataProxy.so.${VERSION} ${LIBDIR}/libDataProxy.so.${MIN_VER}
ln -sfn libDataProxy.so.${MIN_VER} ${LIBDIR}/libDataProxy.so.${MAJ_VER}
ln -sfn libDataProxy.so.${MAJ_VER} ${LIBDIR}/libDataProxy.so
ln -sfn libDataProxy.so.3.1.10 ${LIBDIR}/libDataProxy.so.3.1
ln -sfn libDataProxy.so.3.0.2 ${LIBDIR}/libDataProxy.so.3.0
for lib in `find ${TOPDIR} -name lib*.so.*`; do ln -sfn $(basename $lib) ${LIBDIR}/$(basename $lib .${VERSION}); done
mv ${TOPDIR}/lib* ${LIBDIR}

echo "--> change user:group to adlearn:optimization"
chown -R adlearn:optimization ${TOPDIR}
chown -R adlearn:optimization ${LIBDIR}

echo "--> DONE!"
