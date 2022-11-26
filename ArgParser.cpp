#include "ArgParser.h"
#include "GenericView.h"
#include "Printer.h"

namespace ARLib {
    ArgParser::ArgParser(int argc, char** argv) {
        size_t argcs = static_cast<size_t>(argc);
        m_arguments.reserve(argcs - 1);
        m_program_name = String{argv[0]};
        for (size_t i = 1; i < argcs; i++) {
            m_arguments.push_back(argv[i]);
        }
    }
    ArgParser::ArgParser(int argc, const char** argv) {
        size_t argcs = static_cast<size_t>(argc);
        m_arguments.reserve(argcs - 1);
        m_program_name = String{argv[0]};
        for (size_t i = 1; i < argcs; i++) {
            m_arguments.push_back(argv[i]);
        }
    }

    void ArgParser::add_version(uint8_t version_partial, uint8_t version_edition) {
        m_version_partial = version_partial;
        m_version_edition = version_edition;
    }

    void ArgParser::allow_unmatched(size_t quantity) {
        m_leftover_args_needed = quantity;
    }

    void ArgParser::add_usage_string(StringView usage_string) {
        m_usage_string = usage_string;
    }

    ArgParser::ParseResult ArgParser::parse() {
        auto help_it = m_arguments.find([](const auto& v) { return v == "-h" || v == "--help"; });
        if (help_it != m_arguments.end()) {
            m_help_requested = true;
            m_arguments.remove(help_it);
            return DefaultOk{};
        }
        for (auto& [name, opt] : m_options) {
            auto it = m_arguments.find(name);
            if (it != m_arguments.end()) {
                if (opt.found) {
                    return ArgParserError{
                    Printer::format("Argument parsing error: Option \"{}\" was specified twice\n", name)};
                }
                opt.found = true;
                it = m_arguments.remove(it);
                if (opt.requires_value()) {
                    if (it == m_arguments.end()) {
                        return ArgParserError{Printer::format(
                        "Argument parsing error: Option \"{}\" requires an argument \"{}\" but it wasn't given any\n",
                        name, opt.value_name)};
                    }
                    if (opt.type == Option::Type::String) {
                        if (!opt.assign(*it)) {
                            return ArgParserError{
                            "Internal argument parser error, report this to the developer along with the command line you were using!\n"_s};
                        }
                    } else if (opt.type == Option::Type::Int) {
                        int value = opt.value.get<IntRef>().get();
                        const auto& strval = *it;
                        value = StrViewToInt(strval);
                        if (!opt.assign(value)) {
                            return ArgParserError{
                            "Internal argument parser error, report this to the developer along with the command line you were using!\n"_s};
                        }
                    } else if (opt.type == Option::Type::Uint) {
                        unsigned int value = opt.value.get<UintRef>().get();
                        const auto& strval = *it;
                        value = StrViewToUInt(strval);
                        if (!opt.assign(value)) {
                            return ArgParserError{
                            "Internal argument parser error, report this to the developer along with the command line you were using!\n"_s};
                        }
                    } else if (opt.type == Option::Type::Real) {
                        double value = opt.value.get<RealRef>().get();
                        const auto& strval = *it;
                        value = StrViewToDouble(strval);
                        if (!opt.assign(value)) {
                            return ArgParserError{
                            "Internal argument parser error, report this to the developer along with the command line you were using!\n"_s};
                        }
                    }
                    m_arguments.remove(*it);
                } else if (opt.type == Option::Type::Bool) {
                    if (!opt.assign(true)) {
                        return ArgParserError{
                        "Internal argument parser error, report this to the developer along with the command line you were using!\n"_s};
                    }
                }
            }
        }
        m_unmatched_arguments.reserve(m_arguments.size());
        for (const auto& arg : m_arguments) {
            m_unmatched_arguments.append(String{arg});
        }
        if (m_unmatched_arguments.size() > m_leftover_args_needed) {
            String s{"Argument parsing error: Unrecognized options found: "};
            for (const auto& unm : m_unmatched_arguments) {
                s += "\""_s + unm + "\", "_s;
            }
            s += '\n';
            return ArgParserError{move(s)};
        }
        return DefaultOk{};
    }

    bool ArgParser::help_requested() const {
        return m_help_requested;
    }

    ArgParser& ArgParser::add_option(StringView opt_name, StringView value_name, StringView description,
                                     String& value_ref) {
        m_options.push_back(OptT{opt_name, Option{description, value_name, StringRef{value_ref}}});
        return *this;
    }

