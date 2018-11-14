//
// Created by xiaoc on 2018/11/14.
//

#ifndef HIKARI_HIKARI_HPP
#define HIKARI_HIKARI_HPP

#include <iostream>
#include <string>
#include <functional>

namespace Hikari {
    struct State {
        const char *p;

        int read() {
            return *p;
        }

        State next() const {
            return State(*p ? p + 1 : p);
        }
        int null()const{return 0;}
        State(const char *_p) : p(_p) {}
    };

    using InputType = int;

    template<typename T>
    struct Result {
    private:
        bool flag;
    public:
        bool succ() const { return flag; }

        operator bool() { return flag; }

        T result;
        State state;

        Result(State s, T _result, bool f = true) : state(s), result(_result), flag(f) {}
    };

    template<typename R>
    struct ParserFunc {
        using Combiner = std::function<Result<R>(Result<R>, Result<R>)>;
        using Func = std::function<Result<R>(ParserFunc<R>, State)>;
        Combiner combiner;
        Func func;

        ParserFunc &combineRule(Combiner c) {
            combiner = c;
            return *this;
        }

        Result<R> operator()(const State s) const {
            return func(*this, s);
        }

        ParserFunc(Func _f) : func(_f) {}

        ParserFunc(Func _f, Combiner c) : func(_f), combiner(c) {}

        ParserFunc() {}
    };

    template<typename T>
    ParserFunc<T> atom(InputType x,
                       std::function<T(InputType)> accept,
                       std::function<T(InputType)> reject) {
        return ParserFunc<T>([=](ParserFunc<T>, State s) {
            auto val = s.read();
            if (val == x) {
                return Result<T>(s.next(), accept(val), true);
            } else {
                return Result<T>(s, reject(val), false);
            }
        });
    }
    template<typename T>
    ParserFunc<T> null( std::function<T(InputType)> accept){
        return ParserFunc<T>([=](ParserFunc<T>, State s) {
            return Result<T>(s, accept(s.null()), true);
        });
    }
    template<typename R>
    ParserFunc<R> operator~(ParserFunc<R> func) {
        return func | null([](auto x){return R();});
    }
    template<typename R>
    ParserFunc<R> operator&(ParserFunc<R> first,
                            ParserFunc<R> second) {
        return ParserFunc<R>([=](ParserFunc<R> p, State s) {
            auto r1 = first(s);
            if (!r1) {
                return r1;
            } else {
                auto r2 = second(r1.state);
                return p.combiner(r1, r2);
            }
        });
    }

    template<typename R>
    ParserFunc<R> operator|(ParserFunc<R> first,
                            ParserFunc<R> second) {
        return ParserFunc<R>([=](ParserFunc<R> p, State s) {
            auto r1 = first(s);
            if (r1) {
                return r1;
            } else {
                auto r2 = second(r1.state);
                return r2;
            }
        });
    }

    template<typename R>
    ParserFunc<R> operator+(ParserFunc<R> func) {
        return ParserFunc<R>([=](ParserFunc<R> p, State s) {
            Result<R> result = func(s);
            if (!result) {
                return result;
            }
            while (true) {
                auto r = func(result.state);
                if (!r)break;
                result = p.combiner(result, r);
            }
            return result;
        });
    }

    template<typename R>
    ParserFunc<R> concat(ParserFunc<R> first,
                         ParserFunc<R> second,
                         std::function<Result<R>(Result<R>, Result<R>)> combiner) {
        return (first & second).combineRule(combiner);
    }

}

#endif //HIKARI_HIKARI_HPP
