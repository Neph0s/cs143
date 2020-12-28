class B {
    s : String <- "Hello";
    g(y:String) : Int {
        y.concat(s)
    };
    f(x:Int) : Int {
        x*6
    };
};

class A {
    b : B <- new B;
    f(x:Int) : Int {
        x-2
    };
};

class Main inherits B{
f(x:Int) : Int {
        x+a
    };
   main():Object{
       f(8)
   };
};