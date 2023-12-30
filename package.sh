#!/usr/bin/env bash

mkdir -p dist
version=$(<VERSION)

archive="dist/wnpcli-${version}_linux_amd64.tar.gz"
rm -f $archive
tar -cvzf $archive "build/wnpcli_linux_amd64" --transform='s/build\/wnpcli_linux_amd64/wnpcli/' "CHANGELOG.md" "README.md" "LICENSE" "VERSION" --show-transformed-names
