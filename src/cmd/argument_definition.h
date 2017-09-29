#pragma once

#include "arguments.h"
#include "argument_type.h"
#include "exceptions.h"

#include <memory>
#include <set>

namespace cmd {

class argument_definition_object
{
public:
    template <typename T>
    argument_definition_object(T t)
        : m_self{new model<T>(t)}
    {
    }

    std::string usage() const
    {
        return m_self->usage_();
    }

    std::string full_description() const
    {
        return m_self->full_description_();
    }

private:
    class concept
    {
    public:
        virtual ~concept()
        {
        }

        virtual std::string usage_() const = 0;
        virtual std::string full_description_() const = 0;
    };

    template <typename T>
    class model : public concept
    {
    public:
        model(T t)
            : m_data{t}
        {
        }

        virtual std::string usage_() const
        {
            return m_data.usage();
        }

        virtual std::string full_description_() const
        {
            return m_data.full_description();
        }

    private:
        T m_data;
    };

    std::shared_ptr<concept> m_self;
};

class argument_definition_container : public std::vector<argument_definition_object>
{
public:
    std::string usage() const
    {
        std::string ret;
        for (const auto& i : (*this)) {
            ret += i.usage();
            ret += " ";
        }
        return ret;
    }

    std::string full_description() const
    {
        std::string ret;
        for (const auto& i : (*this)) {
            ret += i.full_description();
        }
        return ret;
    }
};

enum class argument_name_type {
    unnamed,
    boolean,
    named
};

template <argument_name_type t, bool r>
class argument_definition_base
{
public:
    /// @brief Returns usage string of argument.
    std::string usage() const;

    /// @brief Returns full description of argument.
    std::string full_description() const;
};

template <bool r>
class argument_definition_base<argument_name_type::unnamed, r>
{
public:
    argument_definition_base(const std::string& type_string,
            const std::string& description)
        : m_type_string{type_string}
        , m_description{description}
    {
    }

    std::string usage() const
    {
        std::string ret;
        if (!r) {
            ret += '[';
        }
        ret += m_type_string;
        if (!r) {
            ret += ']';
        }
        return ret;
    }

    std::string full_description() const
    {
        std::string ret = "\t";
        ret += m_type_string;
        ret += "\n\t    ";
        ret += m_description;
        return ret;
    }

    const std::string& type_string() const
    {
        return m_type_string;
    }

private:
    std::string m_type_string;
    std::string m_description;
};

template <bool r>
class argument_definition_base<argument_name_type::boolean, r>
{
public:
    argument_definition_base(const std::string& name,
            const std::string& description)
        : m_name{name}
        , m_description{description}
    {
    }

    std::string name() const
    {
        return std::string("-") + m_name;
    }

    std::string usage() const
    {
        std::string ret;
        if (!r) {
            ret += '[';
        }
        ret += name();
        if (!r) {
            ret += ']';
        }
        return ret;
    }

    std::string full_description() const
    {
        std::string ret = "\t";
        ret += name();
        ret += "\n\t    ";
        ret += m_description;
        return ret;
    }

private:
    std::string m_name;
    std::string m_description;
};

template <bool r>
class argument_definition_base<argument_name_type::named, r>
{
public:
    argument_definition_base(const std::string& name,
            const std::string& type_string,
            const std::string& description)
        : m_name{name}
        , m_type_string{type_string}
        , m_description{description}
    {
    }

    std::string name() const
    {
        return std::string("--") + m_name;
    }

    std::string usage() const
    {
        std::string ret;
        if (!r) {
            ret += '[';
        }
        ret += name();
        ret += '=';
        ret += m_type_string;
        if (!r) {
            ret += ']';
        }
        return ret;
    }

    std::string full_description() const
    {
        std::string ret = "\t";
        ret += name();
        ret += "\n\t    ";
        ret += m_description;
        return ret;
    }

private:
    std::string m_name;
    std::string m_type_string;
    std::string m_description;
};

template <typename T, argument_name_type t, bool r>
class argument_definition
{
public:
    /// @brief Returns usage string of argument.
    std::string usage() const;

