//
// Created by johnk on 2022/9/29.
//

#include <Test/Test.h>

#include <RegistryTest.h>
#include <Mirror/Mirror.h>

#include <any>

int v0 = 1;

int F0(const int a, const int b)
{
    return a + b;
}

int& F1()
{
    return v0;
}

void F2(int& outValue)
{
    outValue = 1;
}

int C0::v0 = 0;

int& C0::F0()
{
    return v0;
}

C1::C1(const int inV0)
    : v0(inV0)
{
}

int C1::GetV0() const
{
    return v0;
}

void C1::SetV0(const int inV0)
{
    v0 = inV0;
}

C2::C2(const int inA, const int inB)
    : a(inA), b(inB)
{
}

C3::C3(const int inA, const int inB, const int inC)
    : C2(inA, inB), c(inC)
{
}

TEST(RegistryTest, GlobalScopeTest)
{
    const auto& globalScope = Mirror::GlobalScope::Get();
    {
        const auto& variable = globalScope.GetVariable("v0");
        ASSERT_EQ(variable.GetMeta("testKey"), "v0");

        auto value = variable.GetDyn();
        ASSERT_EQ(value.As<int>(), 1);
        ASSERT_EQ(value.As<const int&>(), 1);
        value.As<int&>() = 2;
        ASSERT_EQ(value.As<int>(), 2);

        variable.Set(3);
        ASSERT_EQ(v0, 3);

        value = 4;
        variable.SetDyn(value);
        ASSERT_EQ(v0, 4);

        Mirror::Any anyValue = 5;
        variable.SetDyn(anyValue);
        ASSERT_EQ(v0, 5);
    }

    {
        const auto& function = globalScope.GetFunction("F0");
        auto result = function.Invoke(1, 2);
        ASSERT_EQ(result.As<int>(), 3);
    }

    {
        const auto& function = globalScope.GetFunction("F1");
        auto result = function.Invoke();
        ASSERT_EQ(result.As<const int&>(), 5);
    }

    {
        const auto& function = globalScope.GetFunction("F2");
        int value = 0;
        function.Invoke(value);
        ASSERT_EQ(value, 1);

        value = 0;
        function.InvokeDyn({ Mirror::Any(std::ref(value)) });
        ASSERT_EQ(value, 1);
    }
}

TEST(RegistryTest, ClassTest)
{
    {
        const auto& clazz = Mirror::Class::Get("C0");
        ASSERT_EQ(clazz.GetMeta("testKey"), "C0");
        {
            const auto& variable = clazz.GetStaticVariable("v0");
            ASSERT_EQ(variable.GetMeta("testKey"), "v0");
            variable.Set(1);
            ASSERT_EQ(variable.GetDyn().As<int>(), 1);
        }

        {
            const auto& function = clazz.GetStaticFunction("F0");
            ASSERT_EQ(function.GetMeta("testKey"), "F0");
            auto result = function.Invoke();
            ASSERT_EQ(result.As<int&>(), 1);
        }
    }

    {
        const auto& clazz = Mirror::Class::Get<C1>();
        const auto& constructor = clazz.GetConstructor("const int");
        const auto& setter = clazz.GetMemberFunction("SetV0");
        const auto& getter = clazz.GetMemberFunction("GetV0");

        auto object = constructor.Construct(1);
        ASSERT_EQ(getter.Invoke(object.As<C1&>()).As<int>(), 1);
        setter.Invoke(object.As<C1&>(), 2);
        ASSERT_EQ(getter.Invoke(object.As<C1&>()).As<int>(), 2);
    }

    {
        const auto& clazz = Mirror::Class::Get<C2>();
        const auto& constructor = clazz.GetConstructor("const int, const int");
        const auto& destructor = clazz.GetDestructor();
        const auto& a = clazz.GetMemberVariable("a");
        const auto& b = clazz.GetMemberVariable("b");

        auto object = constructor.New(1, 2);
        Mirror::Any objectRef = *object.As<C2*>();
        ASSERT_EQ(a.GetDyn(objectRef).As<int>(), 1);
        ASSERT_EQ(b.GetDyn(objectRef).As<int>(), 2);
        destructor.InvokeDyn(objectRef);
    }

    {
        const auto& clazz = Mirror::Class::Get<C3>();
        auto object = clazz.GetConstructor("const int, const int, const int").New(1, 2, 3);
        auto* c2Obj = object.As<C2*>();

        ASSERT_EQ(c2Obj->a, 1);
        ASSERT_EQ(c2Obj->b, 2);
    }
}

TEST(RegistryTest, EnumTest)
{
    const auto& enumInfo = Mirror::Enum::Get<E0>();
    auto a = enumInfo.GetElement("a");
    auto b = enumInfo.GetElement("b");
    auto c = enumInfo.GetElement("c");
    auto max = enumInfo.GetElement("max");

    ASSERT_EQ(a.As<E0>(), E0::a);
    ASSERT_EQ(b.As<E0>(), E0::b);
    ASSERT_EQ(c.As<E0>(), E0::c);
    ASSERT_EQ(max.As<E0>(), E0::max);

    ASSERT_EQ(enumInfo.GetElementName(a), "a");
    ASSERT_EQ(enumInfo.GetElementName(b), "b");
    ASSERT_EQ(enumInfo.GetElementName(c), "c");
    ASSERT_EQ(enumInfo.GetElementName(max), "max");
}
