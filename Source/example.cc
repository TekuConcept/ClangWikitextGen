// clang -c -Xclang -ast-dump ./t.cc

class X {
public:
    virtual ~X() = default;
};

class Y : public X {
public:
    void z() {}
};

class foo {
public:
    virtual ~foo() = default;
};

class bar {
public:
    virtual ~bar() = default;
};

struct tcurts {};

class baz: public foo, public bar {
public:
    void pub_fiz() {}
    static void pub_fiz(double) {}
    int my_property;
protected:
    template <typename T = tcurts>
    void pro_fiz() {}
    short * const * pro_fiz(int, float) { return 0; }
private:
    void prv_fiz() {}
};

template <template <typename,typename> class Container, typename Type>
class yolo {};

enum alp { a, b, c };
enum class asdf { q, w, e, r, t, y };

void f() {
    X x;
}

