#!/bin/bash

version=$(grep 'set(RELEASE "v' CMakeLists.txt | sed -E 's/^set\(RELEASE "v(.*)"\)$/\1/')
sed "s/VERSION/$version/g" manual.1