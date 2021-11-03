
class simple_class {};
struct simple_struct {};

class implicit_xtors {};

class default_xtors {
public:
    default_xtors() = default;
    ~default_xtors() = default;
};

template <typename T>
class template_class {
public:
    void foo() {}
};

class template_functions {
public:
    template <typename T>
    void foo() {}
};

class members {
public:
    void foo();
    virtual void bar() = 0;
    virtual void baz();
    static void joy();
protected:
    void fiz();
private:
    void pop();
};

class poly_a {};
class poly_b : public poly_a {};
class poly_c {};
class poly_d : public poly_b, public poly_c {};
class poly_e : public poly_a, private poly_c {};
