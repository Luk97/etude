#pragma once

#include <etude/ui/interaction.h>

#include <concepts>
#include <memory>

namespace etude::ui {

    /// @brief The children of a layout: a lambda that adds them while the layout is open.
    ///
    /// The lambda takes either nothing or the interaction of its layout, for children that depend on it, such as the
    /// square of a checkbox that lights up while the mouse is over the whole row. Content only points to the lambda and
    /// does not copy it, so handing it on costs nothing. It must not outlive the call that it was given to, which a
    /// lambda written right into the call of a layout never does. A content without a lambda adds no children.
    class Content {
    public:
        /// @brief Adds no children.
        Content() = default;

        /// @brief Points to the lambda. Not explicit, so that a lambda can stand right where a layout expects its
        /// content.
        template <typename Function>
            requires(!std::same_as<Function, Content> &&
                     (std::invocable<const Function&> || std::invocable<const Function&, const Interaction&>))
        Content(const Function& function)
            : lambda(std::addressof(function)),
              call([](const void* stored, [[maybe_unused]] const Interaction& interaction) {
                  const Function& target = *static_cast<const Function*>(stored);
                  if constexpr (std::invocable<const Function&, const Interaction&>) {
                      target(interaction);
                  } else {
                      target();
                  }
              }) {}

        /// @brief Lets the lambda add the children, if there is one.
        void operator()(const Interaction& interaction) const {
            if (call) {
                call(lambda, interaction);
            }
        }

    private:
        /// @brief The lambda, whose type only call still knows.
        const void* lambda = nullptr;

        /// @brief Calls the lambda, with the interaction if it takes one.
        void (*call)(const void*, const Interaction&) = nullptr;
    };
}
