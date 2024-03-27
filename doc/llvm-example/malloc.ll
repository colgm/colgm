
declare i8* @malloc(i64) nounwind
declare i32 @printf(i8*, ...) nounwind
declare void @free(i32*) nounwind

@.str = private unnamed_addr constant [8 x i8] c"%d %d\0a\0d\00"

define i32 @main(i32 %argc, i8** %argv) {
    ; malloc get size 4 for i32
    %p_i8 = call i8* @malloc(i64 4)
    %p_i32 = bitcast i8* %p_i8 to i32*

    ; alloca i32, automatically free before return
    %p_alloca_i32 = alloca i32

    ; store
    store i32 2147483647, i32* %p_alloca_i32
    store i32 1145141919, i32* %p_i32

    ; convert constant string to i8*
    %fmt = getelementptr [8 x i8], [8 x i8]* @.str, i64 0, i64 0
    %value_0 = load i32, i32* %p_alloca_i32
    %value_1 = load i32, i32* %p_i32
    call i32 (i8*, ...) @printf(i8* %fmt, i32 %value_0, i32 %value_1)
    
    ; free p_i32
    call void @free(i32* %p_i32)
    ret i32 0
}