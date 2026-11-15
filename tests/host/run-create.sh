#!/bin/sh
# Builds libsystem_trace/os_log_create.c against stub headers and runs os-log-create.c. Needs clang; no Darling build.
set -eu
here=$(cd "$(dirname "$0")" && pwd)
root="$here/../.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
inc="-I $here/inc-create -I $root/libsystem_trace -I $here/inc"
clang -w -fsanitize=address -x objective-c $inc -c "$root/libsystem_trace/os_log_create.c" -o "$tmp/create.o"
clang -w -fsanitize=address $inc -c "$here/os-log-create.c" -o "$tmp/test.o"
clang -fsanitize=address "$tmp/create.o" "$tmp/test.o" -o "$tmp/test" -lpthread
ulimit -c 0
"$tmp/test"
