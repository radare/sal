#!/bin/sh
#
# make a debian package of sal
#

# configuration
PKGNAME="sal"
VERSION="0.2.0"
ARCH="armel"

# workage
MAEMO=`gcc 2>&1 |grep arm`
[ -n "${MAEMO}" ] && ARCH="armel" && echo "Using ARM cpu target..."
TMP=`mktemp`
PRG=${PWD}
rm -f ${TMP}
mkdir -p ${TMP} || exit 1
cd ${TMP} || exit 1
mkdir -p DEBIAN usr/bin usr/share/sal/examples/ usr/lib 2>&1 > /dev/null
[ -n "${MAEMO}" ] && mkdir -p lib usr/lib

# install
# TODO: make install PREFIX=${TMP}
cp ${PRG}/src/sal usr/bin
cp ${PRG}/src/libsal.so usr/lib
cp ${PRG}/examples/* usr/share/sal/examples
cp ${PRG}/lib/gui/*.sal* usr/share/sal/examples
cp ${PRG}/lib/gui/libsalgui.so usr/lib
cp ${PRG}/lib/regex/libsalregex.so usr/lib
cp ${PRG}/lib/expat/libsalexpat.so usr/lib
chmod 0755 usr/bin/*

SIZE=`du -sb . | awk '{print $1}'`
# control
if [ -n "${MAEMO}" ]; then
cat > DEBIAN/control << EOF
Package: ${PKGNAME}
Version: ${VERSION}
Priority: optional
Section: user/developer
Depends: 
Suggests: 
Architecture: ${ARCH}
Maintainer: pancake <pancake@youterm.com>
Filename: pool/mistral/user/${PKGNAME}_${VERSION}_${ARCH}.deb
Installed-size: ${SIZE}
Description: scripting assembly language
  sal is a small and lightweight virtual machine that executes
  a dinamically designed assembly language.
EOF
else
cat > DEBIAN/control << EOF
Package: ${PKGNAME}
Version: ${VERSION}
Architecture: ${ARCH}
Maintainer: pancake <pancake@youterm.com>
Filename: pool/stable/main/${PKGNAME}_${VERSION}_${ARCH}.deb
Installed-size: ${SIZE}
Description: scripting assembly language
  sal is a small and lightweight virtual machine that executes
  a dinamically designed assembly language.
EOF
fi

dpkg-deb -b . ../${PKGNAME}_${VERSION}_${ARCH}.deb || exit 1

cd ..
find ${TMP}
rm -rf ${TMP}
echo "Package done: ($PWD)"
printf "  - "
ls ${PKGNAME}_* 2>/dev/null

PKGSIZE=`du -sb /tmp/${PKGNAME}_${VERSION}_${ARCH}.deb | awk '{print $1}'`
cat << EOF
------------->8-----------
Package: ${PKGNAME}
Version: ${VERSION}
Architecture: ${ARCH}
Maintainer: pancake <pancake@youterm.com>
Filename: pool/mistral/user/${PKGNAME}_${VERSION}_${ARCH}.deb
Size: ${PKGSIZE}
Installed-size: ${SIZE}
Description: scripting assembly language
  sal is a small and lightweight virtual machine that executes
  a dinamically designed assembly language.
------------8<------------
EOF
exit 0
