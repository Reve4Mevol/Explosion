//
// Created by johnk on 2022/11/20.
//

#pragma once

#if __clang__
#define EProperty(...) __attribute__((annotate("property;" #__VA_ARGS__)))
#define EFunc(...) __attribute__((annotate("func;" #__VA_ARGS__)))
#define EClass(...) __attribute__((annotate("class;" #__VA_ARGS__)))
#define EEnum(...) __attribute__((annotate("enum;" #__VA_ARGS__)))
#define EMeta(...) __attribute__((annotate(#__VA_ARGS__)))
#else
#define EProperty(...)
#define EFunc(...)
#define EClass(...)
#define EEnum(...)
#define EMeta(...)
#endif

namespace Mirror {
    class Class;
}

// TODO maybe need virtual const Mirror::Class& GetClass() ?
#define EClassBody(className) \
private: \
    static int _mirrorRegistry; \
public: \
    static const Mirror::Class& GetClass(); \
