#pragma once
#include "CharConv.h"
#include "Optional.h"
#include "String.h"
#include "Vector.h"

namespace ARLib {
    class ArgParser {
        public:
        enum class ExecPath : bool {
            Keep = 0,
            Skip = 1,
        };
        struct CmdOption {
            private:
            String m_long_name;
            String m_short_name;
            Optional<String> m_value;

            public:
            const String& long_name() { return m_long_name; }
            const String& short_name() { return m_short_name; }
            const Optional<String>& value() { return m_value; }
            void set_long_name(String long_name) { m_long_name = move(long_name); }
            void set_short_name(String short_name) { m_short_name = move(short_name); }
            void set_value(String value) { m_value.put(Forward<String>(value)); }
            bool operator==(const String& name) { return m_long_name == name || m_short_name == name; }
            String as_string() { return m_value.value_or(""); }
            int as_int() { return StrToInt(as_string()); }
            float as_float() { return StrToFloat(as_string()); }
            bool as_bool() { return StrToBool(as_string()); }
        };

        private:
        Vector<String> m_args{};
        Vector<CmdOption> m_options{};

        public:
        ArgParser() = default;
        void add_option(const char* long_name, const char* short_name, const char* optional = nullptr) {
            CmdOption option{};
            option.set_long_name(long_name);
            option.set_short_name(short_name);
            if (optional) option.set_value(optional);
            m_options.append(Forward<CmdOption>(option));
        }

        String strip_prefix(const String& arg) {
            if (arg.starts_with("--"))
                return arg.substring(2);
            else if (arg.starts_with("-")) {
                return arg.substring(1);
            }
            return arg;
        }

        void readopt(char** argv, int argc, ExecPath mode = ExecPath::Skip) {
            m_args.reserve(argc);
            for (size_t i = static_cast<bool>(mode); i < static_cast<size_t>(argc); i++)
                m_args.append(argv[i]);
        }

        void parse() {
            for (auto& arg : m_args) {
                auto name_value = strip_prefix(arg).split("=");
                SOFT_ASSERT((name_value.size() == 2), "Name=value pair of command line arguments isn't respected");
                for (auto& cmd : m_options) {
                    if (cmd == name_value[0]) cmd.set_value(name_value[1]);
                }
            }
        }

        CmdOption& operator[](const char* name) {
            static CmdOption blank_option{};
            for (auto& option : m_options) {
                if (option == name) return option;
            }
            return blank_option;
        }
    };
} // namespace ARLib

using ARLib::ArgParser;