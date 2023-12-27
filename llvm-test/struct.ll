%struct.type = type {i32, i32, i32, double}

declare i32 @printf(i8*, ...) nounwind

@.str = private unnamed_addr constant [14 x i8] c"%d %d %d %lf\0a\00"

define i32 @main(i32 %argc, i8** %argv) {
    %struct = alloca %struct.type

    %p0 = getelementptr %struct.type, %struct.type* %struct, i64 0, i32 0
    %p1 = getelementptr %struct.type, %struct.type* %struct, i64 0, i32 1
    %p2 = getelementptr %struct.type, %struct.type* %struct, i64 0, i32 2
    %p3 = getelementptr %struct.type, %struct.type* %struct, i64 0, i32 3

    store i32 2147483647, i32* %p0
    store i32 1145141919, i32* %p1
    store i32 1, i32* %p2
    store double 2.0, double* %p3

    %value0 = load i32, i32* %p0
    %value1 = load i32, i32* %p1
    %value2 = load i32, i32* %p2
    %value3 = load double, double* %p3
    %value4 = fadd double %value3, 10.0

    %fmt = getelementptr [14 x i8], [14 x i8]* @.str, i64 0, i64 0
    call i32 (i8*, ...) @printf(i8* %fmt, i32 %value0, i32 %value1, i32 %value2, double %value4)
    ret i32 0

    unreachable
}