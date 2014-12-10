libDir="${RPM_INSTALL_PREFIX}/%{libDir}"
appDir="${RPM_INSTALL_PREFIX}/%{appDir}"

# change permissions on top-level dir
chmod 755 ${libDir} ${appDir}

# set ownership to adlearn
chown -R adlearn:optimization ${libDir} ${appDir}
