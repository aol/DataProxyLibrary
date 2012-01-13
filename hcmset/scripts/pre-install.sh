#!/bin/sh

export TOPDIR=/data/app/dpl

if [ -d "${TOPDIR}" ]
then
    echo "--> Creating backup copy of ${TOPDIR}..."
    DATETIMESTRING=`date +%Y-%m-%d_%H:%M`
	OLDDIR=${TOPDIR}.old.${DATETIMESTRING}
    cp -Rf ${TOPDIR} ${OLDDIR}
    echo "    ** Backup location: ${OLDDIR}"
else
    echo "--> No backup necessary prior to installation..."
fi
