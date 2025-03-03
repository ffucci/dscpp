#!/bin/bash

conan install . -of build-release --build=missing -s build_type=Release
conan install . -of build-debug --build=missing -s build_type=Debug