rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile >/dev/null 2>&1
NAME="fu-$(cat VERSION)"
TARBALL="$NAME-src.tar.gz"
tar --exclude "$TARBALL" --transform "s,^\.,$NAME,g" -c . | gzip -9 > "$TARBALL"