    ArgParser& ArgParser::add_option(StringView opt_name, StringView description, bool& value_ref) {
        m_options.push_back(OptT{opt_name, Option{description, StringView{}, BoolRef{value_ref}}});
        return *this;
    }

    ArgParser& ArgParser::add_option(StringView opt_name, StringView description, NoValueTag) {
        m_options.push_back(OptT{opt_name, Option{description, StringView{}, NoValueTag{}}});
        return *this;
    }

    ArgParser& ArgParser::add_option(StringView opt_name, StringView value_name, StringView description,
                                     int& value_ref) {
        m_options.push_back(OptT{opt_name, Option{description, value_name, IntRef{value_ref}}});
        return *this;
    }
    ArgParser& ArgParser::add_option(StringView opt_name, StringView value_name, StringView description,
                                     unsigned int& value_ref) {
        m_options.push_back(OptT{opt_name, Option{description, value_name, UintRef{value_ref}}});
        return *this;
    }
    ArgParser& ArgParser::add_option(StringView opt_name, StringView value_name, StringView description,
                                     double& value_ref) {
        m_options.push_back(OptT{opt_name, Option{description, value_name, RealRef{value_ref}}});
        return *this;
    }

    String ArgParser::construct_help_string() const {
        String builder{};
        builder +=
        Printer::format("{}, Version: {}.{}\n", m_program_name, (int)m_version_edition, (int)m_version_partial);
        if (m_usage_string.size() == 0) {
            builder += "Usage: "_s + m_program_name + " <options> "_s;
            if (m_leftover_args_needed > 0) {
                builder += Printer::format(" [required {} arguments]", m_leftover_args_needed);
            };
            builder += '\n';
        } else {
            builder += String{m_usage_string} + '\n';
        }
        builder += "Options:\n";
        Vector<String> name_value_list{};
        name_value_list.reserve(m_options.size());

        for (const auto& [name, opt] : m_options) {
            if (!(opt.value_name.size() == 0)) {
                name_value_list.push_back(String{name} + " <" + String{opt.value_name} + ">");
            } else {
                name_value_list.push_back(String{name});
            }
        }
        size_t needed_width = *max(IteratorView{name_value_list}.transform_map<Vector<size_t>>(
                              [](const String& s) -> size_t { return s.size(); })) +
                              2;
        size_t idx = 0;
        for (const auto& [name, opt] : m_options) {
            builder += '\t';
            const auto& name_value = name_value_list[idx++];
            builder += name_value + String{needed_width - name_value.size(), ' '};
            builder += String{opt.description};
            if (opt.has_default()) {
                if (opt.type == Option::Type::String) {
                    const auto& str = opt.value.get<StringRef>().get();
                    builder += Printer::format(" (Default value: \"{}\")", (str.size() == 0 ? String{"<empty>"} : str));
                } else if (opt.type == Option::Type::Bool) {
                    const auto& boolv = opt.value.get<BoolRef>().get();
                    builder += Printer::format(" (Default value: {})", boolv);
                } else if (opt.type == Option::Type::Int) {
                    const auto& intv = opt.value.get<IntRef>().get();
                    builder += Printer::format(" (Default value: {})", intv);
                } else if (opt.type == Option::Type::Uint) {
                    const auto& uintv = opt.value.get<UintRef>().get();
                    builder += Printer::format(" (Default value: {})", uintv);
                } else if (opt.type == Option::Type::Real) {
                    const auto& realv = opt.value.get<RealRef>().get();
                    builder += Printer::format(" (Default value: {})", realv);
                }
            }
            builder += '\n';
        }
        return builder;
    }

    const String& ArgParser::help_string() const {
        if (m_help_string.is_empty()) { m_help_string = construct_help_string(); }
        return m_help_string;
    }

    void ArgParser::print_help() const {
        Printer::print("{}", help_string());
    }

    bool ArgParser::Option::requires_value() const {
        return type == Type::Int || type == Type::String || type == Type::Uint || type == Type::Real;
    }

    bool ArgParser::Option::assign(StringView arg_value) {
        if (type == Type::String) {
            value.get<StringRef>().get() = String{arg_value};
        } else {
            return false;
        }
        return true;
    }

    bool ArgParser::Option::assign(bool arg_value) {
        if (type == Type::Bool) {
            value.get<BoolRef>().get() = arg_value;
        } else {
            return false;
        }
        return true;
    }

    bool ArgParser::Option::has_default() const {
        if (type != Type::NoValue) return true;
        return false;
    }
} // namespace ARLib