# Copyright (c) 2023 Dmitry Ponomarev <ponomarevda96@gmail.com>
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

CRNT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
TESTS_DIR := $(abspath $(dir $(lastword $(CRNT_DIR))))
REPO_DIR := $(abspath $(dir $(lastword $(TESTS_DIR))))
BUILD_DIR=${REPO_DIR}/build/tests/ubuntu

all: test_common

test_common: fft algorithms

create_build_dir:
	mkdir -p ${BUILD_DIR}

coverage: create_build_dir
	cd ${BUILD_DIR} && cmake -DCOVERAGE=1 ${CRNT_DIR} && make
	cd ${BUILD_DIR} && ./algorithms
	cd ${BUILD_DIR} && ./ft

	echo "\n1. Algorithms:"
	cd ${BUILD_DIR} && gcov ${BUILD_DIR}/CMakeFiles/algorithms.dir${REPO_DIR}/common/algorithms.cpp.gcda

	echo "\n2. FFT:"
	cd ${BUILD_DIR} && gcov ${BUILD_DIR}/CMakeFiles/fft.dir${REPO_DIR}/common/fft.c.gcda

algorithms: create_build_dir
	cd ${BUILD_DIR} && cmake -S ${CRNT_DIR} && make && ./algorithms

fft: create_build_dir
	cd ${BUILD_DIR} && cmake -S ${CRNT_DIR} && make && ./fft
