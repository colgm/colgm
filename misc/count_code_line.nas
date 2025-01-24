use std.io;
use std.file;

var f = file.recursive_find_files_with_extension;

var count_lines = func(file_list) {
    var total = 0;
    foreach (var f; file_list) {
        var content = io.readfile(f);
        var lines = split("\n", content);
        total += size(lines);
        println(f, ": ", size(lines));
    }
    println("Total: ", total);
}

count_lines(f("bootstrap", "h", "cpp", "hpp"));
count_lines(f("src", "colgm"));