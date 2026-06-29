#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>

namespace exd::app {

/// @brief A single undoable action.
///
/// Implement for every user action that should be undoable (translate object,
/// change BC value, delete entity, etc.).
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute() = 0;
    virtual void undo()    = 0;
    [[nodiscard]] virtual std::string description() const = 0;
};

/// @brief Stack of undoable commands with redo support.
///
/// Usage:
/// @code
///   stack.push<TranslateCommand>(entity, offset);
///   stack.undo();
///   stack.redo();
/// @endcode
class CommandStack {
public:
    CommandStack() = default;

    template <typename T, typename... Args>
    void push(Args&&... args) {
        auto cmd = std::make_unique<T>(std::forward<Args>(args)...);
        cmd->execute();
        // Clear redo stack
        redo_stack_.clear();
        undo_stack_.push_back(std::move(cmd));
    }

    void undo();
    void redo();
    void clear();

    [[nodiscard]] bool can_undo() const { return !undo_stack_.empty(); }
    [[nodiscard]] bool can_redo() const { return !redo_stack_.empty(); }
    [[nodiscard]] const std::string& undo_description() const;
    [[nodiscard]] const std::string& redo_description() const;

    /// Callback invoked when stack state changes (for UI updates).
    void on_change(std::function<void()> callback) { change_callback_ = std::move(callback); }

private:
    std::vector<std::unique_ptr<ICommand>> undo_stack_;
    std::vector<std::unique_ptr<ICommand>> redo_stack_;
    std::function<void()> change_callback_;
};

} // namespace exd::app
