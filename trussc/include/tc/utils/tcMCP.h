#pragma once

// =============================================================================
// tcMCP.h - Model Context Protocol (MCP) Server Implementation
// =============================================================================

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <sstream>
#include <future>

// JSON support
#include "../../nlohmann/json.hpp"
using json = nlohmann::json;

#include "tcLog.h"

namespace trussc {
namespace mcp {

// ---------------------------------------------------------------------------
// Types & Interfaces
// ---------------------------------------------------------------------------

struct ToolArg {
    std::string name;
    std::string type; // "string", "int", "float", "boolean", "object", "array"
    std::string description;
    bool required = true;
};

class Tool {
public:
    std::string name;
    std::string description;
    std::vector<ToolArg> args;
    std::function<json(const json&)> handler;

    json getSchema() const {
        json schema = {
            {"type", "object"},
            {"properties", json::object()},
            {"required", json::array()}
        };

        for (const auto& arg : args) {
            schema["properties"][arg.name] = {
                {"type", arg.type},
                {"description", arg.description}
            };
            if (arg.required) {
                schema["required"].push_back(arg.name);
            }
        }
        return schema;
    }
};

class Resource {
public:
    std::string uri;
    std::string name;
    std::string mimeType;
    std::string description;
    std::function<std::string()> handler; // Returns content (text or base64)
};

// ---------------------------------------------------------------------------
// MCP Server Core
// ---------------------------------------------------------------------------

class Server {
public:
    static Server& instance() {
        static Server server;
        return server;
    }

    // --- Registration API ---

    void registerTool(const Tool& tool) {
        tools_[tool.name] = tool;
    }

    void registerResource(const Resource& res) {
        resources_[res.uri] = res;
    }

    void sendNotification(const std::string& method, const json& params) {
        json res = {
            {"jsonrpc", "2.0"},
            {"method", "notifications/" + method},
            {"params", params}
        };
        std::cout << res.dump() << std::endl;
    }

    // --- Message Processing ---

    void processMessage(const std::string& rawMessage) {
        try {
            auto j = json::parse(rawMessage);
            
            // Validate JSON-RPC
            if (!j.contains("jsonrpc") || j["jsonrpc"] != "2.0") {
                // Ignore non-JSON-RPC messages (might be noise)
                return;
            }

            // Handle Request
            if (j.contains("method")) {
                handleRequest(j);
            } 
            // Handle Response (not implemented for server role)
            else if (j.contains("result") || j.contains("error")) {
                // TODO: Handle client responses if needed
            }

        } catch (const std::exception& e) {
            tc::logError("MCP") << "JSON parse error: " << e.what();
        }
    }

private:
    std::map<std::string, Tool> tools_;
    std::map<std::string, Resource> resources_;

    void handleRequest(const json& req) {
        std::string method = req["method"];
        auto id = req.contains("id") ? req["id"] : json(nullptr);

        if (method == "initialize") {
            handleInitialize(req, id);
        } else if (method == "tools/list") {
            handleToolsList(req, id);
        } else if (method == "tools/call") {
            handleToolsCall(req, id);
        } else if (method == "resources/list") {
            handleResourcesList(req, id);
        } else if (method == "resources/read") {
            handleResourcesRead(req, id);
        } else {
            // Unknown method
            // Only reply if it's a request (has id)
            if (!id.is_null()) {
                sendError(id, -32601, "Method not found: " + method);
            }
        }
    }

    void handleInitialize(const json& req, const json& id) {
        json result = {
            {"protocolVersion", "2024-11-05"},
            {"server", {
                {"name", "TrussC App"},
                {"version", "0.0.1"}
            }},
            {"capabilities", {
                {"tools", {}},
                {"resources", {}}
            }}
        };
        sendResult(id, result);
    }

    void handleToolsList(const json& req, const json& id) {
        json toolList = json::array();
        for (const auto& [name, tool] : tools_) {
            toolList.push_back({
                {"name", tool.name},
                {"description", tool.description},
                {"inputSchema", tool.getSchema()}
            });
        }
        sendResult(id, {{"tools", toolList}});
    }

    void handleToolsCall(const json& req, const json& id) {
        auto params = req["params"];
        std::string name = params["name"];
        auto args = params["arguments"];

        if (tools_.find(name) == tools_.end()) {
            sendError(id, -32601, "Tool not found: " + name);
            return;
        }

        try {
            // Execute tool handler
            json content = tools_[name].handler(args);
            
            // Format result according to MCP spec
            // content should be a list of {type, text} objects, or just a simple result object
            // For simplicity, we wrap simple results in text
            json result;
            if (content.is_array() && content.size() > 0 && content[0].contains("type")) {
                 result = {{"content", content}};
            } else {
                 result = {{"content", {{
                     {"type", "text"},
                     {"text", content.dump()}
                 }}}};
            }
            sendResult(id, result);

        } catch (const std::exception& e) {
            sendError(id, -32000, std::string("Tool execution error: ") + e.what());
        }
    }

    void handleResourcesList(const json& req, const json& id) {
        json resList = json::array();
        for (const auto& [uri, res] : resources_) {
            resList.push_back({
                {"uri", res.uri},
                {"name", res.name},
                {"description", res.description},
                {"mimeType", res.mimeType.empty() ? nullptr : json(res.mimeType)}
            });
        }
        sendResult(id, {{"resources", resList}});
    }

