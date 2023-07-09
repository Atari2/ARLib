#include "JSONObject.hpp"
#include "File.hpp"
#include "JSONParser.hpp"
namespace ARLib {
namespace JSON {
    Bool::operator Value() && {
        return ValueObj::construct<Bool>(move(*this));
    }
    Null::operator Value() && {
        return ValueObj::construct<Null>(move(*this));
    }
    Number::operator Value() && {
        return ValueObj::construct<Number>(move(*this));
    }
    Object::operator Value() && {
        return ValueObj::construct<Object>(move(*this));
    }
    Array::operator Value() && {
        return ValueObj::construct<Array>(move(*this));
    }
    JString::operator Value() && {
        return ValueObj::construct<JString>(move(*this));
    }
    Null::operator nullptr_t() const {
        return nullptr;
    }
    Value::Value(ValueObj&& obj) : UniquePtr{ Forward<ValueObj>(obj) } {}
    Value::Value(const Value& other) : UniquePtr{ ValueObj{ *other.get() } } {}
    Value& Value::operator=(const Value& other) {
        static_cast<UniquePtr<ValueObj>&>(*this).operator=(UniquePtr{ ValueObj{ *other.get() } });
        return *this;
    }
    Value Value::deepcopy() const {
        return Value{ *this };
    }
    Array::Array(const Array& other) : Vector{ } {
        for (const auto& item : other) { append(move(item.deepcopy())); }
    }
    Array& Array::operator=(const Array& other) {
        clear();
        for (const auto& item : other) { append(move(item.deepcopy())); }
        return *this;
    }
    SerializeResult Document::serialize_to_file(const Path& filename) const {
        File f{ filename };
        TRY(f.open(OpenFileMode::Write));
        TRY(m_value->serialize_to_file(f));
        return {};
    }
    SerializeResult serialize_array_to_file(File& f, const Array& arr, size_t indent);
    SerializeResult serialize_object_to_file(File& f, const Object& obj, size_t indent);
    SerializeResult serialize_object_to_file(File& f, const Object& obj, size_t indent) {
        if (obj.size() == 0) {
            TRY(f.write("{}"_s));
            return {};
        }
        String indent_string{ indent, '\t' };
        String prev_indent_string{ indent - 1, '\t' };
        TRY(f.write(prev_indent_string + "{\n"_s));
        size_t i = 0;
        for (const auto& entry : obj) {
            const auto& val = *entry.val();
            const auto& key = entry.key();
            TRY(f.write(indent_string));
            TRY(f.write("\""_s + key + '"'));
            TRY(f.write(": "_s));
            switch (val.type()) {
                case Type::JArray:
                    TRY(serialize_array_to_file(f, val.as<Type::JArray>(), indent + 1));
                    break;
                case Type::JObject:
                    TRY(serialize_object_to_file(f, val.as<Type::JObject>(), indent + 1));
                    break;
                case Type::JNumber:
                    TRY(f.write(val.as<Type::JNumber>().to_string()));
                    break;
                case Type::JNull:
                    TRY(f.write("null"_s));
                    break;
                case Type::JBool:
                    TRY(f.write(BoolToStr(val.as<Type::JBool>().value())));
                    break;
                case Type::JString:
                    TRY(f.write("\""_s + val.as<Type::JString>() + '"'));
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid type in JSON object");
                    break;
            }
            if (++i < obj.size()) {
                TRY(f.write(",\n"_s));
            } else {
                TRY(f.write("\n"_s));
            }
        }
        TRY(f.write(prev_indent_string + "}"_s));
        return {};
    }
    SerializeResult serialize_array_to_file(File& f, const Array& arr, size_t indent) {
        if (arr.size() == 0) {
            TRY(f.write("[]"_s));
            return {};
        }
        String prev_indent_string{ indent - 1, '\t' };
        String indent_string{ indent, '\t' };
        TRY(f.write(prev_indent_string + "[\n"));
        size_t i = 0;
        for (const auto& val_ptr : arr) {
            const auto& val = *val_ptr;
            switch (val.type()) {
                case Type::JArray:
                    TRY(serialize_array_to_file(f, val.as<Type::JArray>(), indent + 1));
                    break;
                case Type::JObject:
                    TRY(serialize_object_to_file(f, val.as<Type::JObject>(), indent + 1));
                    break;
                case Type::JNumber:
                    TRY(f.write(indent_string));
                    TRY(f.write(val.as<Type::JNumber>().to_string()));
                    break;
                case Type::JNull:
                    TRY(f.write(indent_string));
                    TRY(f.write("null"_s));
                    break;
                case Type::JBool:
                    TRY(f.write(indent_string));
                    TRY(f.write(BoolToStr(val.as<Type::JBool>().value())));
                    break;
                case Type::JString:
                    TRY(f.write(indent_string));
                    TRY(f.write("\""_s + val.as<Type::JString>() + '"'));
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid type in JSON object");
                    break;
            }
            if (++i < arr.size()) {
                TRY(f.write(",\n"_s));
            } else {
                TRY(f.write("\n"_s));
            }
        }
        TRY(f.write(prev_indent_string + "]"_s));
        return {};
    }
    SerializeResult ValueObj::serialize_to_file(File& f) const {
        switch (m_type) {
            case Type::JArray:
                TRY(serialize_array_to_file(f, as<Array>(), 1));
                break;
            case Type::JObject:
                TRY(serialize_object_to_file(f, as<Object>(), 1));
                break;
            case Type::JBool:
                TRY(f.write(BoolToStr(as<Bool>().value())));
                break;
            case Type::JNull:
                TRY(f.write("null"_s));
                break;
            case Type::JNumber:
                TRY(f.write(as<Type::JNumber>().to_string()));
                break;
            case Type::JString:
                TRY(f.write("\""_s + as<Type::JString>() + '"'));
                break;
        }
        return {};
    }
}    // namespace JSON
}    // namespace ARLib
