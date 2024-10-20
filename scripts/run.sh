#!/usr/bin/env bash

set -u

ASAN_OPTIONS=abort_on_error=1:disable_coredump=0:unmap_shadow_on_exit=1 ./bin/main &
pid=$!

function action {
    kill $pid
}

trap action EXIT

wait "$pid"
exit_code=$?

trap - EXIT

if [ "$exit_code" -ne 0 ]; then
    coredumpctl debug -A '-q --readnow -ex "thread apply all backtrace full" -ex q' "$pid"
fi
