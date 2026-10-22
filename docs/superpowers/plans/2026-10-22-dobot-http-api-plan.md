# Dobot HTTP API Gateway Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a JSON HTTP gateway to `dobot_v4_bringup` while preserving all existing ROS services and topics.

**Architecture:** Run a small HTTP server in the existing `CRRobot` node. Route information requests to read-only robot/service operations and control requests to the existing ROS service callbacks or shared robot command layer. Keep HTTP conversion isolated from ROS message/service definitions and return one consistent JSON envelope.

**Tech Stack:** ROS Noetic, catkin, C++14, `nlohmann::json`, vendored `cpp-httplib` single-header server.

**Spec:** `docs/http-api-design.md`

## Global Constraints

- Existing ROS service names, request fields, response fields, and topic names remain unchanged.
- HTTP routes use `GET /api/v1/info/<name>` and `POST /api/v1/control/<name>`.
- Default HTTP port is `8080`, configurable through `~http_port`.
- JSON responses contain `success`, `code`, `message`, and `data`.
- Control requests for one robot are serialized.
- No authentication is added in the initial implementation.

## Review Focus

- Missing and incorrectly typed JSON request fields return HTTP 400 with structured JSON.
- Unknown routes return HTTP 404 without calling the robot.
- Robot/ROS service failure returns `success: false` and does not crash the ROS node.
- Concurrent control requests execute serially.
- HTTP shutdown follows ROS shutdown and does not leave a listening thread behind.

### Task 1: HTTP dependency and package configuration

**Files:**
- Create: `include/dobot_v4_bringup/httplib.h` (vendored `cpp-httplib` header)
- Modify: `CMakeLists.txt` (include path, thread linkage, target dependency)
- Modify: `package.xml` if a system dependency is required
- Test: CMake configure/build of the package

**Interfaces:**
- Produces `httplib::Server` availability to the HTTP gateway.

- [ ] **Step 1: Add the vendored HTTP header and document its source/version.**
- [ ] **Step 2: Update catkin target include paths and link dependencies for threads.**
- [ ] **Step 3: Configure and build only `dobot_v4_bringup`.**

Run:

```bash
source /opt/ros/noetic/setup.bash
catkin_make --pkg dobot_v4_bringup -j1
```

Expected: package configures and builds without HTTP code yet.

- [ ] **Step 4: Commit dependency setup.**

### Task 2: JSON and HTTP response utilities

**Files:**
- Create: `include/dobot_v4_bringup/http_api_types.h`
- Create: `src/http_api_types.cpp` only if non-inline helpers are needed
- Test: `test/http_api_types_test.cpp`

**Interfaces:**
- `nlohmann::json makeSuccess(int code, const std::string& message, const nlohmann::json& data)`
- `nlohmann::json makeError(int code, const std::string& message)`
- `bool requireField(const nlohmann::json&, const char*, std::string*)`

- [ ] **Step 1: Write tests for success/error envelopes and missing fields.**
- [ ] **Step 2: Run the tests and verify the new tests fail.**
- [ ] **Step 3: Implement the response and validation helpers.**
- [ ] **Step 4: Run the tests and verify they pass.**
- [ ] **Step 5: Commit the JSON utility layer.**

### Task 3: HTTP server lifecycle and routing layer

**Files:**
- Create: `include/dobot_v4_bringup/http_server.h`
- Create: `src/http_server.cpp`
- Modify: `src/main.cpp` to start/stop the server with ROS
- Test: `test/http_server_test.cpp`

**Interfaces:**
- `class HttpServer` with `start(uint16_t port)`, `stop()`, and `registerRoutes(CRRobot&)`.
- `HttpServer` owns the server thread and stops it when ROS shuts down.

- [ ] **Step 1: Add route tests for `/api/v1/health`, unknown routes, and malformed JSON.**
- [ ] **Step 2: Run tests and verify failures before implementation.**
- [ ] **Step 3: Implement lifecycle, port binding, JSON content type, and route dispatch.**
- [ ] **Step 4: Add `~http_port` parameter handling in `main.cpp`.**
- [ ] **Step 5: Run package tests and verify clean shutdown.**
- [ ] **Step 6: Commit the server lifecycle layer.**

### Task 4: Information endpoint adapters

**Files:**
- Create: `include/dobot_v4_bringup/http_info_api.h`
- Create: `src/http_info_api.cpp`
- Modify: `include/dobot_v4_bringup/cr5_v4_robot.h` only for narrow read-only adapter access
- Test: `test/http_info_api_test.cpp`

**Interfaces:**
- `void registerInfoRoutes(HttpServer&, CRRobot&)`.
- Route names and ROS service mappings follow `docs/http-api-design.md`.

- [ ] **Step 1: Add tests for pose, mode, error ID, command ID, IO, and health response shapes.**
- [ ] **Step 2: Implement request conversion and response conversion for each information service.**
- [ ] **Step 3: Register all information routes, including status topic snapshots.**
- [ ] **Step 4: Verify one endpoint from each information family with a mocked service boundary.**
- [ ] **Step 5: Commit information endpoints.**

### Task 5: Control endpoint adapters

**Files:**
- Create: `include/dobot_v4_bringup/http_control_api.h`
- Create: `src/http_control_api.cpp`
- Modify: `include/dobot_v4_bringup/cr5_v4_robot.h` only for narrow control adapter access
- Test: `test/http_control_api_test.cpp`

**Interfaces:**
- `void registerControlRoutes(HttpServer&, CRRobot&)`.
- Every control route validates the JSON body before invoking a ROS service or robot command.

- [ ] **Step 1: Add tests for enable, stop, emergency stop, MovJ, IO writes, Modbus writes, and malformed bodies.**
- [ ] **Step 2: Implement generic scalar/array/string field conversion.**
- [ ] **Step 3: Implement motion, IO, configuration, safety, Modbus, script, and dashboard route families.**
- [ ] **Step 4: Serialize control calls per robot and map service return values into the JSON envelope.**
- [ ] **Step 5: Verify safety-related calls never bypass existing robot checks.**
- [ ] **Step 6: Commit control endpoints.**

### Task 6: Integration, launch configuration, and documentation

**Files:**
- Modify: `launch/bringup_v4.launch` to expose `http_port`
- Modify: `README.md` or create `docs/http-api.md`
- Modify: `CMakeLists.txt` for all new sources/tests
- Test: ROS launch and HTTP smoke tests

- [ ] **Step 1: Add the HTTP port parameter to the launch file with default `8080`.**
- [ ] **Step 2: Document curl examples for health, pose, enable, and MovJ.**
- [ ] **Step 3: Build the full workspace with ROS Noetic.**
- [ ] **Step 4: Launch the node and verify health, one information endpoint, one control endpoint, and one error response.**
- [ ] **Step 5: Confirm existing ROS service calls still work.**
- [ ] **Step 6: Commit integration changes.**

## Completion Verification

```bash
source /opt/ros/noetic/setup.bash
catkin_make --pkg dobot_v4_bringup -j1
roslaunch dobot_v4_bringup bringup_v4.launch
curl -i http://127.0.0.1:8080/api/v1/health
```

The implementation is complete only when the package builds, the HTTP health endpoint returns valid JSON, representative information and control calls work, malformed requests return structured errors, and existing ROS services remain available.
