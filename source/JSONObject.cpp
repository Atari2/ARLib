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
    Array::Array(const Array& other) {
        for (const auto& item : other) { append(move(item.deepcopy())); }
    }
    Array& Array::operator=(const Array& other) {
        clear();
        for (const auto& item : other) { append(move(item.deepcopy())); }
        return *this;
    }
    void Document::serialize_to_file(const Path& filename) const {
        File f{ filename };
        f.open(OpenFileMode::Write);
        m_value->serialize_to_file(f);
    }
    void serialize_array_to_file(File& f, const Array& arr, size_t indent);
    void serialize_object_to_file(File& f, const Object& obj, size_t indent);

    void serialize_object_to_file(File& f, const Object& obj, size_t indent) {
        if (obj.size() == 0) {
            f.write("{}"_s);
            return;
        }
        String indent_string{ indent, '\t' };
        String prev_indent_string{ indent - 1, '\t' };
        f.write(prev_indent_string + "{\n"_s);
        size_t i = 0;
        for (const auto& entry : obj) {
            const auto& val = *entry.val();
            const auto& key = entry.key();
            f.write(indent_string);
            f.write("\""_s + key + '"');
            f.write(": "_s);
            switch (val.type()) {
                case Type::JArray:
                    serialize_array_to_file(f, val.as<Type::JArray>(), indent + 1);
                    break;
                case Type::JObject:
                    serialize_object_to_file(f, val.as<Type::JObject>(), indent + 1);
                    break;
                case Type::JNumber:
                    f.write(val.as<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    f.write("null"_s);
                    break;
                case Type::JBool:
                    f.write(BoolToStr(val.as<Type::JBool>().value()));
                    break;
                case Type::JString:
                    f.write("\""_s + val.as<Type::JString>() + '"');
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid type in JSON object");
                    break;
            }
            if (++i < obj.size()) {
                f.write(",\n"_s);
            } else {
                f.write("\n"_s);
            }
        }
        f.write(prev_indent_string + "}"_s);
    }
    void serialize_array_to_file(File& f, const Array& arr, size_t indent) {
        if (arr.size() == 0) {
            f.write("[]"_s);
            return;
        }
        String prev_indent_string{ indent - 1, '\t' };
        String indent_string{ indent, '\t' };
        f.write(prev_indent_string + "[\n");
        size_t i = 0;
        for (const auto& val_ptr : arr) {
            const auto& val = *val_ptr;
            switch (val.type()) {
                case Type::JArray:
                    serialize_array_to_file(f, val.as<Type::JArray>(), indent + 1);
                    break;
                case Type::JObject:
                    serialize_object_to_file(f, val.as<Type::JObject>(), indent + 1);
                    break;
                case Type::JNumber:
                    f.write(indent_string);
                    f.write(val.as<Type::JNumber>().to_string());
                    break;
                case Type::JNull:
                    f.write(indent_string);
                    f.write("null"_s);
                    break;
                case Type::JBool:
                    f.write(indent_string);
                    f.write(BoolToStr(val.as<Type::JBool>().value()));
                    break;
                case Type::JString:
                    f.write(indent_string);
                    f.write("\""_s + val.as<Type::JString>() + '"');
                    break;
                default:
                    ASSERT_NOT_REACHED("Invalid type in JSON object");
                    break;
            }
            if (++i < arr.size()) {
                f.write(",\n"_s);
            } else {
                f.write("\n"_s);
            }
        }
        f.write(prev_indent_string + "]"_s);
    }
    void ValueObj::serialize_to_file(File& f) const {
        switch (m_type) {
            case Type::JArray:
                serialize_array_to_file(f, as<Array>(), 1);
                break;
            case Type::JObject:
                serialize_object_to_file(f, as<Object>(), 1);
                break;
            case Type::JBool:
                f.write(BoolToStr(as<Bool>().value()));
                break;
            case Type::JNull:
                f.write("null"_s);
                break;
            case Type::JNumber:
                f.write(as<Type::JNumber>().to_string());
                break;
            case Type::JString:
                f.write("\""_s + as<Type::JString>() + '"');
                break;
        }
    }
}    // namespace JSON
}    // namespace ARLib
