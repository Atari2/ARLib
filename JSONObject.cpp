#include "JSONObject.h"
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
}    // namespace JSON
}    // namespace ARLib