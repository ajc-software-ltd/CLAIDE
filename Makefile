SHELL := /bin/bash

.PHONY: bootstrap dev-build dev-build-first-time dev-release dev-test dev-validate dev-configure dev-build-codex dev-test-codex dev-lint

bootstrap:
	./scripts/bootstrap_prereqs_ubuntu.sh

dev-build:
	./scripts/build_with_prereqs.sh Debug on off off

dev-build-first-time:
	./scripts/build_with_prereqs.sh Debug on off on

dev-release:
	./scripts/build_with_prereqs.sh Release off off off

dev-test:
	ctest --test-dir build --output-on-failure

dev-validate:
	./scripts/build_with_prereqs.sh Debug on on off

# Canonical Codex/workspace workflow aliases
dev-configure:
	./scripts/dev/configure.sh

dev-build-codex:
	./scripts/dev/build.sh

dev-test-codex:
	./scripts/dev/test.sh

dev-lint:
	./scripts/dev/lint.sh full
