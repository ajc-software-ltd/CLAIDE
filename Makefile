SHELL := /bin/bash

.PHONY: dev-build dev-release dev-test dev-validate

dev-build:
	./scripts/build_with_prereqs.sh Debug on

dev-release:
	./scripts/build_with_prereqs.sh Release off

dev-test:
	./scripts/bootstrap_prereqs_ubuntu.sh
	ctest --test-dir build --output-on-failure

dev-validate:
	./scripts/build_with_prereqs.sh Debug on on
