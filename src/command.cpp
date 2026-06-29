#include <exd/app/command.hpp>
#include <stdexcept>

namespace exd::app {

void CommandStack::undo() {
    if (undo_stack_.empty()) return;
    auto& cmd = undo_stack_.back();
    cmd->undo();
    redo_stack_.push_back(std::move(cmd));
    undo_stack_.pop_back();
    if (change_callback_) change_callback_();
}

void CommandStack::redo() {
    if (redo_stack_.empty()) return;
    auto& cmd = redo_stack_.back();
    cmd->execute();
    undo_stack_.push_back(std::move(cmd));
    redo_stack_.pop_back();
    if (change_callback_) change_callback_();
}

void CommandStack::clear() {
    undo_stack_.clear();
    redo_stack_.clear();
    if (change_callback_) change_callback_();
}

const std::string& CommandStack::undo_description() const {
    static std::string empty;
    if (undo_stack_.empty()) return empty;
    return undo_stack_.back()->description();
}

const std::string& CommandStack::redo_description() const {
    static std::string empty;
    if (redo_stack_.empty()) return empty;
    return redo_stack_.back()->description();
}

} // namespace exd::app
