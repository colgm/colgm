use std::json::{stringify, parse}
use std::out::println
use std::io::*

enum example_enum {
    aaa,
    bbb,
    ccc
}

func void_test_function(a: i8, b: i16, c: i32, d: i64) {
    var variable0: i32 = 1;
    var fvar: f32 = 1.0;
    var variable0 = 1;
    var fvar = 1.0;
}

func i32_test_function(a: f32, b: f64) -> i32 {
    var s: str = "12345";
    var s = "";
    var c: i8 = s[0];
    var c = s[0];
    var s: vec<i32> = [];
    var s = [1]; # maybe need at least one literal to get type infer
}

func u32_test_function(c: u8, d: u16, e: u32, f: u64) -> u32 {
    return 0x100000000;
    return 2147483648;
    return 1%5;
}

func f32_test_function(a: example_enum) -> f32 {
    match(a) {
        aaa => return 1.0,
        bbb => return 2.0,
        ccc => return 3.0
    }
    return 1.0/10.0;
}

func main(argc: i32, argv: vec<str>) -> i32 {
    println("hello world");
    io::open("test.txt", "w");
    return 0;
}