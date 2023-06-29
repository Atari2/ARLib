#include "JSONObject.hpp"
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
    Value::Value(ValueObj&& obj) : UniquePtr{ Forward<ValueObj>(obj) } {

    }
    Value::Value(const Value& other) : UniquePtr{ ValueObj{ *other.get() } } {
        
    }
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
}    // namespace JSON
}    // namespace ARLib
