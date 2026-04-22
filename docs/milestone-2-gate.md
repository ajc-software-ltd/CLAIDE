# Milestone 2 Green-Pass Gate (Vulkan Canvas)

Milestone 2 is green-pass when all checks below are satisfied:

1. Vulkan runtime loader and render-host lifecycle API available and coherent.
2. Canvas panel renders checkerboard background and runtime status path.
3. Scoped lint runs complete with Bucket A = 0 for core/ui/vulkan/tests.
4. Shader compile path executes during application startup and logs failures clearly.
5. Canvas multi-instance registry + PiP layer model available for composition workflows.

## Validation commands

```bash
cmake --preset dev
cmake --build build --target CLAIDE
cmake --build build --target CLAIDETests
ctest --test-dir build -R CanvasCoreTests --output-on-failure
ctest --test-dir build -R RendererFoundation --output-on-failure
ctest --test-dir build -R VulkanRuntimeFoundation --output-on-failure
cmake --build build --target lint-core
cmake --build build --target lint-ui
cmake --build build --target lint-vulkan
cmake --build build --target lint-tests
```
