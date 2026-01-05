# AI Automation & MCP Integration

## What is this?

TrussC applications natively support the **Model Context Protocol (MCP)**.
This allows AI agents (like Claude, Gemini, or IDE assistants) to directly connect to, inspect, and control your running application via standard JSON-RPC messages over stdin/stdout.

By enabling MCP mode, your app becomes a "tool" for AI, enabling:

- **Autonomous Debugging:** AI can read logs, check app state, and fix bugs.
- **Automated Testing:** AI can simulate user input (mouse/keyboard) and verify results via screenshots.
- **Game Agents:** AI can play games against humans by reading the game state directly.

## Enabling MCP Mode

To start your app in MCP mode, simply set the `TRUSSC_MCP` environment variable to `1`.

```bash
export TRUSSC_MCP=1
./myApp
```

When enabled:
1. **Logs (`tc::logNotice`)** are redirected to `stderr` to keep `stdout` clean.
2. **MCP JSON-RPC messages** are read from `stdin` and written to `stdout`.
3. **Standard Tools** (mouse, keyboard, screenshot) are automatically registered.

## Standard MCP Tools

These tools are always available in MCP mode:

### Input Simulation
| Tool | Arguments | Description |
|------|-----------|-------------|
| `mouse_move` | `x`, `y`, `button` | Move mouse cursor (and optionally drag) |
| `mouse_click` | `x`, `y`, `button` | Click mouse button (0:left, 1:right) |
| `mouse_scroll` | `dx`, `dy` | Scroll mouse wheel |
| `key_press` | `key` | Press a key (sokol_app keycode) |
| `key_release` | `key` | Release a key |

### Inspection
| Tool | Arguments | Description |
|------|-----------|-------------|
| `get_screenshot` | (none) | Get current screen as Base64 PNG image |
| `enable_input_monitor` | `enabled` | Enable/Disable user input monitoring logs |

## Creating Custom Tools

You can easily expose your own application logic to AI using the `mcp::tool` builder in `setup()`.

```cpp
#include "TrussC.h"

void tcApp::setup() {
    // Expose a function as an MCP tool
    mcp::tool("place_stone", "Place a stone on the board")
        .arg<int>("x", "X coordinate (0-7)")
        .arg<int>("y", "Y coordinate (0-7)")
        .bind([this](int x, int y) {
            bool success = board.place(x, y);
            return json{
                {"status", success ? "ok" : "error"},
                {"turn", (int)board.currentTurn}
            };
        });

    // Expose state as an MCP resource
    mcp::resource("app://board", "Current Board State")
        .mime("application/json")
        .bind([this]() {
            return board.toJSON();
        });
}
```

## Protocol Details

TrussC implements a subset of the **MCP (Model Context Protocol)** specification over Stdio.

### Request (AI -> App)
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "tools/call",
  "params": {
    "name": "place_stone",
    "arguments": { "x": 3, "y": 4 }
  }
}
```

### Response (App -> AI)
```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "content": [{ "type": "text", "text": "{\"status\":\"ok\"}" }]
  }
}
```

### Notifications (App -> AI)
Logs are sent as notifications:
```json
{
  "jsonrpc": "2.0",
  "method": "notifications/message",
  "params": {
    "level": "NOTICE",
    "logger": "Game",
    "data": "Player placed stone at (3, 4)",
    "timestamp": "12:34:56.789"
  }
}
```

## Connecting with AI Agents

### Via Shell Pipe (Simple Interaction)
You can manually interact with the app using named pipes (FIFO), useful for testing scripts or simple agents.

```bash
mkfifo input_pipe
TRUSSC_MCP=1 ./myApp < input_pipe | cat & 
echo '{"jsonrpc":"2.0","method":"initialize","id":1,"params":{}}' > input_pipe
```

### Via MCP Clients (Claude Desktop, etc.)
Configure your MCP client to run the executable with the environment variable set.

```json
{
  "mcpServers": {
    "trussc-app": {
      "command": "/path/to/myApp",
      "env": { "TRUSSC_MCP": "1" }
    }
  }
}
```