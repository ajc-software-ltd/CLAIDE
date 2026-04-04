// ============================================================================
// CLIADE - Cross-platform Text Editor
// ============================================================================
// File:        DocumentStateTests.cpp
// Project:     CLIADE
// Copyright:   © 2026 AJC-Software Ltd
// ============================================================================

#include <catch2/catch_test_macros.hpp>

#include "core/DocumentState.hpp"

using namespace Core;

TEST_CASE("DocumentState starts empty", "[documentstate]") {
    DocumentState state;
    REQUIRE(!state.CanUndo());
    REQUIRE(!state.CanRedo());
    REQUIRE(state.GetCurrentStep() == 0);
    REQUIRE(state.GetTotalSteps() == 0);
}

TEST_CASE("DocumentState push operation", "[documentstate]") {
    DocumentState state;
    state.PushOperation({"brightness", {{"value", 50.0}}});

    REQUIRE(state.CanUndo());
    REQUIRE(!state.CanRedo());
    REQUIRE(state.GetCurrentStep() == 1);
    REQUIRE(state.GetTotalSteps() == 1);
}

TEST_CASE("DocumentState undo and redo", "[documentstate]") {
    DocumentState state;
    state.PushOperation({"brightness", {{"value", 50.0}}});
    state.PushOperation({"contrast", {{"value", 25.0}}});

    REQUIRE(state.GetCurrentStep() == 2);

    state.Undo();
    REQUIRE(state.GetCurrentStep() == 1);
    REQUIRE(state.CanUndo());
    REQUIRE(state.CanRedo());

    state.Redo();
    REQUIRE(state.GetCurrentStep() == 2);
    REQUIRE(!state.CanRedo());
}

TEST_CASE("DocumentState push after undo clears redo", "[documentstate]") {
    DocumentState state;
    state.PushOperation({"op1", {{"v", 1.0}}});
    state.PushOperation({"op2", {{"v", 2.0}}});
    state.Undo();

    REQUIRE(state.GetCurrentStep() == 1);
    state.PushOperation({"op3", {{"v", 3.0}}});

    REQUIRE(state.GetCurrentStep() == 2);
    REQUIRE(state.GetTotalSteps() == 2);
    REQUIRE(!state.CanRedo());
}

TEST_CASE("DocumentState max history limit", "[documentstate]") {
    DocumentState state;
    state.SetMaxHistory(3);

    for (int i = 0; i < 5; ++i) {
        state.PushOperation({"op" + std::to_string(i), {{"v", static_cast<double>(i)}}});
    }

    REQUIRE(state.GetTotalSteps() <= 3);
}

TEST_CASE("DocumentState serialize and deserialize", "[documentstate]") {
    DocumentState state;
    state.PushOperation({"brightness", {{"value", 50.0}}});
    state.PushOperation({"contrast", {{"value", 25.0}}});

    auto serialized = state.Serialize();
    REQUIRE(serialized.has_value());

    DocumentState restored;
    auto result = restored.Deserialize(*serialized);
    REQUIRE(result.has_value());
    REQUIRE(restored.GetTotalSteps() == 2);
    REQUIRE(restored.GetCurrentStep() == 2);
}

TEST_CASE("DocumentState clear history", "[documentstate]") {
    DocumentState state;
    state.PushOperation({"op", {{"v", 1.0}}});
    state.ClearHistory();

    REQUIRE(state.GetTotalSteps() == 0);
    REQUIRE(!state.CanUndo());
    REQUIRE(!state.CanRedo());
}

TEST_CASE("DocumentState replay operations", "[documentstate]") {
    DocumentState state;
    std::vector<Operation> ops = {
        {"brightness", {{"value", 50.0}}},
        {"contrast", {{"value", 25.0}}}
    };

    auto result = state.ReplayOperations(ops);
    REQUIRE(result.has_value());
    REQUIRE(result->size() == 2);
}
