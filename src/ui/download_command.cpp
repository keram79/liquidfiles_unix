#include "download_command.h"
#include "credentials.h"

#include <cmd/exceptions.h>
#include <lf/declarations.h>
#include <lf/engine.h>

namespace ui {

download_command::download_command(lf::engine& e)
    : cmd::command("download",
            "[-k] [--api_key=<key>] [-s] [--report_level=<level>] [--download_to=<path>]\n"
            "\t(([--server=<server>] (--message_id=<id> | --sent_in_the_last=<HOURS> | --sent_after=<YYYYMMDD>)) \n"
            "\t| <url>...)",
            "Download given files.",
            credentials::arg_descriptions() +
            "\t--report_level\n"
            "\t    Level of reporting.\n"
            "\t    Valid values: silent, normal, verbose.\n"
            "\t    Default value: normal.\n\n"
            "\t--download_to\n"
            "\t    Directory path to download files there. Default value: \"\"\n\n"
            "\t--message_id\n"
            "\t    Message id to download attachments of it.\n\n"
            "\t--sent_in_the_last\n"
            "\t    Download files sent in the last specified hours.\n\n"
            "\t--sent_after\n"
            "\t    Download files sent after specified date.\n\n"
            "\t<url>...\n"
            "\t    Url(s) of files to download."
            )
    , m_engine(e)
{
}

void download_command::execute(const cmd::arguments& args)
{
    credentials c;
    try {
        c = credentials::manage(args);
    } catch (cmd::missing_argument&) {
        if (c.api_key() == "") {
            throw;
        }
    }
    const std::string& path = args["--download_to"];
    lf::report_level rl = lf::NORMAL;
    const std::string& rls = args["--report_level"];
    if (rls == "silent") {
        rl = lf::SILENT;
    } else if (rls == "verbose") {
        rl = lf::VERBOSE;
    } else if (rls != "" && rls != "normal") {
        throw cmd::invalid_argument_value("--report_level",
                "silent, normal, verbose");
    }
    const std::string& l = args["--sent_in_the_last"];
    const std::string& f = args["--sent_after"];
    const std::string& id = args["--message_id"];
    const std::set<std::string>& unnamed_args = args.get_unnamed_arguments();
    if (!c.server().empty()) {
        if (!id.empty()) {
            m_engine.download(c.server(), c.api_key(), path, id, rl, c.validate_flag());
        }
        if (!l.empty() || !f.empty()) {
            m_engine.download(c.server(), c.api_key(), path, l, f, rl, c.validate_flag());
        }
    }
    m_engine.download(unnamed_args, c.api_key(), path, rl, c.validate_flag());
}

}