    /// @brief Returns full description of argument.
    std::string full_description() const;

    /// @brief Gets the value of argument.
    T get_value(const arguments& a) const;
};

template <typename T, bool r>
class argument_definition<T, argument_name_type::unnamed, r> :
    public argument_definition_base<argument_name_type::unnamed, r>
{
    using parent = argument_definition_base<argument_name_type::unnamed, r>;

public:
    argument_definition(const std::string& type_string,
            const std::string& description)
        : parent{type_string, description}
    {
    }

    std::string full_description() const
    {
        std::string ret = parent::full_description();
        ret += "\n\n";
        return ret;
    }

    std::set<T> value(const arguments& a) const
    {
        auto& v = a.unnamed_arguments;
        if (r && v.empty()) {
            throw missing_argument(parent::type_string());
        }
        std::set<T> s;
        std::set<std::string>::const_iterator i = v.begin();
        while (i != v.end()) {
            s.insert(string_to_val<T>(*i++));
        }
        return s;
    }
};

template <typename T, bool r>
class argument_definition<T, argument_name_type::boolean, r> :
    public argument_definition_base<argument_name_type::boolean, r>
{
    using parent = argument_definition_base<argument_name_type::boolean, r>;

public:
    argument_definition(const std::string& name,
            const std::string& description)
        : parent{name, description}
    {
    }

    std::string full_description() const
    {
        std::string ret = parent::full_description();
        std::string p = possible_values<T>();
        if (!p.empty()) {
            ret += "\n\t    ";
            ret += p;
        }
        ret += "\n\n";
        return ret;
    }

    T value(const arguments& a)
    {
        return T(get_value(a));
    }

private:
    bool get_value(const arguments& a)
    {
        auto& b = a.boolean_arguments;
        if (b.find(parent::name()) != b.end()) {
            return true;
        }
        return false;
    }
};

template <typename T>
class argument_definition<T, argument_name_type::named, true> :
    public argument_definition_base<argument_name_type::named, true>
{
    using parent = argument_definition_base<argument_name_type::named, true>;

public:
    argument_definition(const std::string& name,
            const std::string& type_string,
            const std::string& description)
        : parent{name, type_string, description}
    {
    }

    std::string full_description() const
    {
        std::string ret = parent::full_description();
        std::string p = possible_values<T>();
        if (!p.empty()) {
            ret += "\n\t    ";
            ret += p;
        }
        ret += "\n\n";
        return ret;
    }

    T value(const arguments& a)
    {
        return string_to_val<T>(get_value(a));
    }

private:
    std::string get_value(const arguments& a)
    {
        if (!a.exists(parent::name())) {
            throw missing_argument(parent::name());
        }
        return a[parent::name()];
    }
};

template <typename T>
class argument_definition<T, argument_name_type::named, false> :
    public argument_definition_base<argument_name_type::named, false>
{
    using parent = argument_definition_base<argument_name_type::named, false>;

public:
    argument_definition(const std::string& name,
            const std::string& type_string,
            const std::string& description,
            T default_value)
        : parent{name, type_string, description}
        , m_default_value{default_value}
        , m_default_value_specified{true}
    {
    }

    argument_definition(const std::string& name,
            const std::string& type_string,
            const std::string& description)
        : parent{name, type_string, description}
        , m_default_value{}
        , m_default_value_specified{false}
    {
    }

    std::string full_description() const
    {
        std::string ret = parent::full_description();
        std::string p = possible_values<T>();
        if (!p.empty()) {
            ret += "\n\t    ";
            ret += p;
        }
        if (m_default_value_specified) {
            ret += "\n\t    ";
            ret += "Default value: \"";
            ret += val_to_string<T>(m_default_value);
            ret += "\".";
        }
        ret += "\n\n";
        return ret;
    }

    T value(const arguments& a)
    {
        if (!a.exists(name())) {
            return m_default_value;
        }
        return string_to_val<T>(a[name()]);
    }

private:
    T m_default_value;
    bool m_default_value_specified;
};

}
