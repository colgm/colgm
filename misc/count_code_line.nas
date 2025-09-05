use std.io;
use std.file;
use std.padding;

var count_lines = func(dir_path, file_list) {
    println("<", dir_path, ">:");
    var total = 0;
    foreach (var f; file_list) {
        var content = io.readfile(f);
        var lines = split("\n", content);
        var line_count = size(lines);
        total += line_count;
        println("  ", padding.leftpad((line_count), 6), "  ", f);
    }
    println("   total  ", total);
}

var find_files = file.recursive_find_files_with_extension;
count_lines("bootstrap", find_files("bootstrap", "h", "cpp", "hpp"));
count_lines("src", find_files("src", "colgm"));
