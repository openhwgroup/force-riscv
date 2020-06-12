//
// Copyright 2013 Michael J. Haertel
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject
// to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef FMT_H
#define FMT_H
#ifdef __GNUC__
#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Weffc++"
#endif

#include <iostream>

//
// Helper class to make iostreams formatted I/O more printf/scanf-like.
// Using Fmt objects, the equivalent to:
//    printf("%08X\n", x);
// is:
//    std::cout << fmtX(x, 8).fill(0) << '\n';
// Fmt objects also work with input streams in a scanf like fashion,
// for example:
//    char s[10];
//    std::cin >> fmt(s, sizeof s);
// Supported formats include:
//    fmt    iostreams default formatting for operand type
//    fmti    base determined by leading 0 or 0x of input
//    fmtd    decimal
//    fmtd0    decimal with fill '0'
//    fmto    octal
//    fmto0    octal with fill '0'
//    fmtx    hex
//    fmtx0    hex with fill '0'
//    fmtX    hex upper case
//    fmtX0    hex upper case with fill 0
// FUTURE WORK: consider adding equivalents to remaining printf/scanf formats:
//    a, A:    C99 hex floating point
//    d, i    signed decimal (for scanf, %i detects base different?)
//    e, E    floating point with lower or uppercase E notation
//    f, F    floating point with lower or uppercase "inf" "nan" etc
//    g, G    auto-choose between [eE] and [fF]
//    m    useful glibc extension: strerror(errno)
//    n    store # of characters emitted/consumed since beginning of format
//    o    unsigned octal
//    p    void *
//    u    unsigned decimal
//    x, X    unsigned hex
//    [    incredibly useful scanf feature
// In all cases the arguments to fmt*() are operand, width, precision.
// If width and precision are not provided they default to 0 and 6
// respectively (following the normal iostream defaults).
//
// The << and >> operators for Fmt objects save and restore the iostream's
// prior to formatting the referenced object, and restore it afterards.
// So, for example, after executing:
//    std::cout.setf(std::ios_base::hex, std::ios_base::basefield);
//    std::cout << fmto(123);
// the output base for std::cout will still be hex.
//
// In order to make "cin >> fmt(x)" work, Fmt objects contain references
// rather than values.  But in order to make "cout << fmtx(123)" work,
// the reference may be to an implicit temporary created by the compiler;
// such a temporary object lives only until the end of evaluation of the
// full expression in which the temporary was created.  The user should
// never directly declare objects of type Fmt.  Due to a C++ 1998/2003
// language bug and lack of imagination for how to work around it, this
// library does not fully enforce that constraint.
//

// FUTURE WORK: maybe should parameterize on charT and traits like the
// basic_ios template.  And then maybe the istream/ostream operators 
// should attach to more abstract istream/ostream types.
template <class TYPE>
class Fmt {
public:
    //
    // Although it might seem odd to put "friend" declarations first,
    // these (non-member) functions are the primary interface to Fmt.
    //

    // iostream << and >>
    template <class T> friend std::istream& operator>>(std::istream&,
                               const Fmt<T>&);
    template <class T> friend std::ostream& operator<<(std::ostream&,
                               const Fmt<T>&);

    // fmt
    template <class T> friend Fmt<const T*> fmt(const T*, int, int);
    template <class T> friend Fmt<const T&> fmt(const T&, int, int);
    template <class T> friend Fmt<T&> fmt(T&, int, int);

    // fmti
    template <class T> friend Fmt<const T*> fmti(const T*, int, int);
    template <class T> friend Fmt<const T&> fmti(const T&, int, int);
    template <class T> friend Fmt<T&> fmti(T&, int, int);

    // fmtd
    template <class T> friend Fmt<const T*> fmtd(const T*, int, int);
    template <class T> friend Fmt<const T&> fmtd(const T&, int, int);
    template <class T> friend Fmt<T&> fmtd(T&, int, int);

    // fmtd0
    template <class T> friend Fmt<const T*> fmtd0(const T*, int, int);
    template <class T> friend Fmt<const T&> fmtd0(const T&, int, int);
    template <class T> friend Fmt<T&> fmtd0(T&, int, int);

    // fmto
    template <class T> friend Fmt<const T*> fmto(const T*, int, int);
    template <class T> friend Fmt<const T&> fmto(const T&, int, int);
    template <class T> friend Fmt<T&> fmto(T&, int, int);

