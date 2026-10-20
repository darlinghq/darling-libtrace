#!/bin/sh
# Builds libsystem_trace/unpack_buffer.m against stub Objective-C headers and runs null-arguments.c.
# Needs clang with the GNUstep Objective-C runtime flag; no Darling build.
set -eu
here=$(cd "$(dirname "$0")" && pwd)
root="$here/../.."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
# a quoted include finds the real os_log_s.h next to the source first, so compile a copy beside the stub one
cp "$root/libsystem_trace/unpack_buffer.m" "$here/inc/os_log_s.h" "$tmp/"
# Apple clang rejects the GNUstep runtime flag (COMDATs on Mach-O); the stubs stand in for the runtime there.
case $(uname -s) in
Darwin) objc_runtime="" ;;
*) objc_runtime="-fobjc-runtime=gnustep-1.9" ;;
esac
# shellcheck disable=SC2086
clang -w -x objective-c $objc_runtime -DOS_LOG_PRIVACY=0 -I "$here/inc" -I "$root/libsystem_trace" -I "$root/libsbuf" \
	-include "$tmp/os_log_s.h" -c "$tmp/unpack_buffer.m" -o "$tmp/unpack.o"
clang -w "-D__FBSDID(x)=" -I "$here/inc" -include "$here/inc/os/base.h" -include stdlib.h -include string.h -include ctype.h \
	-c "$root/libsbuf/libsbuf.c" -o "$tmp/sbuf.o"
clang -w -c "$here/stubs.c" -o "$tmp/stubs.o"
clang -w -c "$here/null-arguments.c" -o "$tmp/test.o"
clang "$tmp/unpack.o" "$tmp/sbuf.o" "$tmp/stubs.o" "$tmp/test.o" -o "$tmp/test"
ulimit -c 0
"$tmp/test"
