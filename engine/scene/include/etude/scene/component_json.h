#pragma once

#include <etude/json/json.h>
#include <etude/math/vec2.h>
#include <etude/scene/reflection.h>

#include <expected>
#include <format>
#include <string>
#include <tuple>

namespace etude {

    /// @brief Turns the value of a field into JSON. A float takes its own shortest digits wherever they read back to
    /// the same float, so that 0.1f appears as 0.1 and not with the 17 digits of the double it becomes. A Vec2 becomes
    /// an array of x and y.
    Json toJson(float value);
    Json toJson(int value);
    Json toJson(const std::string& value);
    Json toJson(Vec2 value);

    /// @brief Reads the value of a field from JSON, or returns what was expected instead. An int has to be a whole
    /// number, and every number has to lie within the range of its type.
    std::expected<void, std::string> fromJson(const Json& json, float& value);
    std::expected<void, std::string> fromJson(const Json& json, int& value);
    std::expected<void, std::string> fromJson(const Json& json, std::string& value);
    std::expected<void, std::string> fromJson(const Json& json, Vec2& value);

    /// @brief Writes the component as a JSON object with one member per field, in the order of the field list.
    template <Reflected Component>
    Json writeComponent(const Component& component) {
        return std::apply(
            [&](const auto&... field) {
                return Json::Object{{std::string(field.name), toJson(component.*field.member)}...};
            },
            Component::fields
        );
    }

    /// @brief Reads a component from a JSON object as writeComponent writes it. Fields that the object leaves out
    /// keep their default values, so that older scenes still load after a component gets a new field. A member that is
    /// no field is an error, because it is most likely a typing mistake.
    template <Reflected Component>
    std::expected<Component, std::string> readComponent(const Json& json) {
        const Json::Object* object = json.tryGet<Json::Object>();
        if (object == nullptr) {
            return std::unexpected(std::format("{} has to be an object", Component::typeName));
        }
        for (const auto& member : *object) {
            if (!hasField<Component>(member.first)) {
                return std::unexpected(std::format("{} has no field {}", Component::typeName, member.first));
            }
        }

        Component component;
        std::string error;
        const auto readField = [&](const auto& field) {
            const Json* value = json.find(field.name);
            if (value == nullptr) {
                return true;
            }
            const auto read = fromJson(*value, component.*field.member);
            if (!read) {
                error = std::format("{}.{}: {}", Component::typeName, field.name, read.error());
            }
            return read.has_value();
        };

        // && stops at the first field that cannot be read.
        if (!std::apply([&](const auto&... field) { return (... && readField(field)); }, Component::fields)) {
            return std::unexpected(error);
        }
        return component;
    }
}