    // fmto0
    template <class T> friend Fmt<const T*> fmto0(const T*, int, int);
    template <class T> friend Fmt<const T&> fmto0(const T&, int, int);
    template <class T> friend Fmt<T&> fmto0(T&, int, int);

    // fmtx
    template <class T> friend Fmt<const T*> fmtx(const T*, int, int);
    template <class T> friend Fmt<const T&> fmtx(const T&, int, int);
    template <class T> friend Fmt<T&> fmtx(T&, int, int);

    // fmtx0
    template <class T> friend Fmt<const T*> fmtx0(const T*, int, int);
    template <class T> friend Fmt<const T&> fmtx0(const T&, int, int);
    template <class T> friend Fmt<T&> fmtx0(T&, int, int);

    // fmtX
    template <class T> friend Fmt<const T*> fmtX(const T*, int, int);
    template <class T> friend Fmt<const T&> fmtX(const T&, int, int);
    template <class T> friend Fmt<T&> fmtX(T&, int, int);

    // fmtX0
    template <class T> friend Fmt<const T*> fmtX0(const T*, int, int);
    template <class T> friend Fmt<const T&> fmtX0(const T&, int, int);
    template <class T> friend Fmt<T&> fmtX0(T&, int, int);

    //
    // The following member functions allow additional tweaks to the
    // default formatting done the fmt*() pseudo-constructors above.
    //
    Fmt& fill(std::ostream::char_type fc) {
        c = fc;
        return *this;
    }
    Fmt& internal() {
        f = (f & ~std::ios_base::adjustfield) | std::ios_base::internal;
        return *this;
    }
    Fmt& left() {
        f = (f & ~std::ios_base::adjustfield) | std::ios_base::left;
        return *this;
    }
    Fmt& right() {
        f = (f & ~std::ios_base::adjustfield) | std::ios_base::right;
        return *this;
    }
    Fmt& any() {
        f &= ~std::ios_base::basefield;
        return *this;
    }
    Fmt& dec() {
        f = (f & ~std::ios_base::basefield) | std::ios_base::dec;
        return *this;
    }
    Fmt& hex() {
        f = (f & ~std::ios_base::basefield) | std::ios_base::hex;
        return *this;
    }
    Fmt& oct() {
        f = (f & ~std::ios_base::basefield) | std::ios_base::oct;
        return *this;
    }
    Fmt& fixed() {
        f = (f & ~std::ios_base::floatfield) | std::ios_base::fixed;
        return *this;
    }
    Fmt& scientific() {
        f = (f & ~std::ios_base::floatfield) | std::ios_base::scientific;
        return *this;
    }
    Fmt& boolalpha(bool b = true) {
        if (b)
            f |= std::ios_base::boolalpha;
        else
            f &= ~std::ios_base::boolalpha;
        return *this;
    }
    Fmt& showbase(bool b = true) {
        if (b)
            f |= std::ios_base::showbase;
        else
            f &= ~std::ios_base::showbase;
        return *this;
    }
    Fmt& showpoint(bool b = true) {
        if (b)
            f |= std::ios_base::showpoint;
        else
            f &= ~std::ios_base::showpoint;
        return *this;
    }
    Fmt& showpos(bool b = true) {
        if (b)
            f |= std::ios_base::showpos;
        else
            f &= ~std::ios_base::showpos;
        return *this;
    }
    Fmt& skipws(bool b = true) {
        if (b)
            f |= std::ios_base::skipws;
        else
            f &= ~std::ios_base::skipws;
        return *this;
    }
    Fmt& unitbuf(bool b = true) {
        if (b)
            f |= std::ios_base::unitbuf;
        else
            f &= ~std::ios_base::unitbuf;
        return *this;
    }
    Fmt& uppercase(bool b = true) {
        if (b)
            f |= std::ios_base::uppercase;
        else
            f &= ~std::ios_base::uppercase;
        return *this;
    }

    // FIXME: Unfortunately C++ 1998 and 2003 require this constructor to
    // be public order to pass arguments of const Fmt&.  This means the
    // user can make copies of Fmt<const T&> objects that outlive a
    // referenced temporary.  Find some way to prevent this.  This language
    // bug was fixed in C++ 2011, so we could in principle insert an #ifdef
    // to make this copy constructor private under C++ 2011 or later.
    Fmt(const Fmt& f): v(f.v), c(f.c), f(f.f), p(f.p), w(f.w) {}

private:
    Fmt(TYPE v, int w, int p): v(v), c(' '),
        f(std::ios_base::dec | std::ios_base::skipws), p(p), w(w) {}
    Fmt& operator=(const Fmt&);
    TYPE v;
    std::ostream::char_type c;
    std::ios_base::fmtflags f;
    std::streamsize p;
    std::streamsize w;
};

