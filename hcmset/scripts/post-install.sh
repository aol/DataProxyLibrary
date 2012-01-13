#!/bin/bash

set -e

TOPDIR=/data/app/dpl
LIBDIR=/data/lib

VERSION=v3.1.0
SHELL_EXE=dplShell
SHELL_VERSION_EXE=${SHELL_EXE}_${VERSION}
SERVICE_EXE=dplService
SERVICE_VERSION_EXE=${SERVICE_EXE}_${VERSION}

echo "--> Creating subdirectories..."
mkdir -p ${TOPDIR}/shell/bin
mkdir -p ${TOPDIR}/shell/cfg
mkdir -p ${TOPDIR}/shell/log
mkdir -p ${TOPDIR}/shell/sample
mkdir -p ${TOPDIR}/service/bin
mkdir -p ${TOPDIR}/service/cfg
mkdir -p ${TOPDIR}/service/log
mkdir -p ${TOPDIR}/service/sample

echo "--> Setting up DPL app binaries"
mv ${TOPDIR}/${SHELL_EXE} ${TOPDIR}/shell/bin/${SHELL_VERSION_EXE}
mv ${TOPDIR}/${SERVICE_EXE} ${TOPDIR}/shell/bin/${SERVICE_VERSION_EXE}
ln -sfn ${SHELL_VERSION_EXE} ${TOPDIR}/bin/${SHELL_EXE}
ln -sfn ${SERVICE_VERSION_EXE} ${TOPDIR}/bin/${SERVICE_EXE}

echo "--> change user:group to adlearn:optimization"
chown -R adlearn:optimization ${TOPDIR}

echo "--> DONE!"
