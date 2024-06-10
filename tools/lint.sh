#!/bin/sh

set -e

while getopts "v" opt; do
    case ${opt} in
        v)
            ARGS=--extra-arg=-v
            ;;
    esac
done

cppcheck \
    --inline-suppr \
    --enable=warning,performance,information,missingInclude \
    --error-exitcode=1 \
    --suppress=missingIncludeSystem \
    -i build \
    --project=build/compile_commands.json
find src \( -name *.cpp -o -name *.h \) -print0 | xargs -0 -n1 clang-tidy-14 -p build ${ARGS}
