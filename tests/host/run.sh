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
# Darwin x86_64 has a signed char; make the host agree so the %P sign-extension case is observable on arm hosts.
clang -w -fsigned-char -x objective-c $objc_runtime -DOS_LOG_PRIVACY=0 -I "$here/inc" -I "$root/libsystem_trace" -I "$root/libsbuf" \
	-include "$tmp/os_log_s.h" -c "$tmp/unpack_buffer.m" -o "$tmp/unpack.o"
clang -w "-D__FBSDID(x)=" -I "$here/inc" -include "$here/inc/os/base.h" -include stdlib.h -include string.h -include ctype.h \
	-c "$root/libsbuf/libsbuf.c" -o "$tmp/sbuf.o"
clang -w -c "$here/stubs.c" -o "$tmp/stubs.o"
clang -w -c "$here/null-arguments.c" -o "$tmp/test.o"
clang "$tmp/unpack.o" "$tmp/sbuf.o" "$tmp/stubs.o" "$tmp/test.o" -o "$tmp/test"
ulimit -c 0
"$tmp/test"

# _os_log_pack_size/_os_log_pack_fill against the real os/log_private.h (tests/host/inc/os/log.h stubs the rest).
# The stub inc dir comes first so its os/log.h wins; log_private.h is found in the real include dir.
clang -w -g -fsanitize=address -I "$here/inc" -I "$root/include" -c "$root/libsystem_trace/os_log_pack.c" -o "$tmp/pack.o"
clang -w -g -fsanitize=address -I "$here/inc" -I "$root/include" -c "$here/os-log-pack.c" -o "$tmp/pack-test.o"
clang -fsanitize=address "$tmp/pack.o" "$tmp/pack-test.o" -o "$tmp/pack-test"
"$tmp/pack-test"
