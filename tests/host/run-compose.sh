#!/bin/sh
# Builds libsystem_trace/os_log_compose.c against the real os/log_private.h (tests/host/inc/os/log.h stubs the rest)
# and runs os-log-compose.c. The stub inc dir comes first so its os/log.h wins. No Darwin SDK needed.
set -eu
here=$(cd "$(dirname "$0")" && pwd)
root="$here/../.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
clang -w -g -fsanitize=address -I "$here/inc" -I "$root/include" -c "$root/libsystem_trace/os_log_compose.c" -o "$tmp/compose.o"
clang -w -g -fsanitize=address -I "$here/inc" -I "$root/include" -c "$here/os-log-compose.c" -o "$tmp/test.o"
clang -fsanitize=address "$tmp/compose.o" "$tmp/test.o" -o "$tmp/test"
ulimit -c 0
"$tmp/test"
