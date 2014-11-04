#include "send_command.h"
#include "engine.h"
#include "exceptions.h"

namespace lf {

send_command::send_command(engine& e)
    : command("send", "Sends the file to the server")
    , m_engine(e)
{
}

void send_command::execute(const arguments& args)
{
    std::string user = args["--to"];
    if (user == "") {
        throw missing_argument("--to");
    }
    std::string server = args["--server"];
    if (server == "") {
        throw missing_argument("--server");
    }
    std::string api_key = args["--api_key"];
    if (api_key == "") {
        throw missing_argument("--api_key");
    }
    engine::report_level rl = engine::NORMAL;
    std::string rls = args["--report_level"];
    if (rls == "silent") {
        rl = engine::SILENT;
    } else if (rls == "verbose") {
        rl = engine::VERBOSE;
    } else if (rls != "" && rls != "normal") {
        throw exception("Invalid value for argument '--report_level'.\n"
                "    Valid values: silent, normal, verbose.");
    }
    std::string subject = args["--subject"];
    std::string message = args["--message"];
    std::set<std::string> unnamed_args = args.get_unnamed_arguments();
    engine::validate_cert val = engine::VALIDATE;
    if (unnamed_args.find("-k") != unnamed_args.end()) {
        val = engine::NOT_VALIDATE;
        unnamed_args.erase("-k");
    }
    m_engine.send(server, user, api_key, subject, message, unnamed_args,
            rl, val);
}

}
