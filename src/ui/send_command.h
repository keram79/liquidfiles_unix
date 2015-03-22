#pragma once

#include <cmd/command.h>

namespace lf {
class engine;
}

namespace ui {

/**
 * @class send_command.
 * @brief Class for 'send' command.
 */
class send_command : public cmd::command
{
public:
    /// @brief Constructor.
    /// @param e Engine.
    send_command(lf::engine& e);

public:
    /// @brief Executes command by given arguments.
    virtual void execute(const cmd::arguments& args);

private:
    lf::engine& m_engine;
};

}