#!/bin/bash

set -xe

cd ./native
npx node-gyp configure
npx node-gyp build
cd ..
