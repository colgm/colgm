%struct.type = type {i32, i32, i32, double}

declare i32 @printf(i8*, ...) nounwind

@.str = private unnamed_addr constant [5 x i8] c"%ld\0a\00"

define i64 @struct.type.size() {
    %sz = getelementptr %struct.type, %struct.type* null, i64 1
    %usz = ptrtoint %struct.type* %sz to i64
    ret i64 %usz
}

define i32 @main(i32 %argc, i8** %argv) {
    %usz = call i64 @struct.type.size()
    %fmt = getelementptr [5 x i8], [5 x i8]* @.str, i64 0, i64 0
    call i32 (i8*, ...) @printf(i8* %fmt, i64 %usz)

    ret i32 0
}