cmake -P clean-all.cmake
NAME="fu-$(cat VERSION)"
TARBALL="$NAME-src.tar.gz"
tar --exclude ".git" --exclude "$TARBALL" --transform "s,^\.,$NAME,g" -c . | gzip -9 > "$TARBALL"
