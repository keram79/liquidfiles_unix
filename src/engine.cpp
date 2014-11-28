#include "engine.h"
#include "exceptions.h"
#include "messenger.h"

#include <vector>

namespace lf {

namespace {

unsigned s_normal_attach_responce_size = 22;

std::string s_data;

size_t data_get(void* ptr, size_t size, size_t nmemb, void*)
{
    s_data += std::string(static_cast<char*>(ptr), static_cast<char*>(ptr) + nmemb);
    return size * nmemb;
}

}

void engine::init_curl(std::string key, report_level s, validate_cert v)
{
    if (m_curl != 0) {
        return;
    }
    m_curl = curl_easy_init();
    if (m_curl == 0) {
        throw curl_error("Failed to initialize CURL");
    }
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &data_get);
    key += ":x";
    curl_easy_setopt(m_curl, CURLOPT_USERPWD, key.c_str());
    if (v == NOT_VALIDATE) {
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, false);
    }
    if (s == VERBOSE) {
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);
    }
}

engine::engine()
    : m_curl(0)
{
}

engine::~engine()
{
    if (m_curl != 0) {
        curl_easy_cleanup(m_curl);
        m_curl = 0;
    }
}

std::string engine::send(std::string server, const std::string& user,
        std::string key, const std::string& subject,
        const std::string& message, const files& fs,
        report_level s, validate_cert v)
{
    init_curl(key, s, v);
    std::set<std::string> attachments;
    files::const_iterator i = fs.begin();
    for (; i != fs.end(); ++i) {
        std::string a = attach(server, *i, s);
        attachments.insert(a);
    }
    return send_attachments(server, user, subject, message,
            attachments, s);
}

std::string engine::attach(std::string server, const std::string& file,
        report_level s)
{
    server += "/attachments";
    curl_easy_setopt(m_curl, CURLOPT_URL, server.c_str());
    struct curl_httppost* formpost=NULL;
    struct curl_httppost* lastptr=NULL;
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "Filedata",
               CURLFORM_FILE, file.c_str(),
               CURLFORM_END);
    curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, formpost);
    if (s >= NORMAL) {
        messenger::get() << "Uploading file '" << file << "'.";
        messenger::get().endline();
    }
    CURLcode res = curl_easy_perform(m_curl);
    curl_formfree(formpost);
    if(res != CURLE_OK) {
        throw curl_error(std::string(curl_easy_strerror(res)));
    }
    std::string r = s_data;
    process_attach_responce(r, s);
    s_data.clear();
    return r;
}

std::string engine::send_attachments(std::string server,
        const std::string& user,
        const std::string& subject, const std::string& message,
        const files& fs, report_level s)
{
    server += "/message";
    curl_easy_setopt(m_curl, CURLOPT_URL, server.c_str());
    struct curl_slist* slist = 0;
    slist = curl_slist_append(slist, "Content-Type: text/xml");
    curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, slist);
    std::string data = std::string(
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\
  <message>\
    <recipients type=\"array\">\
      <recipient>") + user + std::string("</recipient>\
    </recipients>\
    <subject>") + subject + std::string("</subject>\
    <message>") + message + std::string("</message>\
    <send_email>true</send_email>\
    <authorization>3</authorization>\
    <attachments type='array'>\
");
    for (files::const_iterator i = fs.begin(); i != fs.end(); ++i) {
        data += "      <attachment>";
        data += *i;
        data += "</attachment>\n";
    }
    data += "    </attachments>\
  </message>\n";
    curl_easy_setopt(m_curl, CURLOPT_HTTPPOST, 0);
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, data.c_str()); 
    if (s >= NORMAL) {
        messenger::get() << "Sending message to user '" << user << "'";
        messenger::get().endline();
    }
    CURLcode res = curl_easy_perform(m_curl);
    curl_slist_free_all(slist);
    if(res != CURLE_OK) {
        throw curl_error(std::string(curl_easy_strerror(res)));
    }
    std::string r = s_data;
    s_data.clear();
    return process_send_responce(r, s);
}

void engine::process_attach_responce(const std::string& r, report_level s) const
{
    if (r.size() == s_normal_attach_responce_size) {
        if (s >= NORMAL) {
            messenger::get() << "File uploaded successfully. ID: " << r;
            messenger::get().endline();
        }
        return;
    }
    throw upload_error(r);
}

std::string engine::process_send_responce(const std::string& r,
        report_level s) const
{
    size_t f = r.rfind("Message ID: ");
    if (f != std::string::npos) {
        size_t e = r.find('\n', f + 12);
        if (e != std::string::npos) {
            std::string q = r.substr(f + 12, e - f - 12);
            if (s >= NORMAL) {
                messenger::get() << "Message sent successfully. ID: " << q;
                messenger::get().endline();
            }
            return q;
        }
    }
    throw send_error(r);
    return "";
}

}