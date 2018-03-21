# uipset
# Description
TEST  
IPset server  
(tested with Debian 9 on x86)  
# How to build
* Setting up a chroot

```sh
apt-get install coreutils bash debootstrap
```

```sh
ARCH="i386"; SUITE="stretch"; CHROOTDIR="/storage/"${SUITE}"-chroot_"${ARCH}""; LC="ru_RU.UTF-8";
mkdir -p "${CHROOTDIR}" && \
debootstrap --arch="${ARCH}" --variant=minbase --include=locales,apt-utils,dialog,findutils,file,sed,gawk,bzip2 \
"${SUITE}" "${CHROOTDIR}" http://mirror.mephi.ru/debian/ && \
echo "LANG="${LC}"" > "${CHROOTDIR}"/etc/default/locale && \
sed -i 's/# '"${LC}"' UTF-8/'"${LC}"' UTF-8/' "${CHROOTDIR}"/etc/locale.gen && \
chmod 777 "${CHROOTDIR}"/home && \
chroot "${CHROOTDIR}" /bin/bash -c "su - -c \"locale-gen\"";

```

* compile test

```sh
make clean test && LD_LIBRARY_PATH=./build build/testclient
```