    void handleResourcesRead(const json& req, const json& id) {
        std::string uri = req["params"]["uri"];
        
        if (resources_.find(uri) == resources_.end()) {
            sendError(id, -32602, "Resource not found: " + uri);
            return;
        }

        try {
            std::string content = resources_[uri].handler();
            json resourceContent = {
                {"uri", uri},
                {"mimeType", resources_[uri].mimeType}
            };

            // If mimeType starts with "text/", treat as text, otherwise blob (base64)
            // For simplicity, current implementation assumes handler returns text/json
            // TODO: Implement is_binary check and base64 handling
            resourceContent["text"] = content;

            sendResult(id, {{"contents", {resourceContent}}});

        } catch (const std::exception& e) {
            sendError(id, -32000, std::string("Resource read error: ") + e.what());
        }
    }

    void sendResult(const json& id, const json& result) {
        if (id.is_null()) return; // Notification, no reply
        json res = {
            {"jsonrpc", "2.0"},
            {"id", id},
            {"result", result}
        };
        // Ensure output to stdout (MCP transport)
        std::cout << res.dump() << std::endl;
    }

    void sendError(const json& id, int code, const std::string& message) {
        if (id.is_null()) return;
        json res = {
            {"jsonrpc", "2.0"},
            {"id", id},
            {"error", {
                {"code", code},
                {"message", message}
            }}
        };
        std::cout << res.dump() << std::endl;
    }
};

// ---------------------------------------------------------------------------
// Argument Type Traits & Builder Helpers
// ---------------------------------------------------------------------------

template<typename T> struct TypeName { static constexpr const char* value = "string"; };
template<> struct TypeName<int> { static constexpr const char* value = "integer"; };
template<> struct TypeName<float> { static constexpr const char* value = "number"; };
template<> struct TypeName<double> { static constexpr const char* value = "number"; };
template<> struct TypeName<bool> { static constexpr const char* value = "boolean"; };
template<> struct TypeName<json> { static constexpr const char* value = "object"; };

// Tool Builder
class ToolBuilder {
public:
    ToolBuilder(const std::string& name, const std::string& desc) {
        tool_.name = name;
        tool_.description = desc;
    }

    template<typename T>
    ToolBuilder& arg(const std::string& name, const std::string& desc, bool required = true) {
        tool_.args.push_back({name, TypeName<T>::value, desc, required});
        return *this;
    }

    // Bind generic lambda
    // Note: This does simple JSON parsing. For stricter type safety, 
    // we could use variadic templates to unpack arguments automatically.
    // Here we use a simpler approach where the user receives 'json' args if they want full control,
    // or we can implement automatic unpacking later.
    
    // Simple bind: function receives (const json& args)
    void bind(std::function<json(const json&)> func) {
        tool_.handler = func;
        Server::instance().registerTool(tool_);
    }

    // Typed bind helpers (up to 4 args for simplicity)
    
    // 0 args
    void bind(std::function<json()> func) {
        tool_.handler = [func](const json&) { return func(); };
        Server::instance().registerTool(tool_);
    }

    // 1 arg
    template<typename T1>
    void bind(std::function<json(T1)> func) {
        auto argName1 = tool_.args[0].name;
        tool_.handler = [func, argName1](const json& args) {
            return func(args.at(argName1).get<T1>());
        };
        Server::instance().registerTool(tool_);
    }

    // 2 args
    template<typename T1, typename T2>
    void bind(std::function<json(T1, T2)> func) {
        auto argName1 = tool_.args[0].name;
        auto argName2 = tool_.args[1].name;
        tool_.handler = [func, argName1, argName2](const json& args) {
            return func(args.at(argName1).get<T1>(), args.at(argName2).get<T2>());
        };
        Server::instance().registerTool(tool_);
    }
    
    // 3 args
    template<typename T1, typename T2, typename T3>
    void bind(std::function<json(T1, T2, T3)> func) {
        auto a1 = tool_.args[0].name;
        auto a2 = tool_.args[1].name;
        auto a3 = tool_.args[2].name;
        tool_.handler = [func, a1, a2, a3](const json& args) {
            return func(args.at(a1).get<T1>(), args.at(a2).get<T2>(), args.at(a3).get<T3>());
        };
        Server::instance().registerTool(tool_);
    }

private:
    Tool tool_;
};

// Resource Builder
class ResourceBuilder {
public:
    ResourceBuilder(const std::string& uri, const std::string& name) {
        res_.uri = uri;
        res_.name = name;
    }

    ResourceBuilder& desc(const std::string& d) {
        res_.description = d;
        return *this;
    }

    ResourceBuilder& mime(const std::string& m) {
        res_.mimeType = m;
        return *this;
    }

    void bind(std::function<std::string()> func) {
        res_.handler = func;
        Server::instance().registerResource(res_);
    }

private:
    Resource res_;
};

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

inline ToolBuilder tool(const std::string& name, const std::string& desc) {
    return ToolBuilder(name, desc);
}

inline ResourceBuilder resource(const std::string& uri, const std::string& name) {
    return ResourceBuilder(uri, name);
}

inline void processInput(const std::string& input) {
    Server::instance().processMessage(input);
}

} // namespace mcp
} // namespace trussc