template <class TYPE>
inline
std::istream&
operator>>(std::istream& is, const Fmt<TYPE>& fmt)
{
    std::ostream::char_type c = is.fill(fmt.c);
    std::ios_base::fmtflags f = is.flags(fmt.f);
    std::streamsize p = is.precision(fmt.p);
    std::streamsize w = is.width(fmt.w);
    is >> fmt.v;
    is.fill(c);
    is.flags(f);
    is.width(w);
    is.precision(p);
    return is;
}

template <class TYPE>
inline
std::ostream&
operator<<(std::ostream& os, const Fmt<TYPE>& fmt)
{
    std::ostream::char_type c = os.fill(fmt.c);
    std::ios_base::fmtflags f = os.flags(fmt.f);
    std::streamsize p = os.precision(fmt.p);
    std::streamsize w = os.width(fmt.w);
    os << fmt.v;
    os.fill(c);
    os.flags(f);
    os.width(w);
    os.precision(p);
    return os;
}

template <class ATYPE>
Fmt<const ATYPE*>
fmt(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p);
}

template <class ATYPE>
Fmt<const ATYPE&>
fmt(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p);
}

template <class ATYPE>
Fmt<ATYPE&>
fmt(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p);
}

template <class ATYPE>
Fmt<const ATYPE*>
fmtd(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p).dec();
}

template <class ATYPE>
Fmt<const ATYPE&>
fmtd(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p).dec();
}

template <class ATYPE>
Fmt<ATYPE&>
fmtd(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p).dec();
}

template <class ATYPE>
Fmt<const ATYPE*>
fmtd0(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p).fill('0').dec();
}

template <class ATYPE>
Fmt<const ATYPE&>
fmtd0(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p).fill('0').dec();
}

template <class ATYPE>
Fmt<ATYPE&>
fmtd0(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p).fill('0').dec();
}

template <class ATYPE>
Fmt<const ATYPE*>
fmti(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p).any();
}

template <class ATYPE>
Fmt<const ATYPE&>
fmti(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p).any();
}

template <class ATYPE>
Fmt<ATYPE&>
fmti(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p).any();
}

template <class ATYPE>
Fmt<const ATYPE*>
fmto(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p).oct();
}

template <class ATYPE>
Fmt<const ATYPE&>
fmto(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p).oct();
}

template <class ATYPE>
Fmt<ATYPE&>
fmto(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p).oct();
}

template <class ATYPE>
Fmt<const ATYPE*>
fmto0(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p).fill('0').oct();
}

template <class ATYPE>
Fmt<const ATYPE&>
fmto0(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p).fill('0').oct();
}

template <class ATYPE>
Fmt<ATYPE&>
fmto0(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p).fill('0').oct();
}

template <class ATYPE>
Fmt<const ATYPE*>
fmtx(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p).hex();
}

template <class ATYPE>
Fmt<const ATYPE&>
fmtx(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p).hex();
}

template <class ATYPE>
Fmt<ATYPE&>
fmtx(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p).hex();
}

template <class ATYPE>
Fmt<const ATYPE*>
fmtx0(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p).fill('0').hex();
}

template <class ATYPE>
Fmt<const ATYPE&>
fmtx0(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p).fill('0').hex();
}

template <class ATYPE>
Fmt<ATYPE&>
fmtx0(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p).fill('0').hex();
}

template <class ATYPE>
Fmt<const ATYPE*>
fmtX(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p).hex().uppercase();
}

template <class ATYPE>
Fmt<const ATYPE&>
fmtX(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p).hex().uppercase();
}

template <class ATYPE>
Fmt<ATYPE&>
fmtX(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p).hex().uppercase();
}

template <class ATYPE>
Fmt<const ATYPE*>
fmtX0(const ATYPE* v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE*>(v, w, p).fill('0').hex().uppercase();
}

template <class ATYPE>
Fmt<const ATYPE&>
fmtX0(const ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<const ATYPE&>(v, w, p).fill('0').hex().uppercase();
}

template <class ATYPE>
Fmt<ATYPE&>
fmtX0(ATYPE& v, int w = 0, int p = 6)
{
    return Fmt<ATYPE&>(v, w, p).fill('0').hex().uppercase();
}
#ifdef __GNUC__
#pragma GCC diagnostic pop 
#endif

#endif // FMT_H
