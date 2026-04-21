SHELL := /bin/bash

.PHONY: bootstrap dev-build dev-build-first-time dev-release dev-test dev-validate lint-fast lint-core lint-ui lint-vulkan lint-tests

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

lint-fast:
	./scripts/run_lint_changed.sh build

lint-core:
	LINT_SCOPE='src/(core|platform)/' ./scripts/run_lint_changed.sh build

lint-ui:
	LINT_SCOPE='src/ui/' ./scripts/run_lint_changed.sh build

lint-vulkan:
	LINT_SCOPE='src/vulkan/' ./scripts/run_lint_changed.sh build

lint-tests:
	LINT_SCOPE='tests/' ./scripts/run_lint_changed.sh build
